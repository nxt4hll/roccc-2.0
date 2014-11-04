/*

  This file contains all of the Constant nodes in the abstract syntax tree. 
   These nodes represent an actual value and should all contain 
   a concrete constant value.

*/

#ifndef __CSTNODES_DOT_H__
#define __CSTNODES_DOT_H__

#include <common/lstring.h>

#include "baseNode.h"

// Forward declarations of my classes
class Option ;
class Function ;
class Program ;

// Forward declarations of SUIF classes
class ExecutionObject ;
class ValueBlock ;
class VariableSymbol ;

using namespace std ;

class IntegerCst : public Node
{
 private:
  Option* typeop ;
  Option* highop ;
  Option* lowop ;

  Node* type ;

  // The value stored in high is usually absent (pretty much only
  //  used in array domain declarations)
  int high ;

  // The value stored in low is the actual value of the int constant
  int low ;

 public:

  IntegerCst(Option* t, Option* h, Option* l) ;
  ~IntegerCst() ;
  void connect(Function* t);
  void flatten() ;

  // Return the actual value
  int theSize() ;

  ExecutionObject* generateSuif(Program* p) ;

  // This function is used in the middle of array initializations
  ValueBlock* generateSuifInitialization(Program* p, 
					 ValueBlock* topLevel = NULL,
					 int currentLevel = 0,
					 Type* elementType = NULL) ;

  Type* suifType(Program* p) ;

};

class RealCst : public Node
{
 private:
  Option* typeop;

  Node* type ;

  double low ; // This is the actual value

 public:
  RealCst(Option* t, double l) ;
  ~RealCst() ;
  void connect(Function* t);
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;

};

/*

  Currently, string constants are translated into static arrays with
   a MultiValueBlock that holds their ASCII representation.  This
   array then has its address taken everywhere the constant is referenced.

  This has the same problem the rest of the code does in the sense that 
   initializations are not handled.

*/

class StringCst : public Node
{
 private:
  Option* typeop;
  Option* strgop;
  Option* lngtop;

  Node* type ;
  LString strg ;
  int lngt ;

  VariableSymbol* stringLocation ;

 public:
  StringCst(Option* t, Option* s, Option* l) ;
  ~StringCst() ;
  void connect(Function* t);
  void flatten() ;

  Symbol* getVariable(Program* p) ;

};

// These classes exist, but are currently unsupported

class ComplexCst : public Node
{
 private:
 public:
};

// New class
class VectorCst : public Node
{
 private:
 public:
};

class PtrmemCst : public Node
{
 private:
 public:
};

#endif


