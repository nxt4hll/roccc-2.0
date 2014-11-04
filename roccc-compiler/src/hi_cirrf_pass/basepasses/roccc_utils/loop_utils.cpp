// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include "common/system_specific.h"
#include <common/suif_copyright.h>
#include <common/suif_list.h>

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
#include "transforms/procedure_walker_utilities.h"
#include <utils/expression_utils.h>
#include <utils/symbol_utils.h>
#include <utils/type_utils.h>
#include <utils/cloning_utils.h>
#include <cfenodes/cfe.h>
#include "list_utils.h"
#include "print_utils.h"
#include "symbol_utils.h"
#include "loop_utils.h"

using namespace std;

/**************************** Declerations ************************************/

static int error_code = 0;

/************************** Implementations ***********************************/

VariableSymbol* get_c_for_basic_induction_variable(CForStatement *the_c_for){
 
// returns the basic induction variable inside the step statement which is in the following form: i=i +/- some_constant;

   Statement *the_step = the_c_for->get_step();

   list<StoreVariableStatement*>* step_stores = collect_objects<StoreVariableStatement>(the_step);
   list<StoreVariableStatement*>::iterator iter = step_stores->begin();
   if(iter != step_stores->end()){
      VariableSymbol *basic_induction_var = (*iter)->get_destination();
      delete step_stores;
      return basic_induction_var;
   }

   delete step_stores; 

   error_code = 1;
   return 0; 
}

list<VariableSymbol*>* get_c_for_induction_variables(CForStatement *the_c_for){
 
// currently only returns the basic induction variables inside the step statement 

   list<VariableSymbol*>* induction_variables = new list<VariableSymbol*>;

   Statement *the_step = the_c_for->get_step();

   list<StoreVariableStatement*>* step_stores = collect_objects<StoreVariableStatement>(the_step);
   for (list<StoreVariableStatement*>::iterator iter = step_stores->begin();
	iter != step_stores->end(); iter++){

	StoreVariableStatement *store_stmt = *iter;
 	VariableSymbol *dest_var = store_stmt->get_destination();

   	if(is_a<BinaryExpression>(store_stmt->get_value())){
	   BinaryExpression *the_binary_expr = to<BinaryExpression>(store_stmt->get_value());
	   if(is_a<LoadVariableExpression>(the_binary_expr->get_source1()) && is_a<IntConstant>(the_binary_expr->get_source2())){
	      LoadVariableExpression *the_load_var_expr = to<LoadVariableExpression>(the_binary_expr->get_source1());   
 	      if(dest_var == the_load_var_expr->get_source()){
 	         induction_variables->push_back(dest_var);
               }
 	   }else if(is_a<LoadVariableExpression>(the_binary_expr->get_source2()) && is_a<IntConstant>(the_binary_expr->get_source1())){
 	      LoadVariableExpression *the_load_var_expr = to<LoadVariableExpression>(the_binary_expr->get_source2());   
 	      if(dest_var == the_load_var_expr->get_source()){
 	         induction_variables->push_back(dest_var);
              }
 	   }  
 	}
   }

   delete step_stores;
 
   return induction_variables;
}

list<VariableSymbol*>* get_loop_invariants(Statement *loop_body, list<VariableSymbol*> *induction_variables){

    list<VariableSymbol*> *loop_invariants = collect_variable_symbols(loop_body);

    for (list<VariableSymbol*>::iterator iter = induction_variables->begin();
         iter != induction_variables->end(); iter++)
	 remove_from_list(*iter, loop_invariants);

    list<StoreVariableStatement*> *the_stores = collect_objects<StoreVariableStatement>(loop_body);
    
    for (list<StoreVariableStatement*>::iterator iter = the_stores->begin();
         iter != the_stores->end(); iter++) {
 
         VariableSymbol *store_destination = (*iter)->get_destination();
	 remove_from_list(store_destination, loop_invariants);

    }

    delete the_stores;
    return loop_invariants;
}

list<VariableSymbol*>* get_loop_invariants(Statement *loop_body, VariableSymbol* the_induction_variable){

    list<VariableSymbol*> *loop_invariants = collect_variable_symbols(loop_body);

    remove_from_list(the_induction_variable, loop_invariants);

    list<StoreVariableStatement*> *the_stores = collect_objects<StoreVariableStatement>(loop_body);

    for (list<StoreVariableStatement*>::iterator iter = the_stores->begin();
         iter != the_stores->end(); iter++){

         VariableSymbol *store_destination = (*iter)->get_destination();
	 remove_from_list(store_destination, loop_invariants);

    }

    delete the_stores;
    return loop_invariants;
}

int get_c_for_lower_bound(CForStatement *the_c_for){

   list<VariableSymbol*>* induction_variables = get_c_for_induction_variables(the_c_for);

   Statement *before = the_c_for->get_before();

   for (Iter<StoreVariableStatement> iter = object_iterator<StoreVariableStatement>(before);
	iter.is_valid();
	iter.next())
   {
	VariableSymbol *dest_var = iter.current().get_destination();

   	if(dest_var == induction_variables->front() && is_a<IntConstant>(iter.current().get_value()))
        {
	   IntConstant *the_int_const = to<IntConstant>(iter.current().get_value());
	   delete induction_variables; 
	   return the_int_const->get_value().c_int();
 	}
   }

   delete induction_variables; 
   error_code = 1;
   return 0;
}

void set_c_for_lower_bound(CForStatement *the_c_for, int lb){

   list<VariableSymbol*>* induction_variables = get_c_for_induction_variables(the_c_for);

   Statement *before = the_c_for->get_before();

   for (Iter<StoreVariableStatement> iter = object_iterator<StoreVariableStatement>(before);
	iter.is_valid();
	iter.next())
   {
	VariableSymbol *dest_var = iter.current().get_destination();

   	if(dest_var == induction_variables->front())
        {
	   IntConstant *the_int_const = create_int_constant(dest_var->get_suif_env(), get_data_type(dest_var), IInteger(lb));
	   iter.current().set_value(the_int_const);
 	}
   }

}

Expression* get_c_for_lower_bound_expr(CForStatement *the_c_for){

   list<VariableSymbol*>* induction_variables = get_c_for_induction_variables(the_c_for);

   Statement *before = the_c_for->get_before();

   for (Iter<StoreVariableStatement> iter = object_iterator<StoreVariableStatement>(before);
	iter.is_valid();
	iter.next())
   {
	VariableSymbol *dest_var = iter.current().get_destination();

   	if(dest_var == induction_variables->front()){
	   Expression *lower_bound = iter.current().get_value();
	   delete induction_variables;
	   return lower_bound;
	}
   }

   delete induction_variables; 
   error_code = 1;
   return 0;
}

void set_c_for_lower_bound_expr(CForStatement *the_c_for, Expression *lb_expr){

   list<VariableSymbol*>* induction_variables = get_c_for_induction_variables(the_c_for);

   Statement *before = the_c_for->get_before();

   for (Iter<StoreVariableStatement> iter = object_iterator<StoreVariableStatement>(before);
	iter.is_valid();
	iter.next())
   {
	VariableSymbol *dest_var = iter.current().get_destination();

   	if(dest_var == induction_variables->front())
	   iter.current().set_value(lb_expr);
   }

}

int get_c_for_upper_bound(CForStatement *the_c_for){

   Expression *test = the_c_for->get_test();

   for (Iter<BinaryExpression> iter = object_iterator<BinaryExpression>(test);
	iter.is_valid();
	iter.next())
   {
	BinaryExpression *the_binary_expr = to<BinaryExpression>(&iter.current());
	if(!is_a<CForStatement>(the_binary_expr->get_parent()))
	   continue;
        if(is_a<IntConstant>(the_binary_expr->get_source2())){
	   IntConstant *the_int_const = to<IntConstant>(the_binary_expr->get_source2());
	   return the_int_const->get_value().c_int();
 	}
        if(is_a<IntConstant>(the_binary_expr->get_source1())){
	   IntConstant *the_int_const = to<IntConstant>(the_binary_expr->get_source1());
	   return the_int_const->get_value().c_int();
 	}
   }
 
   error_code = 1;
   return 0;
}

Expression* get_c_for_upper_bound_expr(CForStatement *the_c_for){

   Expression *test = the_c_for->get_test();

   if(is_a<BinaryExpression>(test)){
      return (to<BinaryExpression>(test))->get_source2();
   }

   error_code = 1;
   return 0;
}

void set_c_for_upper_bound(CForStatement *the_c_for, int ub){

   Expression *test = the_c_for->get_test();

   for (Iter<BinaryExpression> iter = object_iterator<BinaryExpression>(test);
	iter.is_valid();
	iter.next())
   {
	BinaryExpression *the_binary_expr = to<BinaryExpression>(&iter.current());
	if(!is_a<CForStatement>(the_binary_expr->get_parent()))
	   continue;
        if(is_a<IntConstant>(the_binary_expr->get_source2())){
	   IntConstant *ub_expr = create_int_constant(the_binary_expr->get_suif_env(),
				the_binary_expr->get_source2()->get_result_type(), IInteger(ub));	
	   the_binary_expr->set_source2(ub_expr);
 	}
        if(is_a<IntConstant>(the_binary_expr->get_source1())){
	   IntConstant *ub_expr = create_int_constant(the_binary_expr->get_suif_env(),
				the_binary_expr->get_source1()->get_result_type(), IInteger(ub));	
	   the_binary_expr->set_source1(ub_expr);
 	}
   }
 
}

void set_c_for_upper_bound_expr(CForStatement *the_c_for, Expression *ub_expr){

   list<VariableSymbol*>* induction_variables = get_c_for_induction_variables(the_c_for);

   Expression *test = the_c_for->get_test();

   if(is_a<BinaryExpression>(test)){
      BinaryExpression *the_binary_expr = to<BinaryExpression>(test);
      if(is_a<LoadVariableExpression>(the_binary_expr->get_source1())){ 
	 LoadVariableExpression *the_load_var_expr = to<LoadVariableExpression>(the_binary_expr->get_source1());   
 	 if(the_load_var_expr->get_source() == induction_variables->front())
	    the_binary_expr->set_source2(ub_expr);
      }
      if(is_a<LoadVariableExpression>(the_binary_expr->get_source2())){
	 LoadVariableExpression *the_load_var_expr = to<LoadVariableExpression>(the_binary_expr->get_source2());   
 	 if(the_load_var_expr->get_source() == induction_variables->front())
	    the_binary_expr->set_source1(ub_expr);
      }
   }
 
   error_code = 1;
}

String get_c_for_test_opcode(CForStatement *the_c_for){

   Expression *test = the_c_for->get_test();

   BinaryExpression *the_binary_expr = to<BinaryExpression>(test);

   return String(the_binary_expr->get_opcode()); 
 
}

void set_c_for_test_opcode(CForStatement *the_c_for, String new_test_opcode){

   Expression *test = the_c_for->get_test();

   BinaryExpression *the_binary_expr = to<BinaryExpression>(test);

   the_binary_expr->set_opcode(new_test_opcode); 

}

int get_c_for_step(CForStatement *the_c_for){

   list<VariableSymbol*>* induction_variables = get_c_for_induction_variables(the_c_for);

   Statement *the_step = the_c_for->get_step();

   for (Iter<StoreVariableStatement> iter = object_iterator<StoreVariableStatement>(the_step);
	iter.is_valid();
	iter.next())
   {
	VariableSymbol *dest_var = iter.current().get_destination();

   	if(is_a<BinaryExpression>(iter.current().get_value())){
	   BinaryExpression *the_binary_expr = to<BinaryExpression>(iter.current().get_value());
	   if(is_a<LoadVariableExpression>(the_binary_expr->get_source1()) && is_a<IntConstant>(the_binary_expr->get_source2())){
	      LoadVariableExpression *the_load_var_expr = to<LoadVariableExpression>(the_binary_expr->get_source1());   
 	      if(dest_var == the_load_var_expr->get_source() && dest_var == induction_variables->front()){
                 IntConstant *the_int_const = to<IntConstant>(the_binary_expr->get_source2());
	         String opcode = String(the_binary_expr->get_opcode());
		 delete induction_variables;
                 return (opcode == String("subtract")) ? (0 - (the_int_const->get_value().c_int())) : (the_int_const->get_value().c_int());
               }
 	   }else if(is_a<LoadVariableExpression>(the_binary_expr->get_source2()) && is_a<IntConstant>(the_binary_expr->get_source1())){
 	      LoadVariableExpression *the_load_var_expr = to<LoadVariableExpression>(the_binary_expr->get_source2());   
 	      if(dest_var == the_load_var_expr->get_source() && dest_var == induction_variables->front()){
                 IntConstant *the_int_const = to<IntConstant>(the_binary_expr->get_source1());
	         String opcode = String(the_binary_expr->get_opcode());
		 delete induction_variables;
                 return (opcode == String("subtract")) ? (0 - (the_int_const->get_value().c_int())) : (the_int_const->get_value().c_int());
              }
 	   }  
 	}
   }

   delete induction_variables; 
   error_code = 1;
   return 0;
}

void set_c_for_step(CForStatement *the_c_for, int step){

   VariableSymbol* induction_variable = get_c_for_basic_induction_variable(the_c_for);

   Statement *the_step = the_c_for->get_step();

   for (Iter<StoreVariableStatement> iter = object_iterator<StoreVariableStatement>(the_step);
	iter.is_valid();
	iter.next())
   {
	VariableSymbol *dest_var = iter.current().get_destination();

   	if(is_a<BinaryExpression>(iter.current().get_value())){
	   BinaryExpression *the_binary_expr = to<BinaryExpression>(iter.current().get_value());
	   if(is_a<LoadVariableExpression>(the_binary_expr->get_source1()) && is_a<IntConstant>(the_binary_expr->get_source2())){
	      LoadVariableExpression *the_load_var_expr = to<LoadVariableExpression>(the_binary_expr->get_source1());   
 	      if(dest_var == the_load_var_expr->get_source() && dest_var == induction_variable){
                 IntConstant *the_int_const = create_int_constant(the_step->get_suif_env(), get_data_type(dest_var), IInteger(step));
		 the_binary_expr->set_source2(the_int_const);
              }
 	   }else if(is_a<LoadVariableExpression>(the_binary_expr->get_source2()) && is_a<IntConstant>(the_binary_expr->get_source1())){
 	      LoadVariableExpression *the_load_var_expr = to<LoadVariableExpression>(the_binary_expr->get_source2());   
 	      if(dest_var == the_load_var_expr->get_source() && dest_var == induction_variable){
                 IntConstant *the_int_const = create_int_constant(the_step->get_suif_env(), get_data_type(dest_var), IInteger(step));
		 the_binary_expr->set_source1(the_int_const);
              }
 	   }  
 	}
   }

}

int get_c_for_iteration_count(CForStatement *the_c_for){

    double lower_bound = (double)get_c_for_lower_bound(the_c_for);
    if(is_error_code_set()) return 0;
    double upper_bound = (double)get_c_for_upper_bound(the_c_for);
    if(is_error_code_set()) return 0;
    double step = (double)get_c_for_step(the_c_for);
    if(is_error_code_set()) return 0;

    String test_opcode = get_c_for_test_opcode(the_c_for);

    if (test_opcode == String("is_less_than"))
        return (int)ceil((upper_bound - lower_bound) / step);

    if (test_opcode == String("is_less_than_or_equal_to"))
        return (int)ceil((upper_bound - lower_bound + 1) / step);

    if (test_opcode == String("is_greater_than"))
        return (int)ceil((upper_bound - lower_bound) / step);

    if (test_opcode == String("is_greater_than_or_equal_to"))
        return (int)ceil((upper_bound - lower_bound + 1) / step);

    return 0;
}

int is_error_code_set(){

   return error_code;

}

void reset_error_code(){

   error_code = 0;

}


/*
int get_statement_index_in_statementlist(Statement *stmt, StatementList *l){

   int index = 0;

   for (Iter<Statement*> iter = l.get_child_statement_iterator();
        iter.is_valid();
        iter.next(), index++)
        if (iter.current() == item)
	    return index;

   return -1;
}

*/


