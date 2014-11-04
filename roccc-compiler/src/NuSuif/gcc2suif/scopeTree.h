/*

  This file contains the declaration of the both the Scope class 
   and the Scope Tree class.  These classes are used in order to keep track
   of where variables are declared and which variable definitions
   currently is valid.

  Should there be one scope tree for the whole program, or should
   there be one scope tree per function?  I'm not sure which is the way to 
   go just yet.  I would like to be able to have a global function, but
   the problem is the file I parse isn't structured that way.  The 
   global variables are all located inside the different functions and 
   have no scope attached to them, so I would have to make a dedicated pass
   through all functions just to locate global variables, and then
   I would have to treat them specially and very carefully in each
   function that uses them (and I still don't know where the
   initialization functionality would be placed).
*/

#ifndef __SCOPE_TREE_DOT_H__
#define __SCOPE_TREE_DOT_H__

#include <common/suif_list.h>
#include <common/lstring.h>

#include "symtab.h"

using namespace std ;

class Scope
{
 private:
  SymbolTableMine theTable ;

  list<Scope*> children ;
  Scope* parent ;

  void setParent(Scope* p) ;

 public:

  Scope() ;
  ~Scope() ;

  void addElement(Type t, LString i) ;
  void addElement(Type t, LString i, int addr) ;

  void addScope(Scope* newChild) ;
  Scope* getParent() ;

} ;

class ScopeTree
{
 private:
  Scope* root ;
  Scope* currentScope ;
 public:

  ScopeTree() ;
  ~ScopeTree() ;

  Scope* pushNewScope() ;
  Scope* popScope() ;

  void addElement(Type t, LString id) ;
  void addElement(Type t, LString id, int addr) ;
} ;

#endif 
