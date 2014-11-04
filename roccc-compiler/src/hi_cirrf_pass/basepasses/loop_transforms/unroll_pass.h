// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#ifndef UNROLL_PASS_H
#define UNROLL_PASS_H

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"

class UnrollPass : public PipelinablePass {
public:
  UnrollPass(SuifEnv *pEnv);
  Module* clone() const { return (Module*)this; }
  void initialize(); 
  void do_procedure_definition(ProcedureDefinition *proc_def);
private:
  String loop_label_argument;
  int unroll_count_argument;
};

#endif
