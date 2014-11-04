// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#ifndef UNREACHABLE_CODE_ELIMINATION_PASS_H
#define UNREACHABLE_CODE_ELIMINATION_PASS_H

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"
#include "common/suif_hash_map.h"

class UnreachableCodeEliminationPass : public PipelinablePass {
public:
  UnreachableCodeEliminationPass(SuifEnv *pEnv);
  Module* clone() const { return (Module*)this; }
  void do_procedure_definition(ProcedureDefinition *proc_def);
};

#endif
