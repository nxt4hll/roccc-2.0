
#include <cassert>

#include "castPass.h"

#include <suifkernel/utilities.h>
#include <basicnodes/basic_factory.h>
#include <cfenodes/cfe_factory.h>
#include <suifnodes/suif_factory.h>

#include "roccc_utils/warning_utils.h"

//  The times we instantiate a cast are as follows:
//  1. A single assignment from one concrete value to another.
//  2. Floats changed into ints
//  3. Ints changed into floats.
//  4. Floats changed into floats of other sizes.

CastPass::CastPass(SuifEnv* pEnv) : 
  PipelinablePass(pEnv, "CastPass") 
{
  theEnv = pEnv ;
  procDef = NULL ;

  FloatToInt = NULL ;
  IntToFloat = NULL ;
  IntToInt = NULL ;
  FloatToFloat = NULL ;

  FloatToIntType = NULL ;
  IntToFloatType = NULL ;
  IntToIntType = NULL ;
  FloatToFloatType = NULL ;

  FloatToIntUsed = false ;
  IntToFloatUsed = false ;
  IntToIntUsed = false ;
  FloatToFloatUsed = false ;

}

CastPass::~CastPass()
{
  ; // Nothing to delete yet...
}

void CastPass::do_procedure_definition(ProcedureDefinition* proc_def)
{
  procDef = proc_def ;
  assert(procDef != NULL) ;

  OutputInformation("Cast Pass Begins") ;

  CreateProcedures() ;

  HandleCallStatements() ;
  HandleBinaryExpressions() ;
  HandleStoreVariableStatements() ;
  HandleStoreStatements() ;

  OutputInformation("Cast Pass Ends") ;
}

// This should handle all module instantiations and boolean select statements
void CastPass::HandleCallStatements()
{
  assert(procDef != NULL) ;
  list<CallStatement*>* allCalls = 
    collect_objects<CallStatement>(procDef->get_body()) ;
  list<CallStatement*>::iterator callIter = allCalls->begin() ;
  while (callIter != allCalls->end())
  {

    // For each call instruction, go through all of the inputs
    //  and make a cast where appropriate.
    Expression* address = (*callIter)->get_callee_address() ;
    SymbolAddressExpression* procAddressExp = 
      dynamic_cast<SymbolAddressExpression*>(address) ;
    assert(procAddressExp != NULL) ;
    Symbol* addressedSymbol = procAddressExp->get_addressed_symbol() ;
    ProcedureSymbol* procSym = 
      dynamic_cast<ProcedureSymbol*>(addressedSymbol) ;
    assert(procSym != NULL) ;
    CProcedureType* procType = 
      dynamic_cast<CProcedureType*>(procSym->get_type()) ;
    assert(procType != NULL) ;

    for (int i = 0 ; i < (*callIter)->get_argument_count() ; ++i)
    {
      Expression* currentArgument = (*callIter)->get_argument(i) ;
      DataType* expectedType = procType->get_argument(i)->get_base_type() ;

      // The argument is either a load variable expression, a field access
      //  expression, or a symbol address expression.  The symbol address
      //  expression is used for output variables.
      if (dynamic_cast<SymbolAddressExpression*>(currentArgument) != NULL)
      {
	continue ;
      }      

      DataType* currentType = currentArgument->get_result_type() ;
      
      IntegerType* currentInt =
	dynamic_cast<IntegerType*>(currentType) ;
      IntegerType* expectedInt =
	dynamic_cast<IntegerType*>(expectedType) ;
      FloatingPointType* currentFloat =
	dynamic_cast<FloatingPointType*>(currentType) ;
      FloatingPointType* expectedFloat =
	dynamic_cast<FloatingPointType*>(expectedType) ;

      /*
      if (currentInt != NULL && expectedInt != NULL)
      {
	int currentSize = currentInt->get_bit_size().c_int() ;
	int expectedSize = expectedInt->get_bit_size().c_int() ;
	if (currentSize != expectedSize)
	{
	  IntToIntUsed = true ;
	  Expression* value = (*callIter)->get_argument(i) ;
	  value->set_parent(NULL) ;
	  SymbolAddressExpression* addressOfIntToInt = 
	    create_symbol_address_expression(theEnv,
					     baseInt,
					     IntToInt) ;
	  CallExpression* replacement =
	    create_call_expression(theEnv,
				   baseInt,
				   addressOfIntToInt) ;
	  
	  IntConstant* secondArg = 
	    create_int_constant(theEnv,
				baseInt,
				IInteger(expectedSize)) ;
	  // Append the arguments
	  replacement->append_argument(value) ;
	  replacement->append_argument(secondArg) ;
	  
	  (*callIter)->replace_argument(i, replacement) ;
	}
      }
      */
      if (currentFloat != NULL && expectedFloat != NULL)
      {
	int currentSize = currentFloat->get_bit_size().c_int() ;
	int expectedSize = expectedFloat->get_bit_size().c_int() ;
	if (currentSize != expectedSize)
	{
	  FloatToFloatUsed = true ;
	  Expression* value = (*callIter)->get_argument(i) ;
	  value->set_parent(NULL) ;
	  SymbolAddressExpression* addressOfFloatToFloat = 
	    create_symbol_address_expression(theEnv,
					     baseFloat,
					     FloatToFloat) ;
	  CallExpression* replacement =
	    create_call_expression(theEnv,
				   baseFloat,
				   addressOfFloatToFloat) ;
	  
	  IntConstant* secondArg = 
	    create_int_constant(theEnv,
				baseFloat,
				IInteger(expectedSize)) ;
	  // Append the arguments
	  replacement->append_argument(value) ;
	  replacement->append_argument(secondArg) ;
	  
	  (*callIter)->replace_argument(i, replacement) ;
	}
      }
      if (currentInt != NULL && expectedFloat != NULL)
      {
	IntToFloatUsed = true ;
	Expression* value = (*callIter)->get_argument(i) ;
	value->set_parent(NULL) ;
	SymbolAddressExpression* addressOfIntToFloat = 
	  create_symbol_address_expression(theEnv,
					   baseFloat,
					   IntToFloat) ;
	CallExpression* replacement =
	  create_call_expression(theEnv,
				 baseFloat,
				 addressOfIntToFloat) ;
	
	IntConstant* secondArg = 
	  create_int_constant(theEnv,
			      baseFloat,
			      expectedFloat->get_bit_size()) ;
	// Append the arguments
	replacement->append_argument(value) ;
	replacement->append_argument(secondArg) ;
	
	(*callIter)->replace_argument(i, replacement) ;
      }
      if (currentFloat != NULL && expectedInt != NULL)
      {	  
	  FloatToIntUsed = true ;
	  Expression* value = (*callIter)->get_argument(i) ;
	  value->set_parent(NULL) ;
	  SymbolAddressExpression* addressOfFloatToInt = 
	    create_symbol_address_expression(theEnv,
					     baseInt,
					     FloatToInt) ;
	  CallExpression* replacement =
	    create_call_expression(theEnv,
				   baseInt,
				   addressOfFloatToInt) ;
	    
	  IntConstant* secondArg = 
	    create_int_constant(theEnv,
				baseInt,
				expectedInt->get_bit_size()) ;
	  // Append the arguments
	  replacement->append_argument(value) ;
	  replacement->append_argument(secondArg) ;
	    
	  (*callIter)->replace_argument(i, replacement) ;
	}


    }
    ++callIter ;
  }
  delete allCalls ;
}

void CastPass::HandleBinaryExpressions()
{
  assert(procDef != NULL) ;
  list<BinaryExpression*>* allBinaryExpressions =
    collect_objects<BinaryExpression>(procDef->get_body()) ;
  list<BinaryExpression*>::iterator binIter = allBinaryExpressions->begin() ;
  while (binIter != allBinaryExpressions->end())
  {
    Expression* leftExp = (*binIter)->get_source1() ;
    Expression* rightExp = (*binIter)->get_source2() ;
    
    // Don't do any upcasting with constants
    if (dynamic_cast<Constant*>(leftExp) != NULL ||
	dynamic_cast<Constant*>(rightExp) != NULL)
    {
      ++binIter ;
      continue ;
    }
    
    DataType* leftType = leftExp->get_result_type() ;
    DataType* rightType = rightExp->get_result_type() ;

    IntegerType* leftInt = dynamic_cast<IntegerType*>(leftType) ;
    IntegerType* rightInt = dynamic_cast<IntegerType*>(rightType) ;

    FloatingPointType* leftFloat = 
      dynamic_cast<FloatingPointType*>(leftType) ;
    FloatingPointType* rightFloat = 
      dynamic_cast<FloatingPointType*>(rightType) ;


    if (leftFloat != NULL && rightFloat != NULL)
    {
      int leftSize = leftFloat->get_bit_size().c_int() ;
      int rightSize = rightFloat->get_bit_size().c_int() ;
      if (leftSize != rightSize)
      {
	FloatToFloatUsed = true ;
	if (leftSize > rightSize)
	{
	  rightExp->set_parent(NULL) ;
	  // I'm not sure what the result type should be...
	  SymbolAddressExpression* addressOfFloatToFloat = 
	    create_symbol_address_expression(theEnv,
					     baseFloat,
					     FloatToFloat) ;
	  CallExpression* replacement =
	    create_call_expression(theEnv,
				   baseFloat,
				   addressOfFloatToFloat) ;
	  IntConstant* secondArg = 
	    create_int_constant(theEnv,
				baseInt,
				IInteger(leftSize)) ;
	  // Append the arguments
	  replacement->append_argument(rightExp) ;
	  replacement->append_argument(secondArg) ;
	  
	  (*binIter)->set_source2(replacement) ;
	}
	else
	{
	  leftExp->set_parent(NULL) ;
	  // I'm not sure what the result type should be...
	  SymbolAddressExpression* addressOfFloatToFloat = 
	    create_symbol_address_expression(theEnv,
					     baseFloat,
					     FloatToFloat) ;
	  CallExpression* replacement =
	    create_call_expression(theEnv,
				   baseFloat,
				   addressOfFloatToFloat) ;
	  IntConstant* secondArg = 
	    create_int_constant(theEnv,
				baseInt,
				IInteger(rightSize)) ;
	  // Append the arguments
	  replacement->append_argument(leftExp) ;
	  replacement->append_argument(secondArg) ;
	  
	  (*binIter)->set_source1(replacement) ;

	}
      }
    }
    
    /*
    if (leftInt != NULL && rightInt != NULL)
    {
      int leftSize = leftInt->get_bit_size().c_int() ;
      int rightSize = rightInt->get_bit_size().c_int() ;
      if (leftSize != rightSize)
      {
	IntToIntUsed = true ;
	if (leftSize > rightSize)
	{
	  rightExp->set_parent(NULL) ;

	  SymbolAddressExpression* addressOfIntToInt = 
	    create_symbol_address_expression(theEnv,
					     baseInt,
					     IntToInt) ;
	  CallExpression* replacement =
	    create_call_expression(theEnv,
				   baseInt,
				   addressOfIntToInt) ;
	  IntConstant* secondArg = 
	    create_int_constant(theEnv,
				baseInt,
				IInteger(leftSize)) ;
	  // Append the arguments
	  replacement->append_argument(rightExp) ;
	  replacement->append_argument(secondArg) ;
	  
	  (*binIter)->set_source2(replacement) ;
	}
	else
	{
	  leftExp->set_parent(NULL) ;
	  // I'm not sure what the result type should be...
	  SymbolAddressExpression* addressOfIntToInt = 
	    create_symbol_address_expression(theEnv,
					     baseInt,
					     IntToInt) ;
	  CallExpression* replacement =
	    create_call_expression(theEnv,
				   baseInt,
				   addressOfIntToInt) ;
	  IntConstant* secondArg = 
	    create_int_constant(theEnv,
				baseInt,
				IInteger(rightSize)) ;
	  // Append the arguments
	  replacement->append_argument(leftExp) ;
	  replacement->append_argument(secondArg) ;
	  
	  (*binIter)->set_source1(replacement) ;
	}
      }
    }
    */
    if (leftFloat != NULL && rightInt != NULL)
    {
      rightExp->set_parent(NULL) ;
      // Ints get upgraded to floats every time
      int floatSize = leftFloat->get_bit_size().c_int() ;

      SymbolAddressExpression* addressOfIntToFloat = 
	create_symbol_address_expression(theEnv,
					 baseFloat,
					 IntToFloat) ;
      CallExpression* replacement =
	create_call_expression(theEnv,
			       baseFloat,
			       addressOfIntToFloat) ;
      IntConstant* secondArg = 
	create_int_constant(theEnv,
			    baseInt,
			    IInteger(floatSize)) ;
      // Append the arguments
      replacement->append_argument(rightExp) ;
      replacement->append_argument(secondArg) ;
	  
      (*binIter)->set_source2(replacement) ;
    }

    if (leftInt != NULL && rightFloat != NULL)
    {
      leftExp->set_parent(NULL) ;
      // Ints get upgraded to floats every time
      int floatSize = rightFloat->get_bit_size().c_int() ;

      SymbolAddressExpression* addressOfIntToFloat = 
	create_symbol_address_expression(theEnv,
					 baseFloat,
					 IntToFloat) ;
      CallExpression* replacement =
	create_call_expression(theEnv,
			       baseFloat,
			       addressOfIntToFloat) ;
      IntConstant* secondArg = 
	create_int_constant(theEnv,
			    baseInt,
			    IInteger(floatSize)) ;
      // Append the arguments
      replacement->append_argument(leftExp) ;
      replacement->append_argument(secondArg) ;
	  
      (*binIter)->set_source1(replacement) ;
    }
    
    ++binIter ;
  }
  delete allBinaryExpressions ;
}

void CastPass::HandleStoreVariableStatements()
{
  assert(procDef != NULL) ;
  list<StoreVariableStatement*>* allStoreVariables =
    collect_objects<StoreVariableStatement>(procDef->get_body()) ;
  list<StoreVariableStatement*>::iterator storeIter = 
    allStoreVariables->begin() ;
  while (storeIter != allStoreVariables->end())
  {
    VariableSymbol* dest = (*storeIter)->get_destination() ;
    Expression* rightSide = (*storeIter)->get_value() ;

    DataType* destType = dest->get_type()->get_base_type() ;
    DataType* valueType = rightSide->get_result_type() ;
    
    assert(destType != NULL) ;
    assert(valueType != NULL) ;
    
    IntegerType* destInt = dynamic_cast<IntegerType*>(destType) ;
    IntegerType* valueInt = dynamic_cast<IntegerType*>(valueType) ;
    FloatingPointType* destFloat = dynamic_cast<FloatingPointType*>(destType) ;
    FloatingPointType* valueFloat = 
      dynamic_cast<FloatingPointType*>(valueType) ;

    if ( (destInt != NULL && valueFloat != NULL) ||
	 (destFloat != NULL && valueInt != NULL) || 
	 SingularVariable(rightSide))
    {
      (*storeIter)->set_value(NULL) ;
      rightSide->set_parent(NULL) ;
      UnaryExpression* replacement = 
	create_unary_expression(theEnv,
				destType,
				LString("convert"),
				rightSide) ;
      (*storeIter)->set_value(replacement) ;
    }

    // Also, check for floating point values of different sizes
    // Also, if the values are both floats and are different sizes, we
    //  need a convert.
    if (destFloat != NULL && valueFloat != NULL)
    {
      if (destFloat->get_bit_size() != valueFloat->get_bit_size())
      {
	(*storeIter)->set_value(NULL) ;
	rightSide->set_parent(NULL) ;
	UnaryExpression* replacement = 
	  create_unary_expression(theEnv,
				  destType,
				  LString("convert"), 
				  rightSide) ;
	(*storeIter)->set_value(replacement) ;
      }
    }
    ++storeIter ;
  }
  delete allStoreVariables ;
}

void CastPass::CreateProcedures()
{
  baseFloat = create_floating_point_type(theEnv,
					 IInteger(32),
					 0,
					 LString("Float")) ;
  baseInt = create_integer_type(theEnv,
				IInteger(32),
				0,
				true,
				LString("Int")) ;
  
  FloatToIntType = create_c_procedure_type(theEnv,
					   baseInt,
					   false,
					   true,
					   0,
					   LString("ROCCCFloatToIntType")) ;

  FloatToIntType->append_argument(create_qualified_type(theEnv, baseFloat)) ;
  FloatToIntType->append_argument(create_qualified_type(theEnv, baseInt)) ;

  IntToFloatType = create_c_procedure_type(theEnv,
					   baseFloat,
					   false,
					   true,
					   0,
					   LString("ROCCCIntToFloatType")) ;

  IntToFloatType->append_argument(create_qualified_type(theEnv, baseInt)) ;
  IntToFloatType->append_argument(create_qualified_type(theEnv, baseInt)) ;
    
  IntToIntType = create_c_procedure_type(theEnv,
					 baseInt,
					 false,
					 true,
					 0,
					 LString("ROCCCIntToIntType")) ;

  IntToIntType->append_argument(create_qualified_type(theEnv, baseInt)) ;
  IntToIntType->append_argument(create_qualified_type(theEnv, baseInt)) ;
  
  FloatToFloatType = create_c_procedure_type(theEnv,
					     baseFloat,
					     false,
					     true,
					     0,
					     LString("ROCCCFloatToFloatType"));

  FloatToFloatType->append_argument(create_qualified_type(theEnv, baseFloat)) ;
  FloatToFloatType->append_argument(create_qualified_type(theEnv, baseInt)) ;

  FloatToInt = create_procedure_symbol(theEnv,
				       FloatToIntType,
				       LString("ROCCCFPToInt")) ;
  
  IntToFloat = create_procedure_symbol(theEnv,
					   IntToFloatType,
					   LString("ROCCCIntToFP")) ;

  IntToInt = create_procedure_symbol(theEnv,
				     IntToIntType,
				     LString("ROCCCIntToInt")) ;
  FloatToFloat = create_procedure_symbol(theEnv,
					 FloatToFloatType,
					 LString("ROCCCFPToFP")) ;
}

// These should not be array accesses, they should all go to field declarations
//  and be part of module compilation.
void CastPass::HandleStoreStatements()
{
  assert(procDef != NULL) ;
  list<StoreStatement*>* allStores = 
    collect_objects<StoreStatement>(procDef->get_body()) ;
  list<StoreStatement*>::iterator storeIter = allStores->begin() ;
  while (storeIter != allStores->end())
  {
    Expression* dest = (*storeIter)->get_destination_address() ;
    Expression* value = (*storeIter)->get_value() ;

    assert(dest  != NULL) ;
    assert(value != NULL) ;

    DataType* destType  = dest->get_result_type() ;
    DataType* valueType = value->get_result_type() ;

    assert(destType  != NULL) ;
    assert(valueType != NULL) ;
    
    IntegerType* destInt = dynamic_cast<IntegerType*>(destType) ;
    IntegerType* valueInt = dynamic_cast<IntegerType*>(valueType) ;
    FloatingPointType* destFloat = dynamic_cast<FloatingPointType*>(destType) ;
    FloatingPointType* valueFloat = 
      dynamic_cast<FloatingPointType*>(valueType) ;

    // We perform a convert if we are changing from an integer to a float
    //  or vice versa.  We also perform a convert if we are ever
    //  doing a straight copy.
    if ( (destInt != NULL && valueFloat != NULL) ||
	 (destFloat != NULL && valueInt != NULL) || 
	 SingularVariable(value))
    {
      (*storeIter)->set_value(NULL) ;
      value->set_parent(NULL) ;
      UnaryExpression* replacement = 
	create_unary_expression(theEnv, 
				destType,
				LString("convert"),
				value) ;
      (*storeIter)->set_value(replacement) ;
    }

    // Also, if the values are both floats and are different sizes, we
    //  need a convert.
    if (destFloat != NULL && valueFloat != NULL)
    {
      if (destFloat->get_bit_size() != valueFloat->get_bit_size())
      {
	(*storeIter)->set_value(NULL) ;
	value->set_parent(NULL) ;
	UnaryExpression* replacement = 
	  create_unary_expression(theEnv,
				  destType,
				  LString("convert"), 
				  value) ;
	(*storeIter)->set_value(replacement) ;
      }
    }
    ++storeIter ;
  }
  delete allStores ;
}

// This might expand as things get interesting...
bool CastPass::SingularVariable(Expression* e)
{
  if (dynamic_cast<LoadVariableExpression*>(e) != NULL)
  {
    return true ;
  }
  if (dynamic_cast<ArrayReferenceExpression*>(e) != NULL)
  {
    return true ;
  }
  if (dynamic_cast<FieldAccessExpression*>(e) != NULL)
  {
    return true ;
  }  
  if (dynamic_cast<LoadExpression*>(e) != NULL &&
      dynamic_cast<FieldAccessExpression*>(dynamic_cast<LoadExpression*>(e)->get_source_address()) != NULL)
  {
    
    return true ;
    }

  return false ;
}

void CastPass::CleanupProcedures() 
{
  if (!FloatToIntUsed)
  {
    delete FloatToIntType ;
    delete FloatToInt ;
  }
  if (!IntToFloatUsed)
  {
    delete IntToFloatType ;
    delete IntToFloat ;
  }
  if (!IntToIntUsed)
  {
    delete IntToIntType ;
    delete IntToInt ;
  }
  if (!FloatToFloatUsed)
  {
    delete FloatToFloatType ;
    delete FloatToFloat ;
  }
}
