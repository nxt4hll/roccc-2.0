// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*
  This code is mostly unchanged from ROCCC 1.0.
*/

#include "common/system_specific.h"
#include <common/suif_copyright.h>

#include <iostream>
#include <math.h>
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
#include <utils/expression_utils.h>
#include <utils/symbol_utils.h>
#include <utils/type_utils.h>
#include <utils/cloning_utils.h>
#include <cfenodes/cfe.h>
#include <cfenodes/cfe_factory.h>
#include "roccc_utils/loop_utils.h"
#include "roccc_utils/list_utils.h"
#include "roccc_utils/annote_utils.h"
#include "roccc_utils/symbol_utils.h"
#include "roccc_utils/IR_utils.h"
#include "roccc_utils/warning_utils.h"
#include "fuse_pass.h"

using namespace std;

// This pass does not assume that the loops prior to running fuse_pass are normalized.

/**************************** Declarations ************************************/

// These functions are used in this file only.  (I should probably put this
//  in the roccc_utils directory)
bool EquivalentBounds(CForStatement* x, CForStatement* y) ;
bool SameLowerBound(CForStatement* x, CForStatement* y) ;
bool SameUpperBound(CForStatement* x, CForStatement* y) ;

class fp_c_for_statement_walker: public SelectiveWalker {
public:
  fp_c_for_statement_walker(SuifEnv *the_env)
    : SelectiveWalker(the_env, CForStatement::get_class_name()) {}
  Walker::ApplyStatus operator () (SuifObject *x);
};

class fp_load_variable_expression_walker: public SelectiveWalker {
public:
  fp_load_variable_expression_walker(SuifEnv *the_env, list<VariableSymbol*>* ivo, list<VariableSymbol*>* ivn)
    : SelectiveWalker(the_env, LoadVariableExpression::get_class_name()), old_index_variables(ivo),
									  new_index_variables(ivn) {}
  Walker::ApplyStatus operator () (SuifObject *x);
private:
  list<VariableSymbol*>* old_index_variables;
  list<VariableSymbol*>* new_index_variables;
};

class fp_while_statement_walker: public SelectiveWalker {
public:
  fp_while_statement_walker(SuifEnv *the_env)
    : SelectiveWalker(the_env, WhileStatement::get_class_name()) {}
  Walker::ApplyStatus operator () (SuifObject *x);
};

bool change = 1;

/**************************** Implementations ************************************/
FusePass::FusePass(SuifEnv *pEnv) : PipelinablePass(pEnv, "FusePass") {}

void FusePass::do_procedure_definition(ProcedureDefinition* proc_def)
{
  OutputInformation("Fuse pass begins") ;
    if (proc_def){
        change = 1;
	while(change){
	   change = 0;
           fp_c_for_statement_walker walker(get_suif_env());
           proc_def->walk(walker);
        }
        change = 1;
	while(change){
	   change = 0;
           fp_while_statement_walker walker(get_suif_env());
           proc_def->walk(walker);
        }
    }
    OutputInformation("Fuse pass ends") ;
}
   
Walker::ApplyStatus fp_c_for_statement_walker::operator () (SuifObject *x) {
    SuifEnv *the_env = get_env();
    CForStatement *the_c_for = to<CForStatement>(x);  

    // Commented out on 4/6/09 - Jason
    //    if(!is_stmt_within_begin_end_hw_marks(the_c_for))
    //  return Walker::Continue;  

    Statement *body = the_c_for->get_body();

    if (body) {
   
      /*
	int the_c_for_iteration_count = get_c_for_iteration_count(the_c_for);
        if (is_error_code_set()){
            reset_error_code();
            return Walker::Continue;
        }
      */

	for (Iter<CForStatement> iter = collect_instance_objects<CForStatement>(the_c_for->get_parent());
	     iter.is_valid(); iter.next())
	{
	     CForStatement *current_c_for = &iter.current(); 

	     // Removed on Nov. 12, 2009 by Jason
	     //	     if(!is_stmt_within_begin_end_hw_marks(current_c_for))
	     //   continue;  

	     if (current_c_for != the_c_for){
		
	       /*
		 int current_c_for_iteration_count = get_c_for_iteration_count(the_c_for);
                 if (is_error_code_set()){
                     reset_error_code();
                     break;
                 }
	       */

		 if (EquivalentBounds(the_c_for, current_c_for))
		 {
		   fp_load_variable_expression_walker walker(the_env, get_c_for_induction_variables(current_c_for),
									get_c_for_induction_variables(the_c_for));
		      current_c_for->get_body()->walk(walker); 	
		      Statement *current_c_for_body = current_c_for->get_body();
		      current_c_for_body->set_parent(0);
		      insert_statement_after(body, current_c_for_body);
		      remove_statement(current_c_for);
		      change = 1;
		      return Walker::Stop;
		 }
	     }
	}
    }
    return Walker::Continue;
}

Walker::ApplyStatus fp_load_variable_expression_walker::operator () (SuifObject *x){

    SuifEnv *the_env = get_env();
    LoadVariableExpression *the_load_var_expr = to<LoadVariableExpression>(x);
    Expression *replacement = NULL;
  
    if (old_index_variables->front() == the_load_var_expr->get_source()) {  
           
        replacement = create_load_variable_expression(the_env, the_load_var_expr->get_result_type(), 
								new_index_variables->front());

        the_load_var_expr->get_parent()->replace(the_load_var_expr, replacement);
    }

    return Walker::Continue; 
}

bool is_fusable(WhileStatement *w_stmt1, WhileStatement *w_stmt2){
 
    // any scalar used in w_stmt1 should not be an element of (w_stmt2's out_stmts - w_stmt1's out stmts)  

    BrickAnnote *w_stmt1_out_stmt = to<BrickAnnote>(w_stmt1->lookup_annote_by_name("out_stmts")); 
    BrickAnnote *w_stmt2_out_stmt = to<BrickAnnote>(w_stmt2->lookup_annote_by_name("out_stmts")); 

    BrickAnnote *difference = subtract_annotes(w_stmt2_out_stmt, w_stmt1_out_stmt);

    list<VariableSymbol*>* w_stmt1_vars = collect_variable_symbols(w_stmt1);;

    for (list<VariableSymbol*>::iterator iter = w_stmt1_vars->begin();
	 iter != w_stmt1_vars->end(); iter++){
         VariableSymbol *var_symbol = *iter;

         for (Iter<SuifBrick*> iter = difference->get_brick_iterator();
     	      iter.is_valid(); iter.next()){
              SuifObjectBrick *sob = to<SuifObjectBrick>(iter.current());
	      if(is_a<StoreVariableStatement>(sob->get_object())){
	         StoreVariableStatement *store_var_stmt = to<StoreVariableStatement>(sob->get_object()); 
		 if(var_symbol == store_var_stmt->get_destination())
		    return 0;
	      }
        }
    }     	

    // any scalar used in w_stmt2 should not be written by w_stmt1  

    list<VariableSymbol*>* w_stmt2_vars = collect_variable_symbols(w_stmt2);

    list<StoreVariableStatement*>* store_var_stmts = collect_objects<StoreVariableStatement>(w_stmt1);
    for (list<StoreVariableStatement*>::iterator iter = store_var_stmts->begin();
	 iter != store_var_stmts->end(); iter++){
	 StoreVariableStatement *store_var_stmt = *iter;
         VariableSymbol *store_symbol = store_var_stmt->get_destination();
	 if(is_in_list(store_symbol, w_stmt2_vars))
	    return 0; 
    }    	

    return 1;
}

Walker::ApplyStatus fp_while_statement_walker::operator () (SuifObject *x) {
    SuifEnv *env = get_env();
    WhileStatement *while_stmt = to<WhileStatement>(x);

    for (Iter<WhileStatement> iter = collect_instance_objects<WhileStatement>(while_stmt->get_parent());
	 iter.is_valid(); iter.next()){

	 WhileStatement *fuse_candidate = &iter.current(); 

	 if(while_stmt == fuse_candidate)
	    continue;

	 if(is_at_the_same_nesting_level(while_stmt, fuse_candidate)  && 
	    is_textually_preceeding(while_stmt, fuse_candidate))
	    continue;

	 if(is_fusable(fuse_candidate, while_stmt)){
	
            Expression *while_condition_expr = while_stmt->get_condition();
            Statement *while_body_stmt = while_stmt->get_body();

	    Expression *fuse_candidate_condition_expr = fuse_candidate->get_condition();
	    Statement *fuse_candidate_body_stmt = fuse_candidate->get_body();

	    Expression *if_condition_expr1 = to<Expression>(deep_suif_clone(fuse_candidate_condition_expr));
	    fuse_candidate_body_stmt->set_parent(0);
	    IfStatement *if_stmt1 = create_if_statement(env, if_condition_expr1, fuse_candidate_body_stmt, NULL);

	    Expression *if_condition_expr2 = to<Expression>(deep_suif_clone(while_condition_expr));
	    while_body_stmt->set_parent(0);
	    IfStatement *if_stmt2 = create_if_statement(env, if_condition_expr2, while_body_stmt, NULL);

	    StatementList *new_body = create_statement_list(env); 
	    new_body->append_statement(if_stmt1);
	    new_body->append_statement(if_stmt2);
	    
	    Expression *new_condition = create_binary_expression(env, while_condition_expr->get_result_type(), 
						String("logical_or"), 
						to<Expression>(deep_suif_clone(fuse_candidate_condition_expr)),
						to<Expression>(deep_suif_clone(while_condition_expr))); 
	    WhileStatement *replacement = create_while_statement(env, new_condition, new_body);
	    
 	    insert_statement_after(while_stmt, replacement);
	    remove_statement(while_stmt);
	    remove_statement(fuse_candidate);
	    
	    change = 1;
	    return Walker::Stop;
	 }
    }

    return Walker::Continue;
}

bool EquivalentBounds(CForStatement* x, CForStatement* y)
{
  return SameLowerBound(x, y) && SameUpperBound(x, y) ;
}

bool SameLowerBound(CForStatement* x, CForStatement* y)
{
  Statement* xBefore = x->get_before() ;
  Statement* yBefore = y->get_before() ;
  assert(xBefore != NULL) ;
  assert(yBefore != NULL) ;

  StoreVariableStatement* xStore = 
    dynamic_cast<StoreVariableStatement*>(xBefore) ;
  StoreVariableStatement* yStore =
    dynamic_cast<StoreVariableStatement*>(yBefore) ;
  assert(xStore != NULL) ;
  assert(yStore != NULL) ;

  Expression* xExp = xStore->get_value() ;
  Expression* yExp = yStore->get_value() ;
  assert(xExp != NULL) ;
  assert(yExp != NULL) ;

  IntConstant* xLowerBound = dynamic_cast<IntConstant*>(xExp) ;
  IntConstant* yLowerBound = dynamic_cast<IntConstant*>(yExp) ;
  assert(xLowerBound != NULL) ;
  assert(yLowerBound != NULL) ;

  return xLowerBound->get_value().c_int() == yLowerBound->get_value().c_int() ;
}

bool SameUpperBound(CForStatement* x, CForStatement* y)
{
  Expression* xTest = x->get_test() ;
  Expression* yTest = y->get_test() ;
  assert(xTest != NULL) ;
  assert(yTest != NULL) ;

  BinaryExpression* xBinExp = dynamic_cast<BinaryExpression*>(xTest) ;
  BinaryExpression* yBinExp = dynamic_cast<BinaryExpression*>(yTest) ;
  assert(xBinExp != NULL) ;
  assert(yBinExp != NULL) ;
  
  Expression* xUpper = xBinExp->get_source2() ;
  Expression* yUpper = yBinExp->get_source2() ;
  assert(xUpper != NULL) ;
  assert(yUpper != NULL) ;

  IntConstant* xUpperConstant = dynamic_cast<IntConstant*>(xUpper) ;
  IntConstant* yUpperConstant = dynamic_cast<IntConstant*>(yUpper) ;
  LoadVariableExpression* xUpperLoad = 
    dynamic_cast<LoadVariableExpression*>(xUpper) ;
  LoadVariableExpression* yUpperLoad = 
    dynamic_cast<LoadVariableExpression*>(yUpper) ;
  
  if (xUpperConstant != NULL && yUpperConstant != NULL)
  {
    return xUpperConstant->get_value().c_int() ==
           yUpperConstant->get_value().c_int() ;
  }
  
  if (xUpperLoad != NULL && yUpperLoad != NULL)
  {
    return xUpperLoad->get_source() == yUpperLoad->get_source() ;
  }
  return false ;
}
