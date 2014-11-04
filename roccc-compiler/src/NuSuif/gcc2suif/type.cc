/*

  This file contains all of the definitions for all of the 
   member functions for all of the type classes.

  I need to figure out a way to reclaim all this memory without
   deleting the same memory twice.  I'm not quite sure of how to 
   accomplish this yet.

*/

#include <common/lstring.h>

#include "type.h"

/***************************** Type_Generic ********************************/

Type_Generic::Type_Generic(LString n)
{
  name = n ;
  isLongLong = false ;
  isLong = false ;
  isShort = false ;
  isSigned = true ;
  isUnsigned = false ;
}

Type_Generic::~Type_Generic()
{
  ; // Nothing to destroy in this file
}

int Type_Generic::size()
{
  return 0 ;
}

/***************************** Type_Integer ********************************/

Type_Integer::Type_Integer(LString n) :Type_Generic(n) 
{
  ; // Nothing to do in here...
}

Type_Integer::~Type_Integer()
{
  ; // Nothing to destroy in here...
}

int Type_Integer::size()
{
  if (isShort)
  {
    return 2 ;
  }
  else if (isLongLong)
  {
    return 8 ;
  }
  else
  {
    return 4 ;
  }
}

/***************************** Type_Char ********************************/

Type_Char::Type_Char(LString n) : Type_Generic(n)
{
  ; // Nothing to do in here...
} 

Type_Char::~Type_Char()
{
  ; // Nothing to do in here either...
}

int Type_Char::size()
{
  // Characters cannot be long or short
  return 1 ;
}

/***************************** Type_Float ********************************/

Type_Float::Type_Float(LString n) : Type_Generic(n) 
{
  ; // Nothing to do in here
}

Type_Float::~Type_Float()
{
  ; // Nothing to do in here
}

int Type_Float::size()
{
  return 4 ;
}

/***************************** Type_Double ********************************/

Type_Double::Type_Double(LString n) : Type_Generic(n) 
{
  ; // Nothing to do in here
}

Type_Double::~Type_Double()
{
  ; // Nothing to destroy here
}

int Type_Double::size()
{
  if (isLong)
  {
    return 12 ;
  }
  return 8 ;
}

/***************************** Type_Boolean ********************************/

Type_Boolean::Type_Boolean(LString n) : Type_Generic(n) 
{
  ; // Nothing to do here
}

Type_Boolean::~Type_Boolean()
{
  ; // Nothing to do here
}

int Type_Boolean::size()
{
  return 1 ;
}

/***************************** Type_Void ********************************/

Type_Void::Type_Void(LString n) : Type_Generic(n)
{
  ; // Nothing to do here
}

Type_Void::~Type_Void()
{
  ; // Nothing to do here
}

int Type_Void::size()
{
  return 0 ;
}

/***************************** Type_Enum ********************************/

// This is going to have to change a lot.  I'm not sure how (or if) 
//  to handle this type yet.

Type_Enum::Type_Enum(LString n) : Type_Generic(n)
{
  ; // I'm not sure what to do here yet...
}

Type_Enum::~Type_Enum()
{
  ; // Nothing really to do here I think...
}

int Type_Enum::size()
{
  return 1 ; // I don't know what to do here....
}

/***************************** Type_Function ********************************/

Type_Function::Type_Function(LString n) : Type_Generic(n) 
{
  ; // Not sure what to do in here yet...
}

Type_Function::~Type_Function()
{
  ; // Not sure what to do in here yet...
}

int Type_Function::size()
{
  return 0 ; // I don't know what size is...
}

/***************************** Type_Array ********************************/

Type_Array::Type_Array(LString n) : Type_Generic(n)
{
  elementType = NULL ;
  length = 0 ;
  startingAddress = 0 ;
  ; // I don't know what to do in here yet.
}

Type_Array::~Type_Array()
{
  ; // I don't know what to destroy in here yet...
}

int Type_Array::size()
{
  if (elementType == NULL)
    return 0 ;
  return length * elementType->size() ;
}

/***************************** Type_Pointer*******************************/

Type_Pointer::Type_Pointer(LString n) : Type_Generic(n)
{
  pointeeType = NULL ;
}

Type_Pointer::~Type_Pointer()
{
  ; // Do I delete the pointeeType?
}

int Type_Pointer::size()
{
  return 4 ; // All pointers are 32 bits in this architecture for now
}

/***************************** Type_Structure ********************************/

Type_Structure::Type_Structure(LString n) : Type_Generic(n)
{
  ; // Nothing here to do?  I'm not sure
}

Type_Structure::~Type_Structure()
{
  ; // Do I delete anything here?
}

// I wanted to go through all of the members and add up the
//  individual sizes, but that doesn't work because not all members
//  may be added when I need to check the size.
//  The size is known, however, when the class is originally declared, 
//  so I should just use that.
int Type_Structure::size()
{
  /*
  int totalSize = 0 ;
  list<Type_Generic*>::iterator sizeIter = memberTypes.begin() ;
  while (sizeIter != memberTypes.end())
  {
    totalSize += (*sizeIter)->size() ;
    ++sizeIter ;
  }
  return totalSize ;
  */
  return maxSize ;
}

/***************************** Type_Union ********************************/

Type_Union::Type_Union(LString n) : Type_Generic(n)
{
  ; // I don't think that I do anything at this point.
}

Type_Union::~Type_Union()
{
  ; // I don't know if I should delete anything here just yet.
}

int Type_Union::size()
{
  list<Type_Generic*>::iterator countIter = memberTypes.begin() ;
  int maxSize = 0 ;
  while (countIter != memberTypes.end())
  {
    if ((*countIter)->size() > maxSize)
    {
      maxSize = (*countIter)->size() ;
    }
    ++countIter ;
  }
  return maxSize ;
}
