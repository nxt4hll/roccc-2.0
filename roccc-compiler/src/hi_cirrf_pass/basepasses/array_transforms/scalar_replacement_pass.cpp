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
#include "roccc_utils/symbol_utils.h"
#include "roccc_utils/type_utils.h"
#include "roccc_utils/warning_utils.h"
#include "scalar_replacement_pass.h"

using namespace std;

// THIS PASS ONLY WORKS FOR THE INNERMOST LOOPS OR FOR NESTED LOOPS WITH NO
// INSTRUCTIONS IN THE OUTER LOOP BODY THAT ARE DATA DEPENDENT ON THE 
// INSTRUCTIONS OF THE INNERMOST LOOP BODY

// FOR THE PASS TO WORK ON NESTED LOOPS WHERE THERE IS DATA DEPENDENCIES BETWEEN 
// LOOP NESTS, ALIAS ANALYSIS NEEDS TO BE INCORPORATED BEFORE THE ARRAY ACCESSES 
// ARE REPLACED BY SCALAR TEMPORARIES.

/**************************** Declarations ************************************/

class c_for_statement_walker: public SelectiveWalker {
public:
  c_for_statement_walker(SuifEnv *the_env, suif_map<Expression*, VariableSymbol*>* load_scalars,
  					   suif_map<Expression*, VariableSymbol*>* store_scalars)
    : SelectiveWalker(the_env, CForStatement::get_class_name()), load_scalar_temporaries(load_scalars),
    								 store_scalar_temporaries(store_scalars) {}
  Walker::ApplyStatus operator () (SuifObject *x);
private:
  suif_map<Expression*, VariableSymbol*>* load_scalar_temporaries;
  suif_map<Expression*, VariableSymbol*>* store_scalar_temporaries;
};

class load_expression_walker: public SelectiveWalker {
public:
  load_expression_walker(SuifEnv *the_env, StatementList *loop_body, suif_map<Expression*, VariableSymbol*>* the_scalars)
    : SelectiveWalker(the_env, LoadExpression::get_class_name()), parent(loop_body), scalar_temporaries(the_scalars) {}

  Walker::ApplyStatus operator () (SuifObject *x);
private:
  StatementList *parent;
  suif_map<Expression*, VariableSymbol*>* scalar_temporaries;
};

class store_statement_walker: public SelectiveWalker {
public:
  store_statement_walker(SuifEnv *the_env, StatementList *loop_body, suif_map<Expression*,
VariableSymbol*>* the_scalars)
    : SelectiveWalker(the_env, StoreStatement::get_class_name()), parent(loop_body), scalar_temporaries(the_scalars) {}

  Walker::ApplyStatus operator () (SuifObject *x);
private:
  StatementList *parent;
  suif_map<Expression*, VariableSymbol*>* scalar_temporaries;
};

list<Statement*>* srp_stmts_to_be_added; 

/**************************** Implementations ************************************/
ScalarReplacementPass::ScalarReplacementPass(SuifEnv *pEnv) : PipelinablePass(pEnv, "ScalarReplacementPass"){}

void ScalarReplacementPass::do_procedure_definition(ProcedureDefinition* proc_def){
  OutputInformation("Scalar replacement pass begins") ;
  if (proc_def)
  {
    SuifEnv *the_env = proc_def->get_suif_env();

    load_scalar_temporaries = new suif_map<Expression*, VariableSymbol*>;
    store_scalar_temporaries = new suif_map<Expression*, VariableSymbol*>;

    for (Iter<LoadExpression> iter = object_iterator<LoadExpression>(proc_def);
	 iter.is_valid(); iter.next()) 
    {
      LoadExpression *ref_load = &iter.current();
      if(!is_a<ArrayReferenceExpression>(ref_load->get_source_address()))
	continue; 
      ArrayReferenceExpression *ref = to<ArrayReferenceExpression>(ref_load->get_source_address());
      VariableSymbol *var = NULL;
      suif_map<Expression*, VariableSymbol*>::iterator iter2 = load_scalar_temporaries->begin();
      bool found = 0;
      while(!found && (iter2 != load_scalar_temporaries->end())){
	found = is_equal(ref, (*iter2).first);
	var = (*iter2).second;
	iter2++;
      }
      if(!found) { 
	Type *t = get_base_type(ref->get_result_type());
	var = new_anonymous_variable(the_env, find_scope(proc_def->get_body()), retrieve_qualified_type(to<DataType>(t)));
	name_variable(var);
      }
      load_scalar_temporaries->enter_value(ref, var);
    }
    
    for (Iter<StoreStatement> iter = object_iterator<StoreStatement>(proc_def); 
	 iter.is_valid(); iter.next()) {
      StoreStatement *ref_store = &iter.current();
      if(!is_a<ArrayReferenceExpression>(ref_store->get_destination_address()))
	continue; 
      ArrayReferenceExpression *ref = to<ArrayReferenceExpression>(ref_store->get_destination_address());
      VariableSymbol *var = NULL;
      suif_map<Expression*, VariableSymbol*>::iterator iter2 = store_scalar_temporaries->begin();
      bool found = 0;
      while(!found && (iter2 != store_scalar_temporaries->end())){
	found = is_equal(ref, (*iter2).first);
	var = (*iter2).second;
	iter2++;
      }
      if(!found){ 	 
	Type *t = get_base_type(ref->get_result_type());
	var = new_anonymous_variable(the_env, find_scope(proc_def->get_body()), retrieve_qualified_type(to<DataType>(t)));
	name_variable(var);
	var->append_annote(create_brick_annote(proc_def->get_suif_env(), 
					      "ScalarReplacedVariable")) ;
      }
      store_scalar_temporaries->enter_value(ref, var);
    }
    
    c_for_statement_walker walker(get_suif_env(), load_scalar_temporaries, store_scalar_temporaries);
    proc_def->walk(walker);
    
    delete load_scalar_temporaries;
    delete store_scalar_temporaries;
  }
  OutputInformation("Scalar replacement pass ends") ;
}

Walker::ApplyStatus c_for_statement_walker::operator () (SuifObject *x) {
    SuifEnv *env = get_env();
    CForStatement *the_c_for = to<CForStatement>(x);

    //    if(!is_stmt_within_begin_end_hw_marks(the_c_for))
    //       return Walker::Continue;

    Statement *body = the_c_for->get_body();

    if (body){

        Iter<CForStatement> iter = object_iterator<CForStatement>(body); 
        if(iter.is_valid())
	   return Walker::Continue;

        BrickAnnote *dp_input_vars = create_brick_annote(env, "dp_input_vars");
        the_c_for->append_annote(dp_input_vars);
        BrickAnnote *dp_output_vars = create_brick_annote(env, "dp_output_vars");
        the_c_for->append_annote(dp_output_vars);
        BrickAnnote *stores = create_brick_annote(env, "stores");
        the_c_for->append_annote(stores);
              
	StatementList *stmt_list_body = NULL;

	if(!is_a<StatementList>(body)){
           the_c_for->set_body(0);

	   stmt_list_body = create_statement_list(env);
	   stmt_list_body->append_statement(body);
	   the_c_for->set_body(stmt_list_body);

	}else stmt_list_body = to<StatementList>(body);

        load_expression_walker walker1(env, stmt_list_body, load_scalar_temporaries);
        stmt_list_body->walk(walker1);
        store_statement_walker walker2(env, stmt_list_body, store_scalar_temporaries);
        stmt_list_body->walk(walker2);

	for(Iter<SuifBrick*> iter = stores->get_brick_iterator();
	    iter.is_valid(); iter.next()){
	   
	    SuifObjectBrick *sob = to<SuifObjectBrick>(iter.current());
	    StoreStatement *store_stmt = to<StoreStatement>(sob->get_object());

            stmt_list_body->append_statement(store_stmt);
	}

	delete (the_c_for->remove_annote_by_name("dp_input_vars"));
	delete (the_c_for->remove_annote_by_name("dp_output_vars"));
	delete (the_c_for->remove_annote_by_name("stores"));
    }

    return Walker::Continue;
}

Walker::ApplyStatus load_expression_walker::operator () (SuifObject *x){
    SuifEnv *the_env = get_env();
    LoadExpression *the_load_expr = to<LoadExpression>(x);
    Expression *replacement = NULL;

    if (is_a<ArrayReferenceExpression>(the_load_expr->get_source_address())){
        Expression *ref = the_load_expr->get_source_address();
        VariableSymbol *var = NULL;

        suif_map<Expression*, VariableSymbol*>::iterator iter = scalar_temporaries->find(ref);
        if (iter == scalar_temporaries->end()) {          
          cout << "load ERROR" << endl;
        }else var = (*iter).second;
        
        replacement = create_load_variable_expression(the_env, the_load_expr->get_result_type(), var);
        the_load_expr->get_parent()->replace(the_load_expr, replacement);

	CForStatement *enclosing_loop = get_enclosing_c_for_stmt(parent);
        BrickAnnote *dp_in = to<BrickAnnote>(enclosing_loop->lookup_annote_by_name("dp_input_vars"));
        bool found = search(dp_in, var);
	if(!found){
           Statement* store_var_stmt = create_store_variable_statement(the_env, var, the_load_expr);
	   parent->insert_statement(0, store_var_stmt);  
           dp_in->append_brick(create_suif_object_brick(the_env, var));
	}
    }

    return Walker::Continue;
}

Walker::ApplyStatus store_statement_walker::operator() (SuifObject *x){
    SuifEnv *the_env = get_env();
    StoreStatement *store_stmt = to<StoreStatement>(x);
 
    if (is_a<ArrayReferenceExpression>(store_stmt->get_destination_address())){

        Expression *ref = store_stmt->get_destination_address();
        VariableSymbol *var = NULL;
        
        suif_map<Expression*, VariableSymbol*>::iterator iter = scalar_temporaries->find(ref);
        if (iter == scalar_temporaries->end()){          
            cout << "store ERROR" << endl;
        }else var = (*iter).second;

        Expression *expr = store_stmt->get_value();
        store_stmt->set_value(0);
 
        LoadVariableExpression *the_symbol_expr = 
				create_load_variable_expression(the_env, ref->get_result_type(), var);
        store_stmt->set_value(the_symbol_expr);

	StoreVariableStatement *store_var_stmt = create_store_variable_statement(the_env, var, expr);

	replace_statement(store_stmt, store_var_stmt);

	CForStatement *enclosing_loop = get_enclosing_c_for_stmt(parent);
        BrickAnnote *dp_out = to<BrickAnnote>(enclosing_loop->lookup_annote_by_name("dp_output_vars"));
        BrickAnnote *stores = to<BrickAnnote>(enclosing_loop->lookup_annote_by_name("stores"));
        bool found = search(dp_out, var);
	if(!found){
           stores->append_brick(create_suif_object_brick(the_env, store_stmt));
           dp_out->append_brick(create_suif_object_brick(the_env, var));
        }
    }

    return Walker::Continue;
}

