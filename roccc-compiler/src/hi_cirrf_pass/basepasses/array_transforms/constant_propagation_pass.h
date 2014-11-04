// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*

  This pass declared in this file is responsible for finding all constant 
   arrays and propagating the individual constant elements.

*/

#ifndef CONSTANT_QUALED_ARRAY_PROPAGATION_PASS_H
#define CONSTANT_QUALED_ARRAY_PROPAGATION_PASS_H

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"
#include "common/suif_map.h"

class ConstantQualedArrayPropagationPass : public PipelinablePass 
{
public:
  ConstantQualedArrayPropagationPass(SuifEnv *pEnv);
  Module* clone() const { return (Module*)this; }
  void do_procedure_definition(ProcedureDefinition *proc_def);
};

#endif
