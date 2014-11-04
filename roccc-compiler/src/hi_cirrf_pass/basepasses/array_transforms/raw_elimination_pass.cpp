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
#include "roccc_utils/annote_utils.h"
#include "roccc_utils/IR_utils.h"
#include "roccc_utils/type_utils.h"
#include "roccc_utils/symbol_utils.h"
#include "roccc_utils/list_utils.h"
#include "roccc_utils/warning_utils.h"
#include "raw_elimination_pass.h"

using namespace std;

// THIS PASS ONLY WORKS FOR THE INNERMOST LOOPS OR FOR NESTED LOOPS WITH NO
// INSTRUCTIONS IN THE OUTER LOOP BODY THAT ARE DATA DEPENDENT ON THE 
// INSTRUCTIONS OF THE INNERMOST LOOP BODY

// FOR THE PASS TO WORK ON NESTED LOOPS WHERE THERE IS DATA DEPENDENCIES BETWEEN 
// LOOP NESTS, ALIAS ANALYSIS NEEDS TO BE INCORPORATED BEFORE THE ARRAY ACCESSES 
// ARE REPLACED BY SCALAR TEMPORARIES.

/**************************** Declarations ************************************/

class are_c_for_statement_walker: public SelectiveWalker {
public:
  are_c_for_statement_walker(SuifEnv *the_env)
    : SelectiveWalker(the_env, CForStatement::get_class_name()) {}
  Walker::ApplyStatus operator () (SuifObject *x);
private:
  friend void process_loop_body();
};

class are_load_expression_walker: public SelectiveWalker {
public:
  are_load_expression_walker(SuifEnv *the_env, suif_map<Expression*, VariableSymbol*>* the_scalars)
    : SelectiveWalker(the_env, LoadExpression::get_class_name()), scalar_temporaries(the_scalars) {}

  Walker::ApplyStatus operator () (SuifObject *x);
private:
  suif_map<Expression*, VariableSymbol*>* scalar_temporaries;
};

class are_store_statement_walker: public SelectiveWalker {
public:
  are_store_statement_walker(SuifEnv *the_env, suif_map<Expression*, VariableSymbol*>* the_scalars,
					       searchable_list<Expression*>* raws,
					       searchable_list<Expression*>* multiples)
    : SelectiveWalker(the_env, StoreStatement::get_class_name()), scalar_temporaries(the_scalars),
								  read_after_writes(raws),
								  multiple_writes(multiples) {}

  Walker::ApplyStatus operator () (SuifObject *x);
private:
  suif_map<Expression*, VariableSymbol*>* scalar_temporaries;
  searchable_list<Expression*>* read_after_writes;
  searchable_list<Expression*>* multiple_writes;
};

ProcedureDefinition *current_proc_def = NULL;
list<Statement*>* re_stmts_to_be_added;

/**************************** Implementations ************************************/
RawEliminationPass::RawEliminationPass(SuifEnv *pEnv) : PipelinablePass(pEnv, "RawEliminationPass"){}

void RawEliminationPass::do_procedure_definition(ProcedureDefinition* proc_def){
  OutputInformation("RAW WAW elimination pass begins") ;
  if (proc_def)
  {
    current_proc_def = proc_def;
    are_c_for_statement_walker walker(get_suif_env());
    proc_def->walk(walker);
  }
  OutputInformation("RAW WAW elimination pass ends") ;
}

void process_loop_body(SuifEnv *the_env, Statement *body) {

    Iter<CForStatement> iter = object_iterator<CForStatement>(body); 
    if (!iter.is_valid()){

        suif_map<Expression*, VariableSymbol*>* store_scalar_temporaries = new suif_map<Expression*, VariableSymbol*>;
        suif_map<Expression*, VariableSymbol*>* load_scalar_temporaries = new suif_map<Expression*, VariableSymbol*>;

	searchable_list<Expression*>* read_after_writes = new searchable_list<Expression*>;
	searchable_list<Expression*>* multiple_writes = new searchable_list<Expression*>;

        for (Iter<Statement> iter = object_iterator<Statement>(body); 
             iter.is_valid(); iter.next()) {

	     Statement *current_stmt = &iter.current();

	     if(is_a<StatementList>(current_stmt))
	 	continue;

             for (Iter<LoadExpression> iter2 = object_iterator<LoadExpression>(current_stmt); 
                  iter2.is_valid(); iter2.next()) {
                  LoadExpression *ref_load = &iter2.current();
	          if(!is_a<ArrayReferenceExpression>(ref_load->get_source_address()))
	             continue; 
                  ArrayReferenceExpression *ref = to<ArrayReferenceExpression>(ref_load->get_source_address());
                  VariableSymbol *var = NULL;
                  Expression *write_ref = NULL;
                  suif_map<Expression*, VariableSymbol*>::iterator iter3 = store_scalar_temporaries->begin();
	          bool found = 0;
	          while(!found && (iter3 != store_scalar_temporaries->end())){
	 	      found = is_equal(ref, (*iter3).first);
	 	      write_ref = (*iter3).first;
	              var = (*iter3).second;
	              iter3++;
	          }
	          if(found){
                     load_scalar_temporaries->enter_value(ref, var);
		     read_after_writes->push_back(write_ref);
	          }
             }

             if(is_a<StoreStatement>(current_stmt)) {
                StoreStatement *ref_store = to<StoreStatement>(current_stmt);
	        if(!is_a<ArrayReferenceExpression>(ref_store->get_destination_address()))
	           continue; 
                ArrayReferenceExpression *ref = to<ArrayReferenceExpression>(ref_store->get_destination_address());
                VariableSymbol *var = NULL;
                Expression *write_ref = NULL;
                suif_map<Expression*, VariableSymbol*>::iterator iter2 = store_scalar_temporaries->begin();
	        bool found = 0;
	        while(!found && (iter2 != store_scalar_temporaries->end())){
	 	    found = is_equal(ref, (*iter2).first);
                    write_ref = (*iter2).first;
                    var = (*iter2).second;
	            iter2++;
	        }
	        if(!found){  	 
		   Type *t = get_base_type(ref->get_result_type());	
                   var = new_anonymous_variable(the_env, current_proc_def->get_symbol_table(), retrieve_qualified_type(to<DataType>(t)));
		   name_variable(var);
	        } else {
		   multiple_writes->push_back(write_ref);
		}
                store_scalar_temporaries->enter_value(ref, var);
	     }
        }
   
        are_load_expression_walker walker1(the_env, load_scalar_temporaries);
        body->walk(walker1);
        are_store_statement_walker walker2(the_env, store_scalar_temporaries, read_after_writes, multiple_writes);
        body->walk(walker2);

        suif_map<Expression*, VariableSymbol*>::iterator iter = store_scalar_temporaries->begin();
        while(iter != store_scalar_temporaries->end()){
	    Expression *store_ref = (*iter).first;
	    bool found = 0;
	    searchable_list<Expression*>::iterator iter1 = read_after_writes->begin();
	    while(!found && iter1 != read_after_writes->end()){
 	        found = is_equal(store_ref,(*iter1));
		iter1++;
	    }
	    if(!found){
	       SymbolTable *sym_tab = current_proc_def->get_symbol_table(); 
               suif_map<Expression*, VariableSymbol*>::iterator temp = iter;
	       temp--;
	       sym_tab->remove_symbol((*iter).second);
	       store_scalar_temporaries->erase(iter);
	       iter = temp;
            }iter++;
        }

	delete store_scalar_temporaries;
        delete load_scalar_temporaries;

        delete read_after_writes;
        delete multiple_writes;
    }
}

Walker::ApplyStatus are_c_for_statement_walker::operator () (SuifObject *x) {
    SuifEnv *env = get_env();
    CForStatement *the_c_for = to<CForStatement>(x);

    //    if(!is_stmt_within_begin_end_hw_marks(the_c_for))
    //    return Walker::Continue;

    Statement *body = the_c_for->get_body();

    if (body)
        process_loop_body(env, body);

    return Walker::Continue;
}

Walker::ApplyStatus are_load_expression_walker::operator () (SuifObject *x){

    SuifEnv *the_env = get_env();
    LoadExpression *the_load_expr = to<LoadExpression>(x);
    Expression *replacement = NULL;

    if (is_a<ArrayReferenceExpression>(the_load_expr->get_source_address())){
        Expression *ref = the_load_expr->get_source_address();
        VariableSymbol *var = NULL;

        suif_map<Expression*, VariableSymbol*>::iterator iter = scalar_temporaries->find(ref);
        if (iter == scalar_temporaries->end()) {          
            return Walker::Continue;
        }else var = (*iter).second;
        
        replacement = create_load_variable_expression(the_env, the_load_expr->get_result_type(), var);
        the_load_expr->get_parent()->replace(the_load_expr, replacement);
    }

    return Walker::Continue;
}

Walker::ApplyStatus are_store_statement_walker::operator() (SuifObject *x){
    SuifEnv *the_env = get_env();
    StoreStatement *store_stmt = to<StoreStatement>(x);
 
    if (is_a<ArrayReferenceExpression>(store_stmt->get_destination_address())){

        Expression *ref = store_stmt->get_destination_address();
        VariableSymbol *var = NULL;
        
        searchable_list<Expression*>::iterator iter = read_after_writes->find(ref);
        if (iter == read_after_writes->end())          
            return Walker::Continue;

        suif_map<Expression*, VariableSymbol*>::iterator iter1 = scalar_temporaries->find(ref);
        if (iter1 == scalar_temporaries->end()) 
            return Walker::Continue;
        else var = (*iter1).second;

        Expression *expr = store_stmt->get_value();
        store_stmt->set_value(0);
 
	StoreVariableStatement *store_var_stmt = create_store_variable_statement(the_env, var, expr);

        searchable_list<Expression*>::iterator iter2 = multiple_writes->find(ref);
        if (iter2 == multiple_writes->end()){ 
	    store_stmt->set_value(create_load_variable_expression(the_env, ref->get_result_type(), var));
	    StatementList *replacement = create_statement_list(the_env);
	    replacement->append_statement(store_var_stmt);
	    store_stmt->get_parent()->replace(store_stmt, replacement);
	    replacement->append_statement(store_stmt);
	}else{
	    replace_statement(store_stmt, store_var_stmt);
	}
    }

    return Walker::Continue;
}

