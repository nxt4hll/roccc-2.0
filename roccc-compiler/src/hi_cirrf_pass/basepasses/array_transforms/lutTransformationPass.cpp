
#include <cassert>

#include <basicnodes/basic.h>
#include <basicnodes/basic_factory.h>

#include <suifnodes/suif.h>
#include <suifnodes/suif_factory.h>

#include <cfenodes/cfe.h>
#include <cfenodes/cfe_factory.h>

#include "roccc_utils/roccc2.0_utils.h"
#include "roccc_utils/warning_utils.h"

#include "lutTransformationPass.h"

LUTTransformationPass::LUTTransformationPass(SuifEnv* pEnv) : 
  PipelinablePass(pEnv, "LookupTableTransformation")
{
  theEnv = pEnv ;
  procDef = NULL ;
  LUTCounter = 0 ;
}

LUTTransformationPass::~LUTTransformationPass()
{
  ; // Nothing to delete yet
}

void LUTTransformationPass::do_procedure_definition(ProcedureDefinition* p)
{
  procDef = p ;
  assert(procDef != NULL) ;

  if (isLegacy(procDef))
  {
    return ;
  }

  OutputInformation("LUT Transformation pass begins") ;

  ReplaceLoads() ;
  ReplaceStores() ;

  OutputInformation("LUT Transformation pass ends") ;
}

void LUTTransformationPass::ReplaceLookup(LoadExpression* container,
					  ArrayReferenceExpression* x) 
{
  assert(procDef != NULL) ;
  
  // I need to create a new function prototype and replace the load expression
  //  with a call expression.

  LString procedureName = "ROCCCLUTLookup" ;
  LString procedureTypeName = "ROCCCLUTLookupType" ;
  
  procedureName = procedureName + LString(LUTCounter) ;
  procedureTypeName = procedureTypeName + LString(LUTCounter) ;

  CProcedureType* lookupType = 
    create_c_procedure_type(theEnv,
			    GetQualifiedTypeOfElement(x)->get_base_type(),
			    false,                        // has varargs
			    true,                         // arguments known
			    0,                            // bit alignment
			    procedureTypeName) ;
  
  ProcedureSymbol* lookupSymbol =
    create_procedure_symbol(theEnv,
			    lookupType,
			    procedureName) ;

  // Add both types and symbols to the symbol table
  procDef->get_symbol_table()->append_symbol_table_object(lookupType) ;
  procDef->get_symbol_table()->append_symbol_table_object(lookupSymbol) ;

  SymbolAddressExpression* lookupSymAddr = 
    create_symbol_address_expression(theEnv,
			     GetQualifiedTypeOfElement(x)->get_base_type(),
				     lookupSymbol) ;

  // Create a call expression
  CallExpression* replacement = 
    create_call_expression(theEnv,
			   GetQualifiedTypeOfElement(x)->get_base_type(),
			   lookupSymAddr) ;
  
  VariableSymbol* arraySym = GetArrayVariable(x) ;
  assert(arraySym != NULL) ;

  LoadVariableExpression* loadArraySym = 
    create_load_variable_expression(theEnv,
				    arraySym->get_type()->get_base_type(),
				    arraySym) ;
 
  lookupType->append_argument(arraySym->get_type()) ;
  replacement->append_argument(loadArraySym) ;

  // Now, we need an offset (which should be an integer)
  
  lookupType->append_argument(create_qualified_type(theEnv, 
 		 	      x->get_index()->get_result_type())) ;
  Expression* index = CreateIndex(x) ;
  replacement->append_argument(index) ;

  // Add a dummy value to the end of the lookup so the Lo-CIRRF side
  //  can use it as a dependency.
  lookupType->append_argument(GetQualifiedBaseInt(theEnv)) ;
  IntConstant* dummyZero = 
    create_int_constant(theEnv, GetBaseInt(theEnv), IInteger(0)) ;
  replacement->append_argument(dummyZero) ;

  // Finally, make the replacement
  container->get_parent()->replace(container, replacement) ;

  // Will these kill me?
  //  delete container ;
  //  delete x ;

  ++LUTCounter ;
}

// We're pretty much guaranteed that this is the top level array reference
Expression* LUTTransformationPass::CreateIndex(ArrayReferenceExpression* x)
{
  int dimensionality = GetDimensionality(x) ;

  VariableSymbol* arraySym = GetArrayVariable(x) ;
  assert(arraySym != NULL) ;
  int elementSize = 
    GetQualifiedTypeOfElement(x)->get_base_type()->get_bit_size().c_int() ;

  DataType* currentType = arraySym->get_type()->get_base_type() ;
  
  // In addition to finding the indicies, I have to determine the
  //  size of each dimension
  list<Expression*> indicies ;
  list<int> sizes ;
  
  ArrayReferenceExpression* currentRef = x ;
  for (int i = 0 ; i < dimensionality ; ++i)
  {
    indicies.push_front(currentRef->get_index()) ;
    assert(currentRef != NULL) ;
    currentRef = 
      dynamic_cast<ArrayReferenceExpression*>(currentRef->
					      get_base_array_address()) ;
    
    assert(dynamic_cast<ArrayType*>(currentType) != NULL) ;
    currentType = 
      dynamic_cast<ArrayType*>(currentType)->get_element_type()->get_base_type() ;
    sizes.push_back(currentType->get_bit_size().c_int() / elementSize) ;
  }
  assert(indicies.size() > 0) ;
  assert(sizes.size() > 0) ;

  // Now, combine the indicies  
  Expression* finalExp ;
  list<Expression*>::iterator indexIter = indicies.begin() ;
  list<int>::iterator sizeIter = sizes.begin() ;
  finalExp = (*indexIter) ;
  finalExp->set_parent(NULL) ;
  ++indexIter ;
  while (indexIter != indicies.end())
  {
    (*indexIter)->set_parent(NULL) ;
    // (Current index times size) + next element
    IntConstant* sizeExpr = 
      create_int_constant(theEnv,
			  GetBaseInt(theEnv),
			  IInteger(*sizeIter)) ;
    finalExp = 
      create_binary_expression(theEnv,
			       finalExp->get_result_type(),
			       LString("multiply"),
			       finalExp, 
			       sizeExpr) ;
    finalExp = 
      create_binary_expression(theEnv, 
			       finalExp->get_result_type(),
			       LString("add"),
			       finalExp,
			       (*indexIter)) ;

    ++indexIter ;
    ++sizeIter ;
  }

  return finalExp ;
}

void LUTTransformationPass::ReplaceStore(StoreStatement* container,
					 ArrayReferenceExpression* x)
{
  // The store statement must be replaced with a call statement

  LString procedureName = "ROCCCLUTStore" ;
  LString procedureTypeName = "ROCCCLUTStoreType" ;
  
  procedureName = procedureName + LString(LUTCounter) ;
  procedureTypeName = procedureTypeName + LString(LUTCounter) ;

  CProcedureType* storeType = 
    create_c_procedure_type(theEnv,
			    GetQualifiedTypeOfElement(x)->get_base_type(),
			    false,                        // has varargs
			    true,                         // arguments known
			    0,                            // bit alignment
			    procedureTypeName) ;
  
  ProcedureSymbol* storeSymbol =
    create_procedure_symbol(theEnv,
			    storeType,
			    procedureName) ;

  // Add both types and symbols to the symbol table
  procDef->get_symbol_table()->append_symbol_table_object(storeType) ;
  procDef->get_symbol_table()->append_symbol_table_object(storeSymbol) ;

  SymbolAddressExpression* functionAddress = 
    create_symbol_address_expression(theEnv,
				     GetQualifiedTypeOfElement(x)->get_base_type(),
				     storeSymbol) ;

  CallStatement* replacement = 
    create_call_statement(theEnv, 
			  NULL, // destination
			  functionAddress) ;

  // The arguments are the Lookup table, and then the index
  VariableSymbol* arraySym = GetArrayVariable(x) ;
  assert(arraySym != NULL) ;

  LoadVariableExpression* storeArraySym = 
    create_load_variable_expression(theEnv,
				    arraySym->get_type()->get_base_type(),
				    arraySym) ;
 
  storeType->append_argument(arraySym->get_type()) ;
  replacement->append_argument(storeArraySym) ;

  // Now, we need an offset (which should be an integer)
  
  storeType->append_argument(create_qualified_type(theEnv, 
 		 	      x->get_index()->get_result_type())) ;
  Expression* index = CreateIndex(x) ;
  replacement->append_argument(index) ;

  // Now, add the actual value
  storeType->append_argument(create_qualified_type(theEnv,
 	 	 	     container->get_value()->get_result_type())) ;
  Expression* value = container->get_value() ;
  value->set_parent(NULL) ;
  replacement->append_argument(value) ;

  // I need to add a dummy argument to the end of the call so Adrian
  //  can create dependencies.
  storeType->append_argument(GetQualifiedBaseInt(theEnv)) ;
  IntConstant* dummyZero = 
    create_int_constant(theEnv, GetBaseInt(theEnv), IInteger(0)) ;
  replacement->append_argument(dummyZero) ;

  // Finally, make the replacement
  container->get_parent()->replace(container, replacement) ;
 
  ++LUTCounter ;
}

void LUTTransformationPass::ReplaceLoads()
{
  assert(procDef != NULL) ;

  list<LoadExpression*>* allLoads = 
    collect_objects<LoadExpression>(procDef->get_body()) ;
  list<LoadExpression*>::iterator loadIter = allLoads->begin() ;
  while (loadIter != allLoads->end())
  {
    ArrayReferenceExpression* currentRef = 
      dynamic_cast<ArrayReferenceExpression*>((*loadIter)->
					      get_source_address());
    assert(currentRef != NULL && "Generic pointers not supported!") ;
    if (IsLookupTable(GetArrayVariable(currentRef)))
    {
      ReplaceLookup((*loadIter), currentRef) ;
    }
    ++loadIter ;
  }
  delete allLoads ;
}

void LUTTransformationPass::ReplaceStores()
{
  assert(procDef != NULL) ;
  list<StoreStatement*>* allStores = 
    collect_objects<StoreStatement>(procDef->get_body()) ;
  list<StoreStatement*>::iterator storeIter = allStores->begin() ;
  while (storeIter != allStores->end())
  {
    ArrayReferenceExpression* currentRef = 
      dynamic_cast<ArrayReferenceExpression*>((*storeIter)->
					      get_destination_address()) ;
    assert(currentRef != NULL && "Generic pointers not supported!") ;
    
    if (IsLookupTable(GetArrayVariable(currentRef)))
    {
      ReplaceStore((*storeIter), currentRef) ;
    }    
    ++storeIter ;
  }
  delete allStores ;
}
