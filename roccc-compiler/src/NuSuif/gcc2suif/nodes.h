/*
  
  This file contains the declarations of the classes that will 
   make up the tree I use based off of the intermediate format of 
   gcc.  The tree isn't really a tree, but instead is a graph.

  This file will be input from the contents dumped out of gcc when the 
   -fdump-tree-original flag is used and will write out the same thing.

*/

#ifndef __NODES_DOT_H__
#define __NODES_DOT_H__

#include <common/lstring.h>
#include <common/suif_list.h>
#include <common/suif_vector.h>

#include "generic.h"
// #include "states.h"

#include "baseNode.h"

using namespace std;

// Forward declarations of Suif objects
class ExecutionObject ; 
class ValueBlock ;
class QualifiedType ;


class Option; // Forward Declaration
class Function; // Forward Declaration

struct opOptions
{
  int op;
  int nodeNumber;
  Node* nodePointer;
};

class ErrorMark : public Node
{
 private:
 public:
  ErrorMark() : Node() { ; }
  ~ErrorMark() { ; }
  // No connection function necessary
};

class IdentifierNode : public Node
{
 private:
  Option* strgop;
  Option* lngtop;
  list<LString>* attributes;

  // These are the actual values associated with the options
  LString name;
  int length ;

 public:
  IdentifierNode(Option* s, Option* l, list<LString>* a) : Node(), 
    strgop(s), lngtop(l), attributes(a) { ; }
  ~IdentifierNode() ;
  // No connection function necessary
  LString theName() { return name ; }
  void flatten() ;
  LString resultRegister() { return name ;} 
};

class TreeList : public Node
{
 private:
  Option* purpop;
  Option* valuop;
  Option* chanop;

  // These are the actual values of the options

  Node* purp ;
  Node* valu ;
  Node* chan ;

 public:
  TreeList(Option* p, Option* v, Option* c) : Node(), purpop(p), valuop(v), 
                                              chanop(c) { ; }
  ~TreeList() ;
  void connect(Function* t);
  void flatten(); 
  LString resultRegister() ;

  ValueBlock* generateSuifInitialization(Program* p, 
					 ValueBlock* topLevel = NULL,
					 int currentLevel = 0,
					 Type* elementType = NULL);

  ExecutionObject* generateSuif(Program* p) ;

  list<QualifiedType*> findParameters(Program* p) ;
  list<Expression*> findArguments(Program* p) ;

};

class TreeVec : public Node
{
 private:
  Option* lngtop;
  list<opOptions>* ops;

  int length;

 public:
  TreeVec(Option* l, list<opOptions>* o) : Node(), lngtop(l), ops(o) { ; }
  ~TreeVec() ;
  void connect(Function* t);
  void flatten() ;
};

class Block : public Node
{
 private:
 public:
  Block() : Node() { ; }
  ~Block() { ; }
};

class IndirectRef : public Node
{
 private:
  Option* typeop ;
  list<opOptions>* ops;

  Node* type ;

 public:
  IndirectRef(Option* t, list<opOptions>* o) : Node(), typeop(t), ops(o) { ; }
  ~IndirectRef() ;
  void connect(Function* t);
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;
  ExecutionObject* generateStore(Program* p) ;

  Symbol* getVariable(Program* p) ;

};

class ComponentRef : public Node
{
 private:
  Option* typeop ;
  list<opOptions>* ops;

  Node* type ;

 public:
  ComponentRef(Option* t, list<opOptions>* o) : Node(), typeop(t), ops(o) { ; }
  ~ComponentRef() ;
  void connect(Function* t);
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;

  Type* suifType(Program* p) ;
  Symbol* getVariable(Program* p) ;

};

// It looks like all array references are different nodes.  For example, two
//  array references of array[0] and array[0] turn into different ArrayRef 
//  nodes even though they refer to the exact same spot in memory.
//  Therefore, using a variable that keeps track of what stage the
//  outputting is in is viable.

class ArrayRef : public Node
{
 private:
  Option* typeop ;
  list<opOptions>* ops;

  int stage ;

  Node* type ;

 public:

  // This is opOne [opTwo] 
  //  opOne could of course be another array reference, making this
  //  a multidimensional array

  ArrayRef(Option* t, list<opOptions>* o) : Node(), typeop(t), ops(o) 
    { stage = 1; }
  ~ArrayRef();
  void connect(Function* t);
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;

  Symbol* getVariable(Program* p) ;
  
};

class Constructor : public Node
{
 private:
  Option* typeop;
  Option* eltsop;

  Node* type ;
  Node* elts ;

 public:
  Constructor(Option* t, Option* e) : Node(), typeop(t), eltsop(e) { ; } 
  ~Constructor() ;
  void connect(Function* t);
  void flatten() ;

  ValueBlock* generateSuifInitialization(Program* p, 
					 ValueBlock* topLevel = NULL,
					 int currentLevel = 0,
					 Type* elementType = NULL) ;

  ExecutionObject* generateSuif(Program* p) ;

  Type* suifType(Program* p) ;

};

class AlignIndirectRef : public Node
{
 private:
 public:
} ;

class MisalignedIndirectRef : public Node
{
 private:
 public:
};

class ArrayRangeRef : public Node
{
 private:
 public:
} ;

class ObjTypeRef : public Node
{
 private:
 public:
};

class SsaName : public Node
{
 private:
 public:
};

class PhiNode : public Node
{
 private:
 public:
};

class ScevKnown : public Node
{
 private:
 public:
};

class ScevNotKnown : public Node
{
 private:
 public:
};

class PolynomialChrec : public Node
{
 private:
 public:
};

// This one I have to worry about
class StatementListMine : public Node
{
 private:
  list<opOptions>* ops ;

 public:

  StatementListMine(list<opOptions>* o) : Node(), ops(o) { ; } 
  ~StatementListMine() ;

  void connect(Function* t) ;
  void flatten() ;  

  ExecutionObject* generateSuif(Program* p) ;

  list<CaseLabelExpr*> collectCaseLabels() ;

};

class ValueHandle : public Node
{
 private:
 public:
};

class TreeBinfo : public Node
{
 private:
 public:
};

class OffsetRef : public Node
{
 private:
 public:
};

class ScopeRef : public Node
{
 private:
 public:
};

class MemberRef : public Node
{
 private:
 public:
};

class Baselink : public Node
{
 private:
 public:
};

class TemplateParmIndex : public Node
{
 private:
 public:
};

class TemplateTemplateParm : public Node
{
 private:
 public:
};

class TemplateTypeParm : public Node
{
 private:
 public:
};

class BoundTemplateTemplateParm : public Node
{
 private:
 public:
};

class UnboundClassTemplate : public Node
{
 private:
 public:
};

class UsingStmt : public Node
{
 private:
 public:
};

class DefaultArg : public Node
{
 private:
 public:
};

class Overload : public Node
{
 private:
 public:
};

class Binfo : public Node
{
 private:
  Option* typeop ;
  Option* basesop ;

  Node* type ;
  int bases ;

 public:

  Binfo(Option* t, Option* b) : Node(), typeop(t), basesop(b) { ; } 
  ~Binfo() ;
  void connect(Function* t) ;
  void flatten() ;
};

#endif
