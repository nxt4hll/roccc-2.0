
#include <cassert>

#include "equivalentUtilities.h"

bool EquivalentExpressions(Expression* x, Expression* y) 
{
  assert(x != NULL) ;
  assert(y != NULL) ;
  
  if (x == y)
  {
    return true ;
  }
  
  // Now check all of the different types of expressions and see if they
  //  are the same.
  if (dynamic_cast<BinaryExpression*>(x) != NULL &&
      dynamic_cast<BinaryExpression*>(y) != NULL) 
  {
    BinaryExpression* xBin = dynamic_cast<BinaryExpression*>(x) ;
    BinaryExpression* yBin = dynamic_cast<BinaryExpression*>(y) ;
    
    LString xOpcode = xBin->get_opcode() ;
    LString yOpcode = yBin->get_opcode() ;

    if (xOpcode == LString("divide") || xOpcode == LString("subtract"))
    {
      return (xOpcode == yOpcode) &&
	(
	 EquivalentExpressions(xBin->get_source1(), yBin->get_source1()) &&
	 EquivalentExpressions(xBin->get_source2(), yBin->get_source2())
        ) ;
    }

    // If the binary expression is associative, we need to check both 
    //  options

    return (xOpcode == yOpcode) &&
      ((
	EquivalentExpressions(xBin->get_source1(), yBin->get_source1()) &&
	EquivalentExpressions(xBin->get_source2(), yBin->get_source2())
       ) ||
       (
	EquivalentExpressions(xBin->get_source1(), yBin->get_source2()) &&
	EquivalentExpressions(xBin->get_source2(), yBin->get_source1())
       )
      ) ;
  }

  if (dynamic_cast<UnaryExpression*>(x) != NULL &&
      dynamic_cast<UnaryExpression*>(y) != NULL)
  {
    UnaryExpression* xUnary = dynamic_cast<UnaryExpression*>(x) ;
    UnaryExpression* yUnary = dynamic_cast<UnaryExpression*>(y) ;
    
    LString xOpcode = xUnary->get_opcode() ;
    LString yOpcode = yUnary->get_opcode() ;
    
    return (xOpcode == yOpcode) &&
           EquivalentExpressions(xUnary->get_source(), yUnary->get_source()) ;
  }

  if (dynamic_cast<ArrayReferenceExpression*>(x) != NULL &&
      dynamic_cast<ArrayReferenceExpression*>(y) != NULL)
  {
    ArrayReferenceExpression* xRef= dynamic_cast<ArrayReferenceExpression*>(x);
    ArrayReferenceExpression* yRef= dynamic_cast<ArrayReferenceExpression*>(y);

    return EquivalentExpressions(xRef->get_base_array_address(),
				 yRef->get_base_array_address()) &&
           EquivalentExpressions(xRef->get_index(), yRef->get_index()) ;
  }

  if (dynamic_cast<SymbolAddressExpression*>(x) != NULL &&
      dynamic_cast<SymbolAddressExpression*>(y) != NULL)
  {
    SymbolAddressExpression* xSym = dynamic_cast<SymbolAddressExpression*>(x) ;
    SymbolAddressExpression* ySym = dynamic_cast<SymbolAddressExpression*>(y) ;

    LString xName = xSym->get_addressed_symbol()->get_name() ;
    LString yName = ySym->get_addressed_symbol()->get_name() ;
    
    return (xName == yName) && (xName != LString("")) ;
  }

  if (dynamic_cast<FieldAccessExpression*>(x) != NULL &&
      dynamic_cast<FieldAccessExpression*>(y) != NULL)
  {
    FieldAccessExpression* xField = dynamic_cast<FieldAccessExpression*>(x) ;
    FieldAccessExpression* yField = dynamic_cast<FieldAccessExpression*>(y) ;
    
    LString xSymName = xField->get_field()->get_name() ;
    LString ySymName = yField->get_field()->get_name() ;
    
    return EquivalentExpressions(xField->get_base_group_address(),
				 yField->get_base_group_address()) &&
           (xSymName == ySymName) && (xSymName != LString("")) ;
  }
  
  if (dynamic_cast<LoadExpression*>(x) != NULL &&
      dynamic_cast<LoadExpression*>(y) != NULL)
  {
    LoadExpression* xLoad = dynamic_cast<LoadExpression*>(x) ;
    LoadExpression* yLoad = dynamic_cast<LoadExpression*>(y) ;

    return EquivalentExpressions(xLoad->get_source_address(),
				 yLoad->get_source_address()) ;
  }

  if (dynamic_cast<LoadVariableExpression*>(x) != NULL &&
      dynamic_cast<LoadVariableExpression*>(y) != NULL)
  {
    LoadVariableExpression* xLoadVar=dynamic_cast<LoadVariableExpression*>(x) ;
    LoadVariableExpression* yLoadVar=dynamic_cast<LoadVariableExpression*>(y) ;
    
    LString xVarName = xLoadVar->get_source()->get_name() ;
    LString yVarName = yLoadVar->get_source()->get_name() ;

    return (xVarName == yVarName) && (xVarName != LString("")) ;
  }

  if (dynamic_cast<IntConstant*>(x) != NULL &&
      dynamic_cast<IntConstant*>(y) != NULL)
  {
    IntConstant* xInt = dynamic_cast<IntConstant*>(x) ;
    IntConstant* yInt = dynamic_cast<IntConstant*>(y) ;

    return xInt->get_value() == yInt->get_value() ;
  }

  // This will only work on constants that have the same representation,
  //  not alternate representations.
  if (dynamic_cast<FloatConstant*>(x) != NULL &&
      dynamic_cast<FloatConstant*>(y) != NULL)
  {
    FloatConstant* xFloat = dynamic_cast<FloatConstant*>(x) ;
    FloatConstant* yFloat = dynamic_cast<FloatConstant*>(y) ;

    return xFloat->get_value() == yFloat->get_value() ;
  }

  if (dynamic_cast<AddressExpression*>(x) != NULL &&
      dynamic_cast<AddressExpression*>(y) != NULL)
  {
    AddressExpression* xAddr = dynamic_cast<AddressExpression*>(x) ;
    AddressExpression* yAddr = dynamic_cast<AddressExpression*>(y) ;

    return EquivalentExpressions(xAddr->get_addressed_expression(),
				 yAddr->get_addressed_expression()) ;
  }

  if (dynamic_cast<NonLvalueExpression*>(x) != NULL &&
      dynamic_cast<NonLvalueExpression*>(y) != NULL)
  {
    NonLvalueExpression* xNon = dynamic_cast<NonLvalueExpression*>(x) ;
    NonLvalueExpression* yNon = dynamic_cast<NonLvalueExpression*>(y) ;
    return EquivalentExpressions(xNon->get_addressed_expression(),
				 yNon->get_addressed_expression()) ;
  }

  // Everything else is a pair of expressions that are of different types
  return false ;
}

// To be implemented...
//bool EquivalentStatements(Statement* x, Statement* y)
//{
//  return false ;
//}

bool EquivalentTypes(DataType* x, DataType* y)
{
  if (x == y)
  {
    return true ;
  }

  BooleanType* xBool = dynamic_cast<BooleanType*>(x) ;
  BooleanType* yBool = dynamic_cast<BooleanType*>(y) ;
  IntegerType* xInt = dynamic_cast<IntegerType*>(x) ;
  IntegerType* yInt = dynamic_cast<IntegerType*>(y) ;
  FloatingPointType* xFloat = dynamic_cast<FloatingPointType*>(x) ;
  FloatingPointType* yFloat = dynamic_cast<FloatingPointType*>(y) ;

  if (xBool != NULL && yBool != NULL)
  {
    return true ;
  }

  if (xInt != NULL && yInt != NULL)
  {
    return (xInt->get_is_signed() == yInt->get_is_signed()) &&
           (xInt->get_bit_size() == yInt->get_bit_size()) ;
  }

  if (xFloat != NULL && yFloat != NULL)
  {
    return (xFloat->get_bit_size() == yFloat->get_bit_size()) ;
  }

  // I currently don't care about the others, but will have to fill this
  //  in later...
  return false ;
}

bool EquivalentTypes(QualifiedType* x, QualifiedType* y)
{
  if (x == y)
  {
    return true ;
  }

  // Check all of the qualifications
  if (x->get_qualification_count() != y->get_qualification_count())
  {
    return false ;
  }

  // The qualifications can be in any order, so check each one
  for (int i = 0 ; i < x->get_qualification_count() ; ++i)
  {
    LString currentQual = x->get_qualification(i) ;
    bool found = false ;
    for (int j = 0 ; j < y->get_qualification_count() ; ++j)
    {
      if (currentQual == y->get_qualification(i))
      {
	found = true ;
	break ;
      }
    }
    if (!found)
    {
      return false ;
    }
  }
  return EquivalentTypes(x->get_base_type(), y->get_base_type()) ;
}
