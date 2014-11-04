// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source 
//  License.  See ROCCCLICENSE.TXT for details.   

#ifndef SYSTOLIC_ARRAY_GENERATION_PASS_H
#define SYSTOLIC_ARRAY_GENERATION_PASS_H

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"
#include "common/suif_map.h"

class SystolicArrayGenerationPass : public PipelinablePass {
public:
  SystolicArrayGenerationPass(SuifEnv *pEnv);
  Module* clone() const { return (Module*)this; }
  void initialize();
  void do_procedure_definition(ProcedureDefinition *proc_def);
private:
  int is_internal_array_values_saved;
};

#endif
