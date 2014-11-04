// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
/*
  This pass is responsible for fusing loops successive loops with the same
   bounds into one loop.
*/

#ifndef FUSE_PASS_H
#define FUSE_PASS_H

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"
#include "cfenodes/cfe.h"

class FusePass : public PipelinablePass {
public:
  FusePass(SuifEnv *pEnv);
  Module* clone() const { return (Module*)this; }
  void do_procedure_definition(ProcedureDefinition *proc_def);
};

#endif
