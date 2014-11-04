// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

#ifndef LOOP_INFO_PASS_H
#define LOOP_INFO_PASS_H

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"

class LoopInfoPass : public PipelinablePass {
public:
  LoopInfoPass(SuifEnv *pEnv);
  Module* clone() const { return (Module*)this; }
  void do_procedure_definition(ProcedureDefinition *proc_def);
};

#endif
