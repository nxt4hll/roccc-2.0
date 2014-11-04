// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#ifndef CONST_QUALED_VAR_PROPAGATION_PASS_H
#define CONST_QUALED_VAR_PROPAGATION_PASS_H

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"
#include "common/suif_map.h"

class ConstQualedVarPropagationPass : public PipelinablePass {
public:
  ConstQualedVarPropagationPass(SuifEnv *pEnv);
  Module* clone() const { return (Module*)this; }
  void do_procedure_definition(ProcedureDefinition *proc_def);
};

#endif
