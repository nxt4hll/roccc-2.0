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
#include "roccc_utils/IR_utils.h"
#include "roccc_utils/annote_utils.h"
#include "roccc_utils/list_utils.h"
#include "roccc_utils/loop_utils.h"
#include "roccc_utils/print_utils.h"
#include "roccc_utils/control_flow_utils.h"
#include "roccc_utils/bit_vector_data_flow_utils.h"
#include "roccc_utils/warning_utils.h"
#include <cfenodes/cfe.h>
#include "dead_code_elimination_pass.h"

using namespace std;

/**************************** Declarations ************************************/

class dcep_store_variable_statement_walker: public SelectiveWalker {
public:
  dcep_store_variable_statement_walker(SuifEnv *env)
    :SelectiveWalker(env, StoreVariableStatement::get_class_name()) {}
  Walker::ApplyStatus operator () (SuifObject *x); 
};

class dcep_call_statement_walker: public SelectiveWalker {
public:
  dcep_call_statement_walker(SuifEnv *env)
    :SelectiveWalker(env, CallStatement::get_class_name()) {}
  Walker::ApplyStatus operator () (SuifObject *x); 
};

class dcep_if_statement_walker: public SelectiveWalker {
public:
  dcep_if_statement_walker(SuifEnv *env)
    :SelectiveWalker(env, IfStatement::get_class_name()) {}
  Walker::ApplyStatus operator () (SuifObject *x); 
};

class dcep_c_for_statement_walker: public SelectiveWalker {
public:
  dcep_c_for_statement_walker(SuifEnv *env)
    :SelectiveWalker(env, CForStatement::get_class_name()) {}
  Walker::ApplyStatus operator () (SuifObject *x); 
};

class dcep_while_statement_walker: public SelectiveWalker {
public:
  dcep_while_statement_walker(SuifEnv *env)
    :SelectiveWalker(env, WhileStatement::get_class_name()) {}
  Walker::ApplyStatus operator () (SuifObject *x); 
};

list<Statement*>* to_be_removed;

/**************************** Implementations ************************************/
DeadCodeEliminationPass::DeadCodeEliminationPass(SuifEnv *pEnv) : PipelinablePass(pEnv, "DeadCodeEliminationPass") {}

void DeadCodeEliminationPass::do_procedure_definition(ProcedureDefinition* proc_def){

  OutputInformation("Dead code elimination pass begins") ;
    if (proc_def){

	to_be_removed = new list<Statement*>;

	dcep_store_variable_statement_walker walker(get_suif_env());
	proc_def->walk(walker);

	dcep_call_statement_walker walker2(get_suif_env());
	proc_def->walk(walker2);

	dcep_if_statement_walker walker3(get_suif_env());
	proc_def->walk(walker3);

	dcep_c_for_statement_walker walker4(get_suif_env());
	proc_def->walk(walker4);

	dcep_while_statement_walker walker5(get_suif_env());
	proc_def->walk(walker5);

	remove_statements(to_be_removed);

	delete to_be_removed;
    }
    OutputInformation("Dead code elimination pass ends") ;
}

Walker::ApplyStatus dcep_store_variable_statement_walker::operator () (SuifObject *x) {
  //	SuifEnv *env = get_env();
	StoreVariableStatement *store_var_stmt = to<StoreVariableStatement>(x);

	VariableSymbol *destination_var = store_var_stmt->get_destination();
	Expression *store_value = store_var_stmt->get_value();
	
	if(is_a<LoadVariableExpression>(store_value))
	   if((to<LoadVariableExpression>(store_value))->get_source() == destination_var){
 	      to_be_removed->push_back(store_var_stmt);
  	      return Walker::Continue;
	   }

	BrickAnnote *reached_uses = to<BrickAnnote>(store_var_stmt->lookup_annote_by_name("reached_uses")); 
	if(reached_uses->get_brick_count() == 0){
	   to_be_removed->push_back(store_var_stmt);
    	   return Walker::Continue;
	}
	
	return Walker::Continue;
}

Walker::ApplyStatus dcep_call_statement_walker::operator () (SuifObject *x) {
	SuifEnv *env = get_env();
	CallStatement *call_stmt = to<CallStatement>(x);

        SymbolAddressExpression *callee_address = to<SymbolAddressExpression>(call_stmt->get_callee_address());
        ProcedureSymbol *macro_sym = to<ProcedureSymbol>(callee_address->get_addressed_symbol());

	if(String(macro_sym->get_name()) == String("ROCCC_lut_write"))
	   return Walker::Continue;

	BrickAnnote *reached_uses = to<BrickAnnote>(call_stmt->lookup_annote_by_name("reached_uses")); 
	if(reached_uses->get_brick_count() == 0){
	   to_be_removed->push_back(call_stmt);
    	   return Walker::Continue;
	}

	if(String(macro_sym->get_name()) == String("ROCCC_boolsel")){

           Expression *op1 = call_stmt->get_argument(0);
           Expression *op2 = call_stmt->get_argument(1);

	   if(is_equal(op1, op2)){

	      op1->set_parent(0);
	      StoreVariableStatement *replacement = create_store_variable_statement(env, call_stmt->get_destination(), op1);
	      call_stmt->get_parent()->replace(call_stmt, replacement);

	      return Walker::Continue;
	   }
	}

	return Walker::Continue;
	
}

Walker::ApplyStatus dcep_if_statement_walker::operator () (SuifObject *x) {
	SuifEnv *env = get_env();
	IfStatement *if_stmt = to<IfStatement>(x);

	Statement *then_part = if_stmt->get_then_part();
	Statement *else_part = if_stmt->get_else_part();

	if(is_a<StatementList>(then_part)){
	   if((to<StatementList>(then_part))->get_statement_count() != 0)
	      return Walker::Continue;
	}else if(then_part)
	   return Walker::Continue;

	if(is_a<StatementList>(else_part)){
	   if((to<StatementList>(else_part))->get_statement_count() != 0)
	      return Walker::Continue;
	}else if(else_part)
	   return Walker::Continue;

	Statement *replacement = create_statement_list(env);

	if_stmt->get_parent()->replace(if_stmt, replacement);

	set_address(replacement);
    	return Walker::Replaced;
}

Walker::ApplyStatus dcep_c_for_statement_walker::operator () (SuifObject *x) {
  //	SuifEnv *env = get_env();
	CForStatement *c_for_stmt = to<CForStatement>(x);

	Statement *body = c_for_stmt->get_body();

	if(is_a<StatementList>(body)){
	   if((to<StatementList>(body))->get_statement_count() != 0)
	      return Walker::Continue;
	}else if(body)
	   return Walker::Continue;

	int iteration_count = get_c_for_iteration_count(c_for_stmt);
	if(is_error_code_set())
	   return Walker::Continue; 

	int final_iteration_count = get_c_for_lower_bound(c_for_stmt) + iteration_count * get_c_for_step(c_for_stmt);
	set_c_for_lower_bound(c_for_stmt, final_iteration_count);

	Statement *replacement = c_for_stmt->get_before();
	c_for_stmt->set_before(0);

	c_for_stmt->get_parent()->replace(c_for_stmt, replacement);

	set_address(replacement);
    	return Walker::Replaced;
}

Walker::ApplyStatus dcep_while_statement_walker::operator () (SuifObject *x) {
	SuifEnv *env = get_env();
	WhileStatement *while_stmt = to<WhileStatement>(x);

	Statement *body = while_stmt->get_body();

	if(is_a<StatementList>(body)){
	   if((to<StatementList>(body))->get_statement_count() != 0)
	      return Walker::Continue;
	}else if(body)
	   return Walker::Continue;

	Statement *replacement = create_statement_list(env);

	while_stmt->get_parent()->replace(while_stmt, replacement);

	set_address(replacement);
    	return Walker::Replaced;
}
