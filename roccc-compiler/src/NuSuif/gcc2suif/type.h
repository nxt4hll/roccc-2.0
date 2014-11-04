/*

  This file contains the declaration of the "Type" class.structures.  There
   should be nothing at the base level, as everything is determined
   by what derived class each object is.

  I have to be careful because some of the nodes in other files have the
   same name as what I would like to call these nodes.  that sort of explains
   why I chose the naming convention I did.

*/

#ifndef __TYPE_DOT_H__
#define __TYPE_DOT_H__

#include <common/suif_list.h>
#include <common/lstring.h>

using namespace std ;

class Type_Generic
{
 private:
 protected:
  LString name ;

  // Modifiers
  bool isLongLong ;
  bool isLong ;
  bool isShort ;
  bool isSigned ;
  bool isUnsigned ;
  
  // Other modifiers exist, but I will get to them later

 public:
  Type_Generic(LString n) ;
  virtual ~Type_Generic() ;
  virtual int size() ;
};

class Type_Integer : public Type_Generic
{
 private:
 protected:
 public:
  Type_Integer(LString n) ;
  ~Type_Integer() ;
  int size() ;
};

class Type_Char : public Type_Generic
{
 private:
 protected:
 public:
  Type_Char(LString n) ;
  ~Type_Char() ;
  int size() ;
} ;

class Type_Float : public Type_Generic
{
 private:
 protected:
 public:
  Type_Float(LString n) ;
  ~Type_Float() ;
  int size() ;
} ;

class Type_Double : public Type_Generic
{
 private:
 protected:
 public:
  Type_Double(LString n) ;
  ~Type_Double() ;
  int size() ;
} ;

class Type_Boolean : public Type_Generic
{
 private:
 protected:
 public:
  Type_Boolean(LString n) ;
  ~Type_Boolean() ;
  int size() ;
} ;

class Type_Void : public Type_Generic
{
 private:
 protected:
 public:
  Type_Void(LString n) ;
  ~Type_Void() ;
  int size() ;
} ;

class Type_Enum : public Type_Generic
{
 private:
 protected:
 public:
  Type_Enum(LString n) ;
  ~Type_Enum() ;
  int size() ;
} ;

class Type_Function : public Type_Generic
{
 private:
  Type_Generic* returnType ;
  list<Type_Generic*> parameterTypes ;
 protected:
 public:
  Type_Function(LString n) ;
  ~Type_Function() ;
  int size() ;
} ;

class Type_Array : public Type_Generic
{
 private:
  Type_Generic* elementType ;
  int length ;
  int startingAddress ;
 protected:
 public:
  Type_Array(LString n) ;
  ~Type_Array() ;
  int size() ;
} ;

class Type_Pointer : public Type_Generic
{
 private:
  Type_Generic* pointeeType ;
 protected:
 public:
  Type_Pointer(LString n) ;
  ~Type_Pointer() ;
  int size() ;
} ;

class Type_Structure : public Type_Generic
{
 private:
  list<Type_Generic*> memberTypes ;
  int maxSize ;
 protected:
 public:
  Type_Structure(LString n) ;
  ~Type_Structure() ;
  int size() ;
} ;

class Type_Union : public Type_Generic
{
 private:
  list<Type_Generic*> memberTypes ;
 protected:
 public:
  Type_Union(LString n) ;
  ~Type_Union() ;
  int size() ;
};

#endif 
