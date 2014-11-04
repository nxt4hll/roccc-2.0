// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

#include "common/system_specific.h"
#include <common/suif_copyright.h>

#include <iostream>
#include <iokernel/cast.h>
#include <iokernel/clone_stream.h>
#include <common/i_integer.h>
#include <basicnodes/basic_factory.h>
#include <suifnodes/suif.h>
#include <suifnodes/suif_factory.h>
#include <suifkernel/command_line_parsing.h>
#include <basicnodes/basic.h>
#include <basicnodes/basic_constants.h>
#include <suifkernel/suifkernel_messages.h>
#include <suifkernel/utilities.h> 
#include <suifkernel/group_walker.h> 
#include "transforms/procedure_walker_utilities.h"
#include <utils/expression_utils.h>
#include <utils/symbol_utils.h>
#include <utils/type_utils.h>
#include <utils/cloning_utils.h>
#include <cfenodes/cfe.h>
#include <cfenodes/cfe_factory.h>
#include <typebuilder/type_builder.h>
#include "roccc_utils/loop_utils.h"
#include "roccc_utils/list_utils.h"
#include "roccc_utils/type_utils.h"
#include "roccc_utils/symbol_utils.h"
#include "roccc_utils/IR_utils.h"
#include "roccc_utils/data_dependence_utils.h"
#include "roccc_extra_types/array_info.h"
#include "feedback_load_elimination_pass.h"
#include "roccc_utils/warning_utils.h"

using namespace std;

// THIS PASS ONLY WORKS FOR THE INNERMOST LOOPS AND ELIMINATES THE FEEDBACK 
// LOAD/STORE PAIRS WITHIN THE LOOP BODY

// THIS PASS SHOULD ONLY BE EXECUTED RIGHT AFTER THE SCALAR_REPLACEMENT_PASS 
// THE PREPROCESSING_PASS and THE FLATTEN_STATEMENT_LIST_PASS

/**************************** Declarations ************************************/

class fle_c_for_statement_walker: public SelectiveWalker {
public:
  fle_c_for_statement_walker(SuifEnv *the_env)
    : SelectiveWalker(the_env, CForStatement::get_class_name()) {}
  Walker::ApplyStatus operator () (SuifObject *x);
};

class fle_load_variable_expression_walker: public SelectiveWalker {
public:
  fle_load_variable_expression_walker(SuifEnv *the_env, VariableSymbol *iv, Expression *lbe)
    : SelectiveWalker(the_env, LoadVariableExpression::get_class_name()), parent_loop_index_var(iv), 
									  parent_loop_lower_bound_expr(lbe) {}
  Walker::ApplyStatus operator () (SuifObject *x);
private:
  VariableSymbol* parent_loop_index_var;
  Expression* parent_loop_lower_bound_expr;
};

/**************************** Implementations ************************************/
FeedbackLoadEliminationPass::FeedbackLoadEliminationPass(SuifEnv *pEnv) 
			: PipelinablePass(pEnv, "FeedbackLoadEliminationPass"){}

void FeedbackLoadEliminationPass::do_procedure_definition(ProcedureDefinition* proc_def)
{

  OutputInformation("feedback load elimination pass begins") ;
  if (proc_def)
  {
    fle_c_for_statement_walker walker(get_suif_env());
    proc_def->walk(walker);
  }
  OutputInformation("feedback load elimination pass ends") ;  
}

Walker::ApplyStatus fle_c_for_statement_walker::operator () (SuifObject *x) {
    SuifEnv *env = get_env();
    CForStatement *c_for_stmt = to<CForStatement>(x);

    //    if(!is_stmt_within_begin_end_hw_marks(c_for_stmt))
    //    return Walker::Continue;

    Statement *body = c_for_stmt->get_body();

    Iter<CForStatement> iter = object_iterator<CForStatement>(body); 
    if(iter.is_valid())
       return Walker::Continue;

    BrickAnnote* c_for_info = to<BrickAnnote>(c_for_stmt->lookup_annote_by_name("c_for_info"));
    String c_for_loop_counter_name = (to<StringBrick>(c_for_info->get_brick(1)))->get_value();
    int c_for_loop_step_size = (to<IntegerBrick>(c_for_info->get_brick(4)))->get_value().c_int();

    if(body){

       list<StoreStatement*>* stores_list = collect_objects<StoreStatement>(body);
           
       if(stores_list->size() <= 0){
          delete stores_list;
          return Walker::Continue;
       }

       ProcedureDefinition* proc_def = get_procedure_definition(c_for_stmt);

       VariableSymbol *c_for_stmt_index_var = get_c_for_basic_induction_variable(c_for_stmt);
       Expression *c_for_stmt_lower_bound_expr = get_c_for_lower_bound_expr(c_for_stmt);

       BrickAnnote *ba = to<BrickAnnote>(c_for_stmt->lookup_annote_by_name("end_of_mem_reads"));
       SuifObjectBrick *sob = to<SuifObjectBrick>(ba->get_brick(0));
       MarkStatement *end_of_mem_reads = to<MarkStatement>(sob->get_object());

       ba = to<BrickAnnote>(c_for_stmt->lookup_annote_by_name("beg_of_mem_writes"));
       sob = to<SuifObjectBrick>(ba->get_brick(0));
       MarkStatement *beg_of_mem_writes = to<MarkStatement>(sob->get_object());
        
       list<VariableSymbol*>* array_names_in_load_exprs = collect_array_name_symbols_used_in_loads(body);
       suif_map<LoadExpression*, ArrayInfo*>* load_expr_array_info_map =  new suif_map<LoadExpression*, ArrayInfo*>;

       list<LoadExpression*>* loads_list = collect_objects<LoadExpression>(body); 
       for(list<LoadExpression*>::iterator iter2 = loads_list->begin(); 
           iter2 != loads_list->end(); iter2++) {
	
           LoadExpression *load_expr = *iter2;
	   StoreVariableStatement *load_parent = to<StoreVariableStatement>(get_expression_owner(load_expr)); 
	   if(!is_a<ArrayReferenceExpression>(load_expr->get_source_address()))
	      continue; 
           ArrayReferenceExpression *source_address_expr = to<ArrayReferenceExpression>(load_expr->get_source_address());
	   
	   BrickAnnote *source_address_info_annote = to<BrickAnnote>(source_address_expr->lookup_annote_by_name("array_ref_info"));
	   sob = to<SuifObjectBrick>(source_address_info_annote->get_brick(0));
	   ArrayInfo *source_address_info = (ArrayInfo*)(sob->get_object());
	
	   load_expr_array_info_map->enter_value(load_expr, source_address_info);
       }
       delete loads_list;

       StatementList* before_beg_of_mem_writes = create_statement_list(env);
       StatementList* after_end_of_mem_reads = create_statement_list(env);
       StatementList* load_inits = create_statement_list(env);
       list<Statement*>* to_be_removed = new list<Statement*>;

       for(list<StoreStatement*>::iterator iter = stores_list->begin(); 
           iter != stores_list->end(); iter++) {

	   StoreStatement *store_stmt = *iter;
	   if(!is_a<ArrayReferenceExpression>(store_stmt->get_destination_address()))
	      continue; 
           ArrayReferenceExpression *destination_address_expr = to<ArrayReferenceExpression>(store_stmt->get_destination_address());

	   BrickAnnote *destination_address_info_annote = to<BrickAnnote>(destination_address_expr->lookup_annote_by_name("array_ref_info"));
  	   SuifObjectBrick *sob = to<SuifObjectBrick>(destination_address_info_annote->get_brick(0));
	   ArrayInfo *destination_address_info = (ArrayInfo*)(sob->get_object());

	   VariableSymbol *array_sym = get_array_name_symbol(destination_address_expr);
           Type *t = get_base_type(destination_address_expr->get_result_type());
           VariableSymbol *feedback_var = NULL;

           for(suif_map<LoadExpression*, ArrayInfo*>::iterator iter2 = load_expr_array_info_map->begin(); 
               iter2 != load_expr_array_info_map->end(); ) {

	       ArrayInfo *source_address_info = (*iter2).second;

      	       if(destination_address_info->get_array_symbol_name() != source_address_info->get_array_symbol_name()){
		  iter2++;
	          continue;
	       }
	
	       if(is_a_feedback_pair(destination_address_info, source_address_info, c_for_loop_counter_name, c_for_loop_step_size)){
  
		  if(!feedback_var){
		
                     feedback_var = new_anonymous_variable(env, find_scope(proc_def->get_body()), retrieve_qualified_type(to<DataType>(t)));
		     name_variable(feedback_var);
		     
                     StoreVariableStatement *feedback_var_set = create_store_variable_statement(env, feedback_var,
							to<LoadVariableExpression>(deep_suif_clone(store_stmt->get_value())));

		     before_beg_of_mem_writes->append_statement(feedback_var_set);
		  }

                  LoadExpression *load_expr = (*iter2).first;
	          StoreVariableStatement *load_parent = to<StoreVariableStatement>(get_expression_owner(load_expr)); 

                  StoreVariableStatement *feedback_var_get = create_store_variable_statement(env, load_parent->get_destination(), 
							create_load_variable_expression(env, to<DataType>(t), feedback_var));
		
		  after_end_of_mem_reads->append_statement(feedback_var_get);

		  suif_map<LoadExpression*, ArrayInfo*>::iterator iter_temp = iter2;
		  iter2++;
		  load_expr_array_info_map->erase(iter_temp);

		  to_be_removed->push_back(load_parent);
		  load_parent->set_parent(0);

		  load_parent->set_destination(feedback_var);
                  load_inits->append_statement(load_parent);

	       }else iter2++;
           }
       }

       int i = 0;
       StatementList* the_list = to<StatementList>(body);
       while(i < the_list->get_statement_count()){
	    if(is_in_list(the_list->get_statement(i), to_be_removed))
	       the_list->remove_statement(i);
	    else i++;	 
       }

       fle_load_variable_expression_walker walker(env, c_for_stmt_index_var, c_for_stmt_lower_bound_expr);
       load_inits->walk(walker);

       insert_statement_before(beg_of_mem_writes, before_beg_of_mem_writes);
       insert_statement_after(end_of_mem_reads, after_end_of_mem_reads);
       insert_statement_before(c_for_stmt, load_inits);

       delete stores_list;
       delete array_names_in_load_exprs;
       delete load_expr_array_info_map;
       delete to_be_removed;
    }

    return Walker::Continue;
}


Walker::ApplyStatus fle_load_variable_expression_walker::operator () (SuifObject *x) {
    LoadVariableExpression *load_var_expr = to<LoadVariableExpression>(x);

    if(load_var_expr->get_source() == parent_loop_index_var){
       load_var_expr->get_parent()->replace(load_var_expr, to<Expression>(deep_suif_clone(parent_loop_lower_bound_expr))); 
    }
    return Walker::Continue;
}

