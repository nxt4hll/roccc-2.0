// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#ifndef CODE_HOISTING_PASS_H
#define CODE_HOISTING_PASS_H

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"

class CodeHoistingPass : public PipelinablePass {
public:
  CodeHoistingPass(SuifEnv *pEnv);
  Module* clone() const { return (Module*)this; }
  void do_procedure_definition(ProcedureDefinition *proc_def);
};

#endif
