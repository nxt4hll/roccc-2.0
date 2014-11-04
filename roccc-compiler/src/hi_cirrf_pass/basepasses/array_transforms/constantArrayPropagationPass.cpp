#include <cassert>

#include "constantArrayPropagationPass.h"

#include "roccc_utils/roccc2.0_utils.h"
#include "roccc_utils/warning_utils.h"

#include <suifkernel/utilities.h>
#include <basicnodes/basic_factory.h>

ConstantArrayPropagationPass::ConstantArrayPropagationPass(SuifEnv* pEnv) :
  PipelinablePass(pEnv, "ConstantArrayPropagationPass")
{
  theEnv = pEnv ;
  procDef = NULL ;
}

void ConstantArrayPropagationPass::do_procedure_definition(ProcedureDefinition* proc_def)
{
  procDef = proc_def ;
  assert(procDef != NULL) ;

  OutputInformation("Constant Array Propagation Pass begins") ;

  // Step one: Find all of definition blocks for constant arrays
  CollectInitializations() ;
  
  // Step two: Go through load expressions and replace them with 
  //  any appropriate initialization.
  ReplaceLoads() ;

  OutputInformation("Constant Array Propagation Pass ends") ;
}

void ConstantArrayPropagationPass::CollectInitializations()
{
  if (!initializations.empty())
  {
    initializations.clear() ;
  }

  DefinitionBlock* procDefBlock = procDef->get_definition_block() ;
  assert(procDefBlock != NULL) ;
  Iter<VariableDefinition*> varDefIter = 
    procDefBlock->get_variable_definition_iterator() ;
  while (varDefIter.is_valid())
  {    
    VariableDefinition* varDef = varDefIter.current() ;
    assert(varDef != NULL) ;

    VariableSymbol* varSym = varDef->get_variable_symbol() ;
    ValueBlock* valBlock = varDef->get_initialization() ;
    assert(varSym != NULL) ;
    assert(valBlock != NULL) ;

    if (ValidSymbol(varSym)) 
    {
      initializations[varSym] = valBlock ;
      varSym->append_annote(create_brick_annote(theEnv, "ConstPropArray")) ;
    }

    varDefIter.next() ;
  }
}

bool ConstantArrayPropagationPass::ValidSymbol(VariableSymbol* var)
{
  assert(var != NULL) ;
  // The variable should be an array type and have the const qualifier.
  if (dynamic_cast<ArrayType*>(var->get_type()->get_base_type()) == NULL)
  {
    return false ;
  }
  QualifiedType* qualType = var->get_type() ;
  while (dynamic_cast<ArrayType*>(qualType->get_base_type()) != NULL)
  {
    ArrayType* array = dynamic_cast<ArrayType*>(qualType->get_base_type()) ;
    qualType = array->get_element_type() ;
  }
  assert(qualType != NULL) ;
  for (int i = 0 ; i < qualType->get_qualification_count(); ++i)
  {
    if (qualType->get_qualification(i) == LString("const"))
    {
      return true ;
    }
  }
  return false ;
}

// In this function I need to worry about and deal with multidimensional 
//  arrays in addition to singular dimensions.
void ConstantArrayPropagationPass::ReplaceLoads()
{
  assert(procDef != NULL) ;

  list<LoadExpression*>* allLoads = 
    collect_objects<LoadExpression>(procDef->get_body()) ;
  list<LoadExpression*>::iterator loadIter = allLoads->begin() ;
  while (loadIter != allLoads->end())
  {
    Expression* source = (*loadIter)->get_source_address() ;
    ArrayReferenceExpression* topRef = 
      dynamic_cast<ArrayReferenceExpression*>(source) ;
    if (topRef != NULL)
    {
      // Find the variable symbol of the array
      VariableSymbol* var = GetArrayVariable(topRef) ;
      assert(var != NULL) ;
      ValueBlock* topBlock = initializations[var] ;
      if (topBlock != NULL)
      {
	ReplaceLoad((*loadIter), topRef, var, topBlock) ;
      }
    }
    ++loadIter ;
  }

  delete allLoads ;
}

void ConstantArrayPropagationPass::ReplaceLoad(LoadExpression* load,
					       ArrayReferenceExpression* ref,
					       VariableSymbol* var,
					       ValueBlock* topBlock) 
{
  list<IntConstant*> allDimensions ;
  
  IntConstant* nextIndex = dynamic_cast<IntConstant*>(ref->get_index()) ;
  if (nextIndex == NULL)
  {   
    OutputWarning("Trying to access a constant array with a nonconstant index!") ;
    delete var->remove_annote_by_name("ConstPropArray") ;
    //    assert(0) ;
    return ;
  }
  allDimensions.push_front(nextIndex) ;
  
  ArrayReferenceExpression* nextDimension = 
    dynamic_cast<ArrayReferenceExpression*>(ref->get_base_array_address()) ;
  while (nextDimension != NULL)
  {
    nextIndex = dynamic_cast<IntConstant*>(nextDimension->get_index()) ;
    assert(nextIndex != NULL) ;
    allDimensions.push_front(nextIndex) ;
    nextDimension = dynamic_cast<ArrayReferenceExpression*>(nextDimension->get_base_array_address()) ;
  }

  ValueBlock* currentBlock = topBlock ;
  
  list<IntConstant*>::iterator dimIter = allDimensions.begin() ;

  for (int i = 0 ; i < allDimensions.size() ; ++i)
  {
    MultiValueBlock* multiBlock = dynamic_cast<MultiValueBlock*>(currentBlock);
    assert(multiBlock != NULL) ;
    currentBlock = multiBlock->lookup_sub_block((*dimIter)->get_value()) ;
    ++dimIter ;
  }
  
  ExpressionValueBlock* finalBlock = 
    dynamic_cast<ExpressionValueBlock*>(currentBlock) ;
  assert(finalBlock != NULL && 
	 "Attempted to use an uninitialized constant value!") ;
  Expression* replacementValue = finalBlock->get_expression() ;
  if (dynamic_cast<IntConstant*>(replacementValue) != NULL)
  {
    IntConstant* constInt = 
      dynamic_cast<IntConstant*>(replacementValue) ;
    IntConstant* replacement =
      create_int_constant(theEnv, 
			  constInt->get_result_type(),
			  constInt->get_value()) ;
    load->get_parent()->replace(load, replacement) ;
    delete load ;
  }
  else if (dynamic_cast<FloatConstant*>(replacementValue) != NULL)
  {
    FloatConstant* constFloat = 
      dynamic_cast<FloatConstant*>(replacementValue) ;
    FloatConstant* replacement = 
      create_float_constant(theEnv,
			    constFloat->get_result_type(),
			    constFloat->get_value()) ;
    load->get_parent()->replace(load, replacement) ;
    delete load ;
  }
  else
  {
    assert(0 && "Unknown constant") ;
  }

}
