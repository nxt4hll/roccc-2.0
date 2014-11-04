// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source 
//  License.  See ROCCCLICENSE.TXT for details.   

#ifndef SCALAR_REPLACEMENT_PASS_H
#define SCALAR_REPLACEMENT_PASS_H

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"
#include "common/suif_map.h"

class ScalarReplacementPass : public PipelinablePass {
public:
  ScalarReplacementPass(SuifEnv *pEnv);
  Module* clone() const { return (Module*)this; }
  void do_procedure_definition(ProcedureDefinition *proc_def);
private:
  suif_map<Expression*, VariableSymbol*>* load_scalar_temporaries;
  suif_map<Expression*, VariableSymbol*>* store_scalar_temporaries;
};

#endif
