/*

  This file contains the definitions to all of the functions for the 
   nodes that represent expressions in the gcc abstract syntax tree.

*/

#include <cassert>
#include <common/lstring.h>
#include <iostream>

// John's code
#include "suifGenerator.h"

#include "exprNodes.h"
#include "declNodes.h"
#include "nodes.h"

using namespace std ;

// A helper function just for default values...
LString tempName()
{
  static int currentNumber = 0 ;

  currentNumber++ ;
  return LString("label_") + LString(currentNumber) ;
}

/********************* TruthNotExpr **************************/

TruthNotExpr::TruthNotExpr(Option* t, list<opOptions>* o) :
  Node(), typeop(t), ops(o)
{
  ;
}

TruthNotExpr::~TruthNotExpr()
{
  if (typeop != NULL)
    delete typeop;
  if (ops != NULL)
    delete ops;
}

void TruthNotExpr::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else 
    return ;

  if (typeop != NULL)
    typeop->connect(t);
  if (ops == NULL)
    return ;
  list<opOptions>::iterator connectIter = ops->begin();
  while (connectIter != ops->end())
  {
    (*connectIter).nodePointer=(*(t->allNodes))[(*connectIter).nodeNumber - 1];
    ++connectIter;
  }
}

void TruthNotExpr::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;
}

/********************* AddrExpr **************************/

AddrExpr::AddrExpr(Option* t, list<opOptions>* o) :
  Node(), typeop(t), ops(o)
{
  ;
}

AddrExpr::~AddrExpr()
{
  if (typeop != NULL)
    delete typeop;
  if (ops != NULL)
    delete ops;
}

void AddrExpr::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else 
    return ;
  if (typeop != NULL)
    typeop->connect(t);
  if (ops == NULL)
    return ;
  list<opOptions>::iterator connectIter = ops->begin();
  while (connectIter != ops->end())
  {
    (*connectIter).nodePointer=(*(t->allNodes))[(*connectIter).nodeNumber - 1];
    ++connectIter;
  }
}

void AddrExpr::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;
}

ExecutionObject* AddrExpr::generateSuif(Program* p)
{
  // I need to create a SymbolAddressExpression

  // Unless I'm calling a function?

  assert(type != NULL) ;
  assert(ops != NULL) ;
  assert(ops->size() >= 1) ;

  DataType* resultType = dynamic_cast<DataType*>(type->suifType(p)) ;

  assert(resultType != NULL) ;

  list<opOptions>::iterator symbolIter = ops->begin() ;
  
  Node* unknownSymbol = (*symbolIter).nodePointer ;  

  // This could be a function, or any other type of symbol (like just a 
  //  plain int).

  if (dynamic_cast<TargetExpr*>(unknownSymbol) != NULL)
  {
    return unknownSymbol->generateSuif(p) ;
  }

  Symbol* functionSymbol = unknownSymbol->getVariable(p) ;

  return p->converter->globalSuifObjfactory->create_symbol_address_expression(resultType, functionSymbol) ;


}

/********************* TruthAndIfExpr **************************/

TruthAndIfExpr::TruthAndIfExpr(Option* t, list<opOptions>* o) :
  Node(), typeop(t), ops(o)
{
  ;
}

TruthAndIfExpr::~TruthAndIfExpr()
{
  if (typeop != NULL)
    delete typeop;
  if (ops != NULL)
    delete ops;
}

void TruthAndIfExpr::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;
  
  if (typeop != NULL)
    typeop->connect(t);
  if (ops == NULL)
    return ;
  list<opOptions>::iterator connectIter = ops->begin();
  while (connectIter != ops->end())
  {
    (*connectIter).nodePointer=(*(t->allNodes))[(*connectIter).nodeNumber - 1];
    ++connectIter;
  }
}

void TruthAndIfExpr::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;
}

/********************* TruthAndExpr **************************/

TruthAndExpr::TruthAndExpr(Option* t, list<opOptions>* o) : 
  Node(), typeop(t), ops(o)
{
  ;
}

TruthAndExpr::~TruthAndExpr()
{
  if (typeop != NULL)
    delete typeop ;
  if (ops != NULL) 
    delete ops ;
}

void TruthAndExpr::connect(Function* t) 
{
  if (!connected)
    connected = true ;
  else
    return ;

  if (typeop != NULL)
    typeop->connect(t);
  if (ops == NULL)
    return ;
  list<opOptions>::iterator connectIter = ops->begin();
  while (connectIter != ops->end())
  {
    (*connectIter).nodePointer=(*(t->allNodes))[(*connectIter).nodeNumber - 1];
    ++connectIter;
  }
}

void TruthAndExpr::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;
}

ExecutionObject* TruthAndExpr::generateSuif(Program* p) 
{
  assert(ops != NULL) ;
  assert(ops->size() == 2) ;

  list<opOptions>::iterator opIter = ops->begin() ;
  Expression* leftHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;
  ++opIter ;
  Expression* rightHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;

  return p->converter->createBinaryExpression(BOP_BITWISE_AND, leftHandSide, rightHandSide) ;

}

ExecutionObject* TruthAndIfExpr::generateSuif(Program* p)
{
  assert(ops != NULL) ;
  assert(ops->size() == 2) ;

  list<opOptions>::iterator opIter = ops->begin() ;
  Expression* leftHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;
  ++opIter ;
  Expression* rightHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;

  return p->converter->createBinaryExpression(BOP_LOGICAL_AND, leftHandSide, rightHandSide) ;

}

/********************* TruthOrExpr **************************/

TruthOrExpr::TruthOrExpr(Option* t, list<opOptions>* o) : 
  Node(), typeop(t), ops(o)
{
  ;
}

TruthOrExpr::~TruthOrExpr()
{
  if (typeop != NULL)
    delete typeop ;
  if (ops != NULL) 
    delete ops ;
}

void TruthOrExpr::connect(Function* t) 
{
  if (!connected)
    connected = true ;
  else
    return ;

  if (typeop != NULL)
    typeop->connect(t);
  if (ops == NULL)
    return ;
  list<opOptions>::iterator connectIter = ops->begin();
  while (connectIter != ops->end())
  {
    (*connectIter).nodePointer=(*(t->allNodes))[(*connectIter).nodeNumber - 1];
    ++connectIter;
  }
}

void TruthOrExpr::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;
}

ExecutionObject* TruthOrExpr::generateSuif(Program* p) 
{
  assert(ops != NULL) ;
  assert(ops->size() == 2) ;

  list<opOptions>::iterator opIter = ops->begin() ;
  Expression* leftHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;
  ++opIter ;
  Expression* rightHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;

  return p->converter->createBinaryExpression(BOP_BITWISE_OR, leftHandSide, rightHandSide) ;

}


/********************* TruthOrIfExpr **************************/

TruthOrIfExpr::TruthOrIfExpr(Option* t, list<opOptions>* o) :
  Node(), typeop(t), ops(o)
{
  ;
}

TruthOrIfExpr::~TruthOrIfExpr()
{
  if (typeop != NULL)
    delete typeop;
  if (ops != NULL)
    delete ops;
}

void TruthOrIfExpr::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;

  if (typeop != NULL)
    typeop->connect(t);
  if (ops == NULL)
    return ;
  list<opOptions>::iterator connectIter = ops->begin();
  while (connectIter != ops->end())
  {
    (*connectIter).nodePointer=(*(t->allNodes))[(*connectIter).nodeNumber - 1];
    ++connectIter;
  }
}

void TruthOrIfExpr::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;
}

ExecutionObject* TruthOrIfExpr::generateSuif(Program* p)
{
  assert(ops != NULL) ;
  assert(ops->size() == 2) ;

  list<opOptions>::iterator opIter = ops->begin() ;
  Expression* leftHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;
  ++opIter ;
  Expression* rightHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;

  return p->converter->createBinaryExpression(BOP_LOGICAL_OR, leftHandSide, rightHandSide) ;
  
}

/********************* InitExpr **************************/

InitExpr::InitExpr(Option* t, list<opOptions>* o) : 
  Node(), typeop(t), ops(o)
{
  ;
}

InitExpr::~InitExpr()
{
  if (typeop != NULL)
    delete typeop;
  if (ops != NULL)
    delete ops;
}

void InitExpr::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;

  if (typeop != NULL)
    typeop->connect(t);
  if (ops == NULL)
    return ;
  list<opOptions>::iterator connectIter = ops->begin();
  while (connectIter != ops->end())
  {
    (*connectIter).nodePointer=(*(t->allNodes))[(*connectIter).nodeNumber - 1];
    ++connectIter;
  }
}

void InitExpr::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;
}

// I had to change this to work with SUIF
LString InitExpr::resultRegister()
{
  return "Not Yet Implemented" ;
}

ExecutionObject* InitExpr::generateSuif(Program* p)
{
  assert(ops != NULL) ;
  assert(ops->size() == 2) ;

  list<opOptions>::iterator opIter = ops->begin() ;
  //  Expression* leftHandSide = 
  //  dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;
  ++opIter ;
  //  Expression* rightHandSide = 
  //  dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;

  return (*opIter).nodePointer->generateSuif(p) ;

}

/********************* ModifyExpr **************************/

ModifyExpr::ModifyExpr(Option* t, list<opOptions>* o) :
  Node(), typeop(t), ops(o)
{
  ;
}

ModifyExpr::~ModifyExpr()
{
  if (typeop != NULL)
    delete typeop;
  if (ops != NULL)
    delete ops;
}

void ModifyExpr::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;

  if (typeop != NULL)
    typeop->connect(t);
  if (ops == NULL)
    return ;
  list<opOptions>::iterator connectIter = ops->begin();
  while (connectIter != ops->end())
  {
    (*connectIter).nodePointer=(*(t->allNodes))[(*connectIter).nodeNumber - 1];
    ++connectIter;
  }
}

void ModifyExpr::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;
}

// There is one special case of a modify expression that I have seen.
//  That is when we want to 

ExecutionObject* ModifyExpr::generateSuif(Program* p) 
{
  assert(ops != NULL) ;
  assert(ops->size() == 2) ;
  
  list<opOptions>::iterator modifyIter = ops->begin() ;
  
  Node* destination = (*modifyIter).nodePointer ;
  ++modifyIter ;
  Node* sourceExpr = (*modifyIter).nodePointer ;

  Expression* exprCode = dynamic_cast<Expression*>(sourceExpr->generateSuif(p)) ;

  // A modify expression is either going to store into an array or a 
  //  variable, and we must do different things for both cases.

  // Could it be going to something else?

  // Added parm decl to work with composable systems
  if (is_a<VarDecl*>(destination) || is_a<ParmDecl*>(destination))
  {
    // Figure out destination variable
    VariableSymbol* destinationVariable ;
  
    destinationVariable = dynamic_cast<VariableSymbol*>(destination->getVariable(p)) ;
    assert(destinationVariable != NULL) ;

    return p->converter->createStoreVariableStatement(destinationVariable,
						    exprCode) ;
  }
  else if (is_a<ArrayRef*>(destination))
  {
    Expression* arrayCode = dynamic_cast<Expression*>(destination->generateSuif(p)) ;
    assert(arrayCode != NULL) ;

    // I should have gotten a loadExpression back, so make sure
    assert(is_a<LoadExpression*>(arrayCode)) ;
    
    Expression* destAddress = dynamic_cast<LoadExpression*>(arrayCode)->get_source_address() ;
    dynamic_cast<LoadExpression*>(arrayCode)->set_source_address(NULL) ;
    
    return p->converter->globalSuifObjfactory->create_store_statement(exprCode, destAddress) ;
    
  }
  else if (is_a<IndirectRef*>(destination))
  {
    Expression* leftHandSide = dynamic_cast<Expression*>(destination->generateStore(p)) ;
    return p->converter->globalSuifObjfactory->create_store_statement(exprCode, leftHandSide);
  }
  else if (is_a<ComponentRef*>(destination))
  {
    Expression* leftHandSide = dynamic_cast<Expression*>(destination->generateSuif(p)) ;
    assert(leftHandSide != NULL) ;

    ExecutionObject* toReturn = p->converter->globalSuifObjfactory->create_store_statement(exprCode,leftHandSide) ;

    assert(toReturn != NULL) ;

    return toReturn ;

  }
  else if (is_a<ResultDecl*>(destination))
  {

    // I've seen this case in a stuct return-by-value scenario,
    //  so I'm only currently handling that case.  
    // This represents a temporary copy that needs to be created, so 
    //  I am going to try a temporary solution

    return exprCode ;

    //    assert(0) ;
    //    return NULL ;
  }
  else
  {
    assert(0) ; // This will prevent an error elsewhere
    return NULL ;
  }
}

/********************* CompoundExpr **************************/

CompoundExpr::CompoundExpr(Option* t, list<opOptions>* o) :
  Node(), typeop(t), ops(o)
{
  ;
}

CompoundExpr::~CompoundExpr()
{
  if (typeop != NULL)
    delete typeop;
  if (ops != NULL)
    delete ops;
}

void CompoundExpr::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;

  if (typeop != NULL)
    typeop->connect(t);
  if (ops == NULL)
    return ;
  list<opOptions>::iterator connectIter = ops->begin();
  while (connectIter != ops->end())
  {
    (*connectIter).nodePointer=(*(t->allNodes))[(*connectIter).nodeNumber - 1];
    ++connectIter;
  }
}

void CompoundExpr::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;
}

ExecutionObject* CompoundExpr::generateSuif(Program* p)
{
  // I am going to have to change this to a statement list, 
  //  and I'm not sure how Suif is going to handle this because
  //  it HAS to be an expression...

  // Apparently, the current suif to c conversion tool splits up the 
  //  comma expression into seperate statements, even going as far as
  //  extracting out expressions from the "middle" of other statements.

  // Compound expressions appear to be left-recursive in the gcc tree,
  //  so op 1 is always the a single expression and op 0 is
  //  everything to the left of the comma when multiple
  //  commas are encountered.

  // Currently, I know that an Eval Statement is the closest thing I 
  //  have to a compound expression.  It is a list of expressions to
  //  be evaluated and then ignored.  I will probably work off of 
  //  this concept and see what happens.

  // All right, here's the problem.  A comma operator can be treated 
  //  as an EvalStatement in Suif only if the operation is a statement
  //  (in other words, only in a for loop's initialization, step, etc).
  //  In all other cases, this has to be treated as an expression, 
  //  and I have to take all of the expressions (except the right-most
  //  one) and move them out to a time before I started processing this
  //  node.  Due to the recursive nature of the gcc abstract syntax tree
  //  (and everything I've implemented so far), I don't see this as possible
  //  without a whole lot of hacking and rewriting.

  return NULL ;
}

/********************* PreincrementExpr **************************/

PreincrementExpr::PreincrementExpr(Option* t, list<opOptions>* o) :
  Node(), typeop(t), ops(o)
{
  ;
}

PreincrementExpr::~PreincrementExpr()
{
  if (typeop != NULL)
    delete typeop;
  if (ops != NULL)
    delete ops;
}

void PreincrementExpr::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;

  if (typeop != NULL)
    typeop->connect(t);
  if (ops == NULL)
    return ;
  list<opOptions>::iterator connectIter = ops->begin();
  while (connectIter != ops->end())
  {
    (*connectIter).nodePointer=(*(t->allNodes))[(*connectIter).nodeNumber - 1];
    ++connectIter;
  }
}

void PreincrementExpr::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;
}

// I'm pretty sure the only things we'll ever be incrementing or decrementing 
//  are array refs, variable symbols, or indirect_refs, so I've got to 
//  make sure I can handle all of them.  Currently, I only handle variables
//  and probably seg fault on all the rest, so I have to do something about 
//  that...

ExecutionObject* PreincrementExpr::generateSuif(Program* p)
{
  assert(ops != NULL) ;
  assert(ops->size() == 2) ;

  // I must create a binary expression that adds the left hand side
  //  with the right hand side and then I must create a store variable
  //  statement that stores the variable away

  list<opOptions>::iterator opIter = ops->begin() ;
  Expression* leftHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;

  VariableSymbol* toStore = dynamic_cast<VariableSymbol*>((*opIter).nodePointer->getVariable(p)) ;
  ++opIter ;
  Expression* rightHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;

  Expression* inc =  p->converter->createBinaryExpression(BOP_ADD, leftHandSide, rightHandSide) ;

  return p->converter->createStoreVariableStatement(toStore , inc) ;

}

/********************* PostincrementExpr **************************/

PostincrementExpr::PostincrementExpr(Option* t, list<opOptions>* o) :
  Node(), typeop(t), ops(o) 
{
  ;
}

PostincrementExpr::~PostincrementExpr()
{
  if (typeop != NULL)
    delete typeop;
  if (ops != NULL)
    delete ops;
}

void PostincrementExpr::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;

  if (typeop != NULL)
    typeop->connect(t);
  if (ops == NULL)
    return ;
  list<opOptions>::iterator connectIter = ops->begin();
  while (connectIter != ops->end())
  {
    (*connectIter).nodePointer=(*(t->allNodes))[(*connectIter).nodeNumber - 1];
    ++connectIter;
  }
}

void PostincrementExpr::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;
}

// Currently this is just a cut-and-paste of the pre increment
//  expression.  This will need to be changed in the final version
//  to accurately reflect the differences between the two
ExecutionObject* PostincrementExpr::generateSuif(Program* p)
{
  assert(ops != NULL) ;
  assert(ops->size() == 2) ;

  // I must create a binary expression that adds the left hand side
  //  with the right hand side and then I must create a store variable
  //  statement that stores the variable away

  list<opOptions>::iterator opIter = ops->begin() ;
  Expression* leftHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;
  VariableSymbol* toStore = dynamic_cast<VariableSymbol*>((*opIter).nodePointer->getVariable(p)) ;
  ++opIter ;
  Expression* rightHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;

  Expression* inc =  p->converter->createBinaryExpression(BOP_ADD, leftHandSide, rightHandSide) ;

  return p->converter->createStoreVariableStatement(toStore , inc) ;

}

/********************* PredecrementExpr **************************/

PredecrementExpr::PredecrementExpr(Option* t, list<opOptions>* o) : Node(), 
								    typeop(t),
								    ops(o)
{
  type = NULL ;
}

PredecrementExpr::~PredecrementExpr()
{
  if (typeop != NULL)
    delete typeop;
  if (ops != NULL)
    delete ops;
}

void PredecrementExpr::connect(Function* t) 
{
  if (!connected)
    connected = true ;
  else
    return ;

  if (typeop != NULL)
    typeop->connect(t);
  if (ops == NULL)
    return ;
  list<opOptions>::iterator connectIter = ops->begin();
  while (connectIter != ops->end())
  {
    (*connectIter).nodePointer=(*(t->allNodes))[(*connectIter).nodeNumber - 1];
    ++connectIter;
  }
}

void PredecrementExpr::flatten()
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

// This function is very similar to the Preincrement expr...
//  so similar in fact that it might look just like a cut-paste :-)
ExecutionObject* PredecrementExpr::generateSuif(Program* p)
{
  assert(ops != NULL) ;
  assert(ops->size() == 2) ;

  // I must create a binary expression that subtracts the right hand side
  //  from the left hand side and then I must create a store variable
  //  statement that stores the variable away

  list<opOptions>::iterator opIter = ops->begin() ;
  Expression* leftHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;
  // This better be a variable symbol, or everything breaks...
  VariableSymbol* toStore = dynamic_cast<VariableSymbol*>((*opIter).nodePointer->getVariable(p)) ;
  ++opIter ;
  Expression* rightHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;

  Expression* inc =  p->converter->createBinaryExpression(BOP_SUB, leftHandSide, rightHandSide) ;

  return p->converter->createStoreVariableStatement(toStore , inc) ;

}

/********************* PostdecrementExpr **************************/

PostdecrementExpr::PostdecrementExpr(Option* t, list<opOptions>* o) 
  : Node(), typeop(t), ops(o)
{
  type = NULL ;
}

PostdecrementExpr::~PostdecrementExpr()
{
  if (typeop != NULL)
    delete typeop;
  if (ops != NULL)
    delete ops;
}

void PostdecrementExpr::connect(Function* t) 
{
  if (!connected)
    connected = true ;
  else
    return ;

  if (typeop != NULL)
    typeop->connect(t);
  if (ops == NULL)
    return ;
  list<opOptions>::iterator connectIter = ops->begin();
  while (connectIter != ops->end())
  {
    (*connectIter).nodePointer=(*(t->allNodes))[(*connectIter).nodeNumber - 1];
    ++connectIter;
  }
}

void PostdecrementExpr::flatten()
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

// This function is very similar to the Preincrement expr...
//  so similar in fact that it might look just like a cut-paste :-)
ExecutionObject* PostdecrementExpr::generateSuif(Program* p)
{
  assert(ops != NULL) ;
  assert(ops->size() == 2) ;

  // I must create a binary expression that subtracts the right hand side
  //  from the left hand side and then I must create a store variable
  //  statement that stores the variable away

  list<opOptions>::iterator opIter = ops->begin() ;
  Expression* leftHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;
  // This better be a variable symbol, or everything breaks...
  VariableSymbol* toStore = dynamic_cast<VariableSymbol*>((*opIter).nodePointer->getVariable(p)) ;
  ++opIter ;
  Expression* rightHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;

  Expression* inc =  p->converter->createBinaryExpression(BOP_SUB, leftHandSide, rightHandSide) ;

  return p->converter->createStoreVariableStatement(toStore , inc) ;

}

/********************* CondExpr **************************/

CondExpr::CondExpr(Option* t, list<opOptions>* o) :
  Node(), typeop(t), ops(o)
{
  ;
}

CondExpr::~CondExpr()
{
  if (typeop != NULL)
    delete typeop;
  if (ops != NULL)
    delete ops;
}

void CondExpr::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;

  if (typeop != NULL)
    typeop->connect(t);
  if (ops == NULL)
    return ;
  list<opOptions>::iterator connectIter = ops->begin();
  while (connectIter != ops->end())
  {
    (*connectIter).nodePointer=(*(t->allNodes))[(*connectIter).nodeNumber - 1];
    ++connectIter;
  }
}

void CondExpr::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;
}

ExecutionObject* CondExpr::generateSuif(Program* p)
{
  assert(ops != NULL) ;
  // Could have an else, or not
  assert(ops->size() == 3 || ops->size() == 2) ;

  Node* selectorNode ;
  Node* firstChoiceNode ;
  Node* secondChoiceNode ;

  list<opOptions>::iterator collectIter = ops->begin() ;
  selectorNode = (*collectIter).nodePointer ;
  ++collectIter ;
  firstChoiceNode = (*collectIter).nodePointer ;
  if (ops->size() == 3)
  {
    ++collectIter ;
    secondChoiceNode = (*collectIter).nodePointer ;
  }
  else
  {
    secondChoiceNode = NULL ;
  }

  Expression* selector = dynamic_cast<Expression*>(selectorNode->generateSuif(p)) ;

  ExecutionObject* tempTest = firstChoiceNode->generateSuif(p) ;
  assert(tempTest != NULL) ;

  //  Expression* selection1 = dynamic_cast<Expression*>(firstChoiceNode->generateSuif(p)) ;

  //Expression* selection1 = dynamic_cast<Expression*>(tempTest) ;

  Statement* selection1 = dynamic_cast<Statement*>(tempTest) ;

  //  Expression* selection2 = NULL ;
  Statement* selection2 = NULL ;

  if (secondChoiceNode != NULL)
  {
    selection2 = dynamic_cast<Statement*>(secondChoiceNode->generateSuif(p)) ;
    assert(selection2 != NULL) ;
  }
  assert(selector != NULL) ;
  assert(selection1 != NULL) ;
  //  assert(selection2 != NULL) ;

  //  return p->converter->globalSuifObjfactory->create_select_expression(selection1->get_result_type(), selector, selection1, selection2) ;

  return p->converter->globalSuifObjfactory->create_if_statement(selector,
								 selection1,
								 selection2) ;

}

/********************* CallExpr **************************/

CallExpr::CallExpr(Option* t, Option* f, Option* a) :
  Node(), typeop(t), fnop(f), argsop(a)
{
  ; 
}

CallExpr::~CallExpr()
{
  if (typeop != NULL)
    delete typeop;
  if (fnop != NULL)
    delete fnop;
  if (argsop != NULL)
    delete argsop;
}

void CallExpr::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;

  if (typeop != NULL)
    typeop->connect(t);
  if (fnop != NULL)
    fnop->connect(t);
  if (argsop != NULL)
    argsop->connect(t);
}

void CallExpr::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;

  if (fnop != NULL)
  {
    fn = fnop->theNodePointer() ;
    delete fnop ;
    fnop = NULL ;
  }
  else
    fn = NULL ;

  if (argsop != NULL)
  {
    args = argsop->theNodePointer() ;
    delete argsop ;
    argsop = NULL ;
  }
  else
    args = NULL ;
}

ExecutionObject* CallExpr::generateSuif(Program* p)
{
  assert(type != NULL) ;
  assert(fn != NULL) ;

  DataType* returnType = dynamic_cast<DataType*>(type->suifType(p)) ;

  assert(returnType != NULL) ;

  Expression* calleeAddress = dynamic_cast<Expression*>(fn->generateSuif(p)) ; 

  // In order to create a call expression in Suif, I need
  //  the return type and the address of the function we are
  //  calling

  // How do I get the arguments set up and passed in?

  CallExpression* lastCall = p->converter->globalCfeObjfactory->create_call_expression(returnType, calleeAddress) ;

  // Create a list of expressions for the arguments and append them
  //  to this function call.

  if (args != NULL)
  {
    list<Expression*> arguments = args->findArguments(p) ;
    list<Expression*>::iterator argIter = arguments.begin() ;
    //    bool first = true ;
    // I'm pretty sure this reverses the arguments, so we'll find out
    //  if this works soon enough.
    while(argIter != arguments.end())
    {
      // If I need a convert expression, I can put it here, but it doesn't 
      //  look like I need it right now (the code crashes the same way
      //  regardless...)
      //      if (first)
      //     {
      //	lastCall->append_argument(p->converter->createUnaryExpression(UOP_CONVERT,*argIter)) ;
      //	first = false;
      //  }
      // else
      // {
	lastCall->append_argument(*argIter) ;
	// }
      ++argIter ;
    }
  }

  return lastCall ;

}

/********************* StmtExpr **************************/

StmtExpr::StmtExpr(Option* t, Option* s) : 
  Node(), typeop(t), stmtop(s)
{
  ;
}

StmtExpr::~StmtExpr()
{
  if (typeop != NULL)
    delete typeop;
  if (stmtop != NULL)
    delete stmtop;
}

void StmtExpr::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;

  if (typeop != NULL)
    typeop->connect(t);
  if (stmtop != NULL)
    stmtop->connect(t);
}

void StmtExpr::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;

  if (stmtop != NULL)
  {
    stmt = stmtop->theNodePointer() ;
    delete stmtop ;
    stmtop = NULL ;
  }
  else
    stmt = NULL ;
}

/********************* TargetExpr **************************/

TargetExpr::TargetExpr(Option* t, Option* d, Option* i, Option* c) : 
  Node(), typeop(t), declop(d), initop(i), clnpop(c)
{
  ;
}

TargetExpr::~TargetExpr()
{
  if (typeop != NULL)
    delete typeop;
  if (declop != NULL)
    delete declop;
  if (initop != NULL)
    delete initop;
  if (clnpop != NULL)
    delete clnpop;
}

void TargetExpr::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;

  if (typeop != NULL)
    typeop->connect(t);
  if (declop != NULL)
    declop->connect(t);
  if (initop != NULL)
    initop->connect(t);
  if (clnpop != NULL)
    clnpop->connect(t);
}

void TargetExpr::flatten()
{  
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;

  if (declop != NULL)
  {
    decl = declop->theNodePointer() ;
    delete declop ;
    declop = NULL ;
  }
  else
    decl = NULL ;

  if (initop != NULL)
  {
    init = initop->theNodePointer() ;
    delete initop ;
    initop = NULL ;
  }
  else
    init = NULL ;

  if (clnpop != NULL)
  {
    clnp = clnpop->theNodePointer() ;
    delete clnpop ;
    clnpop = NULL ;
  }
  else
    clnp = NULL ;
}

ExecutionObject* TargetExpr::generateSuif(Program* p)
{
  assert(init != NULL) ;
  return init->generateSuif(p) ;
}

/********************* PlusExpr **************************/

PlusExpr::PlusExpr(Option* t, list<opOptions>* o) : 
  Node(), typeop(t), ops(o)
{
  ;
}

PlusExpr::~PlusExpr()
{
  if (typeop != NULL)
    delete typeop;
  if (ops != NULL)
    delete ops;
}

void PlusExpr::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;

  if (typeop != NULL)
    typeop->connect(t);
  if (ops == NULL)
    return ;
  list<opOptions>::iterator connectIter = ops->begin();
  while (connectIter != ops->end())
  {
    (*connectIter).nodePointer=(*(t->allNodes))[(*connectIter).nodeNumber - 1];
    ++connectIter;
  }
}

void PlusExpr::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;
}

ExecutionObject* PlusExpr::generateSuif(Program* p)
{
  assert(ops != NULL) ;
  assert(ops->size() == 2) ;

  list<opOptions>::iterator opIter = ops->begin() ;
  Expression* leftHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;
  ++opIter ;
  Expression* rightHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;

  return p->converter->createBinaryExpression(BOP_ADD, leftHandSide, rightHandSide) ;
  
}

/********************* MinusExpr **************************/

MinusExpr::MinusExpr(Option* t, list<opOptions>* o) : 
  Node(), typeop(t), ops(o)
{
  ;
}

MinusExpr::~MinusExpr()
{
  if (typeop != NULL)
    delete typeop;
  if (ops != NULL)
    delete ops;
}

void MinusExpr::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;

  if (typeop != NULL)
    typeop->connect(t);
  if (ops == NULL)
    return ;
  list<opOptions>::iterator connectIter = ops->begin();
  while (connectIter != ops->end())
  {
    (*connectIter).nodePointer=(*(t->allNodes))[(*connectIter).nodeNumber - 1];
    ++connectIter;
  }
}

void MinusExpr::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;
}

ExecutionObject* MinusExpr::generateSuif(Program* p)
{
  assert(ops != NULL) ;
  assert(ops->size() == 2) ;

  list<opOptions>::iterator opIter = ops->begin() ;
  Expression* leftHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;
  ++opIter ;
  Expression* rightHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;

  return p->converter->createBinaryExpression(BOP_SUB, leftHandSide, rightHandSide) ;  
}

/********************* MultExpr **************************/

MultExpr::MultExpr(Option* t, list<opOptions>* o) : 
  Node(), typeop(t), ops(o)
{
  ;
}

MultExpr::~MultExpr()
{
  if (typeop != NULL)
    delete typeop;
  if (ops != NULL)
    delete ops;
}

void MultExpr::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;

  if (typeop != NULL)
    typeop->connect(t);
  if (ops == NULL)
    return ;
  list<opOptions>::iterator connectIter = ops->begin();
  while (connectIter != ops->end())
  {
    (*connectIter).nodePointer=(*(t->allNodes))[(*connectIter).nodeNumber - 1];
    ++connectIter;
  }
}

void MultExpr::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;
}

ExecutionObject* MultExpr::generateSuif(Program* p)
{
  assert(ops != NULL) ;
  assert(ops->size() == 2) ;

  list<opOptions>::iterator opIter = ops->begin() ;
  Expression* leftHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;
  ++opIter ;
  Expression* rightHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;

  return p->converter->createBinaryExpression(BOP_MUL, leftHandSide, rightHandSide) ;
}

/********************* MaxExpr **************************/

MaxExpr::MaxExpr(Option* t, list<opOptions>* o) : 
  Node(), typeop(t), ops(o)
{
  ;
}

MaxExpr::~MaxExpr()
{
  if (typeop != NULL)
    delete typeop ;
  if (ops != NULL)
    delete ops ;
}

void MaxExpr::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;

  if (typeop != NULL)
    typeop->connect(t);
  if (ops == NULL)
    return ;
  list<opOptions>::iterator connectIter = ops->begin();
  while (connectIter != ops->end())
  {
    (*connectIter).nodePointer=(*(t->allNodes))[(*connectIter).nodeNumber - 1];
    ++connectIter;
  }
}

void MaxExpr::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;
}

/********************* LshiftExpr **************************/

LshiftExpr::LshiftExpr(Option* t, list<opOptions>* o) : 
  Node(), typeop(t), ops(o)
{
  ;
}

LshiftExpr::~LshiftExpr()
{
  if (typeop != NULL)
    delete typeop;
  if (ops != NULL)
    delete ops;
}

void LshiftExpr::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;
  
  if (typeop != NULL)
    typeop->connect(t);
  if (ops == NULL)
    return ;
  list<opOptions>::iterator connectIter = ops->begin();
  while (connectIter != ops->end())
  {
    (*connectIter).nodePointer=(*(t->allNodes))[(*connectIter).nodeNumber - 1];
    ++connectIter;
  }
}

void LshiftExpr::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;
}

ExecutionObject* LshiftExpr::generateSuif(Program* p)
{
  assert(ops != NULL) ;
  assert(ops->size() == 2) ;

  list<opOptions>::iterator opIter = ops->begin() ;
  Expression* leftHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;
  ++opIter ;
  Expression* rightHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;

  return p->converter->createBinaryExpression(BOP_LEFT_SHIFT, leftHandSide, rightHandSide) ;
  
}

/********************* RshiftExpr **************************/

RshiftExpr::RshiftExpr(Option* t, list<opOptions>* o) : 
  Node(), typeop(t), ops(o)
{
  ;
}

RshiftExpr::~RshiftExpr()
{
  if (typeop != NULL)
    delete typeop;
  if (ops != NULL)
    delete ops ;
}

void RshiftExpr::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;
  
  if (typeop != NULL)
    typeop->connect(t);
  if (ops == NULL)
    return ;
  list<opOptions>::iterator connectIter = ops->begin();
  while (connectIter != ops->end())
  {
    (*connectIter).nodePointer=(*(t->allNodes))[(*connectIter).nodeNumber - 1];
    ++connectIter;
  }
}

void RshiftExpr::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;
}

ExecutionObject* RshiftExpr::generateSuif(Program* p)
{
  assert(ops != NULL) ;
  assert(ops->size() == 2) ;

  list<opOptions>::iterator opIter = ops->begin() ;
  Expression* leftHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;
  ++opIter ;
  Expression* rightHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;

  return p->converter->createBinaryExpression(BOP_RIGHT_SHIFT, leftHandSide, rightHandSide) ;
  
}



/********************* BitIorExpr **************************/

BitIorExpr::BitIorExpr(Option* t, list<opOptions>* o) : 
  Node(), typeop(t), ops(o)
{
  ;
}

BitIorExpr::~BitIorExpr()
{
  if (typeop != NULL)
    delete typeop;
  if (ops != NULL)
    delete ops ;
}

void BitIorExpr::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;

  if (typeop != NULL)
    typeop->connect(t) ;
  if (ops == NULL)
    return ;
  list<opOptions>::iterator connectIter = ops->begin();
  while (connectIter != ops->end())
  {
    (*connectIter).nodePointer=(*(t->allNodes))[(*connectIter).nodeNumber - 1];
    ++connectIter;
  }
}

void BitIorExpr::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;
}

ExecutionObject* BitIorExpr::generateSuif(Program* p)
{
  assert(ops != NULL) ;
  assert(ops->size() == 2) ;

  list<opOptions>::iterator opIter = ops->begin() ;
  Expression* leftHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;
  ++opIter ;
  Expression* rightHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;

  return p->converter->createBinaryExpression(BOP_BITWISE_OR, leftHandSide, rightHandSide) ;
  
}

/********************* BitXorExpr **************************/

BitXorExpr::BitXorExpr(Option* t, list<opOptions>* o) : 
  Node(), typeop(t), ops(o)
{
  ;
}

BitXorExpr::~BitXorExpr()
{
  if (typeop != NULL)
    delete typeop;
  if (ops != NULL)
    delete ops;
}

void BitXorExpr::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;

  if (typeop != NULL)
    typeop->connect(t);
  if (ops == NULL)
    return ;
  list<opOptions>::iterator connectIter = ops->begin();
  while (connectIter != ops->end())
  {
    (*connectIter).nodePointer=(*(t->allNodes))[(*connectIter).nodeNumber - 1];
    ++connectIter;
  }
}

void BitXorExpr::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;
}

ExecutionObject* BitXorExpr::generateSuif(Program* p)
{
  assert(ops != NULL) ;
  assert(ops->size() == 2) ;

  list<opOptions>::iterator opIter = ops->begin() ;
  Expression* leftHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;
  ++opIter ;
  Expression* rightHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;

  return p->converter->createBinaryExpression(BOP_BITWISE_XOR, leftHandSide, rightHandSide) ;
  
}

/********************* BitAndExpr **************************/

BitAndExpr::BitAndExpr(Option* t, list<opOptions>* o) :
  Node(), typeop(t), ops(o)
{
  ;
}

BitAndExpr::~BitAndExpr()
{
  if (typeop != NULL)
    delete typeop;
  if (ops != NULL)
    delete ops;
}

void BitAndExpr::connect(Function* t)
{
  if (!connected) 
    connected = true ;
  else
    return ;

  if (typeop != NULL)
    typeop->connect(t);
  if (ops == NULL)
    return ;
  list<opOptions>::iterator connectIter = ops->begin();
  while (connectIter != ops->end())
  {
    (*connectIter).nodePointer=(*(t->allNodes))[(*connectIter).nodeNumber - 1];
    ++connectIter;
  }
}

void BitAndExpr::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;
}

ExecutionObject* BitAndExpr::generateSuif(Program* p)
{
  assert(ops != NULL) ;
  assert(ops->size() == 2) ;

  list<opOptions>::iterator opIter = ops->begin() ;
  Expression* leftHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;
  ++opIter ;
  Expression* rightHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;

  return p->converter->createBinaryExpression(BOP_BITWISE_AND, leftHandSide, rightHandSide) ;
  
}

/********************* BitNotExpr **************************/

BitNotExpr::BitNotExpr(Option* t, list<opOptions>* o) : 
  Node(), typeop(t), ops(o)
{
  ;
}

BitNotExpr::~BitNotExpr()
{
  if (typeop != NULL)
    delete typeop;
  if (ops != NULL)
    delete ops;
}

void BitNotExpr::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;

  if (typeop != NULL)
    typeop->connect(t);
  if (ops == NULL)
    return ;
  list<opOptions>::iterator connectIter = ops->begin();
  while (connectIter != ops->end())
  {
    (*connectIter).nodePointer=(*(t->allNodes))[(*connectIter).nodeNumber - 1];
    ++connectIter;
  }
}

void BitNotExpr::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;
}

ExecutionObject* BitNotExpr::generateSuif(Program* p)
{

  // UOP_INVERT
  assert(ops != NULL) ;
  assert(ops->size() == 1) ;

  list<opOptions>::iterator findIt = ops->begin() ;

  return p->converter->createUnaryExpression(UOP_INVERT,
					     dynamic_cast<Expression*>((*findIt).nodePointer->generateSuif(p))) ;


}

/********************* LtExpr **************************/

LtExpr::LtExpr(Option* t, list<opOptions>* o) : 
  Node(), typeop(t), ops(o)
{
  ;
}


LtExpr::~LtExpr()
{
  if (typeop != NULL)
    delete typeop;
  if (ops != NULL)
    delete ops;
}

void LtExpr::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;

  if (typeop != NULL)
    typeop->connect(t);
  if (ops == NULL)
    return ;
  list<opOptions>::iterator connectIter = ops->begin();
  while (connectIter != ops->end())
  {
    (*connectIter).nodePointer=(*(t->allNodes))[(*connectIter).nodeNumber - 1];
    ++connectIter;
  }
}

void LtExpr::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;
}

ExecutionObject* LtExpr::generateSuif(Program* p)
{
  assert(ops != NULL) ;
  assert(ops->size() == 2) ;

  list<opOptions>::iterator opIter = ops->begin() ;
  Expression* leftHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;
  ++opIter ;
  Expression* rightHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;

  return p->converter->createBinaryExpression(BOP_IS_LESS_THAN, leftHandSide, rightHandSide) ;
  
}

/********************* LeExpr **************************/

LeExpr::LeExpr(Option* t, list<opOptions>* o) : 
  Node(), typeop(t), ops(o)
{
  ;
}

LeExpr::~LeExpr()
{
  if (typeop != NULL)
    delete typeop;
  if (ops != NULL)
    delete ops;
}

void LeExpr::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;

  if (typeop != NULL)
    typeop->connect(t);
  if (ops == NULL)
    return ;
  list<opOptions>::iterator connectIter = ops->begin();
  while (connectIter != ops->end())
  {
    (*connectIter).nodePointer=(*(t->allNodes))[(*connectIter).nodeNumber - 1];
    ++connectIter;
  }
}

void LeExpr::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;
}

ExecutionObject* LeExpr::generateSuif(Program* p)
{
  assert(ops != NULL) ;
  assert(ops->size() == 2) ;

  list<opOptions>::iterator opIter = ops->begin() ;
  Expression* leftHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;
  ++opIter ;
  Expression* rightHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;

  return p->converter->createBinaryExpression(BOP_IS_LESS_THAN_OR_EQUAL_TO, leftHandSide, rightHandSide) ;

}

/********************* GeExpr **************************/

GeExpr::GeExpr(Option* t, list<opOptions>* o) : 
  Node(), typeop(t), ops(o)
{
  ;
}

GeExpr::~GeExpr()
{
  if (typeop != NULL)
    delete typeop;
  if (ops != NULL)
    delete ops;
}

void GeExpr::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;

  if (typeop != NULL)
    typeop->connect(t);
  if (ops == NULL)
    return ;
  list<opOptions>::iterator connectIter = ops->begin();
  while (connectIter != ops->end())
  {
    (*connectIter).nodePointer=(*(t->allNodes))[(*connectIter).nodeNumber - 1];
    ++connectIter;
  }
}

void GeExpr::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;
}

ExecutionObject* GeExpr::generateSuif(Program* p)
{
  assert(ops != NULL) ;
  assert(ops->size() == 2) ;

  list<opOptions>::iterator opIter = ops->begin() ;
  Expression* leftHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;
  ++opIter ;
  Expression* rightHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;

  return p->converter->createBinaryExpression(BOP_IS_GREATER_THAN_OR_EQUAL_TO, leftHandSide, rightHandSide) ;

}

/********************* GtExpr **************************/

GtExpr::GtExpr(Option* t, list<opOptions>* o) : 
  Node(), typeop(t), ops(o)
{
  ;
}

GtExpr::~GtExpr()
{
  if (typeop != NULL)
    delete typeop;
  if (ops != NULL)
    delete ops;
}

void GtExpr::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;

  if (typeop != NULL)
    typeop->connect(t);
  if (ops == NULL)
    return ;
  list<opOptions>::iterator connectIter = ops->begin();
  while (connectIter != ops->end())
  {
    (*connectIter).nodePointer=(*(t->allNodes))[(*connectIter).nodeNumber - 1];
    ++connectIter;
  }
}

void GtExpr::flatten()
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

ExecutionObject* GtExpr::generateSuif(Program* p)
{
  assert(ops != NULL) ;
  assert(ops->size() == 2) ;

  list<opOptions>::iterator opIter = ops->begin() ;
  Expression* leftHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;
  ++opIter ;
  Expression* rightHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;

  return p->converter->createBinaryExpression(BOP_IS_GREATER_THAN, leftHandSide, rightHandSide) ;
  
}

/********************* EqExpr **************************/

EqExpr::EqExpr(Option* t, list<opOptions>* o) : 
  Node(), typeop(t), ops(o)
{
  ;
}

EqExpr::~EqExpr()
{
  if (typeop != NULL)
    delete typeop;
  if (ops != NULL)
    delete ops;
}

void EqExpr::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;

  if (typeop != NULL)
    typeop->connect(t);
  if (ops == NULL)
    return ;
  list<opOptions>::iterator connectIter = ops->begin();
  while (connectIter != ops->end())
  {
    (*connectIter).nodePointer=(*(t->allNodes))[(*connectIter).nodeNumber - 1];
    ++connectIter;
  }
}

void EqExpr::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;
}

ExecutionObject* EqExpr::generateSuif(Program* p)
{
  assert(ops != NULL) ;
  assert(ops->size() == 2) ;

  list<opOptions>::iterator opIter = ops->begin() ;
  Expression* leftHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;
  ++opIter ;
  Expression* rightHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;

  return p->converter->createBinaryExpression(BOP_IS_EQUAL_TO, leftHandSide, rightHandSide) ;
  
}

/********************* NeExpr **************************/

NeExpr::NeExpr(Option* t, list<opOptions>* o) : 
  Node(), typeop(t), ops(o)
{
  ;
}

NeExpr::~NeExpr()
{
  if (typeop != NULL)
    delete typeop;
  if (ops != NULL)
    delete ops;
}

void NeExpr::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;

  if (typeop != NULL)
    typeop->connect(t);
  if (ops == NULL)
    return ;
  list<opOptions>::iterator connectIter = ops->begin();
  while (connectIter != ops->end())
  {
    (*connectIter).nodePointer=(*(t->allNodes))[(*connectIter).nodeNumber - 1];
    ++connectIter;
  }
}

void NeExpr::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;
}

ExecutionObject* NeExpr::generateSuif(Program* p)
{
  assert(ops != NULL) ;
  assert(ops->size() == 2) ;

  list<opOptions>::iterator opIter = ops->begin() ;
  Expression* leftHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;
  ++opIter ;
  Expression* rightHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;

  return p->converter->createBinaryExpression(BOP_IS_NOT_EQUAL_TO, leftHandSide, rightHandSide) ;
  
}

/********************* ConvertExpr **************************/

ConvertExpr::ConvertExpr(Option* t, list<opOptions>* o) :
  Node(), typeop(t), ops(o)
{
  ;
}

ConvertExpr::~ConvertExpr()
{
  if (typeop != NULL)
    delete typeop;
  if (ops != NULL)
    delete ops;
}

void ConvertExpr::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else 
    return ;

  if (typeop != NULL)
    typeop->connect(t);
  if (ops == NULL)
    return ;
  list<opOptions>::iterator connectIter = ops->begin();
  while (connectIter != ops->end())
  {
    (*connectIter).nodePointer=(*(t->allNodes))[(*connectIter).nodeNumber - 1];
    ++connectIter;
  }
}

void ConvertExpr::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer();
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;
}

ExecutionObject* ConvertExpr::generateSuif(Program* p)
{
  assert(ops != NULL) ;
  assert(ops->size() == 1) ;
  
  return (*(ops->begin())).nodePointer->generateSuif(p) ;
}

/********************* NopExpr **************************/

NopExpr::NopExpr(Option* t, list<opOptions>* o) : 
  Node(), typeop(t), ops(o)
{
  ;
}

NopExpr::~NopExpr()
{
  if (typeop != NULL)
    delete typeop;
  if (ops != NULL)
    delete ops;
}

void NopExpr::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;

  if (typeop != NULL)
    typeop->connect(t);
  if (ops == NULL)
    return ;
  list<opOptions>::iterator connectIter = ops->begin();
  while (connectIter != ops->end())
  {
    (*connectIter).nodePointer=(*(t->allNodes))[(*connectIter).nodeNumber - 1];
    ++connectIter;
  }
}

void NopExpr::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer();
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;
}

LString NopExpr::resultRegister()
{
  if (ops == NULL)
    return Node::resultRegister();
  list<opOptions>::iterator opIter = ops->begin();
  return (*opIter).nodePointer->resultRegister() ;
}

ExecutionObject* NopExpr::generateSuif(Program* p)
{
  assert(ops != NULL) ;
  assert(ops->size() == 1) ;

  list<opOptions>::iterator genIter = ops->begin() ;

  return (*genIter).nodePointer->generateSuif(p) ;
}

/********************* NonLvalueExpr **************************/

NonLvalueExpr::NonLvalueExpr(Option* t, list<opOptions>* o) : 
  Node(), typeop(t), ops(o)
{
  ;
}

NonLvalueExpr::~NonLvalueExpr()
{
  if (typeop != NULL)
    delete typeop;
  if (ops != NULL)
    delete ops;
}


void NonLvalueExpr::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;

  if (typeop != NULL)
    typeop->connect(t);
  if (ops == NULL)
    return ;
  list<opOptions>::iterator connectIter = ops->begin();
  while (connectIter != ops->end())
  {
    (*connectIter).nodePointer=(*(t->allNodes))[(*connectIter).nodeNumber - 1];
    ++connectIter;
  }
}

void NonLvalueExpr::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;
}

ExecutionObject* NonLvalueExpr::generateSuif(Program* p)
{
  assert(ops != NULL) ;
  assert(ops->size() == 1) ;

  list<opOptions>::iterator goThrough = ops->begin() ;

  ExecutionObject* inner = (*goThrough).nodePointer->generateSuif(p) ;
  Expression* innerExp = dynamic_cast<Expression*>(inner) ;
  assert(innerExp != NULL) ;

  return p->converter->createNonLvalueExpression(innerExp) ;
}

/********************* SaveExpr **************************/

SaveExpr::SaveExpr(Option* t, list<opOptions>* o) : 
  Node(), typeop(t), ops(o)
{
  ;
}

SaveExpr::~SaveExpr()
{
  if (typeop != NULL)
    delete typeop;
  if (ops != NULL)
    delete ops;
}

void SaveExpr::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;
  
  if (typeop != NULL)
    typeop->connect(t);
  if (ops == NULL)
    return ;
  list<opOptions>::iterator connectIter = ops->begin();
  while (connectIter != ops->end())
  {
    (*connectIter).nodePointer=(*(t->allNodes))[(*connectIter).nodeNumber - 1];
    ++connectIter;
  }
}

void SaveExpr::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;
}

/********************* BindExpr **************************/

BindExpr::BindExpr(Option* t, Option* v, Option* b) : Node(), typeop(t), 
						      varsop(v), bodyop(b)
{
  type = NULL ;
  vars = NULL ;
  body = NULL ;
}

BindExpr::~BindExpr()
{
  if (typeop != NULL)
    delete typeop ;
  if (varsop != NULL)
    delete varsop ;
  if (bodyop != NULL)
    delete bodyop ;
}

void BindExpr::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;
  if (typeop != NULL)
    typeop->connect(t) ;
  if (varsop != NULL)
    varsop->connect(t) ;
  if (bodyop != NULL)
    bodyop->connect(t) ;
}

void BindExpr::flatten()
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
  if (varsop != NULL)
  {
    vars = varsop->theNodePointer() ;
    delete varsop ;
    varsop = NULL ;
  }
  else
  {
    vars = NULL ;
  }
  if (bodyop != NULL)
  {
    body = bodyop->theNodePointer() ;
    delete bodyop ;
    bodyop = NULL ;
  }
  else
  {
    body = NULL ;
  }
}

ExecutionObject* BindExpr::generateSuif(Program* p)
{
  ExecutionObject* bodyCode = NULL ;
  if (body != NULL)
  {
    bodyCode = body->generateSuif(p) ;
  }

  if (vars != NULL)
  {
    assert(dynamic_cast<StatementList*>(bodyCode) != NULL) ;

    // Go through all of the vars and attach the pointer to the statement list
    //  that is thier scope.  I'll change the initialization code to be
    //  a store variable statement at the front of that list in the suifpasses.
    VarDecl* currentDecl = dynamic_cast<VarDecl*>(vars) ;
    while (currentDecl != NULL)
    {
      // Create a new annotation and attach to the current variable symbol
      Symbol* currentSym = currentDecl->getVariable(p) ;
      BrickAnnote* pointAnnote = p->converter->globalBasicObjfactory->
	create_brick_annote(LString("InitializationPoint")) ;
      SuifObjectBrick* sobBrick = p->converter->globalBasicObjfactory->
	create_suif_object_brick(bodyCode) ;
      pointAnnote->append_brick(sobBrick) ;
      currentSym->append_annote(pointAnnote) ;

      currentDecl = currentDecl->getNextVariable() ;
    }
  }

  return bodyCode ;
}

/********************* ReturnExpr **************************/

ReturnExpr::ReturnExpr(Option* t, Option* e) : Node(), typeop(t), exprop(e)
{
  type = NULL ;
  expr = NULL ;
}

ReturnExpr::~ReturnExpr()
{
  if (typeop != NULL)
    delete typeop ;
  if (exprop != NULL)
    delete exprop ;
}

void ReturnExpr::connect(Function* t) 
{
  if (typeop != NULL)
    typeop->connect(t) ;
  if (exprop != NULL)
    exprop->connect(t) ;
}

void ReturnExpr::flatten()
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
  if (exprop != NULL)
  {
    expr = exprop->theNodePointer() ;
    delete exprop ;
    exprop = NULL ;
  }
  else
  {
    expr = NULL ;
  }
}

// Now that the return expression works, I need to create a return expression
//  in suif
ExecutionObject* ReturnExpr::generateSuif(Program* p)
{
  if (expr != NULL)
  {
    ExecutionObject* toReturn = expr->generateSuif(p) ;
    return p->converter->globalSuifObjfactory->create_return_statement(dynamic_cast<Expression*>(toReturn)) ;
  }
  else
  {
    return NULL ;
  }
}

/********************* CleanupPointExpr **************************/

CleanupPointExpr::CleanupPointExpr(Option* t, list<opOptions>* o) : Node(),
								    typeop(t),
								    ops(o)
{
  type = NULL ;
}

CleanupPointExpr::~CleanupPointExpr()
{
  if (typeop != NULL)
    delete typeop ;
  if (ops != NULL)
    delete ops ;
}

void CleanupPointExpr::connect(Function* t)
{
  if (typeop != NULL)
    typeop->connect(t) ;
  if (ops == NULL)
    return ;
  list<opOptions>::iterator connectIter = ops->begin();
  while (connectIter != ops->end())
  {
    (*connectIter).nodePointer=(*(t->allNodes))[(*connectIter).nodeNumber - 1];
    ++connectIter;
  }
}

void CleanupPointExpr::flatten()
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

ExecutionObject* CleanupPointExpr::generateSuif(Program* p)
{

  assert (ops != NULL) ;
  assert (ops->size() == 1) ;

  // Generate a new scope here?

  list<opOptions>::iterator suifIter = ops->begin() ;

  return (*suifIter).nodePointer->generateSuif(p) ;
}

/********************* DeclExpr **************************/

DeclExpr::DeclExpr(Option* t) : Node(), typeop(t)
{
  type = NULL ;
}

DeclExpr::~DeclExpr()
{
  if (typeop != NULL)
    delete typeop ;
}

void DeclExpr::connect(Function* t) 
{
  if (!connected)
  {
    connected = true ;
  }
  else
  {
    return ;
  }
  if (typeop != NULL)
    typeop->connect(t) ;
}

void DeclExpr::flatten()
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

ExecutionObject* DeclExpr::generateSuif(Program* p)
{
  // I don't think I need to do anything here.
  return NULL ;
}

/********************* TruncDivExpr **************************/

TruncDivExpr::TruncDivExpr(Option* t, list<opOptions>* o) : Node(), typeop(t),
							    ops(o)
{
  ;
}

TruncDivExpr::~TruncDivExpr()
{
  if (typeop != NULL)
    delete typeop ;
  if (ops != NULL)
    delete ops ;
}

void TruncDivExpr::connect(Function* t) 
{
  if (!connected)
    connected = true ;
  else
    return ;

  if (typeop != NULL)
    typeop->connect(t);
  if (ops == NULL)
    return ;
  list<opOptions>::iterator connectIter = ops->begin();
  while (connectIter != ops->end())
  {
    (*connectIter).nodePointer=(*(t->allNodes))[(*connectIter).nodeNumber - 1];
    ++connectIter;
  }
}

void TruncDivExpr::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;
}

ExecutionObject* TruncDivExpr::generateSuif(Program* p)
{
  assert(ops != NULL) ;
  assert(ops->size() == 2) ;

  list<opOptions>::iterator opIter = ops->begin() ;
  Expression* leftHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;
  ++opIter ;
  Expression* rightHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;

  return p->converter->createBinaryExpression(BOP_DIV, leftHandSide, rightHandSide) ;
}

/********************* RdivExpr **************************/

RdivExpr::RdivExpr(Option* t, list<opOptions>* o) : Node(), typeop(t),
						    ops(o)
{
  ;
}

RdivExpr::~RdivExpr()
{
  if (typeop != NULL)
    delete typeop ;
  if (ops != NULL)
    delete ops ;
}

void RdivExpr::connect(Function* t) 
{
  if (!connected)
    connected = true ;
  else
    return ;

  if (typeop != NULL)
    typeop->connect(t);
  if (ops == NULL)
    return ;
  list<opOptions>::iterator connectIter = ops->begin();
  while (connectIter != ops->end())
  {
    (*connectIter).nodePointer=(*(t->allNodes))[(*connectIter).nodeNumber - 1];
    ++connectIter;
  }
}

void RdivExpr::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;
}

// I don't think suif differentiates between integer division and
//  real division, but if it does I'm going to have to change this.
ExecutionObject* RdivExpr::generateSuif(Program* p)
{
  assert(ops != NULL) ;
  assert(ops->size() == 2) ;

  list<opOptions>::iterator opIter = ops->begin() ;
  Expression* leftHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;
  ++opIter ;
  Expression* rightHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;

  return p->converter->createBinaryExpression(BOP_DIV, leftHandSide, rightHandSide) ;
}

/********************* TruncModExpr **************************/

TruncModExpr::TruncModExpr(Option* t, list<opOptions>* o) : Node(), typeop(t),
							    ops(o)
{
  ;
}

TruncModExpr::~TruncModExpr()
{
  if (typeop != NULL)
    delete typeop ;
  if (ops != NULL)
    delete ops ;
}

void TruncModExpr::connect(Function* t) 
{
  if (!connected)
    connected = true ;
  else
    return ;

  if (typeop != NULL)
    typeop->connect(t);
  if (ops == NULL)
    return ;
  list<opOptions>::iterator connectIter = ops->begin();
  while (connectIter != ops->end())
  {
    (*connectIter).nodePointer=(*(t->allNodes))[(*connectIter).nodeNumber - 1];
    ++connectIter;
  }
}

void TruncModExpr::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;
}

ExecutionObject* TruncModExpr::generateSuif(Program* p)
{
  assert(ops != NULL) ;
  assert(ops->size() == 2) ;

  list<opOptions>::iterator opIter = ops->begin() ;
  Expression* leftHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;
  ++opIter ;
  Expression* rightHandSide = 
    dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;

  return p->converter->createBinaryExpression(BOP_MOD, leftHandSide, rightHandSide) ;
}

/********************* LabelExpr **************************/

LabelExpr::LabelExpr(Option* t, Option* n) : Node(), typeop(t), nameop(n)
{
  ;
}

LabelExpr::~LabelExpr()
{
  if (typeop != NULL)
    delete typeop ;
  if (nameop != NULL)
    delete nameop ;
}

void LabelExpr::connect(Function* t)
{
  if (!connected)
  {
    connected = true ;
  }
  else
  {
    return ;
  }
  if (typeop != NULL)
    typeop->connect(t) ;
  if (nameop != NULL)
    nameop->connect(t) ;  
}

void LabelExpr::flatten()
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
  if (nameop != NULL)
  {
    name = nameop->theNodePointer() ;
    delete nameop ;
    nameop = NULL ;
  }
  else
  {
    name = NULL ;
  }
}

ExecutionObject* LabelExpr::generateSuif(Program* p)
{
  assert(name != NULL) ;
  
  return p->converter->globalSuifObjfactory->create_label_location_statement(dynamic_cast<CodeLabelSymbol*>(name->getVariable(p))) ;
}

/********************* GotoExpr ****************************/

GotoExpr::GotoExpr(Option* t, Option* l) : Node(), typeop(t), lablop(l)
{
  type = NULL ;
  labl = NULL;
} 

GotoExpr::~GotoExpr()
{
  if (typeop != NULL)
    delete typeop ;
  if (lablop != NULL)
    delete lablop ;
}

void GotoExpr::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else 
    return ;

  if (typeop != NULL)
    typeop->connect(t) ;
  if (lablop != NULL)
    lablop->connect(t) ;

}

void GotoExpr::flatten()
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

  if (lablop != NULL)
  {
    labl = lablop->theNodePointer() ;
    delete lablop ;
    lablop = NULL ;
  }
  else
    labl = NULL ;
}

ExecutionObject* GotoExpr::generateSuif(Program* p)
{
  // Causes issues right now

  Symbol* convertMe = labl->getVariable(p) ;
  assert(convertMe != NULL) ;
  assert(dynamic_cast<CodeLabelSymbol*>(convertMe) != NULL) ;

  return p->converter->globalSuifObjfactory->
    create_jump_statement(dynamic_cast<CodeLabelSymbol*>(convertMe)) ;
}

/********************* NegateExpr **************************/

NegateExpr::NegateExpr(Option* t, list<opOptions>* o) : Node(), typeop(t), 
							ops(o)
{
  type = NULL ;
}

NegateExpr::~NegateExpr()
{
  if(typeop != NULL)
    delete typeop ;
  if (ops != NULL)
    delete ops ;    
}

void NegateExpr::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;

  if (typeop != NULL)
    typeop->connect(t);
  if (ops == NULL)
    return ;
  list<opOptions>::iterator connectIter = ops->begin();
  while (connectIter != ops->end())
  {
    (*connectIter).nodePointer=(*(t->allNodes))[(*connectIter).nodeNumber - 1];
    ++connectIter;
  }
}

void NegateExpr::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;  
}

ExecutionObject* NegateExpr::generateSuif(Program* p)
{
  assert(ops != NULL) ;
  assert(ops->size() == 1) ;

  list<opOptions>::iterator negateMe = ops->begin() ;

  Expression* toNegate = dynamic_cast<Expression*>((*negateMe).nodePointer->generateSuif(p)) ;

  return p->converter->createUnaryExpression(UOP_NEGATE, toNegate) ;

}

/********************* FixTruncExpr **************************/

FixTruncExpr::FixTruncExpr(Option* t, list<opOptions>* o) : Node(), typeop(t),
							    ops(o)
{
  ;
}

FixTruncExpr::~FixTruncExpr()
{
  if (typeop != NULL)
    delete typeop ;
  if (ops != NULL)
    delete ops ;
}

void FixTruncExpr::connect(Function* t) 
{
  if (!connected)
    connected = true ;
  else
    return ;

  if (typeop != NULL)
    typeop->connect(t);
  if (ops == NULL)
    return ;
  list<opOptions>::iterator connectIter = ops->begin();
  while (connectIter != ops->end())
  {
    (*connectIter).nodePointer=(*(t->allNodes))[(*connectIter).nodeNumber - 1];
    ++connectIter;
  }
}

void FixTruncExpr::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;
}

ExecutionObject* FixTruncExpr::generateSuif(Program* p)
{
  assert(ops != NULL) ;
  assert(ops->size() == 1) ;

  list<opOptions>::iterator opIter = ops->begin() ;

  return (*opIter).nodePointer->generateSuif(p) ;
}

/********************* CaseLabelExpr **************************/

CaseLabelExpr::CaseLabelExpr(Option* t, Option* l) : typeop(t), lownodeop(l)
{
  type = NULL ;
  lownode = NULL ;
  isDefault = false ;
  mySuifSymbol = NULL ;
}

CaseLabelExpr::~CaseLabelExpr()
{
  if (typeop != NULL)
    delete typeop ;
  if (lownodeop != NULL)
    delete lownodeop ;
}

void CaseLabelExpr::connect(Function* t) 
{
  if (!connected)
  {
    connected = true ;
  }
  else
  {
    return ;
  }
  if (typeop != NULL)
    typeop->connect(t);
  if (lownodeop != NULL)
    lownodeop->connect(t) ;
}

void CaseLabelExpr::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;
  if (lownodeop != NULL)
  {
    lownode = lownodeop->theNodePointer() ;
    delete lownodeop ;
    lownodeop = NULL ;
  }
  else
  {
    lownode = NULL ;
    isDefault = true ;
  }
}

ExecutionObject* CaseLabelExpr::generateSuif(Program* p)
{
  if (isDefault)
  {
    mySuifSymbol = p->converter->globalBasicObjfactory->
      create_code_label_symbol(p->converter->globalTb->get_label_type(), 
			       tempName()) ;

    p->converter->addLabelSymbolToCurrentSymbolTable(mySuifSymbol) ;

  } 
  else
  {
    mySuifSymbol = p->converter->globalBasicObjfactory->
      create_code_label_symbol(p->converter->globalTb->get_label_type(), "") ;

    p->converter->addLabelSymbolToCurrentSymbolTable(mySuifSymbol) ;
  }

  return p->converter->globalSuifObjfactory->
    create_label_location_statement(mySuifSymbol) ;
}

int CaseLabelExpr::theSize()
{
  if (lownode != NULL)
    return lownode->theSize() ;
  else
    return 0 ;
}

CodeLabelSymbol* CaseLabelExpr::theSuifSymbol()
{
  return mySuifSymbol ;
}

/********************* FloatExpr **************************/

FloatExpr::FloatExpr(Option* t, list<opOptions>* o) : Node(), typeop(t),
						      ops(o)
{
  type = NULL ;
}

FloatExpr::~FloatExpr()
{
  if (typeop != NULL)
    delete typeop ;
  if (ops != NULL)
    delete ops ;
}

void FloatExpr::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;

  if (typeop != NULL)
    typeop->connect(t);
  if (ops == NULL)
    return ;
  list<opOptions>::iterator connectIter = ops->begin();
  while (connectIter != ops->end())
  {
    (*connectIter).nodePointer=(*(t->allNodes))[(*connectIter).nodeNumber - 1];
    ++connectIter;
  }
}

void FloatExpr::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
    type = NULL ;
}

ExecutionObject* FloatExpr::generateSuif(Program* p)
{
  assert(ops != NULL) ;
  assert(ops->size() == 1) ;

  list<opOptions>::iterator opIter = ops->begin() ;

  Expression* result = dynamic_cast<Expression*>((*opIter).nodePointer->generateSuif(p)) ;

  return p->converter->createCastExpression(result, p->converter->globalTb->get_floating_point_type(32, 32)) ;

}
