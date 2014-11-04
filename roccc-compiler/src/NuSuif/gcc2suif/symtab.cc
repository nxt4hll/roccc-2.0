
#include <iostream>
#include "symtab.h"


SymbolTableMine::~SymbolTableMine()
{
  list<SymbolTableEntry*>::iterator cleanUp = allSymbols.begin() ;
  while (cleanUp != allSymbols.end())
  {
    delete (*cleanUp) ;
    ++cleanUp ;
  }
}

void SymbolTableMine::addElement(TypeMine t, LString i)
{
  SymbolTableEntry* toAdd = new SymbolTableEntry(t, i) ;
  allSymbols.push_back(toAdd) ;
}

void SymbolTableMine::addElement(TypeMine t, LString i, int a)
{
  SymbolTableEntry* toAdd = new SymbolTableEntry(t, i) ;
  toAdd->setAddress(a) ;
  allSymbols.push_back(toAdd) ;
}

int SymbolTableMine::lookupAddress(LString theName)
{
  list<SymbolTableEntry*>::iterator findIt = allSymbols.begin() ;
  while (findIt != allSymbols.end())
  {
    if ((*findIt)->ID() == theName)
      return (*findIt)->theAddress() ;
    ++findIt ;
  }
  return 0 ;
}

TypeMine SymbolTableMine::lookupType(LString theName)
{
  list<SymbolTableEntry*>::iterator findIt = allSymbols.begin() ;
  while (findIt != allSymbols.end())
  {
    if ((*findIt)->ID() == theName)
      return (*findIt)->myType() ;
    ++findIt ;
  }
  return Error ;
}

void SymbolTableMine::dumpTable()
{
  list<SymbolTableEntry*>::iterator dumpIt = allSymbols.begin() ;
  while (dumpIt != allSymbols.end())
  {  
    cout << (*dumpIt)->ID() << " " ;
    switch( (*dumpIt)->myType())
    {
    case Integer:
      cout << "Integer" << endl ;
      break ;
    case Boolean:
      cout << "Boolean" << endl ;
      break ;
    case Character:
      cout << "Character" << endl ;
      break;
    case Array:
      cout << "Array - address " << (*dumpIt)->theAddress() << endl ;
      break ;
    case Structure:
      cout << "Structure" << endl ;
      break ;
    case Error:
    default:
      cout << "Error" << endl ;
    }
    ++dumpIt ;
  }
}

void SymbolTableMine::dumpDeclarations()
{
  list<SymbolTableEntry*>::iterator dumpIt = allSymbols.begin() ;
  while (dumpIt != allSymbols.end())
  {  
    cout << "variable " ;
    cout << (*dumpIt)->ID() << " : " ;
    switch( (*dumpIt)->myType())
    {
    case Integer:
      // BUG!
      cout << "UNSIGNED(31 downto 0) ;" << endl ;
      break ;
    case Boolean:
      cout << "BOOLEAN ;" << endl ;
      break ;
    case Character:
      cout << "SIGNED(7 downto 0) ;" << endl ;
      break;
    case Array:
      cout << "UNSIGNED(31 downto 0) ; --Array Reference" << endl ;
      break ;
    case Error:
    default:
      cout << "ERROR ;" << endl ;
    }
    ++dumpIt ;
  }
}

/******************* SymbolTableEntry members **********************/

SymbolTableEntry::SymbolTableEntry(TypeMine t, LString i) : theType(t), 
						       theIdentifier(i)
{
  t_g = NULL ;
}

SymbolTableEntry::~SymbolTableEntry()
{
  ;
}

LString SymbolTableEntry::ID()
{
  return theIdentifier ;
}

TypeMine SymbolTableEntry::myType()
{
  return theType ;
}

void SymbolTableEntry::setAddress(int x)
{
  address = x ;
}

int SymbolTableEntry::theAddress()
{
  return address ;
}

// I am assuming 1 byte for a character, 1 byte for a boolean, 
//  and 4 bytes for an integer

int SymbolTableEntry::size()
{
  switch(theType)
  {
  case Integer:
    {
      return 4 ;
    }
  case Boolean:
    {
      return 1 ;
    }
  case Character:
    {
      return 1 ;
    }
  case Array:
    {
      // Currently, I do not have the information 
      //  needed at this level.  This will have to change 
      //  soon if I am to keep track of arrays correctly
      break ;
    }
  case Structure:
    {
      // I shouldn't be here, I should be in the derived function
      break ;
    }
    ;
  case Error:
    {
      break ;
    }
  default:
    {
      break ;
    }
  }
  return 0 ;
}

/********************* StructureEntry members **********************/

StructureEntry::StructureEntry(TypeMine t, LString i) : SymbolTableEntry(t, i)
{
  ; // Nothing to do in here yet
}

StructureEntry::~StructureEntry()
{
  ; // Nothing to destruct yet...
}

int StructureEntry::size()
{
  // Go through the list of all members and sum up the values
  //  of each entry, then return

  int totalSize = 0 ;
  list<SymbolTableEntry*>::iterator countIter = members.begin() ;

  while(countIter != members.end())
  {
    totalSize+= (*countIter)->size() ;
    ++countIter ;
  }

  return totalSize ;
}
