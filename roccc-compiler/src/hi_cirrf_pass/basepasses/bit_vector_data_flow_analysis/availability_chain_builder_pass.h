// The ROCCC Compiler Infrastructure 
//  This file is distributed under the University of California Open Source 
//  License.  See ROCCCLICENSE.TXT for details.   

#ifndef AVAILABILITY_CHAIN_BUILDER_PASS_H
#define AVAILABILITY_CHAIN_BUILDER_PASS_H

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"
#include "common/suif_hash_map.h"

class AvailabilityChainBuilderPass : public PipelinablePass 
{
public:
  AvailabilityChainBuilderPass(SuifEnv *pEnv);
  Module* clone() const { return (Module*)this; }
  void do_procedure_definition(ProcedureDefinition *proc_def);
};

#endif
