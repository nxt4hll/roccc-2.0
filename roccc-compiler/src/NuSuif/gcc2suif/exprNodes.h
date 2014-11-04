/*

  This file contains the declarations of all of the nodes that represent
   expressions in the gcc abstract syntax tree.

*/

#ifndef __EXPRNODES_DOT_H__
#define __EXPRNODES_DOT_H__

#include <common/suif_list.h>
#include <common/lstring.h>

#include "option.h"
#include "nodes.h"
#include "baseNode.h"
#include "program.h"

// Forward declaration
class ExecutionObject ;
class CodeLabelSymbol ;

class TruthNotExpr : public Node
{
 private:
  Option* typeop;
  list<opOptions>* ops;

  Node* type ;

 public:
  TruthNotExpr(Option* t, list<opOptions>* o) ;
  ~TruthNotExpr();
  void connect(Function* t);
  void flatten() ;
};

class AddrExpr : public Node
{
 private:
  Option* typeop;
  list<opOptions>* ops;

  Node* type ;

 public:
  AddrExpr(Option* t, list<opOptions>* o) ;
  ~AddrExpr() ;
  void connect(Function* t);
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;

};

class TruthAndIfExpr : public Node
{
 private:
  Option* typeop;
  list<opOptions>* ops;

  Node* type ;

 public:
  TruthAndIfExpr(Option* t, list<opOptions>* o) ;
  ~TruthAndIfExpr() ;
  void connect(Function* t);
  void flatten() ;
  ExecutionObject* generateSuif(Program* p) ;
};

class TruthAndExpr : public Node
{
 private:
  Option* typeop ;
  list<opOptions>* ops ;

  Node* type ;

 public:
  TruthAndExpr(Option* t, list<opOptions>* o) ;
  ~TruthAndExpr() ;
  void connect(Function* t) ;
  void flatten() ;
  ExecutionObject* generateSuif(Program* p) ;
};

class TruthOrExpr : public Node
{
 private:
  Option* typeop ;
  list<opOptions>* ops ;

  Node* type ;

 public:
  TruthOrExpr(Option* t, list<opOptions>* o) ;
  ~TruthOrExpr() ;
  void connect(Function* t) ;
  void flatten() ;
  ExecutionObject* generateSuif(Program* p) ;
} ;

class TruthOrIfExpr : public Node
{  
 private:
  Option* typeop;
  list<opOptions>* ops;

  Node* type ;

 public:
  TruthOrIfExpr(Option* t, list<opOptions>* o) ;
  ~TruthOrIfExpr() ;
  void connect(Function* t);
  void flatten() ;
  ExecutionObject* generateSuif(Program* p) ;
};

class InitExpr : public Node
{
 private:
  Option* typeop;
  list<opOptions>* ops;

  Node* type ;

 public:
  InitExpr(Option* t, list<opOptions>* o) ;
  ~InitExpr() ;
  void connect(Function* t);
  void flatten() ;
  LString resultRegister() ;
  ExecutionObject* generateSuif(Program* p) ;
};

// Modify expression is pretty weird in SUIF.
//  Store statements in SUIF are STATEMENTS and not EXPRESSIONS
//  We therefore restrict the original C code to not contain modify expressions
//  inside other expressions or statements.  Every store statement must
//  be in the form of (x = y) where x and y do not contain another = .

class ModifyExpr : public Node
{
 private:
  Option* typeop;
  list<opOptions>* ops;

  Node* type ;

 public:
  ModifyExpr(Option* t, list<opOptions>* o) ;
  ~ModifyExpr() ;
  void connect(Function* t);
  void flatten();

  ExecutionObject* generateSuif(Program* p) ;
};

class CompoundExpr : public Node
{
 private:
  Option* typeop;
  list<opOptions>* ops;

  Node* type ;

 public:
  CompoundExpr(Option* t, list<opOptions>* o) ;
  ~CompoundExpr() ;
  void connect(Function* t);
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;
};

class PreincrementExpr : public Node
{
 private:
  Option* typeop;
  list<opOptions>* ops;

  Node* type ;

 public:
  PreincrementExpr(Option* t, list<opOptions>* o) ;
  ~PreincrementExpr() ;
  void connect(Function* t);
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;
};

class PostincrementExpr : public Node
{
 private:
  Option* typeop;
  list<opOptions>* ops;

  Node* type ;

 public:
  PostincrementExpr(Option* t, list<opOptions>* o) ;
  ~PostincrementExpr() ;
  void connect(Function* t);
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;

};

class PredecrementExpr : public Node
{
 private:
  Option* typeop ;
  list<opOptions>* ops ;

  Node* type ;
 public:

  PredecrementExpr(Option* t, list<opOptions>* o) ;
  ~PredecrementExpr() ;
  void connect(Function* t);
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;
};

class PostdecrementExpr : public Node
{
 private:
  Option* typeop;
  list<opOptions>* ops;

  Node* type ;

 public:
  PostdecrementExpr(Option* t, list<opOptions>* o) ;
  ~PostdecrementExpr() ;
  void connect(Function* t);
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;

};

class CondExpr : public Node
{
 private:
  Option* typeop;
  list<opOptions>* ops;

  Node* type ;

 public:
  CondExpr(Option* t, list<opOptions>* o) ;
  ~CondExpr() ;
  void connect(Function* t);
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;

};

class CallExpr : public Node
{
 private:
  Option* typeop;
  Option* fnop;
  Option* argsop;

  Node* type ;
  Node* fn ;
  Node* args ;

 public:
  CallExpr(Option* t, Option* f, Option* a) ;
  ~CallExpr() ;
  void connect(Function* t);
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;
};

class StmtExpr : public Node
{
 private:
  Option* typeop;
  Option* stmtop;

  Node* type ;
  Node* stmt ;

 public:
  StmtExpr(Option* t, Option* s) ;
  ~StmtExpr() ;
  void connect(Function* t);
  void flatten() ;
};

class TargetExpr : public Node
{
 private:
  Option* typeop;
  Option* declop;
  Option* initop;
  Option* clnpop;

  Node* type ;
  Node* decl ;
  Node* init ;
  Node* clnp ;

 public:
  TargetExpr(Option* t, Option* d, Option* i, Option* c) ;
  ~TargetExpr() ;
  void connect(Function* t);
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;

};

class PlusExpr : public Node
{
 private:
  Option* typeop;
  list<opOptions>* ops;

  Node* type ;

 public:
  PlusExpr(Option* t, list<opOptions>* o) ;
  ~PlusExpr() ;
  void connect(Function* t);
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;

};

class MinusExpr : public Node
{
 private:
  Option* typeop;
  list<opOptions>* ops;

  Node* type ;

 public:
  MinusExpr(Option* t, list<opOptions>* o) ;
  ~MinusExpr();
  void connect(Function* t);
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;

};

class MultExpr : public Node
{
 private:
  Option* typeop;
  list<opOptions>* ops;

  Node* type ;

 public:
  MultExpr(Option* t, list<opOptions>* o) ;
  ~MultExpr() ;
  void connect(Function* t);
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;

};

class MaxExpr : public Node
{
 private:
  Option* typeop ;
  list<opOptions>* ops ;

  Node* type ;
 public:
  MaxExpr(Option* t, list<opOptions>* o) ;
  ~MaxExpr();
  void connect(Function* t);
  void flatten() ;
};

class LshiftExpr : public Node
{
 private:
  Option* typeop;
  list<opOptions>* ops;

  Node* type ;

 public:
  LshiftExpr(Option* t, list<opOptions>* o) ;
  ~LshiftExpr();
  void connect(Function* t);
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;

};

class RshiftExpr : public Node
{
 private:
  Option* typeop;
  list<opOptions>* ops;

  Node* type ;

 public:
  RshiftExpr(Option* t, list<opOptions>* o) ;
  ~RshiftExpr();
  void connect(Function* t);
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;

};

class BitIorExpr : public Node
{
 private:
  Option* typeop ;
  list<opOptions>* ops ;

  Node* type ;

 public:

  BitIorExpr(Option* t, list<opOptions>* o) ;
  ~BitIorExpr();
  void connect(Function* t);
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;

};

class BitXorExpr : public Node
{
 private:
  Option* typeop;
  list<opOptions>* ops;

  Node* type ;

 public:
  BitXorExpr(Option* t, list<opOptions>* o) ;
  ~BitXorExpr();
  void connect(Function* t);
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;

};

class BitAndExpr : public Node
{
 private:
  Option* typeop;
  list<opOptions>* ops;

  Node* type ;

 public:
  BitAndExpr(Option* t, list<opOptions>* o) ;
  ~BitAndExpr() ;
  void connect(Function* t);
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;

};

class BitNotExpr : public Node
{
 private:
  Option* typeop;
  list<opOptions>* ops;

  Node* type ;

 public:
  BitNotExpr(Option* t, list<opOptions>* o) ;
  ~BitNotExpr();
  void connect(Function* t);
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;

};

class LtExpr : public Node
{
 private:
  Option* typeop;
  list<opOptions>* ops;

  Node* type ;

 public:
  LtExpr(Option* t, list<opOptions>* o) ;
  ~LtExpr() ;
  void connect(Function* t);
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;

};

class LeExpr : public Node
{
 private:
  Option* typeop;
  list<opOptions>* ops;

  Node* type ;

 public:
  LeExpr(Option* t, list<opOptions>* o) ;
  ~LeExpr();
  void connect(Function* t);
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;

};

class GeExpr : public Node
{
 private:
  Option* typeop;
  list<opOptions>* ops;

  Node* type ;

 public:
  GeExpr(Option* t, list<opOptions>* o) ;
  ~GeExpr();
  void connect(Function* t);
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;
};

class GtExpr : public Node
{
 private:
  Option* typeop;
  list<opOptions>* ops;

  Node* type ;

 public:
  GtExpr(Option* t, list<opOptions>* o) ;
  ~GtExpr() ;
  void connect(Function* t);
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;

};

class EqExpr : public Node
{
 private:
  Option* typeop;
  list<opOptions>* ops;

  Node* type ;

 public:
  EqExpr(Option* t, list<opOptions>* o) ;
  ~EqExpr() ;
  void connect(Function* t);
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;

};

class NeExpr : public Node
{
 private:
  Option* typeop;
  list<opOptions>* ops;

  Node* type ;

 public:
  NeExpr(Option* t, list<opOptions>* o) ;
  ~NeExpr() ;
  void connect(Function* t);
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;

};

class ConvertExpr : public Node
{
 private:
  Option* typeop;
  list<opOptions>* ops;

  Node* type ;

 public:
  ConvertExpr(Option* t, list<opOptions>* o) ;
  ~ConvertExpr() ;
  void connect(Function* t);
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;

};

class NopExpr : public Node
{
 private:
  Option* typeop;
  list<opOptions>* ops;

  Node* type ;

 public:
  NopExpr(Option* t, list<opOptions>* o) ;
  ~NopExpr() ;
  void connect(Function* t);
  void flatten() ;
  LString resultRegister() ;

  ExecutionObject* generateSuif(Program* p) ;

};

class NonLvalueExpr : public Node
{
 private:
  Option* typeop;
  list<opOptions>* ops;

  Node* type ;

 public:
  NonLvalueExpr(Option* t, list<opOptions>* o) ;
  ~NonLvalueExpr() ;
  void connect(Function* t);
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;

};

class SaveExpr : public Node
{
 private:
  Option* typeop;
  list<opOptions>* ops;

  Node* type ;

 public:
  SaveExpr(Option* t, list<opOptions>* o) ;
  ~SaveExpr() ;
  void connect(Function* t);
  void flatten() ;
};

class BindExpr : public Node
{
 private:

  Option* typeop ;
  Option* varsop ;
  Option* bodyop ;

  Node* type ;
  Node* vars ;
  Node* body ;

 public:

  BindExpr(Option* t, Option* v, Option* b) ;
  ~BindExpr() ;
  void connect(Function* t) ;
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;

};

class ReturnExpr : public Node
{
 private:
  Option* typeop ;
  Option* exprop ;

  Node* type ;
  Node* expr ;
 public:

  ReturnExpr(Option* t, Option* e) ;
  ~ReturnExpr() ;
  void connect(Function* t) ;
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;
} ;

class CleanupPointExpr : public Node
{
 private:
  Option* typeop ;
  list<opOptions>* ops ;

  Node* type ;

 public:

  CleanupPointExpr(Option* t, list<opOptions>* o) ;
  ~CleanupPointExpr() ;
  void connect(Function* t) ;
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;

};

class DeclExpr : public Node
{
 private:

  Option* typeop ;

  Node* type ;

 public:
  DeclExpr(Option* t) ;
  ~DeclExpr() ;
  void connect(Function* t) ;
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;

};

class TruncDivExpr : public Node
{
 private:
  Option* typeop;
  list<opOptions>* ops;

  Node* type ;

 public:
  TruncDivExpr(Option* t, list<opOptions>* o) ;
  ~TruncDivExpr() ;
  void connect(Function* t);
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;
};

// Rdiv stands for real division.  
class RdivExpr : public Node 
{
 private:
  Option* typeop;
  list<opOptions>* ops;

  Node* type ;

 public:
  RdivExpr(Option* t, list<opOptions>* o) ;
  ~RdivExpr() ;
  void connect(Function* t);
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;
} ;

class TruncModExpr : public Node
{
 private:
  Option* typeop;
  list<opOptions>* ops;

  Node* type ;

 public:
  TruncModExpr(Option* t, list<opOptions>* o) ;
  ~TruncModExpr() ;
  void connect(Function* t);
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;
};

class LabelExpr : public Node
{
 private:
  Option* typeop ;
  Option* nameop ; // I just created this one, it could be named something else

  Node* type ;
  Node* name ;

 public:
  LabelExpr(Option* t, Option* n) ;
  ~LabelExpr() ;

  void connect(Function* t) ;
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;

};

class GotoExpr : public Node
{
 private:
  Option* typeop ;
  Option* lablop ;

  Node* type ;
  Node* labl ;

 public:

  GotoExpr(Option* t, Option* l) ;
  ~GotoExpr() ;

  void connect(Function* t) ;
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;

} ;

class NegateExpr : public Node
{
 private:
  Option* typeop ;
  list<opOptions>* ops ;

  Node* type ;

 public:

  NegateExpr(Option* t, list<opOptions>* o) ;
  ~NegateExpr() ;

  void connect(Function* t) ;
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;

};

class FixTruncExpr : public Node
{
 private:
  Option* typeop ;
  list<opOptions>* ops;

  Node* type ;

 public:

  FixTruncExpr(Option* t, list<opOptions>* o) ;
  ~FixTruncExpr() ;
  void connect(Function* t);
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;

};

class CaseLabelExpr : public Node
{
 private:
  Option* typeop ;
  Option* lownodeop ;

  Node* type ;
  Node* lownode ;
  bool isDefault ;

  CodeLabelSymbol* mySuifSymbol ;

 public:

  CaseLabelExpr(Option* t, Option* l) ;
  ~CaseLabelExpr() ;
  void connect(Function* t) ;
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;

  inline bool isDefaultCase() { return isDefault; }

  int theSize() ;
  CodeLabelSymbol* theSuifSymbol() ;

} ;

class FloatExpr : public Node
{
 private:

  Option* typeop ;
  list<opOptions>* ops;

  Node* type ;

 public:

  FloatExpr(Option* t, list<opOptions>* o) ;
  ~FloatExpr() ;
  void connect(Function* t);
  void flatten() ;

  ExecutionObject* generateSuif(Program* p) ;

};

class FilterExpr : public Node
{
 private:
 public:
};

class VecCondExpr : public Node
{
 private:
 public:
};

class LtgtExpr : public Node
{
 private:
 public:
};

class ViewConvertExpr : public Node
{
 private:
 public:
};


class ResxExpr : public Node
{
 private:
 public:
};

class AsmExpr : public Node
{
 private:
 public:
};

class CatchExpr : public Node
{
 private:
 public:
};

class EhFilterExpr : public Node
{
 private:
 public:
};

class WithSizeExpr : public Node
{
 private:
 public:
};

class RealignLoadExpr : public Node
{
 private:
 public:
};

class SizeofExpr : public Node
{
 private:
 public:
};

class AlignofExpr : public Node
{
 private:
 public:
};

class ArrowExpr : public Node
{
 private:
 public:
};

class CompoundLiteralExpr : public Node
{
 private:
 public:
};

class NwExpr : public Node
{
 private:
 public:
};

class VecNwExpr : public Node
{
 private:
 public:
};

class DlExpr : public Node
{
 private:
 public:
};

class VecDlExpr : public Node
{
 private:
 public:
};

class TypeExpr : public Node
{
 private:
 public:
};

class AggrInitExpr : public Node
{
 private:
 public:
};

class ThrowExpr : public Node
{
 private:
 public:
};

class EmptyClassExpr : public Node
{
 private:
 public:
};

class TemplateIdExpr : public Node
{
 private:
 public:
};

class PseudoDtorExpr : public Node
{
 private:
 public:
};

class ModopExpr : public Node
{
 private:
 public:
};

class CastExpr : public Node
{
 private:
 public:
};

class ReinterpretCastExpr : public Node
{
 private:
 public:
};

class ConstCastExpr : public Node
{
 private:
 public:
};

class StaticCastExpr : public Node
{
 private:
 public:
};

class DynamicCastExpr : public Node
{
 private:
 public:
};

class DotstarExpr : public Node
{
 private:
 public:
};

class TypeidExpr : public Node
{
 private:
 public:
};

class NonDependentExpr : public Node
{
 private:
 public:
};

#endif
