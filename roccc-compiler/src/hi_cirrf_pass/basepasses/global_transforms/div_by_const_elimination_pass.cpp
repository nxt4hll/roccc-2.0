// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include "common/system_specific.h"
#include <common/suif_copyright.h>

#include <math.h>
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
#include "div_by_const_elimination_pass.h"
#include "roccc_utils/warning_utils.h"

/**************************** Declarations ************************************/

class dbcp_binary_expression_walker: public SelectiveWalker {
public:
  dbcp_binary_expression_walker(SuifEnv *env)
    :SelectiveWalker(env, BinaryExpression::get_class_name()) {}
  Walker::ApplyStatus operator () (SuifObject *x); 
};

/**************************** Implementations ***********************************/
DivByConstEliminationPass::DivByConstEliminationPass(SuifEnv *pEnv) : PipelinablePass(pEnv, "DivByConstEliminationPass") {}

void DivByConstEliminationPass::do_procedure_definition(ProcedureDefinition* proc_def){
  OutputInformation("Division by constant elimination pass begins") ;
  if (proc_def)
  {
    dbcp_binary_expression_walker walker(get_suif_env());
    proc_def->walk(walker);
  }
  OutputInformation("Division by constant elimination pass ends") ;
}
   
list<int>* get_approx_reciprocal_powers(int N){

// computes 1/N in terms of 1/2's power series

	int power = 0;
	double target = 1.0/N;

	list<int>* reciprocal_series = new list<int>;
 
	while(target != 0 && power < 14){
	     if((1/pow(2,power)) <= target){
		target -= (1/pow(2,power)); 
		reciprocal_series->push_back(power);		
	     }
	     power++;	
	}	

	return reciprocal_series;
}

Walker::ApplyStatus dbcp_binary_expression_walker::operator () (SuifObject *x) {
	SuifEnv *env = get_env();
	BinaryExpression *binary_expr = to<BinaryExpression>(x);

	Expression *src1 = binary_expr->get_source1();
	Expression *src2 = binary_expr->get_source2();
	String opcode = String(binary_expr->get_opcode());

	if(!is_a<IntConstant>(src2) || opcode != String("divide"))
    	   return Walker::Continue;

	list<int>* reciprocal_powers = get_approx_reciprocal_powers((to<IntConstant>(src2))->get_value().c_int());
	
	BinaryExpression *replacement = NULL;

        IntConstant *int_src2 = create_int_constant(env, src2->get_result_type(), IInteger(reciprocal_powers->back()));
	reciprocal_powers->pop_back();
	replacement = create_binary_expression(env, binary_expr->get_result_type(), String("right_shift"),
						to<Expression>(deep_suif_clone(src1)), int_src2);
	while(reciprocal_powers->size() > 0){
              int_src2 = create_int_constant(env, src2->get_result_type(), IInteger(reciprocal_powers->back()));
	      reciprocal_powers->pop_back();
	      BinaryExpression *shift_expr = create_binary_expression(env, binary_expr->get_result_type(),
						String("right_shift"), to<Expression>(deep_suif_clone(src1)), int_src2);
	      replacement = create_binary_expression(env, binary_expr->get_result_type(),
						String("add"), replacement, shift_expr);
	}
	
        binary_expr->get_parent()->replace(binary_expr, replacement);
	set_address(get_expression_owner(replacement));
    	return Walker::Replaced;
}

