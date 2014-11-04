// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#ifndef DO_WHILE_TO_WHILE_TRANSFORM_PASS_H
#define DO_WHILE_TO_WHILE_TRANSFORM_PASS_H

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"
#include "common/suif_hash_map.h"

class DoWhileToWhileTransformPass : public PipelinablePass {
public:
  DoWhileToWhileTransformPass(SuifEnv *pEnv);
  Module* clone() const { return (Module*)this; }
  void do_procedure_definition(ProcedureDefinition *proc_def);
};

#endif
