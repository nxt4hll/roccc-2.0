// The ROCCC Compiler Infrastructure 
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

#ifndef SOLVE_AVAILABLE_EXPRESSIONS_PASS_H
#define SOLVE_AVAILABLE_EXPRESSIONS_PASS_H

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"
#include "common/suif_hash_map.h"

class SolveAvailableExpressionsPass : public PipelinablePass {
public:
  SolveAvailableExpressionsPass(SuifEnv *pEnv);
  Module* clone() const { return (Module*)this; }
  void do_procedure_definition(ProcedureDefinition *proc_def);
};

#endif

