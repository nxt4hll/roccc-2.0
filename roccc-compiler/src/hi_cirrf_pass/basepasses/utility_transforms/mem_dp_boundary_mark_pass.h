// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#ifndef MEM_DP_BOUNDARY_MARK_PASS_H
#define MEM_DP_BOUNDARY_MARK_PASS_H

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"

class MEM_DP_BoundaryMarkPass : public PipelinablePass {
public:
  MEM_DP_BoundaryMarkPass(SuifEnv *pEnv);
  Module* clone() const { return (Module*)this; }
  void do_procedure_definition(ProcedureDefinition *proc_def);
};

#endif
