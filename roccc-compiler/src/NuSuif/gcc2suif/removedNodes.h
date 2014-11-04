/*

  This file contains all of the nodes that have been removed from 
    gcc version 4.0 that existed in version 3.4.

  This file should not be compiled, but only exists as a reminder of what 
   came before.

*/

#include "baseNode.h"

class AsmStmt : public Node
{
 private:
 public:
} ;

class CaseLabel : public Node
{
 private:
 public:
} ;

class AsmStmt : public Node
{
 private:
 public:
} ;

class CompoundStmt : public Node // From stmtNodes.h
{
 private:
  Option* lineop;
  Option* bodyop;
  Option* nextop;

  int line ;
  Node* body ;
  Node* next ;

 public:
  CompoundStmt(Option* l, Option* b, Option* n) ;
  ~CompoundStmt() ;
  void connect(Function* t);
  void setupStates(Function* t);
  void flatten() ;
} ;

// From stmtNodes.h
class DeclStmt : public Node
{
 private:
  Option* lineop;
  Option* declop;
  Option* nextop;

  int line ;
  Node* decl ;
  Node* next ;

 public:
  DeclStmt(Option* l, Option* d, Option* n) ;
  ~DeclStmt() ;
  void connect(Function* t);
  void setupStates(Function* t);
  void flatten() ;
};

// From stmtNodes.h
class GotoStmt : public Node
{
 private:
  Option* lineop ;
  Option* destop ;
  Option* nextop ;

  int line ;
  Node* dest ;
  Node* next ;

 public:

  GotoStmt(Option* l, Option* d, Option* n) ;
  ~GotoStmt() ;
  void connect(Function* t);
  void setupStates(Function* t);
  void flatten() ;
};

// From stmtNodes.h
class LabelStmt : public Node
{
 private:
  Option* lineop;
  Option* lablop;
  Option* nextop;

  int line ;
  Node* labl ;
  Node* next ;

 public:
  LabelStmt(Option* l, Option* l2, Option* n) ;
  ~LabelStmt() ;
  void connect(Function* t);
  void flatten() ;
  void setupStates(Function* t);
};

// From stmtNodes.h
class ReturnStmt : public Node
{
 private:
  Option* lineop;
  Option* exprop;
  Option* nextop;

  int line ;
  Node* expr ;
  Node* next ;

 public:
  ReturnStmt(Option* l, Option* e, Option* n) ;
  ~ReturnStmt() ;
  void connect(Function* t);
  void setupStates(Function* t);
  void flatten() ;
  void outputInstruction() ;
};

// From stmtNodes.h
class ScopeStmt : public Node
{
 private:
  Option* lineop;
  list<string>* attributes;
  Option* nextop;

  int line ;
  Node* next ;

 public:
  ScopeStmt(Option* l, list<string>* a, Option* n) ;
  ~ScopeStmt() ;
  void connect(Function* t);
  void setupStates(Function* t);
  void flatten() ;
};

class BufferRef : public Node
{
 private:
 public:
} ;

class MethodCallExpr : public Node
{
 private:
 public:
} ;

class WithRecordExpr : public Node
{
 private:
 public:
} ;

class ExponExpr : public Node
{
 private:
 public:
} ;

class FfsExpr : public Node
{
 private:
 public:
} ;

class BitAndToExpr : public Node
{
 private:
 public:
};

class ExprWithFileLocation : public Node
{
 private:
 public:
} ;

class InExpr : public Node
{
 private:
 public:
} ;

class SetElExpr : public Node
{
 private:
 public:
};

class CarExpr : public Node 
{
 private:
 public:
} ;

class UnsaveExpr : public Node
{
 private:
 public:
};

class RtlExpr : public Node
{
 private:
 public:
};

class ReferenceExpr : public Node
{
 private:
 public:
};

class EntryValueExpr : public Node
{
 private:
 public:
};

class GotoSubroutineExpr : public Node
{
 private:
 public:
};

class LabeledBlockExpr : public Node
{
 private:
 public:
};

class ExitBlockExpr : public Node
{
 private:
 public:
}

// From nodes.h
// This node is apparently just an information node in the tree
//  I really have no clue what it does as the few instances of it I have seen
//  simply point back its parents.
//  Reading the documentation I see that it is information on the base
//  class.
class Binfo : public Node
{
 private:
  Option* typeop;
  Option* baseop;

  // These correspond to the actual values in the options
  Node* type ;
  Node* base ;

 public:
  Binfo(Option* t, Option* b) : Node(), typeop(t), baseop(b) { ; }
  ~Binfo() ;
  void connect(Function* t);
  void flatten();
};

// From stmtNodes.h
class FileStmt : public Node
{
 public:
  FileStmt() ;
  ~FileStmt() ;
};

// From stmtNodes.h
class CtorStmt : public Node
{
 private:
  Option* lineop;
  list<string>* attributes;
  Option* nextop;

  int line ;
  Node* next ;

 public:
  CtorStmt(Option* l, list<string>* a, Option* n) ;
  ~CtorStmt() ;
  void connect(Function* t);
  void flatten() ;
};

// From nodes.h
class SubObject : public Node
{
 private:
  Option* lineop;
  Option* clnpop;
  Option* nextop;

  int line ;
  Node* clnp ;
  Node* next ;

 public:
  SubObject(Option* l, Option* c, Option* n) : Node(),
    lineop(l), clnpop(c), nextop(n) { ; };
  ~SubObject() ;
  void connect(Function* t);
  void flatten() ;
};

class SetType : public Node
{
 private:
 public:
};
