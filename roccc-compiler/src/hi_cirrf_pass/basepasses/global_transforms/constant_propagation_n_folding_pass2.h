// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
/*

  This file is the ROCCC 2.0 version of constant propagation because the
   original pass missed some opportunities for some unknown reason.

   Note: Data flow and UD/DU chain building must be performed before this
   pass.

*/
#ifndef CONSTANT_PROPAGATION_AND_FOLDING_TWO_DOT_H
#define CONSTANT_PROPAGATION_AND_FOLDING_TWO_DOT_H

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"

class ConstantPropagationAndFoldingPass2 : public PipelinablePass 
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;

  bool FoldUnaryExpressions() ;
  
  bool FoldBinaryExpressions() ;

  bool FoldBoolSelect(CallStatement* c) ;
  bool FoldBoolSelects() ;

  bool SimpleFoldBinaryExpression(BinaryExpression* b) ;
  bool ComplexFoldBinaryExpression(BinaryExpression* b) ;

  bool ComplexFoldBinaryExpressions() ;
  bool PropagateConstants() ;

  bool FlipNegativeConstants(BinaryExpression* b) ;
  bool FlipSign(BinaryExpression* b) ;

  float StringToFloat(String x) ;
  String FloatToString(float x) ;

  Expression* Fold(Expression* x, Expression* y, LString opcode) ;

  bool SamePrecedence(LString opcode1, LString opcode2) ;

  Constant* Merge(Constant* x, Constant* y, LString opcode) ;

  Expression* Identity(Expression* x, Expression* y, LString opcode) ;

  Expression* IdentityFirst(Constant* x, Expression* y, LString opcode) ;
  Expression* IdentityFirst(Expression* x, Constant* y, LString opcode) ;

  Expression* IdentitySecond(Expression* x, Expression* y, LString opcode) ;

  Constant* FindConstant(Expression* current, LString opcode) ;

  // This function is for testing only and should be removed
  void DumpName(Constant* toDump) ;

  bool isConstant(Statement* s) ;

  bool IsFloatType(Expression* x) ;

  BinaryExpression* Splay(BinaryExpression* top, BinaryExpression* bottom) ;

 public:
  ConstantPropagationAndFoldingPass2(SuifEnv *pEnv);
  Module* clone() const { return (Module*)this; }
  void do_procedure_definition(ProcedureDefinition *proc_def);
};

#endif
