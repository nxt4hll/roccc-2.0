// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include "common/system_specific.h"
#include <common/suif_copyright.h>

#include <iostream>
#include <iokernel/cast.h>
#include <iokernel/clone_stream.h>
#include <basicnodes/basic_factory.h>
#include <suifnodes/suif.h>
#include <suifnodes/suif_factory.h>
#include <basicnodes/basic.h>
#include <basicnodes/basic_constants.h>
#include <suifkernel/suifkernel_messages.h>
#include <suifkernel/utilities.h>
#include <suifkernel/group_walker.h>
#include "transforms/procedure_walker_utilities.h"
#include "utils/expression_utils.h"
#include "roccc_utils/warning_utils.h"
#include "bit_resize_datapath_constants_pass.h"

using namespace std;

/**************************** Declarations ************************************/

class brdp_int_constant_expression_walker: public SelectiveWalker { 
public:
  brdp_int_constant_expression_walker(SuifEnv *env)
    :SelectiveWalker(env, IntConstant::get_class_name()) {}
  Walker::ApplyStatus operator () (SuifObject *x);
};

/**************************** Implementations ************************************/

BitResizeDatapathConstantsPass::BitResizeDatapathConstantsPass( SuifEnv* suif_env) 
				: PipelinablePass( suif_env, "BitResizeDatapathConstantsPass" ) {}

void BitResizeDatapathConstantsPass::do_procedure_definition(ProcedureDefinition *proc_def){
  OutputInformation("Bit resizing datapath constants pass begins") ;
  if(proc_def)
  {
    brdp_int_constant_expression_walker walker2(get_suif_env());
    proc_def->walk(walker2);
  }
  OutputInformation("Bit resizing datapath constants pass ends") ;    
}

Walker::ApplyStatus brdp_int_constant_expression_walker::operator () (SuifObject *x) {
        SuifEnv *env = get_env();
	IntConstant *int_constant_expr = to<IntConstant>(x);
	
	Statement *parent_stmt = get_expression_owner(int_constant_expr);

	SuifObject *current_parent_obj = int_constant_expr->get_parent();
	while(current_parent_obj != parent_stmt){
	   if(is_a<BinaryExpression>(current_parent_obj)){
	      BinaryExpression *binary_expr = to<BinaryExpression>(current_parent_obj);
	      if(binary_expr->get_source1() == int_constant_expr)
	         int_constant_expr->set_result_type(binary_expr->get_source2()->get_result_type());
	      else int_constant_expr->set_result_type(binary_expr->get_source1()->get_result_type());
	      return Walker::Continue;
	   }
	   current_parent_obj = current_parent_obj->get_parent();
	}

	if(is_a<StoreVariableStatement>(parent_stmt)){
	   StoreVariableStatement *store_var_stmt = to<StoreVariableStatement>(current_parent_obj);
	   int_constant_expr->set_result_type(store_var_stmt->get_destination()->get_type()->get_base_type());
	   return Walker::Continue;
	}

	if(is_a<CallStatement>(parent_stmt)){
	   CallStatement *call_stmt = to<CallStatement>(current_parent_obj);

	   SymbolAddressExpression *callee_address = to<SymbolAddressExpression>(call_stmt->get_callee_address());
           ProcedureSymbol *macro_sym = to<ProcedureSymbol>(callee_address->get_addressed_symbol());

	   if(String(macro_sym->get_name()).starts_with("ROCCC_cam"))
	      return Walker::Continue;
	  
	   if(String(macro_sym->get_name()).starts_with("ROCCC_convert"))
	      return Walker::Continue;
	  
	   int_constant_expr->set_result_type(call_stmt->get_destination()->get_type()->get_base_type());
	   return Walker::Continue;
	}

    	return Walker::Continue;
}


