
#include <cassert>
#include <sstream>

#include <cfenodes/cfe.h>
#include <basicnodes/basic_factory.h>
#include <suifnodes/suif_factory.h>
#include <suifkernel/utilities.h>

#include "transform_unrolled_arrays.h"
#include "roccc_utils/warning_utils.h"
#include "roccc_utils/roccc2.0_utils.h"

// Equivalent References definitions

EquivalentReferences::EquivalentReferences()
{
}

EquivalentReferences::~EquivalentReferences()
{
  allEquivalent.clear_list() ;
}

// Transformation pass definitions

TransformUnrolledArraysPass::TransformUnrolledArraysPass(SuifEnv* pEnv) :
  PipelinablePass(pEnv, "TransformUnrolledArraysPass") 
{
  theEnv = pEnv ;
  procDef = NULL ;
  innermost = NULL ;
}

TransformUnrolledArraysPass::~TransformUnrolledArraysPass()
{
  ClearReferences() ;
}

void TransformUnrolledArraysPass::ClearReferences()
{
  if (!currentReferences.empty())
  {
    list<EquivalentReferences*>::iterator delIter = currentReferences.begin() ;
    while (delIter != currentReferences.end())
    {
      delete (*delIter) ;
      ++delIter ;
    }
    currentReferences.clear_list() ;  
  }
}

void TransformUnrolledArraysPass::do_procedure_definition(ProcedureDefinition* proc_def)
{
  procDef = proc_def ;
  assert(procDef != NULL) ;
  
  OutputInformation("Transforming Unrolled Arrays Pass Begins") ;

  innermost = InnermostList(procDef) ;
  assert(innermost != NULL) ;

  int maxD = MaxDimension() ;
  for (int i = maxD ; i > 0 ; --i)
  {    
    TransformNDIntoNMinusOneD(i) ;
  }

  OutputInformation("Transforming Unrolled Arrays Pass Ends") ;
}

void TransformUnrolledArraysPass::TransformNDIntoNMinusOneD(int N)
{
  // Get all unique N-dimensional arrays
  CollectArrays(N) ;

  // For every array access that has a constant as one of its offsets,
  //  we have to create a new array

  list<VariableSymbol*> arraysToRemove ;
  list<EquivalentReferences*>::iterator refIter = currentReferences.begin() ;
  while (refIter != currentReferences.end())
  {
    VariableSymbol* originalSymbol = GetArrayVariable((*refIter)->original) ;
    // Lookup tables should not be transformed
    if (originalSymbol->lookup_annote_by_name("LUT") == NULL)
    {
      bool replaced = ReplaceNDReference(*refIter) ;
      if (replaced)
      {
	if (!InList(arraysToRemove, originalSymbol))
	{
	  arraysToRemove.push_back(originalSymbol) ;
	}
      }
    }
    ++refIter ;
  }

  // Remove all of the arrays that need to be removed
  list<VariableSymbol*>::iterator arrayIter = arraysToRemove.begin() ;
  while (arrayIter != arraysToRemove.end())
  {
    procDef->get_symbol_table()->remove_symbol_table_object(*arrayIter) ;
    ++arrayIter ;
  }

}

int TransformUnrolledArraysPass::MaxDimension()
{
  assert(innermost != NULL) ;

  int maxSize = 0 ;
  list<ArrayReferenceExpression*>* allRefs = 
    collect_objects<ArrayReferenceExpression>(innermost) ;
  list<ArrayReferenceExpression*>::iterator refIter = allRefs->begin() ;
  refIter = allRefs->begin() ;
  while (refIter != allRefs->end())
  {
    int currentSize = GetDimensionality(*refIter) ;
    if (currentSize > maxSize)
    {
      maxSize = currentSize ;
    }
    ++refIter ;
  }
  delete allRefs ;
  return maxSize ;
}

void TransformUnrolledArraysPass::CollectArrays(int dimensionality)
{
  ClearReferences() ;

  assert(innermost != NULL) ;

  list<ArrayReferenceExpression*>* allRefs = 
    collect_objects<ArrayReferenceExpression>(innermost) ;

  list<ArrayReferenceExpression*>::iterator refIter = allRefs->begin() ;
  while(refIter != allRefs->end())
  {
    if (GetDimensionality(*refIter) == dimensionality)
    {
      // Create a new Equivalent reference 
      EquivalentReferences* nextReference = new EquivalentReferences() ;
      nextReference->original = (*refIter) ;
      currentReferences.push_back(nextReference) ;
    }
    ++refIter ;
  }
  delete allRefs ;

  Uniquify() ;
}

// This function goes through the collected references and finds equivalences
void TransformUnrolledArraysPass::Uniquify()
{
  list<EquivalentReferences*>::iterator refIter = currentReferences.begin() ;
  while (refIter != currentReferences.end())
  {
    // Search all of the other references and find equivalencies
    list<EquivalentReferences*>::iterator searchIter = refIter ;
    ++searchIter ;
    while (searchIter != currentReferences.end())
    {
      if (EquivalentExpressions((*refIter)->original, (*searchIter)->original))
      {
	(*refIter)->allEquivalent.push_back((*searchIter)->original) ;
	// Also remove it from the original list
	searchIter = currentReferences.erase(searchIter) ;	
      }
      else
      {
	++searchIter ;
      }
    }
    ++refIter ;
  }
}

// All of the array references expressions in the passed in the struct are
//  equivalent, so we can determine types of the original and use that
//  to create a new expression with which to replace everything.
bool TransformUnrolledArraysPass::ReplaceNDReference(EquivalentReferences* a)
{
  assert(a != NULL) ;
  assert(a->original != NULL) ;

  // Check to see if the reference at this stage is a constant or not
  IntConstant* constantIndex = 
    dynamic_cast<IntConstant*>(a->original->get_index()) ;
  
  if (constantIndex == NULL)
  {
    // There was no replacement made
    return false ;
  }

  Expression* baseAddress = a->original->get_base_array_address() ;
  assert(baseAddress != NULL) ;
  assert(constantIndex != NULL) ;

  // Create a replacement expression for this value.  This will either
  //  be another array reference expression or a single variable.
  Expression* replacementExp = NULL ;
  //  QualifiedType* elementType = GetQualifiedTypeOfElement(a->original) ;
  VariableSymbol* originalSymbol = GetArrayVariable(a->original) ;
  assert(originalSymbol != NULL) ;
  LString replacementName = 
    GetReplacementName(originalSymbol->get_name(), 
		       constantIndex->get_value().c_int()) ;
  int dimensionality = GetDimensionality(a->original) ;
  
  QualifiedType* elementType = originalSymbol->get_type() ;
  while (dynamic_cast<ArrayType*>(elementType->get_base_type()) != NULL)
  {
    elementType = dynamic_cast<ArrayType*>(elementType->get_base_type())->get_element_type() ;
  }
  
  // There is a special case for one dimensional arrays as opposed to all
  //  other dimensional arrays.  It only should happen if we are truly
  //  replacing an array with a one dimensional array.
  if (dimensionality == 1 && 
      dynamic_cast<ArrayReferenceExpression*>(a->original->get_parent())==NULL)
  {

    VariableSymbol* replacementVar = 
      create_variable_symbol(theEnv,
			     GetQualifiedTypeOfElement(a->original),
			     TempName(replacementName)) ;
    procDef->get_symbol_table()->append_symbol_table_object(replacementVar) ;
    
    replacementExp = 
      create_load_variable_expression(theEnv,
				      elementType->get_base_type(),
				      replacementVar) ;
  }
  else
  {
    // Create a new array with one less dimension.  This requires a new
    //  array type.
    ArrayType* varType = 
      dynamic_cast<ArrayType*>(originalSymbol->get_type()->get_base_type()) ;
    assert(varType != NULL) ;
   
    ArrayType* replacementArrayType =
      create_array_type(theEnv,
	varType->get_element_type()->get_base_type()->get_bit_size(),
	0, // bit alignment
	OneLessDimension(originalSymbol->get_type(), dimensionality),
	dynamic_cast<Expression*>(varType->get_lower_bound()->deep_clone()),
	dynamic_cast<Expression*>(varType->get_upper_bound()->deep_clone()),
	TempName(varType->get_name())) ;

    procDef->get_symbol_table()->append_symbol_table_object(replacementArrayType) ;

    VariableSymbol* replacementArraySymbol = 
      create_variable_symbol(theEnv,
			     create_qualified_type(theEnv,
						   replacementArrayType,
						   TempName(LString("qualType"))),
			     TempName(replacementName)) ;

    procDef->get_symbol_table()->append_symbol_table_object(replacementArraySymbol) ;

    // Create a new symbol address expression for this variable symbol
    SymbolAddressExpression* replacementAddrExp =
      create_symbol_address_expression(theEnv,
				       replacementArrayType,
				       replacementArraySymbol) ;

    // Now, replace the symbol address expression in the base
    //  array address with this symbol.
    ReplaceSymbol(a->original, replacementAddrExp) ;
    
    // And replace this reference with the base array address.
    replacementExp = a->original->get_base_array_address() ;
    a->original->set_base_array_address(NULL) ;
    replacementExp->set_parent(NULL) ;
  }

  // Replace all of the equivalent expressions with the newly generated
  //  replacement expression.
  assert(replacementExp != NULL) ;
  a->original->get_parent()->replace(a->original, replacementExp) ;
   
  //  ReplaceChildExpression(a->original->get_parent(),
  //			 a->original,
  //			 replacementExp) ;

  list<ArrayReferenceExpression*>::iterator equivIter = 
    a->allEquivalent.begin() ;
  while (equivIter != a->allEquivalent.end()) 
  {
    (*equivIter)->get_parent()->replace((*equivIter),
					dynamic_cast<Expression*>(replacementExp->deep_clone())) ;
    //    ReplaceChildExpression((*equivIter)->get_parent(),
    //			   (*equivIter),
    //			   dynamic_cast<Expression*>(replacementExp->deep_clone())) ;
    ++equivIter ;
  }

  return true ;
}

void TransformUnrolledArraysPass::ReplaceSymbol(ArrayReferenceExpression* x,
						SymbolAddressExpression* addr)
{
  Expression* address = x->get_base_array_address() ;
  SymbolAddressExpression* symAddr =
    dynamic_cast<SymbolAddressExpression*>(address) ;
  ArrayReferenceExpression* anotherRef = 
    dynamic_cast<ArrayReferenceExpression*>(address) ;
  
  if (symAddr != NULL)
  {
    symAddr->set_parent(NULL) ;
    x->set_base_array_address(addr) ;
    delete symAddr ;    
    return ;
  }
  if (anotherRef != NULL)
  {
    ReplaceSymbol(anotherRef, addr) ;
    return ;
  }
  assert(0) ;
}


bool TransformUnrolledArraysPass::InList(list<VariableSymbol*>& theList,
					  VariableSymbol* v)
{
  list<VariableSymbol*>::iterator listIter = theList.begin() ;
  while (listIter != theList.end())
  {
    if (v == (*listIter))
    {
      return true ;
    }
    ++listIter ;
  }
  return false ;
}

LString TransformUnrolledArraysPass::GetReplacementName(LString baseName, 
							int offset)
{
   LString finalName = baseName ;
   finalName = finalName + "_" ;
   if (offset < 0)
   {
     offset = -offset ;
     finalName = finalName + "minus" ;
   }
   finalName = finalName + LString(offset) ;
   finalName = finalName + "_" ;
   return finalName ;
}

QualifiedType* TransformUnrolledArraysPass::OneLessDimension(QualifiedType* original, int dimensionality)
{
  ArrayType* topLevelType = 
    dynamic_cast<ArrayType*>(original->get_base_type()) ;
  assert(topLevelType != NULL) ;

  if (dynamic_cast<ArrayType*>(topLevelType->get_element_type()->get_base_type()) != NULL)
  {
    ArrayType* nextLevel = 
      dynamic_cast<ArrayType*>(topLevelType->get_element_type()->get_base_type()) ;
    return nextLevel->get_element_type() ;
  }
  else
  {
    return topLevelType->get_element_type() ;
  }
  
}
