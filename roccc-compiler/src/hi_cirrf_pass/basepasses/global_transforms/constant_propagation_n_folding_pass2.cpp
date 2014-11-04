// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

#include <cassert>
#include <sstream>

#include <suifkernel/utilities.h>
#include <basicnodes/basic_factory.h>
#include <suifnodes/suif_factory.h>

#include "roccc_utils/warning_utils.h"
#include "roccc_utils/roccc2.0_utils.h"
#include "constant_propagation_n_folding_pass2.h"


ConstantPropagationAndFoldingPass2::ConstantPropagationAndFoldingPass2(SuifEnv* pEnv) :
  PipelinablePass(pEnv, "ConstantPropagationAndFoldingPass2") 
{
  theEnv = pEnv ;
  procDef = NULL ;
}

void ConstantPropagationAndFoldingPass2::do_procedure_definition(ProcedureDefinition* proc_def)
{
  OutputInformation("Constant propagation and folding pass 2.0 begins") ;
  procDef = proc_def ;
  assert(procDef != NULL) ;

  // This pass will continually work as long as some change has occurred
  bool change = false ;

  do
  {
    change = false ;
    change |= PropagateConstants() ;
    change |= FoldUnaryExpressions() ;
    change |= FoldBinaryExpressions() ;
    change |= FoldBoolSelects() ;
  } while (change) ;

  OutputInformation("Constant propagation and folding pass 2.0 ends") ;
}

bool ConstantPropagationAndFoldingPass2::FoldUnaryExpressions()
{
  assert(procDef != NULL) ;
  
  StatementList* innermostList = InnermostList(procDef) ;
  assert(innermostList != NULL) ;

  bool changed = false ;

  list<UnaryExpression*>* allUnaryExpressions = 
    collect_objects<UnaryExpression>(innermostList) ;

  assert(allUnaryExpressions != NULL) ;
  
  list<UnaryExpression*>::iterator unaryIter = allUnaryExpressions->begin() ;
  while (unaryIter != allUnaryExpressions->end())
  {
    Expression* source = (*unaryIter)->get_source() ;
    String opcode = String((*unaryIter)->get_opcode()) ;
    if (dynamic_cast<IntConstant*>(source) != NULL)
    {
      int oldValue = dynamic_cast<IntConstant*>(source)->get_value().c_int() ;
      int newValue ;

      // Perform the operation now instead of at run time
      if (opcode == String("negate"))
      {
	newValue = -oldValue ;
      }
      else if (opcode == String("invert"))
      {
	// Invert appears to be the same as bitwise not
	newValue =  ~oldValue ;
      }
      else if (opcode == String("bitwise_not"))
      {
	newValue = ~oldValue ;
      }
      else if (opcode == String("logical_not"))
      {
	newValue = !oldValue ;
      }
      else if (opcode == String("convert")) 
      {
	newValue = oldValue ;
      }
      else
      {
	assert(0 && "Unknown unary expression!") ;
      }

      Expression* replacement = 
	create_int_constant(theEnv,
			    (*unaryIter)->get_result_type(),
			    IInteger(newValue)) ;
      (*unaryIter)->get_parent()->replace((*unaryIter), replacement) ;
      changed = true ;
      // Set address replacement?
      // Delete the unary expression?
      //delete (*unaryIter) ;
    }
    if (dynamic_cast<FloatConstant*>(source) != NULL)
    {
      // Floats are represented as strings.  So I've written a couple of
      //  functions to translate them.
      float oldValue = 
	StringToFloat(dynamic_cast<FloatConstant*>(source)->get_value()) ;

      float newValue ;
      if (opcode == String("negate"))
      {
	newValue = -oldValue ;
      }
      else if (opcode == String("invert"))
      {
	// Invert is the same as bitwise not...which doesn't work on floats!
	assert(0 && "Cannot bitwise not a floating point constant!") ;
      }
      else if (opcode == String("bitwise_not"))
      {
	// Bitwise not does not work on floating point values...
	//	newValue = ~oldValue ;
	assert(0 && "Cannot bitwise not a floating point constant!") ;
      }
      else if (opcode == String("logical_not"))
      {
	newValue = !oldValue ;
      }      
      Expression* replacement = 
	create_float_constant(theEnv,
			      (*unaryIter)->get_result_type(),
			      FloatToString(newValue)) ;
      (*unaryIter)->get_parent()->replace((*unaryIter), replacement) ;
      changed = true ;
      // Set address replacement?
      // Delete the unary expression?
      //delete (*unaryIter) ;
    }
    ++unaryIter ;
  }
  delete allUnaryExpressions ;

  return changed ;
}

bool ConstantPropagationAndFoldingPass2::FoldBinaryExpressions()
{
  assert(procDef != NULL) ;
  bool changed = false ;
  StatementList* innermostList = InnermostList(procDef) ;

  list<BinaryExpression*>* allBinaryExpressions = 
    collect_objects<BinaryExpression>(innermostList) ;
  
  list<BinaryExpression*>::iterator binIter = allBinaryExpressions->begin() ;
  while (binIter != allBinaryExpressions->end())
  {
    changed |= FlipNegativeConstants(*binIter) ;
    changed |= SimpleFoldBinaryExpression(*binIter) ;
    changed |= ComplexFoldBinaryExpression(*binIter) ;
    ++binIter ;
  }

  delete allBinaryExpressions ;
  return changed ;
}

bool ConstantPropagationAndFoldingPass2::FoldBoolSelects()
{
  assert(procDef != NULL) ;
  bool changed = false ;
  StatementList* innermostList = InnermostList(procDef) ;
  assert(innermostList != NULL) ;
  list<CallStatement*>* allCalls = 
    collect_objects<CallStatement>(innermostList) ;
  list<CallStatement*>::iterator callIter = allCalls->begin() ;
  while (callIter != allCalls->end())
  {
    if (IsBoolSel(*callIter))
    {
      changed |= FoldBoolSelect(*callIter) ;
    }
    ++callIter ;
  }
  delete allCalls ;
  return changed ;
}

bool ConstantPropagationAndFoldingPass2::FoldBoolSelect(CallStatement* c)
{
  // Check to see if the final argument is a constant
  assert(c->get_argument_count() == 3) ;

  Expression* thirdArgument = c->get_argument(2) ;
  IntConstant* thirdInt = dynamic_cast<IntConstant*>(thirdArgument) ;

  if (thirdInt != NULL)
  {
    int value = thirdInt->get_value().c_int() ;
    Expression* propagatedValue = NULL ;      
    if (value == 0)
    {
      propagatedValue = c->get_argument(1) ;
    }
    else
    {
      propagatedValue = c->get_argument(0) ;
    }
    assert(propagatedValue != NULL) ;
    propagatedValue->set_parent(NULL) ;

    StoreVariableStatement* replacement =
      create_store_variable_statement(theEnv,
				      c->get_destination(),
				      propagatedValue) ;
    c->get_parent()->replace(c, replacement) ;
    return true ;
  }
  return false ;
}

// Turn code like X + -1 into X - 1
//  Also, change any instance of "X - 0" into "X + 0" 
bool ConstantPropagationAndFoldingPass2::FlipNegativeConstants(BinaryExpression* b)
{
  assert(b != NULL) ;
  UnaryExpression* rightUnary = 
    dynamic_cast<UnaryExpression*>(b->get_source2()) ;
  IntConstant* rightIntConst =
    dynamic_cast<IntConstant*>(b->get_source2()) ;
  FloatConstant* rightFloatConst =
    dynamic_cast<FloatConstant*>(b->get_source2()) ;
  if (rightUnary != NULL)
  {
    if (rightUnary->get_opcode() == LString("negate"))
    {
      bool flipped = FlipSign(b) ;
      if (!flipped)
      {
	return false ;
      }
      Expression* internal = rightUnary->get_source() ;
      assert(internal != NULL) ;
      internal->set_parent(NULL) ;
      b->set_source2(internal) ;
      // Will this delete be the death of me?
      //delete rightUnary ;
      return true ;
    }
  }
  if (rightIntConst != NULL)
  {
    if (rightIntConst->get_value().is_negative())
    {
      bool flipped = FlipSign(b) ;
      if (flipped)
      {
	rightIntConst->set_value(rightIntConst->get_value().negate()) ;
	return true ;
      }
    }
    /*
    int value = rightIntConst->get_value().c_int() ;
    if (value < 0)
    {
      bool flipped = FlipSign(b) ;
      if (flipped)
      {
	rightIntConst->set_value(IInteger(-value)) ;
	return true ;
      }
    }
    */
    if (rightIntConst->get_value() == IInteger(0))
    {
      if (b->get_opcode() == LString("subtract"))
      {
	b->set_opcode(LString("add")) ;
	return true ;
      }
    }
    /*
    if (value == 0)
    {
      if (b->get_opcode() == LString("subtract"))
      {
	b->set_opcode(LString("add")) ;
	return true ;
      }
    }
    */
  }
  if (rightFloatConst != NULL)
  {
    float value = StringToFloat(rightFloatConst->get_value()) ;
    if (value < 0)
    {
      bool flipped = FlipSign(b) ;
      if (flipped)
      {
	rightFloatConst->set_value(FloatToString(-value)) ;
	return true ;
      }
    }
  }
  return false ;
}

bool ConstantPropagationAndFoldingPass2::FlipSign(BinaryExpression* b)
{
  LString op = b->get_opcode() ;
  if (op == LString("add"))
  {
    b->set_opcode(LString("subtract")) ;
    return true ;
  }
  else if (op == LString("subtract"))
  {
    b->set_opcode(LString("add")) ;
    return true ;
  }
  return false ;
}

bool ConstantPropagationAndFoldingPass2::SimpleFoldBinaryExpression(BinaryExpression* b) 
{
  assert(b != NULL) ;
  String opcode = b->get_opcode() ;
  Expression* leftHandSide  = b->get_source1() ;
  Expression* rightHandSide = b->get_source2() ;

  assert(leftHandSide != NULL) ;
  assert(rightHandSide != NULL) ;

  Expression* replacement = Fold(leftHandSide, rightHandSide, opcode) ;
  if (replacement != NULL)
  {
    if (b->get_parent() != NULL)
    {
      b->get_parent()->replace(b, replacement) ; 
      return true ;
    }
    else
    {
      delete replacement ;
      return false ;
    }
  }
  return false ;
}

bool ConstantPropagationAndFoldingPass2::ComplexFoldBinaryExpression(BinaryExpression* b)
{
  Constant* leftConstant = dynamic_cast<Constant*>(b->get_source1()) ;
  Constant* rightConstant = dynamic_cast<Constant*>(b->get_source2()) ;
  BinaryExpression* leftBin = 
    dynamic_cast<BinaryExpression*>(b->get_source1()) ;
  BinaryExpression* rightBin =
    dynamic_cast<BinaryExpression*>(b->get_source2()) ;
    
  if (leftConstant != NULL && rightBin != NULL)
  {
    Expression* replacement = Splay(b, rightBin) ;
    if (replacement != NULL)
    {
      assert(replacement == b) ;
      return true ;
    }
  }
  if (rightConstant != NULL && leftBin != NULL)
  {
    Expression* replacement = Splay(b, leftBin) ;
    if (replacement != NULL)
    {
      assert(replacement == b) ;
      return true ;
    }
  }  

  return false ;
}

// This function determines which function it actually calls.
Expression* ConstantPropagationAndFoldingPass2::Fold(Expression* x,
						     Expression* y,
						     LString opcode)
{
  Constant* leftConstant = dynamic_cast<Constant*>(x) ;
  Constant* rightConstant = dynamic_cast<Constant*>(y) ;

  // If both are constants, then merge them
  if (leftConstant != NULL && rightConstant != NULL)
  {
    return Merge(leftConstant, rightConstant, opcode) ;
  }
  
  // Some of the identities are reflexive, but others aren't.
  if (leftConstant != NULL && rightConstant == NULL)
  {
    return IdentityFirst(leftConstant, y, opcode) ;
  }
  
  if (leftConstant == NULL && rightConstant != NULL)
  {
    return IdentityFirst(x, rightConstant, opcode) ;
  }

  // If neither are constants, perform the second batch of Identity
  //  operations
  if (leftConstant == NULL && rightConstant == NULL)
  {
    return IdentitySecond(x, y, opcode) ;
  }
  
  // If we somehow got here, we could not perform any folding
  return NULL ;
}

// This function performs any upscaling as necessary
//  All of the meat of the constant propagation takes place in this function.
//  Also, because we can have both 32-bit and 64-bit values, I'm going
//  to have to treat everything into 64-bit values and propagate from there.
//  PROBLEM: This does not work with double precision values
Constant* ConstantPropagationAndFoldingPass2::Merge(Constant* x,
						    Constant* y,
						    LString opcode)
{
  assert(theEnv != NULL) ;

  IntConstant* xInt = dynamic_cast<IntConstant*>(x) ;
  IntConstant* yInt = dynamic_cast<IntConstant*>(y) ;
  FloatConstant* xFloat = dynamic_cast<FloatConstant*>(x) ;
  FloatConstant* yFloat = dynamic_cast<FloatConstant*>(y) ;

  if (opcode == LString("add"))
  {
    if (xInt != NULL && yInt != NULL)
    {
      int mergedValue = xInt->get_value().c_int() + yInt->get_value().c_int() ;
      return create_int_constant(theEnv, xInt->get_result_type(),
				 IInteger(mergedValue)) ;
    }
    if (xFloat != NULL && yFloat != NULL)
    {
      float mergedValue = StringToFloat(xFloat->get_value()) + 
	                  StringToFloat(yFloat->get_value()) ;
      return create_float_constant(theEnv, xFloat->get_result_type(),
				   FloatToString(mergedValue)) ;
    }
    // If mixed...
    if (xInt != NULL && yFloat != NULL)
    {
      float mergedValue = xInt->get_value().c_int() + 
	                  StringToFloat(yFloat->get_value()) ;
      return create_float_constant(theEnv, yFloat->get_result_type(),
				   FloatToString(mergedValue)) ;
    }
    if (xFloat != NULL && yInt != NULL)
    {
      float mergedValue = StringToFloat(xFloat->get_value()) + 
	                  yInt->get_value().c_int() ;
      return create_float_constant(theEnv, xFloat->get_result_type(),
				   FloatToString(mergedValue)) ;
    }
  }

  if (opcode == LString("subtract"))
  {
    if (xInt != NULL && yInt != NULL)
    {
      int mergedValue = xInt->get_value().c_int() - 
	                      yInt->get_value().c_int() ;
      return create_int_constant(theEnv, xInt->get_result_type(),
				 IInteger(mergedValue)) ;
    }
    if (xFloat != NULL && yFloat != NULL)
    {
      float mergedValue = StringToFloat(xFloat->get_value()) - 
	                  StringToFloat(yFloat->get_value()) ;
      return create_float_constant(theEnv, xFloat->get_result_type(),
				   FloatToString(mergedValue)) ;
    }
    // If mixed...
    if (xInt != NULL && yFloat != NULL)
    {
      float mergedValue = xInt->get_value().c_int() - 
	                  StringToFloat(yFloat->get_value()) ;
      return create_float_constant(theEnv, yFloat->get_result_type(),
				   FloatToString(mergedValue)) ;
    }
    if (xFloat != NULL && yInt != NULL)
    {
      float mergedValue = StringToFloat(xFloat->get_value()) - 
	                  yInt->get_value().c_int() ;
      return create_float_constant(theEnv, xFloat->get_result_type(),
				   FloatToString(mergedValue)) ;
    }
  }

  if (opcode == LString("bitwise_and"))
  {
    if (xInt != NULL && yInt != NULL)
    {
      int mergedValue = xInt->get_value().c_int() & 
	                      yInt->get_value().c_int() ;
      return create_int_constant(theEnv, xInt->get_result_type(),
				 IInteger(mergedValue)) ;
    }
    assert(0 && "Bitwise AND only supported between integers!") ;
  }
  
  if (opcode == LString("bitwise_or"))
  {
    if (xInt != NULL && yInt != NULL)
    {
      int mergedValue = xInt->get_value().c_int() | 
	                      yInt->get_value().c_int() ;
      return create_int_constant(theEnv, xInt->get_result_type(),
				 IInteger(mergedValue)) ;
    }
    assert(0 && "Bitwise OR only supported between integers!") ;
  }

  if (opcode == LString("bitwise_xor"))
  {
    if (xInt != NULL && yInt != NULL)
    {
      int mergedValue = xInt->get_value().c_int() ^ 
	                      yInt->get_value().c_int() ;
      return create_int_constant(theEnv, xInt->get_result_type(),
				 IInteger(mergedValue)) ;
    }
    assert(0 && "Bitwise XOR only supported between integers!") ;
  }

  if (opcode == LString("multiply"))
  {
    if (xInt != NULL && yInt != NULL)
    {
      int mergedValue = xInt->get_value().c_int() * 
	                      yInt->get_value().c_int() ;
      return create_int_constant(theEnv, xInt->get_result_type(),
				 IInteger(mergedValue)) ;
    }
    if (xFloat != NULL && yFloat != NULL)
    {
      float mergedValue = StringToFloat(xFloat->get_value()) *
	                  StringToFloat(yFloat->get_value()) ;
      
      return create_float_constant(theEnv, xFloat->get_result_type(),
				   FloatToString(mergedValue)) ;
    }
    // If mixed...
    if (xInt != NULL && yFloat != NULL)
    {
      float mergedValue = xInt->get_value().c_int() * 
	                  StringToFloat(yFloat->get_value()) ;
      return create_float_constant(theEnv, yFloat->get_result_type(),
				   FloatToString(mergedValue)) ;
    }
    if (xFloat != NULL && yInt != NULL)
    {
      float mergedValue = StringToFloat(xFloat->get_value()) * 
	                  yInt->get_value().c_int() ;
      return create_float_constant(theEnv, xFloat->get_result_type(),
				   FloatToString(mergedValue)) ;
    }
    
  }

  if (opcode == LString("divide"))
  {
    if (xInt != NULL && yInt != NULL)
    {
      int mergedValue = xInt->get_value().c_int() / 
	                      yInt->get_value().c_int() ;
      return create_int_constant(theEnv, xInt->get_result_type(),
				 IInteger(mergedValue)) ;
    }
    if (xFloat != NULL && yFloat != NULL)
    {
      float mergedValue = StringToFloat(xFloat->get_value()) / 
	                  StringToFloat(yFloat->get_value()) ;
      return create_float_constant(theEnv, xFloat->get_result_type(),
				   FloatToString(mergedValue)) ;
    }
    // If mixed...
    if (xInt != NULL && yFloat != NULL)
    {
      float mergedValue = xInt->get_value().c_int() / 
	                  StringToFloat(yFloat->get_value()) ;
      return create_float_constant(theEnv, yFloat->get_result_type(),
				   FloatToString(mergedValue)) ;
    }
    if (xFloat != NULL && yInt != NULL)
    {
      float mergedValue = StringToFloat(xFloat->get_value()) / 
	                  yInt->get_value().c_int() ;
      return create_float_constant(theEnv, xFloat->get_result_type(),
				   FloatToString(mergedValue)) ;
    }
  }

  if (opcode == LString("left_shift")) 
  {
    if (xInt != NULL && yInt != NULL)
    {
      int xValue = xInt->get_value().c_int() ;
      int yValue = yInt->get_value().c_int() ;
      int mergedValue = xValue << yValue ;
      return create_int_constant(theEnv, xInt->get_result_type(), 
				 IInteger(mergedValue)) ;
    }
    assert(0 && "Left shift only works on two integer constants!") ;
  }

  if (opcode == LString("right_shift"))
  {
    if (xInt != NULL && yInt != NULL)
    {
      int xValue = xInt->get_value().c_int() ;
      int yValue = yInt->get_value().c_int() ;
      int mergedValue = xValue >> yValue ;
      return create_int_constant(theEnv, xInt->get_result_type(), 
				 IInteger(mergedValue)) ;
    }
    assert(0 && "Right shift only works on two integer constants!") ;
  }

  if (opcode == LString("is_equal_to"))
  {
    if (xInt != NULL && yInt != NULL)
    {
      int xValue = xInt->get_value().c_int() ;
      int yValue = yInt->get_value().c_int() ;
      if (xValue == yValue)
      {
	return create_int_constant(theEnv, 
				   xInt->get_result_type(), 
				   IInteger(1)) ;
      }
      else
      {
	return create_int_constant(theEnv,
				   xInt->get_result_type(),
				   IInteger(0)) ;
      }
    }
    if (xFloat != NULL && yFloat != NULL)
    {
      float xValue = StringToFloat(xFloat->get_value()) ;
      float yValue = StringToFloat(yFloat->get_value()) ;
      if (xValue == yValue)
      {
	return create_int_constant(theEnv,
				   create_integer_type(theEnv,
						       1,
						       0,
						       true),
				   IInteger(1)) ;
      }
      else
      {
	return create_int_constant(theEnv,
				   create_integer_type(theEnv,
						       1,
						       0,
						       true),
				   IInteger(0)) ;
      }
    }
    if (xInt != NULL && yFloat != NULL)
    {
      int xValue = xInt->get_value().c_int() ;
      float yValue = StringToFloat(yFloat->get_value()) ;
      if (xValue == yValue)
      {
	return create_int_constant(theEnv,
				   xInt->get_result_type(),
				   IInteger(1)) ;
      }
      else
      {
	return create_int_constant(theEnv,
				   xInt->get_result_type(),
				   IInteger(0)) ;
      }
    }
    if (xFloat != NULL && yInt != NULL)
    {
      float xValue = StringToFloat(xFloat->get_value()) ;
      int yValue = yInt->get_value().c_int() ;
      if (xValue == yValue) 
      {
	return create_int_constant(theEnv,
				   yInt->get_result_type(),
				   IInteger(1)) ;
      }
      else
      {
	return create_int_constant(theEnv,
				   yInt->get_result_type(),
				   IInteger(0)) ;
      }      
    }
    assert("Comparison between two incorrect types!") ;
  }

  if (opcode == LString("is_not_equal_to"))
  {
    if (xInt != NULL && yInt != NULL)
    {
      int xValue = xInt->get_value().c_int() ;
      int yValue = yInt->get_value().c_int() ;
      if (xValue != yValue)
      {
	return create_int_constant(theEnv, 
				   xInt->get_result_type(), 
				   IInteger(1)) ;
      }
      else
      {
	return create_int_constant(theEnv,
				   xInt->get_result_type(),
				   IInteger(0)) ;
      }
    }
    if (xFloat != NULL && yFloat != NULL)
    {
      float xValue = StringToFloat(xFloat->get_value()) ;
      float yValue = StringToFloat(yFloat->get_value()) ;
      if (xValue != yValue)
      {
	return create_int_constant(theEnv,
				   create_integer_type(theEnv,
						       1,
						       0,
						       true),
				   IInteger(1)) ;
      }
      else
      {
	return create_int_constant(theEnv,
				   create_integer_type(theEnv,
						       1,
						       0,
						       true),
				   IInteger(0)) ;
      }
    }
    if (xInt != NULL && yFloat != NULL)
    {
      int xValue = xInt->get_value().c_int() ;
      float yValue = StringToFloat(yFloat->get_value()) ;
      if (xValue != yValue)
      {
	return create_int_constant(theEnv,
				   xInt->get_result_type(),
				   IInteger(1)) ;
      }
      else
      {
	return create_int_constant(theEnv,
				   xInt->get_result_type(),
				   IInteger(0)) ;
      }
    }
    if (xFloat != NULL && yInt != NULL)
    {
      float xValue = StringToFloat(xFloat->get_value()) ;
      int yValue = yInt->get_value().c_int() ;
      if (xValue != yValue) 
      {
	return create_int_constant(theEnv,
				   yInt->get_result_type(),
				   IInteger(1)) ;
      }
      else
      {
	return create_int_constant(theEnv,
				   yInt->get_result_type(),
				   IInteger(0)) ;
      }      
    }
    assert("Comparison between two incorrect types!") ;
  }

  if (opcode == LString("is_less_than"))
  {    
    if (xInt != NULL && yInt != NULL)
    {
      int xValue = xInt->get_value().c_int() ;
      int yValue = yInt->get_value().c_int() ;
      if (xValue < yValue)
      {
	return create_int_constant(theEnv, 
				   xInt->get_result_type(), 
				   IInteger(1)) ;
      }
      else
      {
	return create_int_constant(theEnv,
				   xInt->get_result_type(),
				   IInteger(0)) ;
      }
    }
    if (xFloat != NULL && yFloat != NULL)
    {
      float xValue = StringToFloat(xFloat->get_value()) ;
      float yValue = StringToFloat(yFloat->get_value()) ;
      if (xValue < yValue)
      {
	return create_int_constant(theEnv,
				   create_integer_type(theEnv,
						       1,
						       0,
						       true),
				   IInteger(1)) ;
      }
      else
      {
	return create_int_constant(theEnv,
				   create_integer_type(theEnv,
						       1,
						       0,
						       true),
				   IInteger(0)) ;
      }
    }
    if (xInt != NULL && yFloat != NULL)
    {
      int xValue = xInt->get_value().c_int() ;
      float yValue = StringToFloat(yFloat->get_value()) ;
      if (xValue < yValue)
      {
	return create_int_constant(theEnv,
				   xInt->get_result_type(),
				   IInteger(1)) ;
      }
      else
      {
	return create_int_constant(theEnv,
				   xInt->get_result_type(),
				   IInteger(0)) ;
      }
    }
    if (xFloat != NULL && yInt != NULL)
    {
      float xValue = StringToFloat(xFloat->get_value()) ;
      int yValue = yInt->get_value().c_int() ;
      if (xValue < yValue) 
      {
	return create_int_constant(theEnv,
				   yInt->get_result_type(),
				   IInteger(1)) ;
      }
      else
      {
	return create_int_constant(theEnv,
				   yInt->get_result_type(),
				   IInteger(0)) ;
      }      
    }
    assert("Comparison between two incorrect types!") ;
  }

  if (opcode == LString("is_greater_than"))
  {
    if (xInt != NULL && yInt != NULL)
    {
      int xValue = xInt->get_value().c_int() ;
      int yValue = yInt->get_value().c_int() ;
      if (xValue > yValue)
      {
	return create_int_constant(theEnv, 
				   xInt->get_result_type(), 
				   IInteger(1)) ;
      }
      else
      {
	return create_int_constant(theEnv,
				   xInt->get_result_type(),
				   IInteger(0)) ;
      }
    }
    if (xFloat != NULL && yFloat != NULL)
    {
      float xValue = StringToFloat(xFloat->get_value()) ;
      float yValue = StringToFloat(yFloat->get_value()) ;
      if (xValue > yValue)
      {
	return create_int_constant(theEnv,
				   create_integer_type(theEnv,
						       1,
						       0,
						       true),
				   IInteger(1)) ;
      }
      else
      {
	return create_int_constant(theEnv,
				   create_integer_type(theEnv,
						       1,
						       0,
						       true),
				   IInteger(0)) ;
      }
    }
    if (xInt != NULL && yFloat != NULL)
    {
      int xValue = xInt->get_value().c_int() ;
      float yValue = StringToFloat(yFloat->get_value()) ;
      if (xValue > yValue)
      {
	return create_int_constant(theEnv,
				   xInt->get_result_type(),
				   IInteger(1)) ;
      }
      else
      {
	return create_int_constant(theEnv,
				   xInt->get_result_type(),
				   IInteger(0)) ;
      }
    }
    if (xFloat != NULL && yInt != NULL)
    {
      float xValue = StringToFloat(xFloat->get_value()) ;
      int yValue = yInt->get_value().c_int() ;
      if (xValue > yValue) 
      {
	return create_int_constant(theEnv,
				   yInt->get_result_type(),
				   IInteger(1)) ;
      }
      else
      {
	return create_int_constant(theEnv,
				   yInt->get_result_type(),
				   IInteger(0)) ;
      }      
    }
    assert("Comparison between two incorrect types!") ;
  }

  if (opcode == LString("is_less_than_or_equal_to"))
  {
    if (xInt != NULL && yInt != NULL)
    {
      int xValue = xInt->get_value().c_int() ;
      int yValue = yInt->get_value().c_int() ;
      if (xValue <= yValue)
      {
	return create_int_constant(theEnv, 
				   xInt->get_result_type(), 
				   IInteger(1)) ;
      }
      else
      {
	return create_int_constant(theEnv,
				   xInt->get_result_type(),
				   IInteger(0)) ;
      }
    }
    if (xFloat != NULL && yFloat != NULL)
    {
      float xValue = StringToFloat(xFloat->get_value()) ;
      float yValue = StringToFloat(yFloat->get_value()) ;
      if (xValue <= yValue)
      {
	return create_int_constant(theEnv,
				   create_integer_type(theEnv,
						       1,
						       0,
						       true),
				   IInteger(1)) ;
      }
      else
      {
	return create_int_constant(theEnv,
				   create_integer_type(theEnv,
						       1,
						       0,
						       true),
				   IInteger(0)) ;
      }
    }
    if (xInt != NULL && yFloat != NULL)
    {
      int xValue = xInt->get_value().c_int() ;
      float yValue = StringToFloat(yFloat->get_value()) ;
      if (xValue <= yValue)
      {
	return create_int_constant(theEnv,
				   xInt->get_result_type(),
				   IInteger(1)) ;
      }
      else
      {
	return create_int_constant(theEnv,
				   xInt->get_result_type(),
				   IInteger(0)) ;
      }
    }
    if (xFloat != NULL && yInt != NULL)
    {
      float xValue = StringToFloat(xFloat->get_value()) ;
      int yValue = yInt->get_value().c_int() ;
      if (xValue <= yValue) 
      {
	return create_int_constant(theEnv,
				   yInt->get_result_type(),
				   IInteger(1)) ;
      }
      else
      {
	return create_int_constant(theEnv,
				   yInt->get_result_type(),
				   IInteger(0)) ;
      }      
    }
    assert("Comparison between two incorrect types!") ;
  }
  if (opcode == LString("is_greater_than_or_equal_to"))
  {
    if (xInt != NULL && yInt != NULL)
    {
      int xValue = xInt->get_value().c_int() ;
      int yValue = yInt->get_value().c_int() ;
      if (xValue >= yValue)
      {
	return create_int_constant(theEnv, 
				   xInt->get_result_type(), 
				   IInteger(1)) ;
      }
      else
      {
	return create_int_constant(theEnv,
				   xInt->get_result_type(),
				   IInteger(0)) ;
      }
    }
    if (xFloat != NULL && yFloat != NULL)
    {
      float xValue = StringToFloat(xFloat->get_value()) ;
      float yValue = StringToFloat(yFloat->get_value()) ;
      if (xValue >= yValue)
      {
	return create_int_constant(theEnv,
				   create_integer_type(theEnv,
						       1,
						       0,
						       true),
				   IInteger(1)) ;
      }
      else
      {
	return create_int_constant(theEnv,
				   create_integer_type(theEnv,
						       1,
						       0,
						       true),
				   IInteger(0)) ;
      }
    }
    if (xInt != NULL && yFloat != NULL)
    {
      int xValue = xInt->get_value().c_int() ;
      float yValue = StringToFloat(yFloat->get_value()) ;
      if (xValue >= yValue)
      {
	return create_int_constant(theEnv,
				   xInt->get_result_type(),
				   IInteger(1)) ;
      }
      else
      {
	return create_int_constant(theEnv,
				   xInt->get_result_type(),
				   IInteger(0)) ;
      }
    }
    if (xFloat != NULL && yInt != NULL)
    {
      float xValue = StringToFloat(xFloat->get_value()) ;
      int yValue = yInt->get_value().c_int() ;
      if (xValue >= yValue) 
      {
	return create_int_constant(theEnv,
				   yInt->get_result_type(),
				   IInteger(1)) ;
      }
      else
      {
	return create_int_constant(theEnv,
				   yInt->get_result_type(),
				   IInteger(0)) ;
      }      
    }
    assert("Comparison between two incorrect types!") ;
  }

  assert(0 && "Unsupported binary opcode detected!") ;
  return NULL ;
}

Expression* ConstantPropagationAndFoldingPass2::IdentityFirst(Constant* x,
							      Expression* y,
							      LString opcode)
{
  assert(x != NULL) ;
  assert(y != NULL) ;
  // Identities checked in this function:
  //  1 * X = X
  //  0 * X = 0
  //  0 + X = X
  //  0 | X = X
  //  0 & X = 0

  // First, check the value of the constant
  IntConstant* xInt = dynamic_cast<IntConstant*>(x) ;
  FloatConstant* xFloat = dynamic_cast<FloatConstant*>(x) ;

  bool isZero = false ;
  bool isOne = false ;

  if (xInt == NULL && xFloat == NULL)
  {
    return NULL ;
  }

  if (xInt != NULL)
  {
    isZero = ((xInt->get_value()) == IInteger(0)) ;
    isOne  = ((xInt->get_value()) == IInteger(1)) ;
  }
  
  if (xFloat != NULL)
  {
    isZero = (StringToFloat(xFloat->get_value()) == 0) ;
    isOne  = (StringToFloat(xFloat->get_value()) == 1) ;
  }

  if (!isZero && !isOne)
  {
    // I can't do any identities
    return NULL ;
  }

  if (isZero)
  {
    if (opcode == LString("multiply") || opcode == LString("bitwise_and"))
    {
      // I either return an integer or float depending on 
      //  the type of the expression
      if (IsFloatType(y))
      {
	return create_float_constant(theEnv, y->get_result_type(),
				     FloatToString(0)) ;
      }
      else
      {
	return create_int_constant(theEnv, y->get_result_type(), IInteger(0));
      }
    }

    if (opcode == LString("add") || opcode == LString("bitwise_or"))
    {
      return dynamic_cast<Expression*>(y->deep_clone()) ;
    }
  }

  if (isOne)
  {
    if (opcode == LString("multiply"))
    {
      return dynamic_cast<Expression*>(y->deep_clone()) ;
    }
  }

  // I don't know how I got here...
  return NULL ;
}

Expression* ConstantPropagationAndFoldingPass2::IdentityFirst(Expression* x,
							      Constant* y,
							      LString opcode)
{
  // Identities checked in this function:
  //  X * 1 = X
  //  X / 1 = X
  //  X * 0 = 0
  //  X + 0 = X
  //  X | 0 = X
  //  X & 0 = 0
  //  X >> 0 = X
  //  X << 0 = X

  assert(x != NULL) ;
  assert(y != NULL) ;

  // First, check the value of the constant
  IntConstant* yInt = dynamic_cast<IntConstant*>(y) ;
  FloatConstant* yFloat = dynamic_cast<FloatConstant*>(y) ;

  bool isZero = false ;
  bool isOne = false ;

  if (yInt == NULL && yFloat == NULL)
  {
    return NULL ;
  }

  if (yInt != NULL)
  {
    isZero = ((yInt->get_value()) == IInteger(0)) ;
    isOne  = ((yInt->get_value()) == IInteger(1)) ;
  }
  
  if (yFloat != NULL)
  {
    isZero = (StringToFloat(yFloat->get_value()) == 0) ;
    isOne  = (StringToFloat(yFloat->get_value()) == 1) ;
  }

  if (!isZero && !isOne)
  {
    // I can't do any identities
    return NULL ;
  }

  if (isZero)
  {
    if (opcode == LString("multiply") || opcode == LString("bitwise_and"))
    {
      // I either return an integer or float depending on 
      //  the type of the expression
      if (IsFloatType(x))
      {
	return create_float_constant(theEnv, x->get_result_type(),
				     FloatToString(0)) ;
      }
      else
      {
	return create_int_constant(theEnv, x->get_result_type(), IInteger(0));
      }
    }

    if (opcode == LString("add") || opcode == LString("bitwise_or") ||
	opcode == LString("right_shift") || opcode == LString("left_shift"))
    {
      return dynamic_cast<Expression*>(x->deep_clone()) ;
    }
        
    assert(opcode != LString("divide") && "Cannot divide by zero!") ;
  }

  if (isOne)
  {
    if (opcode == LString("multiply") || opcode == LString("divide"))
    {
      return dynamic_cast<Expression*>(x->deep_clone()) ;
    }    
  }
  // I don't know how I got here...
  return NULL ;
}


Expression* ConstantPropagationAndFoldingPass2::IdentitySecond(Expression* x,
							       Expression* y,
							       LString opcode)
{
  // These are all handled by gcc before it even gets to me.
  /*
  // Identities checked in this function:
  //   X - X = 0 
  //   X / X = 1
  //   X & X = X
  //   X | X = X 
  //   X ^ X = 0  
  //  -X + X = 0
  //  ~X & X = 0
  //   X && !X = 0 
  //   X || !X = 1
  //   X + -X = 0
  //   X & ~X = 0
  //  !X && X = 1
  //  !X || X = 0

  */

  return NULL ;
}

bool ConstantPropagationAndFoldingPass2::IsFloatType(Expression* x)
{
  Type* resultType = x->get_result_type() ;
  return (dynamic_cast<FloatingPointType*>(resultType) != NULL) ;
}

bool ConstantPropagationAndFoldingPass2::PropagateConstants()
{
  assert(procDef != NULL) ;
  bool changed = false ;

  StatementList* innermostList = InnermostList(procDef) ;
  assert(innermostList != NULL) ;

  // Collect all of the load variable expressions.
  list<LoadVariableExpression*>* allLoadVariables =
    collect_objects<LoadVariableExpression>(innermostList) ;
  assert(allLoadVariables != NULL) ;
  
  list<LoadVariableExpression*>::iterator loadIter = 
    allLoadVariables->begin() ;
  while (loadIter != allLoadVariables->end())
  {
    // If I'm the only load variable expression in a store statement,
    //  then I was specially created and should not be propagated
    if (dynamic_cast<StoreStatement*>((*loadIter)->get_parent()) != NULL)
    {
      ++loadIter ;
      continue ;
    }

    BrickAnnote* reachingDefs = 
      to<BrickAnnote>((*loadIter)->lookup_annote_by_name("reaching_defs")) ;
    assert(reachingDefs != NULL) ;
    if (reachingDefs->get_brick_count() == 1)
    {
      SuifBrick* onlyBrick = reachingDefs->get_brick(0) ;
      assert(onlyBrick != NULL) ;
      SuifObjectBrick* sob = dynamic_cast<SuifObjectBrick*>(onlyBrick) ;
      assert(sob != NULL) ;
      
      SuifObject* obj = sob->get_object() ;
      assert(obj != NULL) ;
      // Definitions should be either a store variable statement or a call
      //  statement.
      Statement* definition = dynamic_cast<Statement*>(obj) ;
      assert(definition != NULL) ;
      if(isConstant(definition))
      {
	// This should be a store variable statement
	StoreVariableStatement* storeVariableDefinition = 
	  dynamic_cast<StoreVariableStatement*>(definition) ;
	assert(storeVariableDefinition != NULL) ;
	Expression* rightHandSide = storeVariableDefinition->get_value() ;
	Constant* propagatingConstant = dynamic_cast<Constant*>(rightHandSide);

	(*loadIter)->get_parent()->replace((*loadIter), propagatingConstant->deep_clone()) ;

	storeVariableDefinition->append_annote(create_brick_annote(theEnv,
								   "NonPrintable")) ;
								  

      }
    }

    ++loadIter ;
  }
  delete allLoadVariables ;

  return changed ;
}


float ConstantPropagationAndFoldingPass2::StringToFloat(String x) 
{
  float toReturn ;
  std::stringstream convert ;
  
  if (x == String("inf"))
  {
    unsigned int maxint = 0x7f800000 ;
    toReturn =  *((float*)(&maxint));
    return toReturn ;
  }
  if (x == String("-inf"))
  {
    unsigned int minint = 0xff800000 ;
    toReturn = *((float*)(&minint)) ;
    return toReturn ;
  }
  
  convert << x ;
  convert >> toReturn ;

  return toReturn ;
}

String ConstantPropagationAndFoldingPass2::FloatToString(float x)
{
  String toReturn ;
  std::stringstream convert ;
  convert << x ;
  toReturn = convert.str().c_str() ;

  return toReturn ;
}

void ConstantPropagationAndFoldingPass2::DumpName(Constant* toDump)
{
  if (dynamic_cast<IntConstant*>(toDump) != NULL)
  {
    std::cout << "Int constant:" 
	      << dynamic_cast<IntConstant*>(toDump)->get_value().c_int() 
	      << std::endl ;

  }
  else if (dynamic_cast<FloatConstant*>(toDump) != NULL)
  {
    std::cout << "Float constant:" ;

      float oldValue = 
	StringToFloat(dynamic_cast<FloatConstant*>(toDump)->get_value()) ;

      std::cout << "Original string: " 
		<< dynamic_cast<FloatConstant*>(toDump)->get_value() 
		<< std::endl ;
      std::cout << "Converted float: " 
		<< StringToFloat(oldValue) 
		<< std::endl ;
      std::cout << "Converted back: " << FloatToString(oldValue) 
		<< std::endl ;	      

  }
  else
  {
    assert(0) ;
  }
}

bool ConstantPropagationAndFoldingPass2::isConstant(Statement* s)
{
  StoreVariableStatement* storeVar = dynamic_cast<StoreVariableStatement*>(s) ;
  CallStatement* call = dynamic_cast<CallStatement*>(s) ;
  
  if (storeVar != NULL)
  {
    Expression* rightHandSide = storeVar->get_value() ;    
    if (dynamic_cast<Constant*>(rightHandSide) != NULL)
    {
      return true ;
    }
  }
  else if (call != NULL)
  {
    // There is no currently supported way to have an output of a call
    //  statement be a constant
    return false ;
  }
  else
  {
    // Nothing here...
  }
  
  return false ;
}

bool ConstantPropagationAndFoldingPass2::SamePrecedence(LString opcode1, 
							LString opcode2)
{
  // Also handles xor, or, and
  if (opcode1 == opcode2)
  {
    return true ;
  }

  if (opcode1 == LString("add") || opcode1 == LString("subtract"))
  {
    return (opcode2 == LString("add") || opcode2 == LString("subtract")) ;
  }

  if (opcode1 == LString("right_shift") || opcode1 == LString("left_shift"))
  {
    return (opcode2 == LString("right_shift") || 
	    opcode2 == LString("left_shift")) ;
  }

  if (opcode1 == LString("multiply") || opcode1 == LString("divide"))
  {
    return (opcode2 == LString("multiply") || opcode2 == LString("divide")) ;
  }

  if (opcode1 == LString("bitwise_not") || opcode1 == LString("logical_not") ||
      opcode1 == LString("invert"))
  {
    return (opcode2 == LString("bitwise_not") || 
      opcode2 == LString("logical_not") ||
	    opcode2 == LString("invert")) ;
  }
							   
  // Something else?
  return false ;
}

// Not really a splay procedure, but it sounded like it at the time.
//  This function only deals with additions and subtractions at this time.

// I have to clean up the constants that get merged away and the binary
//  expressions that get routed around.

BinaryExpression* 
ConstantPropagationAndFoldingPass2::Splay(BinaryExpression* top,
					  BinaryExpression* bottom)
{
  Constant* topLeftConstant = dynamic_cast<Constant*>(top->get_source1()) ;
  Constant* topRightConstant = dynamic_cast<Constant*>(top->get_source2()) ;
  Constant* bottomLeftConstant = 
    dynamic_cast<Constant*>(bottom->get_source1());
  Constant* bottomRightConstant = 
    dynamic_cast<Constant*>(bottom->get_source2()) ;

  if (bottomLeftConstant == NULL && bottomRightConstant == NULL)
  {
    return NULL ;
  }

  Constant* topConstant = NULL ;
  Constant* bottomConstant = NULL ;
  Expression* nonConstant = NULL ;
  if (topLeftConstant != NULL)
  {
    topConstant = topLeftConstant ;
  }
  else
  {
    topConstant = topRightConstant ;
  }
  if (bottomLeftConstant != NULL)
  {
    bottomConstant = bottomLeftConstant ;
    nonConstant = bottom->get_source2() ;
  }
  else
  {
    bottomConstant = bottomRightConstant ;
    nonConstant = bottom->get_source1() ;
  }

  assert(topConstant != NULL) ;
  assert(bottomConstant != NULL) ;
  assert(nonConstant != NULL) ;

  if (dynamic_cast<Constant*>(nonConstant) != NULL)
  {
    // There were two constants at the bottom, this will be dealt with
    //  in the next iteration of the main loop
    return NULL ;
  }

  assert(dynamic_cast<Constant*>(nonConstant) == NULL) ;

  // Case 1: Addition on the top and Addition on the bottom
  if (top->get_opcode() == bottom->get_opcode() && 
      top->get_opcode() == LString("add"))
  {
    Constant* finalConstant = 
      Merge(topConstant, bottomConstant, LString("add")) ;

    nonConstant->set_parent(NULL) ;
    top->set_source1(nonConstant) ;
    top->set_source2(finalConstant) ;

    //    delete topConstant ;
    //    delete bottomConstant ;

    return top ;
  }
  // Case 2: Addition on the top and Subtraction on the bottom
  else if (top->get_opcode() == LString("add") &&
      bottom->get_opcode() == LString("subtract"))
  {
    // Four cases, four different things to do in each instance.
    if (topRightConstant != NULL && bottomRightConstant != NULL)
    {
      Constant* finalConstant = 
	Merge(topConstant, bottomConstant, LString("add")) ;
      nonConstant->set_parent(NULL) ;
      top->set_source1(nonConstant) ;
      top->set_source2(finalConstant) ;
      top->set_opcode(LString("subtract")) ;

      //      delete topConstant ;
      //      delete bottomConstant ;
      //      delete bottom ;

      return top ;
    }
    if (topRightConstant != NULL && bottomLeftConstant != NULL)
    {
      Constant* finalConstant = 
	Merge(topConstant, bottomConstant, LString("add")) ;
      nonConstant->set_parent(NULL) ;
      top->set_source1(finalConstant) ;
      top->set_source2(nonConstant) ;
      top->set_opcode(LString("subtract")) ;

      //      delete topConstant ;
      //      delete bottomConstant ;
      //      delete bottom ;

      return top ;
    }
    if (topLeftConstant != NULL && bottomLeftConstant != NULL)
    {
      Constant* finalConstant =
	Merge(topConstant, bottomConstant, LString("add")) ;
      nonConstant->set_parent(NULL) ;
      top->set_source1(finalConstant) ;
      top->set_source2(nonConstant) ;
      top->set_opcode(LString("subtract")) ;

      //      delete topConstant ;
      //delete bottomConstant ;
      //      delete bottom ;

      return top ;
    }
    if (topLeftConstant != NULL && bottomRightConstant != NULL)
    {
      Constant* finalConstant = 
	Merge(topConstant, bottomConstant, LString("subtract")) ;
      nonConstant->set_parent(NULL) ;
      top->set_source1(finalConstant) ;
      top->set_source2(nonConstant) ;
      // Remain an add

      //      delete topConstant ;
      //delete bottomConstant ;
      //      delete bottom ;

      return top ;
    }
  }
  // Case 3: Subtraction on the top and Addition on the bottom
  else if (top->get_opcode() == LString("subtract") &&
      bottom->get_opcode() == LString("add"))
  {
    // Four internal cases
    if (topRightConstant != NULL && bottomRightConstant != NULL)
    {
      Constant* finalConstant = 
	Merge(bottomConstant, topConstant, LString("subtract")) ;
      nonConstant->set_parent(NULL) ;
      top->set_source1(nonConstant) ;
      top->set_source2(finalConstant) ;
      top->set_opcode(LString("add")) ;

      //      delete topConstant ;
      //      delete bottomConstant ;
      //      delete bottom ;

      return top ;
    }
    if (topRightConstant != NULL && bottomLeftConstant != NULL)
    {
      Constant* finalConstant = 
	Merge(bottomConstant, topConstant, LString("subtract")) ;
      nonConstant->set_parent(NULL) ;
      top->set_source1(finalConstant) ;
      top->set_source2(nonConstant) ;
      top->set_opcode(LString("add")) ;
      //      delete topConstant ;
      //      delete bottomConstant ;
      //      delete bottom ;

      return top ;
    }
    if ((topLeftConstant != NULL && bottomLeftConstant != NULL) ||
	(topLeftConstant != NULL && bottomRightConstant != NULL))
    {
      Constant* finalConstant = 
	Merge(topConstant, bottomConstant, LString("subtract")) ;
      nonConstant->set_parent(NULL) ;
      top->set_source1(finalConstant) ;
      top->set_source2(nonConstant) ;
      top->set_opcode(LString("subtract")) ;
      //      delete topConstant ;
      //      delete bottomConstant ;
      //      delete bottom ;

      return top ;
    }
  }
  // Case 4: Subtraction on the top and the bottom
  else if (top->get_opcode() == LString("subtract") &&
      bottom->get_opcode() == LString("subtract"))
  {
    // Four internal cases again.
    if (topRightConstant != NULL && bottomRightConstant != NULL)
    {
      Constant* finalConstant = 
	Merge(bottomConstant, topConstant, LString("add")) ;
      nonConstant->set_parent(NULL) ;
      top->set_source1(nonConstant) ;
      top->set_source2(finalConstant) ;
      // Keep it subtract
      //      delete topConstant ;
      //      delete bottomConstant ;
      //      delete bottom ;

      return top ;
    }
    if (topRightConstant != NULL && bottomLeftConstant != NULL)
    {
      Constant* finalConstant =
	Merge(bottomConstant, topConstant, LString("subtract")) ;
      nonConstant->set_parent(NULL) ;
      top->set_source1(finalConstant) ;
      top->set_source2(nonConstant) ;
      // Keep it subtract
      //      delete topConstant ;
      //      delete bottomConstant ;
      //      delete bottom ;

      return top ;
    }
    if (topLeftConstant != NULL && bottomLeftConstant != NULL)
    {
      Constant* finalConstant = 
	Merge(topConstant, bottomConstant, LString("subtract")) ;
      nonConstant->set_parent(NULL) ;
      top->set_source1(finalConstant) ;
      top->set_source2(nonConstant) ;
      top->set_opcode(LString("add")) ;
      //      delete topConstant ;
      //      delete bottomConstant ;
      //      delete bottom ;

      return top ;
    }
    if (topLeftConstant != NULL && bottomRightConstant != NULL)
    {
      Constant* finalConstant =
	Merge(topConstant, bottomConstant, LString("add")) ;
      nonConstant->set_parent(NULL) ;
      top->set_source1(finalConstant) ;
      top->set_source2(nonConstant) ;
      // Keep it subtract

      //      delete topConstant ;
      //      delete bottomConstant ;
      //      delete bottom ;

      return top ;
    }
  }
  // Case 5: Multiplication on the top and bottom
  else if (top->get_opcode() == bottom->get_opcode() &&
	   top->get_opcode() == LString("multiply"))
  {
    Constant* finalConstant = 
      Merge(topConstant, bottomConstant, LString("multiply")) ;

    nonConstant->set_parent(NULL) ;
    top->set_source1(nonConstant) ;
    top->set_source2(finalConstant) ;

    //    delete topConstant ;
    //    delete bottomConstant ;

    return top ;
  }

  // All other cases aren't yet supported.
  return NULL ;
}
