
/*

  This file contains the declarations of both the Symbol Table class as well
   as the Symbol Table Entry class. 

  I am going to have to significantly change this if we are going to 
   parse classes and record types.

*/

#ifndef __SYMTAB_DOT_H__
#define __SYMTAB_DOT_H__

#include <common/suif_list.h>
#include <common/lstring.h>

#include "type.h"

using namespace std ;

enum TypeMine { Integer, Boolean, Character, Array, Structure, Error } ;

class SymbolTableEntry
{
 private:
  
  Type_Generic* t_g ;
  TypeMine theType ;
  LString theIdentifier ;
  // If this happens to be an array, this will keep track of its
  //  starting location
  int address ; 

 public:
  SymbolTableEntry(TypeMine t, LString i) ;
  virtual ~SymbolTableEntry() ;
  inline LString ID() ;
  inline TypeMine myType() ;
  inline void setAddress(int x) ;
  inline int theAddress() ;
  virtual int size() ;
};

class SymbolTableMine
{
 private:
  // I really don't care about efficiency at this point, so I will
  //  use a list

  list<SymbolTableEntry*> allSymbols ;

 public:

  SymbolTableMine() { ; } 
  ~SymbolTableMine() ;
  void addElement(TypeMine t, LString i) ;
  void addElement(TypeMine t, LString i, int a) ;
  int lookupAddress(LString theName) ;
  TypeMine lookupType(LString theName) ;
  void dumpTable() ;
  void dumpDeclarations() ;
};

class StructureEntry : public SymbolTableEntry
{
 private:
  list<SymbolTableEntry*> members ;
 public:

  int size() ;

  StructureEntry(TypeMine t, LString i) ;
  ~StructureEntry() ;
} ;


#endif
