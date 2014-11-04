/*

  This file contains the declaration of the base class for the entire Node 
   hierarchy.  This node structure is used to represent each individual 
   tree node from the original gcc abstract syntax tree.

  At the very least, every derived class should define a default constructor,
   default destructor, a connect function, and a flatten function.

*/

#ifndef __BASENODE_DOT_H__
#define __BASENODE_DOT_H__

#include <common/lstring.h>

#include "function.h"
#include "program.h"

using namespace std ;

// Forward declarations
//  These are all SUIF types that I use
class ExecutionObject ;
class Symbol ;
class Type ;
class ValueBlock ;
class QualifiedType ;
class Expression ;

// A forward declaration of a derived class...
class CaseLabelExpr ;

class Node
{
 protected:

  // This number corresponds to the @# output in the .original file.
  int number ;
  
  // This boolean is here to prevent infinite loops in the connection process.
  //  It is set to true the first time a node is encountered and never touched
  //  after that.
  bool connected;

  // A base node should never be created by an outside user
  Node() ; 

 public:

  // These two functions deal with the node number as assigned by the 
  //  .original file
  int getNumber() ;
  void setNumber(int newNumber) ;

  // Virtual functions

  virtual ~Node() ;

  // The connect function goes through and makes all of the children point
  //  to the actual nodes that correspond to the children's number.
  //  This must be done because the abstract syntax tree refers to 
  //  nodes before they are created.
  virtual void connect(Function* t) ;

  // The flatten function removes the option pointers and fills in the
  //  actual Node pointers or values associated with each option
  virtual void flatten() ;

  // The resultRegister function returns the name of the temporary associated
  //  with this node or the actual string associated with this node if this
  //  node corresponds to a variable
  virtual LString resultRegister() ;

  //  This is mainly used with respect to determining the size of
  //  multidimensional arrays.
  virtual int theSize() ;

  // This function will return the Node* that refers to the scope of 
  //  the current node.  This is pretty much only used now in VarDecl
  //  and ParmDecl nodes and all other instantiations should return NULL
  virtual Node* getScope() ;

  // The main function that converts a single node of gcc AST into
  //  the corresponding suif AST.
  virtual ExecutionObject* generateSuif(Program* p) ;

  // Returns the variable that was placed inside the program's 
  //  symbol table for suif or NULL if this Node is not associated with
  //  a symbol.
  virtual Symbol* getVariable(Program* p) ;

  // This will return the suif type of the node (mainly
  //  used by typeNodes).
  virtual Type* suifType(Program* p) ;

  // This is used only in function decl nodes and returns the type of the
  //  return value for a function.  Should this also be a general Type?
  virtual QualifiedType* findReturnType(Program* p) ;

  // This will only return the max of a domain (type nodes only)
  virtual ExecutionObject* getMax(Program* p) ;

  // This will only return the min of a domain (type nodes only)
  virtual ExecutionObject* getMin(Program* p) ;

  // This function will return initialization code from certain
  //  nodes.  It is similar to generating execution objects, but suif
  //  requires a different type of object for initialization.
  //
  // The element type parameter is only used in the creation of 
  //  array initializations and should be ignored in other cases.
  //  The only reason it exists is so we can support our special types
  //  that are not recognized by the gcc front end.
  virtual ValueBlock* generateSuifInitialization(Program* p,
						 ValueBlock* topLevel = NULL,
						 int currentLevel = 0,
						 Type* elementType = NULL) ;

  // This function builds up parameters into a list of qualified types
  virtual list<QualifiedType*> findParameters(Program* p) ;

  // This function goes through and builds up a list of arguments that
  //  will be passed to a function.  It is only used for call_expression
  //  and call statement nodes
  virtual list<Expression*> findArguments(Program* p) ;

  // If the node has a name field, return the name field
  virtual LString theName() ;

  // Used for indirect references only!
  virtual ExecutionObject* generateStore(Program* p) ;

  // Used for type nodes only!
  virtual bool isConst() ;
  virtual bool isVolatile() ;

  // Used in determining the sizes of arrays
  virtual int getMaxNumber() ;

  // Suif requires a listing of all case labels and their numbers, 
  //  so each node must collect and append these labels
  virtual list<CaseLabelExpr*> collectCaseLabels() ;

};

#endif 
