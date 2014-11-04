
#include <common/lstring.h>
#include <cassert>
#include <cstdio>

#include "cstNodes.h"
#include "suifGenerator.h"
#include "option.h"
#include "function.h"

using namespace std ;

/************ IntegerCst functions ****************/

IntegerCst::IntegerCst(Option* t, Option* h, Option* l) : Node(), 
							  typeop(t),
							  highop(h),
							  lowop(l)
{
  ;
}

IntegerCst::~IntegerCst()
{
  if (typeop != NULL)
    delete typeop;
  if (highop != NULL)
    delete highop;
  if (lowop != NULL)
    delete lowop;
}

void IntegerCst::connect(Function* t)
{
  if (!connected)
  {
    connected = true;
  }
  else
  {
    return;
  }
  if (typeop != NULL)
    typeop->connect(t);
  if (highop != NULL)
    highop->connect(t);
  if (lowop != NULL)
    lowop->connect(t);
}

void IntegerCst::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
  {
    type = NULL ;
  }
  if (highop != NULL)
  {
    high = highop->theNumber() ;
    delete highop ;
    highop = NULL ;
  }
  else
  {
    high = -1 ;
  }
  if (lowop != NULL)
  {
    low = lowop->theNumber() ;
    delete lowop ;
    lowop = NULL ;
  }
  else
  {
    low = -1 ;
  }
}

int IntegerCst::theSize()
{
  return low ;
}

ExecutionObject* IntegerCst::generateSuif(Program* p) 
{

  if (type != NULL)
  {
    return p->converter->createIntegerConstantExpression(low,
							 type->theSize()) ;
  }
  else
  {
    return p->converter->createIntegerConstantExpression(low) ;
  }


}

ValueBlock* IntegerCst::generateSuifInitialization(Program* p,
						   ValueBlock* topLevel,
						   int currentLevel,
						   Type* elementType)
{
  // Just make a value block based upon my expression

  // In the special case of creating an integer constant with a different 
  //  bit size (i.e. in an array initialization) we have to take the element
  //  type that was passed in and make sure that our type works

  if (elementType == NULL)
  {
    Expression* myValue = dynamic_cast<Expression*>(generateSuif(p)) ;
    return p->converter->createExpressionValueBlock(myValue) ;
  }
  else
  {
    Expression* myValue = dynamic_cast<Expression*>(generateSuif(p)) ;
    assert(myValue != NULL) ;
    assert(is_a<DataType*>(elementType));
    myValue->set_result_type(dynamic_cast<DataType*>(elementType)) ;
    return p->converter->createExpressionValueBlock(myValue) ;
  }
}

Type* IntegerCst::suifType(Program* p)
{
  assert(type != NULL) ;
  return type->suifType(p) ;
}

/************ RealCst functions ****************/

RealCst::RealCst(Option* t, double l) : Node(), typeop(t), low(l)
{
  ;
}

RealCst::~RealCst()
{
  if (typeop != NULL)
    delete typeop;
}

void RealCst::connect(Function* t)
{
  if (!connected)
  {
    connected = true;
  }
  else
  {
    return;
  }
  if (typeop != NULL)
    typeop->connect(t);
}

void RealCst::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
  {
    type = NULL ;
  }
}

ExecutionObject* RealCst::generateSuif(Program* p)
{

  assert(type != NULL) ;

  // Figure out the size
  int size = type->theSize() ;

  return p->converter->createFloatingPointConstantExpression(low, size) ;
}

/************ StringCst functions ****************/

StringCst::StringCst(Option* t, Option* s, Option* l) : Node(),
							typeop(t),
							strgop(s),
							lngtop(l)
{
  // This should be deleted when the symbol table gets
  //  deleted (so not my problem (I think...).
  stringLocation = NULL ;
}

StringCst::~StringCst()
{
  if (typeop != NULL)
    delete typeop;
  if (strgop != NULL)
    delete strgop;
  if (lngtop != NULL)
    delete lngtop;
}

void StringCst::connect(Function* t)
{
  if (!connected)
  {
    connected = true;
  }
  else
  {
    return;
  }
  if (typeop != NULL)
    typeop->connect(t);
  if (strgop != NULL)
    strgop->connect(t);
  if (lngtop != NULL)
    lngtop->connect(t);
}

void StringCst::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
  {
    type = NULL ;
  }
  if (strgop != NULL)
  {
    strg = (dynamic_cast<StrgOption*>(strgop))->theName() ;
    delete strgop ;
    strgop = NULL;
  }
  else
  {
    strg = "" ;
  }
  if (lngtop != NULL)
  {
    lngt = lngtop->theNumber() ;
    delete lngtop ;
    lngtop = NULL ;
  }
  else
  {
    lngt = -1 ;
  }
}


Symbol* StringCst::getVariable(Program* p)
{

  if (stringLocation == NULL)
  {

    // Generate a variable and a value block initialized to the constant
    //  Add these to the symbol table, and set my variable to point at it.

    IntegerType* charType = p->converter->globalTb->get_integer_type(8, 8, true) ;

    list<LString> quals ;

    QualifiedType* qualCharType = p->converter->globalTb->get_qualified_type(charType, quals);

    // Not -1 because of the NULL character

    ArrayType* stringType = p->converter->globalTb->get_array_type(strg.size() * 8, 8, qualCharType, p->converter->createIntegerConstantExpression(0), p->converter->createIntegerConstantExpression(strg.size())) ;

    quals.push_back("static") ;

    QualifiedType* qualStringType = p->converter->globalTb->get_qualified_type(stringType, quals) ; 

    MultiValueBlock* initialization ;

    initialization = p->converter->globalSuifObjfactory->create_multi_value_block(stringType) ;

    int i = 0 ;
    for(i = 0 ; i < strg.size(); ++i)
    {
      ValueBlock* nextBlock = p->converter->createExpressionValueBlock(p->converter->createIntegerConstantExpression(strg[i], 8)) ;
      initialization->add_sub_block(IInteger(i*8), nextBlock) ;
      // Here, this *8 really should be something else depending on the
      //  alignment perhaps?  Oh well, I know this is a string constant and 
      //  this is how the EDG front end did things, so I will follow this
      //  convention.
    }

    // Add the final \0 character
    ValueBlock* nullBlock = p->converter->createExpressionValueBlock(p->converter->createIntegerConstantExpression(0, 8)) ;
    initialization->add_sub_block(IInteger(i*8), nullBlock) ;

    // This temporary should not have a name (blank name in the field)

    stringLocation = p->converter->addVariableSymbol(LString(""), 
						     qualStringType,
						     initialization, true) ;
  }

  return stringLocation ;

  // Return the address of the constant string
  //  Actually, no I don't
  //  return p->converter->createAddressExpression(stringLocation) ;

}
