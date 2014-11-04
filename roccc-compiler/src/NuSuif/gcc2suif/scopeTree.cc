/*

  This file contains the definitions for the Scope and ScopeTree classes.

*/

#include <cassert>

#include "scopeTree.h"

/*************************** Scope Functions ***************************/

Scope::Scope()
{
  // Nothing to do here
}

Scope::~Scope()
{
  // Recursively delete my children
  list<Scope*>::iterator deleteIter = children.begin() ;
  while (deleteIter != children.end())
  {
    delete (*deleteIter) ;
    ++deleteIter ;
  }
}

void Scope::addElement(Type t, LString id)
{
  theTable.addElement(t, id) ;
}

void Scope::addElement(Type t, LString id, int addr)
{
  theTable.addElement(t, id, addr) ;
}

void Scope::addScope(Scope* newChild)
{
  assert(newChild != NULL) ;
  newChild->setParent(this) ;

  children.push_back(newChild) ;
}

Scope* Scope::getParent()
{
  return parent ;
}

void Scope::setParent(Scope* p)
{
  assert(p != NULL); 
  parent = p ;
}

/************************* ScopeTree Functions *************************/

ScopeTree::ScopeTree()
{
  root = new Scope() ;
  currentScope = root ;
}

ScopeTree::~ScopeTree()
{
  assert(root != NULL) ;
  delete root ;
}

Scope* ScopeTree::pushNewScope()
{
  Scope* newScope = new Scope() ;
  currentScope->addScope(newScope) ;
  currentScope = newScope ;
  return currentScope ;
}

Scope* ScopeTree::popScope()
{
  // Don't pop too far
  if (currentScope->getParent() != NULL)
  {
    currentScope = currentScope->getParent() ;
  }
  return currentScope ;
}

void ScopeTree::addElement(Type t, LString id)
{
  currentScope->addElement(t, id) ;
}

void ScopeTree::addElement(Type t, LString id, int addr)
{
  currentScope->addElement(t, id, addr) ;
}
