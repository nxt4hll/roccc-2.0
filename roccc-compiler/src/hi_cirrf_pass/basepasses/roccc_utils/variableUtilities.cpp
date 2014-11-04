
#include <cassert>
#include <suifnodes/suif.h>
#include <suifkernel/utilities.h>

#include "variableUtilities.h"
#include "identificationUtilities.h"

bool isConst(VariableSymbol* v)
{
  QualifiedType* q = v->get_type() ;
  while (dynamic_cast<ArrayType*>(q->get_base_type()) != NULL)
  {
    ArrayType* a = dynamic_cast<ArrayType*>(q->get_base_type()) ;
    q = a->get_element_type() ;
  }
  for (int i = 0 ; i < q->get_qualification_count() ; ++i)
  {
    if (q->get_qualification(i) == LString("const"))
    {
      return true ;
    }
  }
  return false ;
}

// To implement...
bool isOutput(VariableSymbol* v)
{
  return false ;
}

// To implement...
bool isInput(VariableSymbol* v)
{
  return false ;
}

// Return by value, so a copy is made...
list<VariableSymbol*> AllDefinedVariables(Statement* s)
{
  list<VariableSymbol*> toReturn ;

  // Only call statements, store statements, and store variable
  //  statements can be definitions
  StoreVariableStatement* storeVar = dynamic_cast<StoreVariableStatement*>(s) ;
  CallStatement* call = dynamic_cast<CallStatement*>(s) ;
  StoreStatement* store = dynamic_cast<StoreStatement*>(s) ;
  
  if (store != NULL)
  {
    Expression* dest = store->get_destination_address() ;
    if (dynamic_cast<FieldAccessExpression*>(dest) != NULL)
    {
      toReturn.push_back(dynamic_cast<FieldAccessExpression*>(dest)->
			 get_field()) ;
    }
  }

  if (storeVar != NULL)
  {
    toReturn.push_back(storeVar->get_destination()) ;
  }

  if (call != NULL)
  {
    if (call->get_destination() != NULL)
    {
      toReturn.push_back(call->get_destination()) ;
    }
    // Go through all of the arguments and see which ones are output variables
    Expression* nextArg ; 
    for (unsigned int i = 0 ; i < call->get_argument_count() ; ++i)
    {
      nextArg = call->get_argument(i) ;
      if (dynamic_cast<SymbolAddressExpression*>(nextArg) != NULL)
      {
	// This is an output variable
	toReturn.push_back(dynamic_cast<VariableSymbol*>(dynamic_cast<SymbolAddressExpression*>(nextArg)->get_addressed_symbol())) ;
      }
    }
  }
  return toReturn ;
}

// Given a location, find the definition
VariableSymbol* GetDefinedVariable(Statement* def, int location)
{
  assert(location >= 0) ;

  StoreVariableStatement* storeDef = 
    dynamic_cast<StoreVariableStatement*>(def) ;
  CallStatement* callDef =
    dynamic_cast<CallStatement*>(def) ;

  if (storeDef != NULL)
  {
    assert(location == 0 && "Inappropriate location!") ;
    return storeDef->get_destination() ;
  }

  if (callDef != NULL)
  {
    if (callDef->get_destination() != NULL && location == 0)
    {
      return callDef->get_destination() ;
    }
    int currentLocation = 0 ;
    for (unsigned int i = 0 ; i < callDef->get_argument_count() ; ++i)
    {
      Expression* nextExp = callDef->get_argument(i) ;
      if (dynamic_cast<SymbolAddressExpression*>(nextExp) != NULL)
      {
	if (location == currentLocation)
	{
	  VariableSymbol* var = 
	    dynamic_cast<VariableSymbol*>(dynamic_cast<SymbolAddressExpression*>(nextExp)->get_addressed_symbol()) ;
	  return var ;
	}
	++currentLocation ;
      }
    }
  }
  return NULL ;
}

bool IsStream(Expression* argument)
{
  assert(argument != NULL) ;
  // Either an input, or an output
  LoadVariableExpression* inExpr = 
    dynamic_cast<LoadVariableExpression*>(argument) ;
  SymbolAddressExpression* outExpr =
    dynamic_cast<SymbolAddressExpression*>(argument) ;
  if (inExpr != NULL)
  {
    return IsStream(inExpr->get_source()) ;
  }
  else if (outExpr != NULL)
  {
    return 
      IsStream(dynamic_cast<VariableSymbol*>(outExpr->get_addressed_symbol()));
  }
  else
  {
    return false ;
  }
}

bool IsStream(VariableSymbol* v)
{
  assert(v != NULL) ;

  DataType* dType = v->get_type()->get_base_type() ;
  while (dynamic_cast<ReferenceType*>(dType) != NULL)
  {
    Type* tmpType = dynamic_cast<ReferenceType*>(dType)->get_reference_type() ;
    QualifiedType* qType = dynamic_cast<QualifiedType*>(tmpType) ;
    assert(qType != NULL) ;
    dType = qType->get_base_type() ;
  }

  return (dynamic_cast<PointerType*>(dType) != NULL) ;

}

// This function will return either NULL or a variable symbol based on 
//  if the expression (which should be an argument to a function call)
//  is an output parameter, which is determined either by a symbol address
//  expression or a reference variable
VariableSymbol* GetOutputVariable(Expression* argument)
{
  assert(argument != NULL) ;
  SymbolAddressExpression* outputSym = 
    dynamic_cast<SymbolAddressExpression*>(argument) ;
  if (outputSym != NULL)
  {
    Symbol* argSym = outputSym->get_addressed_symbol() ;
    return dynamic_cast<VariableSymbol*>(argSym) ;
  }
  return NULL ;
}

// Simply state if this argument is an output variable.
bool IsOutputVariable(Expression* argument)
{
  return (GetOutputVariable(argument) != NULL) ;
}

bool IsOutputVariable(VariableSymbol* v, Statement* s)
{
  assert(v != NULL) ;
  assert(s != NULL) ;

  bool found = false ;

  list<VariableSymbol*> allDefined = AllDefinedVariables(s) ;
  list<VariableSymbol*>::iterator findIter = allDefined.begin() ;
  while (findIter != allDefined.end())
  {
    found |= (v == (*findIter)) ;
    ++findIter ;
  }
  return found ;
}

// This currently will return a non unique list...
list<VariableSymbol*> AllUsedVariables(Statement* s)
{
  list<VariableSymbol*> usedVars ;
  list<LoadVariableExpression*>* allLoadVars = 
    collect_objects<LoadVariableExpression>(s) ;
  list<LoadVariableExpression*>::iterator loadIter = allLoadVars->begin() ;
  while (loadIter != allLoadVars->end())
  {
    usedVars.push_back((*loadIter)->get_source()) ;
    ++loadIter ;
  }
  delete allLoadVars ;
  return usedVars ;
}

list<VariableSymbol*> AllUsedVariablesBut(Statement* s, VariableSymbol* v)
{
  list<VariableSymbol*> usedVars ;
  list<LoadVariableExpression*>* allLoadVars =
    collect_objects<LoadVariableExpression>(s) ;
  list<LoadVariableExpression*>::iterator loadIter = allLoadVars->begin() ;
  while (loadIter != allLoadVars->end())
  {
    if ((*loadIter)->get_source() != v)
    {
      usedVars.push_back((*loadIter)->get_source()) ;
    }
    ++loadIter ;
  }
  delete allLoadVars ;
  return usedVars ;
}

// Note: This function can only be called after we have done both scalar 
//  replacement and fifo identification
VariableSymbol* ScalarReplacedVariable(VariableSymbol* array, 
				       list<int> dimOffsets)
{
  // Get the annotations associated with the array
  Annote* dimAnnote = array->lookup_annote_by_name("DimensionAnnote") ;
  BrickAnnote* dimBrick = dynamic_cast<BrickAnnote*>(dimAnnote) ;
  assert(dimBrick != NULL) ;

  Annote* indexAnnote = array->lookup_annote_by_name("IndexAnnote") ;
  BrickAnnote* indexBrick = dynamic_cast<BrickAnnote*>(indexAnnote) ;
  assert(indexBrick != NULL) ;

  IntegerBrick* dimValue = dynamic_cast<IntegerBrick*>(dimBrick->get_brick(0));
  int dimensionality = dimValue->get_value().c_int() ;
  assert(dimensionality == dimOffsets.size()) ;

  for (int i = 0 ; i < indexBrick->get_brick_count() ; i += 1 + dimensionality)
  {
    bool correctOffsets = true ;
    list<int>::iterator currentOffset = dimOffsets.begin() ;
    for (int j = 1 ; j <= dimensionality ; ++j)
    {
      SuifObjectBrick* offsetBrick =
        dynamic_cast<SuifObjectBrick*>(indexBrick->get_brick(i+j)) ;
      assert(offsetBrick != NULL) ;
      IntConstant* offset =
        dynamic_cast<IntConstant*>(offsetBrick->get_object()) ;
      assert(offset != NULL) ;
      if (offset->get_value().c_int() != (*currentOffset))
      {
	correctOffsets = false ;
	break ;
      }
      ++currentOffset ;
    }
    if (correctOffsets)
    {
      SuifObjectBrick* scalarReplacedVarBrick =
        dynamic_cast<SuifObjectBrick*>(indexBrick->get_brick(i)) ;
      assert(scalarReplacedVarBrick != NULL) ;
      VariableSymbol* scalarReplacedVar =
        dynamic_cast<VariableSymbol*>(scalarReplacedVarBrick->get_object()) ;
      assert(scalarReplacedVar != NULL) ;
      return scalarReplacedVar ;
    }
  }
  return NULL ;
}
