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
#include "roccc_utils/loop_utils.h"
#include "roccc_utils/IR_utils.h"
#include "roccc_utils/print_utils.h"
#include "roccc_utils/control_flow_utils.h"
#include "roccc_utils/bit_vector_data_flow_utils.h"
#include "roccc_utils/warning_utils.h"
#include "unrchable_code_elimination_pass.h"

using namespace std;

/**************************** Declarations ************************************/

class ucep_c_for_statement_walker: public SelectiveWalker {
public:
  ucep_c_for_statement_walker(SuifEnv *env)
    :SelectiveWalker(env, CForStatement::get_class_name()) {}
  Walker::ApplyStatus operator () (SuifObject *x); 
};

class ucep_while_statement_walker: public SelectiveWalker {
public:
  ucep_while_statement_walker(SuifEnv *env)
    :SelectiveWalker(env, WhileStatement::get_class_name()) {}
  Walker::ApplyStatus operator () (SuifObject *x); 
};

class ucep_if_statement_walker: public SelectiveWalker {
public:
  ucep_if_statement_walker(SuifEnv *env)
    :SelectiveWalker(env, IfStatement::get_class_name()) {}
  Walker::ApplyStatus operator () (SuifObject *x); 
};

class ucep_call_statement_walker: public SelectiveWalker {
public:
  ucep_call_statement_walker(SuifEnv *env)
    :SelectiveWalker(env, CallStatement::get_class_name()) {}
  Walker::ApplyStatus operator () (SuifObject *x);
};

class ucep_statement_list_walker: public SelectiveWalker {
public:
  ucep_statement_list_walker(SuifEnv *env)
    :SelectiveWalker(env, StatementList::get_class_name()) {}
  Walker::ApplyStatus operator () (SuifObject *x); 
};

/**************************** Implementations ************************************/
UnreachableCodeEliminationPass::UnreachableCodeEliminationPass(SuifEnv *pEnv) : PipelinablePass(pEnv, "UnreachableCodeEliminationPass") {}

void UnreachableCodeEliminationPass::do_procedure_definition(ProcedureDefinition* proc_def){
  OutputInformation("Unreachable code elimination begins") ;
  if (proc_def)
  {
    ucep_statement_list_walker walker4(get_suif_env());
    proc_def->walk(walker4);
    
    ucep_c_for_statement_walker walker1(get_suif_env());
    proc_def->walk(walker1);
    ucep_while_statement_walker walker2(get_suif_env());
    proc_def->walk(walker2);
    ucep_if_statement_walker walker3(get_suif_env());
    proc_def->walk(walker3);
    ucep_call_statement_walker walker5(get_suif_env());
    proc_def->walk(walker5);
  }
  OutputInformation("Unreachable code elimination ends") ;
}

Walker::ApplyStatus ucep_c_for_statement_walker::operator () (SuifObject *x) {
	SuifEnv *env = get_env();
	CForStatement *c_for_stmt = to<CForStatement>(x);

	int iteration_count = get_c_for_iteration_count(c_for_stmt);

	if(is_error_code_set()) 
	   return Walker::Continue;
	
	if(iteration_count == 0){

           Statement *replacement = c_for_stmt->get_before();
	   c_for_stmt->set_before(0);

	   c_for_stmt->get_parent()->replace(c_for_stmt, replacement);

	   return Walker::Truncate;

	}else if(iteration_count == 1){

	   Statement *c_for_before = c_for_stmt->get_before();
	   c_for_stmt->set_before(0);

	   Statement *c_for_body = c_for_stmt->get_body();
	   c_for_stmt->set_body(0);

	   Statement *c_for_step = c_for_stmt->get_step();
	   c_for_stmt->set_step(0);

           StatementList *replacement = create_statement_list(env);

	   replacement->append_statement(c_for_before);
	   replacement->append_statement(c_for_body);
	   replacement->append_statement(c_for_step);

	   c_for_stmt->get_parent()->replace(c_for_stmt, replacement); 

	   set_address(replacement);
	   return Walker::Replaced;
	}

    	return Walker::Continue;
}

Walker::ApplyStatus ucep_while_statement_walker::operator () (SuifObject *x) {
	SuifEnv *env = get_env();
	WhileStatement *while_stmt = to<WhileStatement>(x);

	Expression *condition = while_stmt->get_condition();

        if(is_a<IntConstant>(condition)){
           IInteger condition_value = (to<IntConstant>(condition))->get_value();
           bool condition_true = (bool)(condition_value.c_int());
           if(!condition_true){

              Statement *replacement = create_statement_list(env);

	      while_stmt->get_parent()->replace(while_stmt, replacement); 

	      set_address(replacement);
	      return Walker::Replaced;
	   } 
	}

    	return Walker::Continue;
}

Walker::ApplyStatus ucep_if_statement_walker::operator () (SuifObject *x) {
	SuifEnv *env = get_env();
	IfStatement *if_stmt = to<IfStatement>(x);

	Expression *condition = if_stmt->get_condition();
	Statement *replacement = NULL;

        if(is_a<IntConstant>(condition)){
           IInteger condition_value = (to<IntConstant>(condition))->get_value();
           bool condition_true = (bool)(condition_value.c_int());
           if(condition_true){
	      replacement = if_stmt->get_then_part();
	      if_stmt->set_then_part(0);
	   }else if(if_stmt->get_else_part()){
	      replacement = if_stmt->get_else_part();
	      if_stmt->set_else_part(0);
	   }else replacement = create_statement_list(env);
	}

	if(replacement){

	   if_stmt->get_parent()->replace(if_stmt, replacement); 
	   
	   set_address(replacement);
	   return Walker::Replaced;
	}

    	return Walker::Continue;
}

Walker::ApplyStatus ucep_call_statement_walker::operator () (SuifObject *x) {
        SuifEnv *env = get_env();
        CallStatement *call_stmt = to<CallStatement>(x);

        SymbolAddressExpression *callee_address = to<SymbolAddressExpression>(call_stmt->get_callee_address());
        ProcedureSymbol *macro_sym = to<ProcedureSymbol>(callee_address->get_addressed_symbol());

	Statement *replacement = NULL;

        if(String(macro_sym->get_name()).starts_with("ROCCC_mux")){

           Expression *index_expr = call_stmt->get_argument(call_stmt->get_argument_count()-1);

           if(is_a<IntConstant>(index_expr)){
              int index = (to<IntConstant>(index_expr))->get_value().c_int();
              Expression *op_at_index = call_stmt->get_argument(index);
	      op_at_index->set_parent(0);
	      replacement = create_store_variable_statement(env, call_stmt->get_destination(), op_at_index);
	   }

        }

        if(String(macro_sym->get_name()) == String("ROCCC_boolsel")){

           Expression *op1 = call_stmt->get_argument(0);
           Expression *op2 = call_stmt->get_argument(1);

  	   Expression *condition = call_stmt->get_argument(2);

           if(is_a<IntConstant>(condition)){
              IInteger condition_value = (to<IntConstant>(condition))->get_value();
              bool condition_true = (bool)(condition_value.c_int());
              if(condition_true){
	         op1->set_parent(0);
	         replacement = create_store_variable_statement(env, call_stmt->get_destination(), op1);
	      }else{
	         op2->set_parent(0);
	         replacement = create_store_variable_statement(env, call_stmt->get_destination(), op2);
	      }
	   }
	}

	if(replacement){

	   call_stmt->get_parent()->replace(call_stmt, replacement); 
	   
	   set_address(replacement);
	   return Walker::Replaced;
	}

        return Walker::Continue;

}

Walker::ApplyStatus ucep_statement_list_walker::operator () (SuifObject *x) {
	StatementList *stmt_list = to<StatementList>(x);

        bool is_removable_stmt = 0;

        list<Statement*> *removable_stmts = new list<Statement*>;

        for (Iter<Statement*> iter = stmt_list->get_statement_iterator();
             iter.is_valid(); iter.next()){

             if(is_removable_stmt)
                removable_stmts->push_back(iter.current());
             else if(is_a<ReturnStatement>(iter.current()))
                is_removable_stmt = 1;
        }

        if(removable_stmts->size() > 0){

           remove_statements(removable_stmts);

           delete removable_stmts;
           return Walker::Truncate;
        }

        delete removable_stmts;

    	return Walker::Continue;
}
