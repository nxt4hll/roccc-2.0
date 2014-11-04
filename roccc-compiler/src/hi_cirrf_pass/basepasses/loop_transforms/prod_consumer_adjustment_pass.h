// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#ifndef PROD_CONSUMER_ADJUSTMENT_PASS_H
#define PROD_CONSUMER_ADJUSTMENT_PASS_H

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"

class ProdConsumerAdjustmentPass : public PipelinablePass {
public:
  ProdConsumerAdjustmentPass(SuifEnv *pEnv);
  Module* clone() const { return (Module*)this; }
  void do_procedure_definition(ProcedureDefinition *proc_def);
};

#endif
