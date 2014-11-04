// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#ifndef HW_SW_BOUNDARY_MARK_PASS_H
#define HW_SW_BOUNDARY_MARK_PASS_H

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"

class HW_SW_BoundaryMarkPass : public PipelinablePass {
public:
  HW_SW_BoundaryMarkPass(SuifEnv *pEnv);
  Module* clone() const { return (Module*)this; }
  void do_procedure_definition(ProcedureDefinition *proc_def);
};

#endif
