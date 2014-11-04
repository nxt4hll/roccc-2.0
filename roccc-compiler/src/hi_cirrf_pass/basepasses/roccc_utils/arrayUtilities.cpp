
#include <cassert>

#include "arrayUtilities.h"

bool IsLookupTable(VariableSymbol* v)
{
  assert(v != NULL) ;
  return (v->lookup_annote_by_name("LUT") != NULL) ;
}

// This function is responsible for returning the variable symbol associated
//  with an array reference.  This array reference could be multi-dimensional.
VariableSymbol* GetArrayVariable(ArrayReferenceExpression* a) 
{
  if (a == NULL)
  {
    return NULL ;
  }
  
  Expression* addressExp = a->get_base_array_address() ;
  SymbolAddressExpression* symAddr = 
    dynamic_cast<SymbolAddressExpression*>(addressExp) ;
  ArrayReferenceExpression* nextDimension = 
    dynamic_cast<ArrayReferenceExpression*>(addressExp) ;
  while (symAddr == NULL && nextDimension != NULL)
  {
    addressExp = nextDimension->get_base_array_address() ;
    symAddr = dynamic_cast<SymbolAddressExpression*>(addressExp) ;
    nextDimension = dynamic_cast<ArrayReferenceExpression*>(addressExp) ;
  }
  if (symAddr != NULL)
  {
    return dynamic_cast<VariableSymbol*>(symAddr->get_addressed_symbol()) ;
  }
  return NULL ;
}

QualifiedType* GetQualifiedTypeOfElement(ArrayReferenceExpression* ref) 
{
  assert(ref != NULL) ;
  
  // Check for multidimensionality
  while(dynamic_cast<ArrayReferenceExpression*>(ref->get_base_array_address())
	!= NULL)
  {
    ref=dynamic_cast<ArrayReferenceExpression*>(ref->get_base_array_address());
  }

  Expression* finalExp = ref->get_base_array_address() ;
  assert(finalExp != NULL) ;
  SymbolAddressExpression* finalSym =
    dynamic_cast<SymbolAddressExpression*>(finalExp) ;
  assert(finalSym != NULL) ;
  Symbol* internalSym = finalSym->get_addressed_symbol() ;
  assert(internalSym != NULL) ;
  VariableSymbol* finalVarSym = dynamic_cast<VariableSymbol*>(internalSym) ;
  assert(finalVarSym != NULL) ;
  
  // I now have the variable symbol of the array, now I must dig deep into
  //  the type to find the element type...
  DataType* arrayType = finalVarSym->get_type()->get_base_type() ;
  assert(arrayType != NULL) ;
  ArrayType* trueArrayType = dynamic_cast<ArrayType*>(arrayType) ;
  assert(trueArrayType != NULL) ;

  while (dynamic_cast<ArrayType*>(trueArrayType->get_element_type()->
				  get_base_type()) != NULL)
  {
    trueArrayType = dynamic_cast<ArrayType*>(trueArrayType->
					     get_element_type()->
					     get_base_type()) ;
  }
  return trueArrayType->get_element_type() ;
}

QualifiedType* GetQualifiedTypeOfElement(VariableSymbol* v)
{
  assert(v != NULL) ;
  ArrayType* trueArrayType = 
    dynamic_cast<ArrayType*>(v->get_type()->get_base_type()) ;
  assert(trueArrayType != NULL) ;
  while (dynamic_cast<ArrayType*>(trueArrayType->get_element_type()->
				  get_base_type()) != NULL)
  {
    trueArrayType = dynamic_cast<ArrayType*>(trueArrayType->
					     get_element_type()->
					     get_base_type()) ;
  }
  return trueArrayType->get_element_type() ;
}

int GetDimensionality(ArrayReferenceExpression* a)
{
  assert(a != NULL) ;
  ArrayReferenceExpression* nextDimension =
    dynamic_cast<ArrayReferenceExpression*>(a->get_base_array_address()) ;
  if (nextDimension != NULL)
  {
    return 1 + GetDimensionality(nextDimension) ;
  }
  return 1 ;
}

int GetDimensionality(VariableSymbol* v)
{
  Annote* dimAnnote = v->lookup_annote_by_name("DimensionAnnote") ;
  BrickAnnote* dimBrick = dynamic_cast<BrickAnnote*>(dimAnnote) ;
  if (dimBrick == NULL)
  {
    return 0 ;
  }
  IntegerBrick* dimValue = dynamic_cast<IntegerBrick*>(dimBrick->get_brick(0));
  assert(dimValue != NULL) ;
  return dimValue->get_value().c_int() ;
}

// This function will return a list of all indicies that are used in an
//  array reference expression.  The caller is responsible for deleting
//  the memory allocated with this function call.

list<VariableSymbol*>* UsedIndicies(ArrayReferenceExpression* a)
{
  list<VariableSymbol*>* toReturn = new list<VariableSymbol*> ;
  
  Expression* index = a->get_index() ;

  // This index should be a load variable expression or a binary expression
  LoadVariableExpression* loadIndex = 
    dynamic_cast<LoadVariableExpression*>(index) ;
  BinaryExpression* binIndex = 
    dynamic_cast<BinaryExpression*>(index) ;
  if (loadIndex != NULL)
  {
    toReturn->push_back(loadIndex->get_source()) ;
  }
  if (binIndex != NULL)
  {
    // One of the sources should be a load variable expression
    LoadVariableExpression* leftLoad = 
      dynamic_cast<LoadVariableExpression*>(binIndex->get_source1()) ;
    LoadVariableExpression* rightLoad =
      dynamic_cast<LoadVariableExpression*>(binIndex->get_source2()) ;
    if (leftLoad != NULL)
    {
      toReturn->push_back(leftLoad->get_source()) ;
    }
    if (rightLoad != NULL)
    {
      toReturn->push_back(rightLoad->get_source()) ;
    }
  }

  ArrayReferenceExpression* moreDimensions = 
    dynamic_cast<ArrayReferenceExpression*>(a->get_base_array_address()) ;
  if (moreDimensions != NULL)
  {
    list<VariableSymbol*>* moreIndicies = UsedIndicies(moreDimensions) ;
    list<VariableSymbol*>::iterator moreIter = moreIndicies->begin() ;
    while (moreIter != moreIndicies->end())
    {
      toReturn->push_back(*moreIter) ;
      ++moreIter ;
    }
    delete moreIndicies ;
  }
  return toReturn ;
}
