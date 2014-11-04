// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

// The purpose of this pass is to propagate constants in small circumstances
//  Data flow and UD/DU chains are not necessary for this pass as everything
//  is local within basic blocks.

#ifndef MINI_CONSTANT_PROPAGATION_DOT_H
#define MINI_CONSTANT_PROPAGATION_DOT_H

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>

class MiniConstantPropagationPass : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;

  void ProcessList(StatementList* s) ;
  bool MiniFold(StatementList* s) ;
  Constant* Merge(Constant* x, Constant* y, LString opcode) ;
  float StringToFloat(String x) ;
  String FloatToString(float x) ;

 public:
  MiniConstantPropagationPass(SuifEnv* pEnv) ;
  Module* clone() const { return (Module*) this ; }
  void do_procedure_definition(ProcedureDefinition* p) ;
} ;

#endif
