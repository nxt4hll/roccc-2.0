// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

#include <cassert>

#include <suifkernel/utilities.h>
#include <basicnodes/basic_factory.h>
#include <suifnodes/suif_factory.h>

#include "pointer_conversion.h"

#include "roccc_utils/warning_utils.h"
#include "roccc_utils/roccc2.0_utils.h"

PointerConversionPass::PointerConversionPass(SuifEnv* pEnv) :
  PipelinablePass(pEnv, "PointerConversionPass") 
{
  theEnv = pEnv ;
  procDef = NULL ;
}

void PointerConversionPass::do_procedure_definition(ProcedureDefinition* proc_def)
{
  procDef = proc_def ;
  assert(procDef != NULL) ;
  
  OutputInformation("Pointer conversion pass begins") ;

  // In order to handle N-dimensional arrays, I have to do this
  //  until there is no longer any change
  bool change = false ;
  do 
  {
    change = false ;
    list<LoadExpression*>* allLoads = 
      collect_objects<LoadExpression>(procDef->get_body()) ;
    list<LoadExpression*>::iterator loadIter = allLoads->begin() ;
    while (loadIter != allLoads->end())
    {
      change |= ReplaceLoad(*loadIter) ;
      ++loadIter ;
    }
    delete allLoads ;
  } while (change == true) ;

  // Now do the stores... again, until there is no change
  do 
  {
    change = false ;
    list<StoreStatement*>* allStores = 
      collect_objects<StoreStatement>(procDef->get_body()) ;
    list<StoreStatement*>::iterator storeIter = allStores->begin() ;
    while (storeIter != allStores->end())
    {
      change |= ReplaceStore(*storeIter) ;
      ++storeIter ;
    }
  } while (change == true) ;

  // Handle the special case of code that wrote the code p[0], which is
  //  going to look like a load of a pointer variable but really should be
  //  an array access.
  HandleFormSix() ;

  OutputInformation("Pointer conversion pass ends") ;
}

// Stores have to go to reference variables which look compeletly different
bool PointerConversionPass::ReplaceStore(StoreStatement* top)
{
  Expression* destination = top->get_destination_address() ;
  BinaryExpression* binDest = dynamic_cast<BinaryExpression*>(destination) ;

  if (binDest == NULL)
  {
    return false ;
  }

  // Comment from here 
  /*
  int form = StoreFormInternal(binDest) ;

  while (form == 0 && 
	 dynamic_cast<BinaryExpression*>(binDest->get_source1()) != NULL)
  {
    binDest = dynamic_cast<BinaryExpression*>(binDest->get_source1()) ;
    form = StoreFormInternal(binDest) ;
  }
  
  if (form == 0)
  {
    return false ;
  }

  // First, construct the offset
  IntConstant* offset = GetStoreOffset(binDest, form) ;
  
  // Next, construct the index
  Expression* index = GetStoreIndex(binDest, form) ;

  Expression* combined = CombinedOffset(offset, index, binDest->get_opcode()) ;

  DataType* elementType = GetElementType(binDest, form) ;
  Expression* arrayAddress = GetArrayAddress(binDest, form) ;

  // Finally, make the replacement
  ArrayReferenceExpression* replacement = 
    create_array_reference_expression(theEnv,
				      elementType,
				      arrayAddress,
				      combined) ;
  binDest->set_parent(NULL) ;
  top->set_destination_address(NULL) ;
  top->set_destination_address(replacement) ;
  return true ;
  */  
    
  int form = FormInternal(binDest) ;

  while (form == 0 && 
	 dynamic_cast<BinaryExpression*>(binDest->get_source1()) != NULL)
  {
    binDest = dynamic_cast<BinaryExpression*>(binDest->get_source1()) ;
    form = FormInternal(binDest) ;
  }
  
  if (form == 0)
  {
    return false ;
  }

  // First, construct the offset
  IntConstant* offset = GetOffset(binDest, form) ;
  Expression* index = GetIndex(binDest, form) ;
  Expression* combined = CombinedOffset(offset, index, binDest->get_opcode()) ;

  // Now, determine the element type
  DataType* elementType = GetElementType(binDest, form) ;

  // Next, get the address of the array
  Expression* arrayAddress = GetArrayAddress(binDest, form) ;

  // Finally, make the replacement
  ArrayReferenceExpression* replacement = 
    create_array_reference_expression(theEnv,
				      elementType,
				      arrayAddress,
				      combined) ;
  binDest->set_parent(NULL) ;
  top->set_destination_address(NULL) ;
  top->set_destination_address(replacement) ;
  return true ;
  
}

bool PointerConversionPass::ReplaceLoad(LoadExpression* top)
{
  BinaryExpression* topBin = 
    dynamic_cast<BinaryExpression*>(top->get_source_address()) ;
  NonLvalueExpression* topNonL = 
    dynamic_cast<NonLvalueExpression*>(top->get_source_address()) ;
  if (topNonL != NULL)
  {
    // This is [0] and must be adjusted accordingly

    // This is where the problem is...
    Expression* arrayAddress = GetArrayAddress(topNonL) ;
    IntConstant* zeroOffset = create_int_constant(theEnv, 
						  GetBaseInt(theEnv),
						  IInteger(0)) ;
    ArrayReferenceExpression* replacement = 
      create_array_reference_expression(theEnv,
					topNonL->get_result_type(),
					arrayAddress,
					zeroOffset) ;
    top->set_source_address(NULL) ;
    top->set_source_address(replacement) ;  
    return true ;    
  }

  if (topBin == NULL)
  {
    return false ;
  }

  int form = FormInternal(topBin) ;

  while (form == 0 &&
	 dynamic_cast<BinaryExpression*>(topBin->get_source1()) != NULL)
  {
    topBin = dynamic_cast<BinaryExpression*>(topBin->get_source1()) ;
    form = FormInternal(topBin) ;
  }

  if (form == 0)
  {
    return false ;
  }
    
  IntConstant* offset = 
    GetOffset(dynamic_cast<BinaryExpression*>(top->get_source_address()), 
	      form) ;

  Expression* index = 
    GetIndex(dynamic_cast<BinaryExpression*>(top->get_source_address()), form);

  Expression* combined ;

  if (index != NULL)
  {
    combined = CombinedOffset(offset, index, topBin->get_opcode()) ;
  }
  else
  {
    combined = offset ;
  }

  // Now, determine the element type
  DataType* elementType = 
    GetElementType(dynamic_cast<BinaryExpression*>(top->get_source_address()),
		   form);
 
  // Next, get the address of the array
  Expression* arrayAddress = 
    GetArrayAddress(dynamic_cast<BinaryExpression*>(top->get_source_address()),
		    form) ;

  // Finally, make the replacement
  ArrayReferenceExpression* replacement = 
    create_array_reference_expression(theEnv,
				      elementType,
				      arrayAddress,
				      combined) ;
  top->set_source_address(NULL) ;
  top->set_source_address(replacement) ;  
  return true ;
}

// Form One: A one dimensional array access with no offset
// The tricky part is that the format will be different if the type
//  is a character or integer.  This is true for all forms.
bool PointerConversionPass::FormOne(BinaryExpression* topAdd) 
{
  assert(topAdd != NULL) ;

  Expression* leftHandSide = topAdd->get_source1() ;
  Expression* rightHandSide = topAdd->get_source2() ;

  assert(leftHandSide != NULL) ;
  assert(rightHandSide != NULL) ;

  if (dynamic_cast<LoadVariableExpression*>(rightHandSide) == NULL)
  {
    return false ;
  }

  LoadExpression* leftLoad = 
    dynamic_cast<LoadExpression*>(leftHandSide) ;
  if (leftLoad != NULL &&
      dynamic_cast<ArrayReferenceExpression*>(leftLoad->get_source_address()) 
       != NULL)
  {
    return false ;
  }
  return true ;

}

bool PointerConversionPass::StoreFormOne(BinaryExpression* topAdd)
{
  assert(topAdd != NULL) ;
  Expression* leftHandSide = topAdd->get_source1() ;
  Expression* rightHandSide = topAdd->get_source2() ;

  LoadVariableExpression* leftLoadVar = 
    dynamic_cast<LoadVariableExpression*>(leftHandSide) ;
  LoadVariableExpression* rightLoadVar = 
    dynamic_cast<LoadVariableExpression*>(rightHandSide) ;
  BinaryExpression* rightMult = 
    dynamic_cast<BinaryExpression*>(rightHandSide) ;

  if (leftLoadVar != NULL && rightLoadVar != NULL)
  {
    return true ;
  }
  if (leftLoadVar != NULL && rightMult != NULL)
  {
    return true ;
  }

  return false ;
}

// Form Two: A one dimensional array access with offset
bool PointerConversionPass::FormTwo(BinaryExpression* topAdd)
{
  assert(topAdd != NULL) ;
  
  Expression* leftHandSide = topAdd->get_source1() ;
  Expression* rightHandSide = topAdd->get_source2() ;

  if (dynamic_cast<IntConstant*>(rightHandSide) == NULL)
  {
    return false ;
  }

  LoadExpression* leftLoad = dynamic_cast<LoadExpression*>(leftHandSide) ;

  if (leftLoad != NULL &&
      dynamic_cast<ArrayReferenceExpression*>(leftLoad->get_source_address()) 
        != NULL)
  {
    return false ;
  }

  BinaryExpression* leftBin = dynamic_cast<BinaryExpression*>(leftHandSide) ;
  if (leftBin != NULL &&
      dynamic_cast<LoadExpression*>(leftBin->get_source1()) != NULL)
  {
    return false ;
  }
  if (leftBin == NULL)
  {
    return false ;
  }
  return true ;
}

// A one dimensional array with offset
bool PointerConversionPass::StoreFormTwo(BinaryExpression* topAdd)
{
  assert(topAdd != NULL) ;
  Expression* leftHandSide = topAdd->get_source1() ;
  Expression* rightHandSide = topAdd->get_source2() ;
  
  BinaryExpression* leftPlus = dynamic_cast<BinaryExpression*>(leftHandSide) ;
  IntConstant* rightConst = dynamic_cast<IntConstant*>(rightHandSide) ;
  
  if (leftPlus == NULL || rightConst == NULL)
  {
    return false ;
  }

  LoadVariableExpression* bottomLoad = 
    dynamic_cast<LoadVariableExpression*>(leftPlus->get_source1()) ;
  BinaryExpression* bottomMult = 
    dynamic_cast<BinaryExpression*>(leftPlus->get_source2()) ;
  
  if (bottomLoad == NULL || bottomMult == NULL)
  {
    return false ;
  }
  return true ;
}

// Form Three: A two dimensional array access with no offset
bool PointerConversionPass::FormThree(BinaryExpression* topAdd)
{
  assert(topAdd != NULL) ;
  
  Expression* leftHandSide = topAdd->get_source1() ;
  Expression* rightHandSide = topAdd->get_source2() ;

  LoadExpression* leftLoad = 
    dynamic_cast<LoadExpression*>(leftHandSide) ;

  if (leftLoad != NULL &&
      dynamic_cast<ArrayReferenceExpression*>(leftLoad->get_source_address()) 
       != NULL)
  {
    return true ;
  }

  return false ;
}

// A two dimensional array with no offset
bool PointerConversionPass::StoreFormThree(BinaryExpression* topAdd)
{
  assert(topAdd != NULL) ;

  Expression* leftHandSide = topAdd->get_source1() ;
  Expression* rightHandSide = topAdd->get_source2() ;

  BinaryExpression* leftBin = dynamic_cast<BinaryExpression*>(leftHandSide) ;
  BinaryExpression* rightBin = dynamic_cast<BinaryExpression*>(rightHandSide) ;
  LoadVariableExpression* rightLoadVar = 
    dynamic_cast<LoadVariableExpression*>(rightHandSide) ;

  if (leftBin == NULL)
  {
    return false ;
  }
  
  if (rightBin == NULL && rightLoadVar == NULL)
  {
    return false ;
  }

  LoadVariableExpression* bottomLeft = 
    dynamic_cast<LoadVariableExpression*>(leftBin->get_source1()) ;
  BinaryExpression* bottomRight = 
    dynamic_cast<BinaryExpression*>(leftBin->get_source2()) ;
  
  if (bottomLeft == NULL || bottomRight == NULL)
  {
    return false ;
  }

  return true ;
}

// Form Four: A two dimensional array access with offset
bool PointerConversionPass::FormFour(BinaryExpression* topAdd)
{
  assert(topAdd != NULL) ;
  
  Expression* leftHandSide = topAdd->get_source1() ;
  Expression* rightHandSide = topAdd->get_source2() ;

  if (dynamic_cast<IntConstant*>(rightHandSide) == NULL)
  {
    return false ;
  }

  BinaryExpression* leftAdd = dynamic_cast<BinaryExpression*>(leftHandSide) ;
  if (leftAdd == NULL || leftAdd->get_opcode() != LString("add"))
  {
    return false ;
  }

  LoadExpression* finalLoad = 
    dynamic_cast<LoadExpression*>(leftAdd->get_source1()) ;
  if (finalLoad == NULL ||
      dynamic_cast<ArrayReferenceExpression*>(finalLoad->get_source_address()) 
      == NULL)
  {
    return false ;
  }  
  return true ;
}

bool PointerConversionPass::StoreFormFour(BinaryExpression* topAdd)
{
  assert(topAdd != NULL) ;

  Expression* leftHandSide = topAdd->get_source1() ;
  Expression* rightHandSide = topAdd->get_source2() ;

  BinaryExpression* leftPlus = dynamic_cast<BinaryExpression*>(leftHandSide) ;
  IntConstant* rightConst = dynamic_cast<IntConstant*>(rightHandSide) ;

  if (leftPlus == NULL || rightConst == NULL)
  {
    return false ;
  }

  BinaryExpression* MiddleLeftPlus = 
    dynamic_cast<BinaryExpression*>(leftPlus->get_source1()) ;
  BinaryExpression* MiddleRightPlus = 
    dynamic_cast<BinaryExpression*>(leftPlus->get_source2()) ;
  LoadVariableExpression* MiddleRightLoadVar = 
    dynamic_cast<LoadVariableExpression*>(leftPlus->get_source2());

  if (MiddleLeftPlus == NULL)
  {
    return false ;
  }

  if (MiddleRightPlus == NULL && MiddleRightLoadVar == NULL)
  {
    return false ;
  }

  BinaryExpression* bottomLeftPlus = 
    dynamic_cast<BinaryExpression*>(MiddleLeftPlus->get_source1()) ;
  IntConstant* bottomRightConst = 
    dynamic_cast<IntConstant*>(MiddleLeftPlus->get_source2()) ;

  if (bottomLeftPlus == NULL || bottomRightConst == NULL)
  {
    return false ;
  }
  
  return true ;
}

// Form Five: A one dimensional array with a constant offset (not based
//  on any array index)
bool PointerConversionPass::FormFive(BinaryExpression* topAdd)
{
  assert(topAdd != NULL) ;

  Expression* leftHandSide = topAdd->get_source1() ;
  Expression* rightHandSide = topAdd->get_source2() ;

  return (dynamic_cast<LoadVariableExpression*>(leftHandSide) != NULL) &&
         (dynamic_cast<IntConstant*>(rightHandSide) != NULL ) ;
}

bool PointerConversionPass::StoreFormFive(BinaryExpression* topAdd)
{
  return false ;
}

// Convert all load variables of pointer variables into array accesses
//  with constant offset 0
void PointerConversionPass::HandleFormSix()
{
  list<NonLvalueExpression*>* allNonLvalues = 
    collect_objects<NonLvalueExpression>(procDef->get_body()) ;
  list<NonLvalueExpression*>::iterator lvalIter = allNonLvalues->begin() ;
  while (lvalIter != allNonLvalues->end())
  {
    LoadVariableExpression* innerExp = 
      dynamic_cast<LoadVariableExpression*>((*lvalIter)->get_addressed_expression()) ;
    assert(innerExp != NULL) ;

    VariableSymbol* currentVar = innerExp->get_source() ;

    PointerType* pointType = 
      dynamic_cast<PointerType*>(currentVar->get_type()->get_base_type()) ;
    if (pointType == NULL)
    {
      return ;
    }
    assert(pointType != NULL) ;
    QualifiedType* qualElementType = 
      dynamic_cast<QualifiedType*>(pointType->get_reference_type()) ;
    assert(qualElementType != NULL) ;
    DataType* elementType = qualElementType->get_base_type() ;
    assert(elementType != NULL) ;

    IntConstant* zero = 
      create_int_constant(theEnv, 
			  GetBaseInt(theEnv),
			  IInteger(0)) ;
    assert(zero != NULL) ;


    VariableSymbol* copy = FindReplacement(currentVar) ;
    assert(copy != NULL) ;    
    
    SymbolAddressExpression* arrayAddress = 
      create_symbol_address_expression(theEnv,
				       copy->get_type()->get_base_type(),
				       copy) ;
    // Finally, make the replacement
    ArrayReferenceExpression* replacement = 
      create_array_reference_expression(theEnv,
					elementType,
					arrayAddress,
					zero) ;
    (*lvalIter)->get_parent()->replace((*lvalIter), replacement) ;
  
    ++lvalIter ;
  }
  delete allNonLvalues ;
}

IntConstant* PointerConversionPass::GetOffset(BinaryExpression* topmostAdd, 
					      int form)
{
  IntConstant* offset = NULL ;

  assert(topmostAdd != NULL) ;

  if (form == 1 || form == 3)
  {
    // No offset, just create an offset of 0 which will be eliminated in
    //  constant propagation.
    offset = create_int_constant(theEnv, 
				 GetBaseInt(theEnv),
				 IInteger(0)) ;
    assert(offset != NULL) ;
    return offset ;
  }
  else if (form == 2)
  {
    // The offset is located in the same location in both cases.
    //  We do have to determine the element type and divide by the
    //  size in order to get an integer offset.
    IntConstant* originalOffset = 
      dynamic_cast<IntConstant*>(topmostAdd->get_source2()) ;
    assert(originalOffset != NULL) ;
    int originalValue = originalOffset->get_value().c_int() ;
    
    BinaryExpression* leftAdd = 
      dynamic_cast<BinaryExpression*>(topmostAdd->get_source1()) ;
    assert(leftAdd != NULL) ;

    BinaryExpression* bottomMultiply = 
      dynamic_cast<BinaryExpression*>(leftAdd->get_source1()) ;
    
    int elementSize ;

    if (bottomMultiply != NULL)
    {
      IntConstant* value =
	dynamic_cast<IntConstant*>(bottomMultiply->get_source2()) ;
      assert(value != NULL) ;
      elementSize = value->get_value().c_int() ;
    }
    else
    {
      elementSize = 1 ;
    }
    
    offset = create_int_constant(theEnv, 
				 GetBaseInt(theEnv),
				 IInteger(originalValue/elementSize)) ;
    

  }
  else if (form == 4)
  {
    IntConstant* originalOffset = 
      dynamic_cast<IntConstant*>(topmostAdd->get_source2()) ;
    assert(originalOffset != NULL) ;
    int originalValue = originalOffset->get_value().c_int() ;

    BinaryExpression* leftAdd = 
      dynamic_cast<BinaryExpression*>(topmostAdd->get_source1()) ;
    assert(leftAdd != NULL) ;
    
    int elementSize ;
    BinaryExpression* bottomMultiply = 
      dynamic_cast<BinaryExpression*>(leftAdd->get_source2()) ;
    if (bottomMultiply != NULL)
    {
      IntConstant* value = 
	dynamic_cast<IntConstant*>(bottomMultiply->get_source2()) ;
      assert(value != NULL) ;
      elementSize = value->get_value().c_int() ;
    }
    else
    {
      elementSize = 1 ;
    }

    offset = create_int_constant(theEnv, 
				 GetBaseInt(theEnv),
				 IInteger(originalValue/elementSize)) ;    
  }
  else if (form == 5)
  {
    IntConstant* originalOffset = 
      dynamic_cast<IntConstant*>(topmostAdd->get_source2()) ;
    assert(originalOffset != NULL) ;
    int originalValue = originalOffset->get_value().c_int() ;
    int elementSize ;
    
    LoadVariableExpression* loadVar = 
      dynamic_cast<LoadVariableExpression*>(topmostAdd->get_source1()) ;
    assert(loadVar != NULL) ;
    VariableSymbol* pointerVar = loadVar->get_source() ;
    assert(pointerVar != NULL) ;
    PointerType* varType = 
      dynamic_cast<PointerType*>(pointerVar->get_type()->get_base_type()) ;
    assert(varType != NULL) ;
    QualifiedType* qualType = 
      dynamic_cast<QualifiedType*>(varType->get_reference_type()) ;
    assert(qualType != NULL) ;

    if (IsARocccType(qualType))
    {
      elementSize = 4 ;
    }
    else
    {
      elementSize = qualType->get_base_type()->get_bit_size().c_int() ;
      elementSize /= 8 ;
    }

    offset = create_int_constant(theEnv,
				 GetBaseInt(theEnv),
				 IInteger(originalValue/elementSize)) ;
  }
  else
  {
    assert(0 && "Unsupported pointer expression detected") ;
  }
  
  assert(offset != NULL) ;
  return offset ;
}

IntConstant* PointerConversionPass::GetStoreOffset(BinaryExpression* top, 
						   int form)
{
  IntConstant* offset = NULL ;
  Expression* rightHandSide = top->get_source2() ;
  
  switch(form)
  {
  case 1:
    {
      if (dynamic_cast<IntConstant*>(rightHandSide) != NULL)
      {
	int originalValue = 
	  dynamic_cast<IntConstant*>(rightHandSide)->get_value().c_int() ;

	// Determine the element size in bytes...typically it should be 4
	int elementSize ;

	Expression* leftHandSide = top->get_source1() ;
	BinaryExpression* middleAdd = 
	  dynamic_cast<BinaryExpression*>(leftHandSide) ;
	assert(middleAdd != NULL) ;
	
	Expression* leftBottom = middleAdd->get_source1() ;
	BinaryExpression* bottomMultiply = 
	  dynamic_cast<BinaryExpression*>(leftBottom) ;
	assert(bottomMultiply != NULL) ;
	
	Expression* finalRight = bottomMultiply->get_source2() ;
	IntConstant* finalConstant = dynamic_cast<IntConstant*>(finalRight) ;
	assert(finalConstant != NULL) ;
	elementSize = finalConstant->get_value().c_int() ;
	
	offset = create_int_constant(theEnv,
				     GetBaseInt(theEnv),
				     IInteger(originalValue/elementSize)) ;
      }
      else
      {
	offset = create_int_constant(theEnv, 
				     GetBaseInt(theEnv),
				     IInteger(0)) ;
      }
    }
    break ;
  case 2:
    {
    }
    break ;
  case 3:
    {
    }
    break ;
  case 4:
    {
    }
    break ;
  case 5:
    {
    }
    break ;
  default:
    {
    }
    break ;
  }

  assert(offset != NULL) ;
  return offset ;
}

Expression* PointerConversionPass::GetArrayAddress(NonLvalueExpression* top)
{
  assert(top != NULL) ;
  Expression* internalExpr = top->get_addressed_expression() ;
  LoadVariableExpression* internalLoadVar = 
    dynamic_cast<LoadVariableExpression*>(internalExpr) ;
  NonLvalueExpression* internalNonLvalue = 
    dynamic_cast<NonLvalueExpression*>(internalExpr) ;
  BinaryExpression* internalBin = 
    dynamic_cast<BinaryExpression*>(internalExpr) ;
  if (internalLoadVar != NULL)
  {
    VariableSymbol* arraySym = 
      dynamic_cast<VariableSymbol*>(internalLoadVar->get_source()) ;
      assert(arraySym != NULL) ;
      VariableSymbol* copy = FindReplacement(arraySym) ; 
      assert(copy != NULL) ; 
   
      return create_symbol_address_expression(theEnv,
					copy->get_type()->get_base_type(),
					copy) ;
  }
  else if (internalNonLvalue != NULL)
  {
    return GetArrayAddress(internalNonLvalue) ;
  }
  else if (internalBin != NULL)
  {
    GetArrayAddress(internalBin, FormInternal(internalBin)) ;
  }
  else
  {
    assert(0) ;
  }
  
}

Expression* PointerConversionPass::GetArrayAddress(BinaryExpression* top, 
						   int form)
{
  assert(top != NULL) ;

  switch (form)
  {
  case 1: // One D - No Offset
    {
      LoadVariableExpression* arrayLoad = 
	dynamic_cast<LoadVariableExpression*>(top->get_source1()) ;
      if (arrayLoad == NULL)
      {
	arrayLoad = dynamic_cast<LoadVariableExpression*>(top->get_source2()) ;
      }
      assert(arrayLoad != NULL) ;
     
      VariableSymbol* arraySym = 
	dynamic_cast<VariableSymbol*>(arrayLoad->get_source()) ;
      assert(arraySym != NULL) ;
      VariableSymbol* copy = FindReplacement(arraySym) ; 
      assert(copy != NULL) ;
    
      return create_symbol_address_expression(theEnv,
					copy->get_type()->get_base_type(),
					copy) ;
    }
    break ;
  case 2: // One D - With Offset
    {
      BinaryExpression* leftAdd = 
	dynamic_cast<BinaryExpression*>(top->get_source1()) ;
      assert(leftAdd != NULL) ;
      LoadVariableExpression* arrayLoad = 
	dynamic_cast<LoadVariableExpression*>(leftAdd->get_source1()) ;
      if (arrayLoad == NULL)
      {
	arrayLoad = 
	  dynamic_cast<LoadVariableExpression*>(leftAdd->get_source2()) ;
      }
      assert(arrayLoad != NULL) ;

      VariableSymbol* arraySym = 
	dynamic_cast<VariableSymbol*>(arrayLoad->get_source()) ;
      assert(arraySym != NULL) ;
      VariableSymbol* copy = FindReplacement(arraySym) ; 
      assert(copy != NULL) ;
    
      return create_symbol_address_expression(theEnv,
					copy->get_type()->get_base_type(),
					copy) ;      
    }
    break ;
  case 3: // Two D - No Offset
    {
      // This should be the array reference, not the load expression!
      LoadExpression* arrayLoad = 
	dynamic_cast<LoadExpression*>(top->get_source1()) ;
      assert(arrayLoad != NULL) ;
      Expression* toReturn = arrayLoad->get_source_address() ;
      toReturn->set_parent(NULL) ;
      return toReturn ;    
    }
    break ;
  case 4: // Two D - With Offset
    {
      BinaryExpression* leftAdd = 
	dynamic_cast<BinaryExpression*>(top->get_source1()) ;
      assert(leftAdd != NULL) ;
      LoadExpression* arrayLoad = 
	dynamic_cast<LoadExpression*>(leftAdd->get_source1()) ;
      assert(arrayLoad != NULL) ;
      Expression* toReturn = arrayLoad->get_source_address() ;
      toReturn->set_parent(NULL) ;
      return toReturn ;
      //      return leftAdd->get_source1() ;
    }
    break ;
  case 5: // One dimensional with constant offset (not based on index variable)
    {
      LoadVariableExpression* arrayLoad = 
	dynamic_cast<LoadVariableExpression*>(top->get_source1()) ;
      if (arrayLoad == NULL)
      {
	arrayLoad = dynamic_cast<LoadVariableExpression*>(top->get_source2()) ;
      }
      assert(arrayLoad != NULL) ;
     
      VariableSymbol* arraySym = 
	dynamic_cast<VariableSymbol*>(arrayLoad->get_source()) ;
      assert(arraySym != NULL) ;
      VariableSymbol* copy = FindReplacement(arraySym) ; 
      assert(copy != NULL) ;
    
      return create_symbol_address_expression(theEnv,
					copy->get_type()->get_base_type(),
					copy) ;
    }
    break ;
  default:
    return NULL ;
  }  
}

DataType* PointerConversionPass::GetElementType(BinaryExpression* top,
						int form)
{
  assert(top != NULL) ;

  PointerType* accessType = NULL ;
  ReferenceType* accessRefType = NULL ;

  switch (form)
  {
  case 1: // One D - No Offset
    {
      BinaryExpression* leftMultiply = 
	dynamic_cast<BinaryExpression*>(top->get_source1()) ;
      if (leftMultiply != NULL)
      {
	LoadVariableExpression* arrayLoad = 
	  dynamic_cast<LoadVariableExpression*>(top->get_source2()) ;
	assert(arrayLoad != NULL) ;
	VariableSymbol* arraySym = 
	  dynamic_cast<VariableSymbol*>(arrayLoad->get_source()) ;
	assert(arraySym != NULL) ;
	accessType =
	  dynamic_cast<PointerType*>(arraySym->get_type()->get_base_type()) ;
	accessRefType = 
	  dynamic_cast<ReferenceType*>(arraySym->get_type()->get_base_type()) ;
      }
      else
      {
	LoadVariableExpression* arrayLoad = 
	  dynamic_cast<LoadVariableExpression*>(top->get_source1()) ;	
	assert(arrayLoad != NULL) ;
	VariableSymbol* arraySym = 
	  dynamic_cast<VariableSymbol*>(arrayLoad->get_source()) ;
	assert(arraySym != NULL) ;
	accessType = 
	  dynamic_cast<PointerType*>(arraySym->get_type()->get_base_type()) ;
	accessRefType = 
	  dynamic_cast<ReferenceType*>(arraySym->get_type()->get_base_type()) ;
      }
    }
    break ;
  case 2: // One D - With Offset
    {
      BinaryExpression* leftAdd = 
	dynamic_cast<BinaryExpression*>(top->get_source1()) ;
      assert(leftAdd) ;
      BinaryExpression* bottomMultiply = 
	dynamic_cast<BinaryExpression*>(leftAdd->get_source1()) ;
      if (bottomMultiply != NULL)
      {
	LoadVariableExpression* arrayLoad = 
	  dynamic_cast<LoadVariableExpression*>(leftAdd->get_source2()) ;
	assert(arrayLoad != NULL) ;
	VariableSymbol* arraySym = 
	  dynamic_cast<VariableSymbol*>(arrayLoad->get_source()) ;
	assert(arraySym != NULL) ;
	accessType = 
	  dynamic_cast<PointerType*>(arraySym->get_type()->get_base_type()) ;
	accessRefType = 
	  dynamic_cast<ReferenceType*>(arraySym->get_type()->get_base_type()) ;
      }
      else
      {
	LoadVariableExpression* arrayLoad = 
	  dynamic_cast<LoadVariableExpression*>(leftAdd->get_source1()) ;
	assert(arrayLoad != NULL) ;
	VariableSymbol* arraySym =
	  dynamic_cast<VariableSymbol*>(arrayLoad->get_source()) ;	
	assert(arraySym != NULL) ;
	accessType = 
	  dynamic_cast<PointerType*>(arraySym->get_type()->get_base_type()) ;
	accessRefType = 
	  dynamic_cast<ReferenceType*>(arraySym->get_type()->get_base_type()) ;
      }
    }
    break ;
  case 3: // Two D - No Offset
    {
      LoadExpression* arrayLoad = 
	dynamic_cast<LoadExpression*>(top->get_source1()) ;
      assert(arrayLoad != NULL) ;
      ArrayReferenceExpression* oneDRef = 
      dynamic_cast<ArrayReferenceExpression*>(arrayLoad->get_source_address());
      assert(oneDRef != NULL) ;
      DataType* returnType = 
	dynamic_cast<DataType*>(oneDRef->get_result_type()) ;
      assert(returnType != NULL) ;
      return returnType ;
    }
    break ;
  case 4: // Two D - With Offset
    {
      BinaryExpression* leftAdd = 
	dynamic_cast<BinaryExpression*>(top->get_source1()) ;
      assert(leftAdd != NULL) ;
      LoadExpression* arrayLoad = 
	dynamic_cast<LoadExpression*>(leftAdd->get_source1()) ;
      assert(arrayLoad != NULL) ;
      ArrayReferenceExpression* oneDRef = 
	dynamic_cast<ArrayReferenceExpression*>(arrayLoad->get_source_address()) ;
      assert(oneDRef != NULL) ;
      DataType* returnType = 
	dynamic_cast<DataType*>(oneDRef->get_result_type()) ;
      assert(returnType != NULL) ;
      return returnType ;
    }
    break ;
  case 5: // One D -- Constant offset
    {
      LoadVariableExpression* arrayLoad = 
	dynamic_cast<LoadVariableExpression*>(top->get_source1()) ;
      if (arrayLoad == NULL)
      {
	arrayLoad = dynamic_cast<LoadVariableExpression*>(top->get_source2()) ;
      }
      assert(arrayLoad != NULL) ;
     
      VariableSymbol* arraySym = 
	dynamic_cast<VariableSymbol*>(arrayLoad->get_source()) ;
      assert(arraySym != NULL) ;
      return arraySym->get_type()->get_base_type() ;
    }
    break ;
  default:
    return NULL ;
  }

  if (accessRefType != NULL) 
  {
    QualifiedType* qualType = 
      dynamic_cast<QualifiedType*>(accessRefType->get_reference_type()) ;
    assert(qualType != NULL) ;
    accessType = 
      dynamic_cast<PointerType*>(qualType->get_base_type()) ;
  }
  assert(accessType != NULL) ;
  Type* refType ;
  while (accessType != NULL)
  {
    refType = accessType->get_reference_type() ;
    QualifiedType* qualType = 
      dynamic_cast<QualifiedType*>(accessType->get_reference_type()) ;
    assert(qualType != NULL) ;
    accessType = dynamic_cast<PointerType*>(qualType->get_base_type()) ;
  }
  
  if (dynamic_cast<QualifiedType*>(refType) != NULL)
  {
    return dynamic_cast<QualifiedType*>(refType)->get_base_type() ;
  }
  else if (dynamic_cast<DataType*>(refType))
  {
    return dynamic_cast<DataType*>(refType) ;
  }
  else
  {
    assert(0 && "What is this?") ;
    return NULL ;
  }    
}

DataType* PointerConversionPass::GetStoreElementType(BinaryExpression* top,
						     int form)
{
  assert(top != NULL) ;

  PointerType* accessType = NULL ;
  ReferenceType* accessRefType = NULL ;

  switch (form)
  {
  case 1:
    {
      Expression* leftHandSide = top->get_source1() ;
      LoadVariableExpression* leftLoad = 
	dynamic_cast<LoadVariableExpression*>(leftHandSide) ;
      assert(leftLoad != NULL) ;
      VariableSymbol* var = leftLoad->get_source() ;
      accessRefType = 
	dynamic_cast<ReferenceType*>(var->get_type()->get_base_type()) ;
      assert(accessRefType != NULL) ;
      accessType = 
	dynamic_cast<PointerType*>(accessRefType->get_reference_type()) ;
      assert(accessType != NULL) ;
      QualifiedType* elementQualType = 
	dynamic_cast<QualifiedType*>(accessType->get_reference_type()) ;
      assert(elementQualType != NULL) ;
      return elementQualType->get_base_type() ;
    }
    break ;
  case 2:
    break ;
  case 3:
    break ;
  case 4:
    break ;
  case 5:
    break ;
  default:
    {
    }
    break ;
  }
  
}

Expression* PointerConversionPass::GetIndex(BinaryExpression* top, int form)
{
  assert(top != NULL) ;

  Expression* found = NULL ;

  switch (form)
  {
  case 1: // One D - No Offset
    {
      BinaryExpression* leftMultiply = 
	dynamic_cast<BinaryExpression*>(top->get_source1()) ;
      if (leftMultiply != NULL)
      {
	found = leftMultiply->get_source1() ;
      }
      else
      {
	// For pointers to characters, the variables are switched
	found = top->get_source2() ;
      }
    }
    break ;
  case 2: // One D - With Offset
    {
      BinaryExpression* leftAdd = 
	dynamic_cast<BinaryExpression*>(top->get_source1()) ;
      assert(leftAdd != NULL) ;
      BinaryExpression* bottomMultiply = 
	dynamic_cast<BinaryExpression*>(leftAdd->get_source1()) ;
      if (bottomMultiply != NULL)
      {
	found = bottomMultiply->get_source1() ;
      }
      else
      {
	found = leftAdd->get_source2() ;
      }
    }
    break ;
  case 3: // Two D - No Offset
    {
      BinaryExpression* rightMultiply = 
	dynamic_cast<BinaryExpression*>(top->get_source2()) ;
      if (rightMultiply != NULL)
      {
	found = rightMultiply->get_source1() ;
      }
      else
      {
	found = top->get_source2() ;
      }
    }
    break ;
  case 4: // Two D - With Offset
    {
      BinaryExpression* leftAdd = 
	dynamic_cast<BinaryExpression*>(top->get_source1()) ;
      assert(leftAdd != NULL) ;
      BinaryExpression* bottomMultiply =
	dynamic_cast<BinaryExpression*>(leftAdd->get_source2()) ;
      if (bottomMultiply != NULL)
      {
	found = bottomMultiply->get_source1() ;
      }
      else
      {
	found = leftAdd->get_source2() ;
      }
    }
    break ;
  default: // No index
    {
    return NULL ;
    }
  }

  assert(found != NULL) ;
  found->set_parent(NULL) ;
  return found ;
}

Expression* PointerConversionPass::GetStoreIndex(BinaryExpression* top,
						 int form)
{
    assert(top != NULL) ;

  Expression* found = NULL ;

  switch (form)
  {
  case 1: // One D - No Offset
    {
      BinaryExpression* rightMultiply = 
	dynamic_cast<BinaryExpression*>(top->get_source2()) ;
      assert(rightMultiply != NULL) ;
      found = rightMultiply->get_source1() ;
    }
    break ;
  case 2: // One D - With Offset
    {
      BinaryExpression* leftPlus = 
	dynamic_cast<BinaryExpression*>(top->get_source1()) ;
      assert(leftPlus != NULL) ;
      BinaryExpression* bottomMult = 
	dynamic_cast<BinaryExpression*>(leftPlus->get_source2()) ;
      assert(bottomMult != NULL) ;
      found = bottomMult->get_source1() ;
    }
    break ;
  case 3: // Two D - No Offset
    {
      return NULL ;
    }
    break ;
  case 4: // Two D - With Offset
    {
      return NULL ;
    }
    break ;
  default: // No index
    {
    return NULL ;
    }
  }

  assert(found != NULL) ;
  found->set_parent(NULL) ;
  return found ;
}

Expression* PointerConversionPass::CombinedOffset(IntConstant* offset,
						  Expression* index,
						  LString opcode)
{
  return create_binary_expression(theEnv,
				  index->get_result_type(),
				  opcode,
				  index,
				  offset) ;
}


VariableSymbol* PointerConversionPass::FindReplacement(VariableSymbol* original) 
{
  assert(procDef != NULL) ;
  assert(original != NULL) ;

  if (dynamic_cast<ParameterSymbol*>(original) == NULL)
  {
    OutputError("Cannot declare a local variable to be a pointer!") ;
    assert(0) ;
  }

  if (replacementMap[original] == NULL)
  {
    // Create a copy
    VariableSymbol* replacement = 
      dynamic_cast<VariableSymbol*>(original->deep_clone()) ;
    assert(replacement != NULL) ;

    DataType* originalType = original->get_type()->get_base_type() ;
    PointerType* originalPointer = dynamic_cast<PointerType*>(originalType) ;
    if (originalPointer == NULL)
    {
      ReferenceType* originalRef = dynamic_cast<ReferenceType*>(originalType) ;
      assert(originalRef != NULL) ;
      QualifiedType* qualType = 
	dynamic_cast<QualifiedType*>(originalRef->get_reference_type()) ;
      assert(qualType != NULL) ;
      originalPointer = dynamic_cast<PointerType*>(qualType->get_base_type()) ;
    }
    assert(originalPointer != NULL) ;

    ArrayType* replacementType = ConvertType(originalPointer) ;
    replacement->set_type(create_qualified_type(theEnv, replacementType)) ;
    
    // Append an annotation with it's original name
    BrickAnnote* originalNameAnnote = create_brick_annote(theEnv,
							  "OriginalName") ;
    assert(originalNameAnnote != NULL) ;
    StringBrick* originalNameBrick = 
      create_string_brick(theEnv, replacement->get_name());
    assert(originalNameBrick != NULL) ;
    originalNameAnnote->append_brick(originalNameBrick) ;
    replacement->append_annote(originalNameAnnote) ;

    LString replacementName  = original->get_name() ;
    //    replacementName = replacementName + LString("_parameter") ;
    //    replacement->set_name(TempName(replacementName)) ;
    replacement->set_name(replacementName) ;

    procDef->get_symbol_table()->append_symbol_table_object(replacement) ;
    replacementMap[original] = replacement ;

    return replacement ;
  }
  else
  {
    return replacementMap[original] ;
  }
}

ArrayType* PointerConversionPass::ConvertType(PointerType* originalPointer)
{
  assert(originalPointer != NULL) ;

    //  I'm just going to create a lower bound of 0 and an upper bound
    //  of 100 because it doesn't really matter to me what the bounds
    //  on this temporary array are.  We use other information in order
    //  to determine this.

  DataType* baseInt = GetBaseInt(theEnv) ;

  IntConstant* lowerBound = create_int_constant(theEnv, 
						baseInt,
						IInteger(0)) ;
  IntConstant* upperBound = create_int_constant(theEnv,
						baseInt,
						IInteger(100)) ; 
  QualifiedType* elementType = 
    dynamic_cast<QualifiedType*>(originalPointer->get_reference_type()) ;
  assert(elementType != NULL) ;

  if (dynamic_cast<PointerType*>(elementType->get_base_type()) != NULL)
  {
    PointerType* elementPointer = 
      dynamic_cast<PointerType*>(elementType->get_base_type()) ;
    elementType = create_qualified_type(theEnv, ConvertType(elementPointer)) ;
  }

  ArrayType* replacementType = 
    create_array_type(theEnv,
		      elementType->get_base_type()->get_bit_size().c_int(), // bit size
		      0, // bit alignment
		      elementType, // Qualified element type
		      lowerBound,
		      upperBound,
		      TempName(LString("ArrayType"))
		      ) ;
  return replacementType ;
}

int PointerConversionPass::Form(LoadExpression* top)
{
  BinaryExpression* topAdd = 
    dynamic_cast<BinaryExpression*>(top->get_source_address()) ;
  if (topAdd == NULL || topAdd->get_opcode() != LString("add"))
  {
    return 0 ;
  }
  
  return FormInternal(topAdd) ;
}

int PointerConversionPass::StoreFormInternal(BinaryExpression* topAdd)
{
  if (StoreFormOne(topAdd))
  {
    return 1 ;
  }
  if (StoreFormTwo(topAdd)) 
  {
    return 2 ;
  }
  if (StoreFormThree(topAdd))
  {
    return 3 ;
  }
  if (StoreFormFour(topAdd))
  {
    return 4 ;
  }
  if (StoreFormFive(topAdd))
  {
    return 5 ;
  }
  return 0 ;
}

int PointerConversionPass::FormInternal(BinaryExpression* topAdd)
{
  if (FormOne(topAdd))
  {
    return 1 ;
  }
  if (FormTwo(topAdd))
  {
    return 2 ;
  }
  if (FormThree(topAdd))
  {
    return 3 ;
  }
  if (FormFour(topAdd))
  {
    return 4 ;
  }
  if (FormFive(topAdd))
  {
    return 5 ;
  }
  return 0 ;
}
