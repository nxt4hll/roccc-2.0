// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*
  The flow of this code is to find all of the constant arrays and store them
   in a global map with their associated value block.  Then, we walk over all
   load expressions and see if any of these are are accesses to a previously
   determined constant array.  If so, we make the replacement at that point.

*/

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
#include "constant_propagation_pass.h"
#include "roccc_utils/warning_utils.h"

using namespace std;

// THIS CONSTANT PROPAGATION PASS PROPAGATES ELEMENTS OF CONSTANT ARRAYS

/**************************** Declarations ************************************/

suif_map<VariableSymbol*, ValueBlock*> const_array_defs;

class cp_load_expression_walker: public SelectiveWalker {
public:
  cp_load_expression_walker(SuifEnv *env)
    :SelectiveWalker(env, LoadExpression::get_class_name()) {}
  Walker::ApplyStatus operator () (SuifObject *x); 
};

/**************************** Implementations ************************************/
ConstantQualedArrayPropagationPass::ConstantQualedArrayPropagationPass(SuifEnv *pEnv) 
				: PipelinablePass(pEnv, "ConstantQualedArrayPropagationPass") {}

void ConstantQualedArrayPropagationPass::do_procedure_definition(ProcedureDefinition* proc_def){

  OutputInformation("Propagation of constant arrays begins") ;

    if (proc_def){

	DefinitionBlock *proc_def_block = proc_def->get_definition_block();
	for(Iter<VariableDefinition*> iter = proc_def_block->get_variable_definition_iterator();
	    iter.is_valid(); iter.next()){
	    VariableDefinition *var_def = iter.current();
	    VariableSymbol *var_sym = var_def->get_variable_symbol();
	    QualifiedType *qualed_var_type = var_sym->get_type();	
	    if(is_a<ArrayType>(qualed_var_type->get_base_type())){
	       QualifiedType *qualed_element_type = to<ArrayType>(qualed_var_type->get_base_type())->get_element_type(); 
	       while(is_a<ArrayType>(qualed_element_type->get_base_type()))
	           qualed_element_type = to<ArrayType>(qualed_element_type->get_base_type())->get_element_type(); 
	       bool found = 0;
	       for(Iter<LString> iter2 = qualed_element_type->get_qualification_iterator();
	           iter2.is_valid(); iter2.next())
	           if(iter2.current() == LString("const"))
		      found = 1;
	       if(found)
	       {
		 // Added by Jason -> I add an annotation to the 
		 //  symbol so we will skip the printing of this variable
		 //  later.
		 const_array_defs.enter_value(var_sym, var_def->get_initialization());
		 BrickAnnote* constPropArrayAnnote = create_brick_annote(get_suif_env(), "ConstPropArray") ;
		 var_sym->append_annote(constPropArrayAnnote) ;
	       }
	    }
	}

	if(const_array_defs.size() <= 0)
	   return;

 	cp_load_expression_walker walker(get_suif_env());
	proc_def->walk(walker);
    }

    OutputInformation("Propagation of constant arrays ends") ;
}
   
Walker::ApplyStatus cp_load_expression_walker::operator () (SuifObject *x) {
	LoadExpression *load_expr = to<LoadExpression>(x);

	int array_ref_index_count = 0;
	list<Expression*> array_ref_indices;
	list<int> array_dimension_bit_sizes;
	list<int> value_block_indices;

	Expression *load_source_address = load_expr->get_source_address();
	if(!is_a<ArrayReferenceExpression>(load_source_address))
	   return Walker::Continue;

	Expression *base_address = load_source_address;
	while(is_a<ArrayReferenceExpression>(base_address)){

	    ArrayReferenceExpression *array_ref_expr = to<ArrayReferenceExpression>(base_address);
	    array_ref_indices.push_front(array_ref_expr->get_index());

	    if(!is_a<IntConstant>(array_ref_indices[array_ref_index_count]))
	       return Walker::Continue;

	    base_address = array_ref_expr->get_base_array_address();
	    array_ref_index_count++;
	}

	if(!is_a<SymbolAddressExpression>(base_address))
	   return Walker::Continue;

	SymbolAddressExpression *array_name_expr = to<SymbolAddressExpression>(base_address);
	VariableSymbol *array_symbol = to<VariableSymbol>(array_name_expr->get_addressed_symbol());

	suif_map<VariableSymbol*,ValueBlock*>::iterator iter = const_array_defs.find(array_symbol);
	if(iter == const_array_defs.end())
	   return Walker::Continue;

	QualifiedType *array_qualed_type = array_symbol->get_type();
	if(!is_a<ArrayType>(array_qualed_type->get_base_type()))
	   return Walker::Continue;

	QualifiedType *qualed_element_type = array_qualed_type; 
	for(int i = 0; i < array_ref_index_count; i++){
	    qualed_element_type = to<ArrayType>(qualed_element_type->get_base_type())->get_element_type(); 
	    int element_bit_size = (qualed_element_type->get_base_type()->get_bit_size()).c_int();
	    array_dimension_bit_sizes.push_back(element_bit_size);
	}

	int array_value_block_key = 0;
	for(int i = 0; i < array_ref_index_count; i++){
	    int array_ref_index = to<IntConstant>(array_ref_indices[i])->get_value().c_int();
	    array_value_block_key = array_ref_index * array_dimension_bit_sizes[i];
	    value_block_indices.push_back(array_value_block_key);
	}

	ValueBlock *element_value_block = (*iter).second;
	for(int i = 0; i < array_ref_index_count; i++)
	    element_value_block = (to<MultiValueBlock>(element_value_block))->lookup_sub_block(value_block_indices[i]);
	if(!is_a<ExpressionValueBlock>(element_value_block))
    	   return Walker::Continue;
	Expression *element_value_expr = to<ExpressionValueBlock>(element_value_block)->get_expression();
	if(!is_a<IntConstant>(element_value_expr))
      	   return Walker::Continue;
	Expression *replacement = to<Expression>(deep_suif_clone(element_value_expr));
	load_expr->get_parent()->replace(load_expr, replacement);

	array_ref_indices.clear_list();
	array_dimension_bit_sizes.clear_list();
	value_block_indices.clear_list();

    	return Walker::Continue;
}

