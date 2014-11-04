// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#ifndef CODE_SINKING_PASS_H
#define CODE_SINKING_PASS_H

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"

class CodeSinkingPass : public PipelinablePass {
public:
  CodeSinkingPass(SuifEnv *pEnv);
  Module* clone() const { return (Module*)this; }
  void do_procedure_definition(ProcedureDefinition *proc_def);
};

#endif
