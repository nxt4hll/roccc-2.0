// The ROCCC Compiler Infrastructure 
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.   

#ifndef RAW_ELIMINATION_PASS_H
#define RAW_ELIMINATION_PASS_H

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"
#include "common/suif_map.h"

// RAW = Read After Write

class RawEliminationPass : public PipelinablePass {
public:
  RawEliminationPass(SuifEnv *pEnv);
  Module* clone() const { return (Module*)this; }
  void do_procedure_definition(ProcedureDefinition *proc_def);
};

#endif
