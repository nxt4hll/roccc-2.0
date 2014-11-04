/*

  This file contains the definitions for all of the Node base class 
   functions and virtual functions.  Most of these functions don't do anything 
   and are here only to make sure every node has some functionality.

*/

// Included for NULL
#include <cstdio>

#include "baseNode.h"

// When initializing a Node, we must make sure that the connected field
//  is false.
Node::Node()
{
  connected = false ;
  number = -1 ;
}

int Node:: getNumber()
{
  return number ;
}

void Node::setNumber(int newNumber)
{
  number = newNumber;
}

Node::~Node()
{
  // Nothing to destroy at this level
  ;
}

void Node::connect(Function* t)
{
  return ;
}

void Node::flatten() 
{
  return ;
}

LString Node::resultRegister()
{
  return "Not Implemented" ;
}

int Node::theSize()
{
  return 1 ;
}

Node* Node::getScope()
{
  return NULL ;
}

ExecutionObject* Node::generateSuif(Program* p)
{
  return NULL ;
}

Symbol* Node::getVariable(Program* p)
{
  return NULL ;
}

Type* Node::suifType(Program* p)
{
  return NULL ;
}

QualifiedType* Node::findReturnType(Program* p)
{
  return NULL ;
}

ExecutionObject* Node::getMax(Program* p)
{
  return NULL ;
}

ExecutionObject* Node::getMin(Program* p)
{
  return NULL ;
}

ValueBlock* Node::generateSuifInitialization(Program* p, ValueBlock* topLevel,
					     int currentLevel,
					     Type* elementType) 
{
  return NULL ;
}

list<QualifiedType*> Node::findParameters(Program* p)
{
  // Return a copy of an empty list
  list<QualifiedType*> empty ;
  return empty ;
}

list<Expression*> Node::findArguments(Program* p)
{
  // Return a copy of an empty list
  list<Expression*> empty ;
  return empty ;
}

ExecutionObject* Node::generateStore(Program* p)
{
  return NULL ;
}

bool Node::isConst()
{
  return false ;
}

bool Node::isVolatile()
{
  return false ;
}

LString Node::theName()
{
  return "ERR_NAME" ;
}

list<CaseLabelExpr*> Node::collectCaseLabels()
{
  list<CaseLabelExpr*> emptyList ;
  return emptyList ;
}

int Node::getMaxNumber()
{
  return 0 ;
}
