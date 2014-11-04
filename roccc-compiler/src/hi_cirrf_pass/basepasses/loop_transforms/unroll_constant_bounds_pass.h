// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#ifndef UNROLL_CONSTANT_BOUNDS_PASS_H
#define UNROLL_CONSTANT_BOUNDS_PASS_H

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"

class UnrollConstantBoundsPass : public PipelinablePass {
public:
  UnrollConstantBoundsPass(SuifEnv *pEnv);
  void initialize();	
  Module* clone() const { return (Module*)this; }
  void do_procedure_definition(ProcedureDefinition *proc_def);
private:
  String loop_label_argument;
  int unroll_factor_limit_argument;
};

#endif
