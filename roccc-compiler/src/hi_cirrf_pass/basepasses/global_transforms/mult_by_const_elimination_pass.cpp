
#include <cassert>

#include <basicnodes/basic.h>
#include <basicnodes/basic_factory.h>

#include <suifnodes/suif.h>
#include <suifnodes/suif_factory.h>

#include <suifkernel/utilities.h>

#include "mult_by_const_elimination_pass.h"
#include "roccc_utils/warning_utils.h"

MultiplyByConstEliminationPass2::MultiplyByConstEliminationPass2(SuifEnv* pEnv)
  : PipelinablePass(pEnv, "MultiplyByConstEliminationPass")
{
  theEnv = pEnv ;
  procDef = NULL ;
}

void MultiplyByConstEliminationPass2::do_procedure_definition(ProcedureDefinition* p)
{
  procDef = p ;
  assert(procDef != NULL) ;
  OutputInformation("Multiply by const elimination pass 2 begins") ;
  list<BinaryExpression*>* allBin = 
    collect_objects<BinaryExpression>(procDef->get_body()) ;
  list<BinaryExpression*>::iterator binIter = allBin->begin() ;
  while (binIter != allBin->end())
  {
    ProcessBinaryExpression(*binIter) ;
    ++binIter ;
  }
  OutputInformation("Multiply by const elimination pass 2 ends") ;
}

void MultiplyByConstEliminationPass2::ProcessBinaryExpression(BinaryExpression* b)
{
  assert(b != NULL) ;
  if (b->get_opcode() != LString("multiply"))
  {
    return ;
  }
  Expression* leftSide = b->get_source1() ;
  Expression* rightSide = b->get_source2() ;
  assert(leftSide != NULL) ;
  assert(rightSide != NULL) ;

  IntConstant* leftConstant = dynamic_cast<IntConstant*>(leftSide) ;
  IntConstant* rightConstant = dynamic_cast<IntConstant*>(rightSide) ;

  IntConstant* originalConstant = NULL ;
  Expression* nonConstantSrc = NULL ;
  
  if (leftConstant != NULL && rightConstant == NULL)
  {
    // Verify that the right side is an integer type
    DataType* rightType = rightSide->get_result_type() ;
    IntegerType* rightIntType = dynamic_cast<IntegerType*>(rightType) ;
    if (rightIntType == NULL)
    {
      return ;
    }
    originalConstant = leftConstant ;
    nonConstantSrc = rightSide ;
  }
  else if (leftConstant == NULL && rightConstant != NULL)
  {
    // Verify that the left side is an integer type
    DataType* leftType = leftSide->get_result_type() ;
    IntegerType* leftIntType = dynamic_cast<IntegerType*>(leftType) ;
    if (leftIntType == NULL)
    {
      return ;
    }
    originalConstant = rightConstant ;
    nonConstantSrc = leftSide ;
  }
  else
  {
    // No replacement to perform
    return ;
  }

  assert(originalConstant != NULL) ;
  assert(nonConstantSrc != NULL) ;

  int constantValue = originalConstant->get_value().c_int() ;
  if (constantValue == 0)
  {
    // Cannot replace multiply of 0 with any shifts
    return ;
  }
  int sign ;
  if (constantValue < 0)
  {
    sign = -1 ;
  }
  else
  {
    sign = 1 ;
  }

  // Now, perform the actual replacement
  list<int> powerSeries = ComputePowerSeries(constantValue * sign) ;

  // Create a replacement expression that consists of adding all of the
  //  appropriate shifts together.

  Expression* replacement = NULL ;  

  list<int>::iterator powerIter = powerSeries.begin() ;
  while (powerIter != powerSeries.end())
  {
    IntConstant* shiftAmount = 
      create_int_constant(theEnv,
			  originalConstant->get_result_type(),
			  IInteger(*powerIter)) ;
    BinaryExpression* nextShift = 
      create_binary_expression(theEnv,
			       b->get_result_type(),
			       LString("left_shift"),
		       dynamic_cast<Expression*>(nonConstantSrc->deep_clone()),
			       shiftAmount) ;
    if (replacement == NULL)
    {
      replacement = nextShift ;
    }
    else
    {
      BinaryExpression* addAllShifts = 
	create_binary_expression(theEnv,
				 b->get_result_type(),
				 LString("add"),
				 replacement,
				 nextShift) ;
      replacement = addAllShifts ;
    }
    ++powerIter ;
  }

  assert(replacement != NULL) ;

  if (sign == -1)
  {
    replacement = 
      create_unary_expression(theEnv, 
			      b->get_result_type(),
			      LString("negate"),
			      replacement) ;
  }

  b->get_parent()->replace(b, replacement) ;
    
}

list<int> MultiplyByConstEliminationPass2::ComputePowerSeries(int n)
{
  list<int> toReturn ;
  
  // Break n into the sum of all of its powers
  int power = 0 ;
  while (power < 32)
  {
    if ( ((n >> power) & 0x1) != 0)
    {
      toReturn.push_front(power) ;
    }
    ++power ;
  }
  return toReturn ;
 
}
