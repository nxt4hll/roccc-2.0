// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#ifndef FLATTEN_SCOPE_STATEMENTS_PASS_H 
#define FLATTEN_SCOPE_STATEMENTS_PASS_H

#include "suifpasses/passes.h"
#include "suifnodes/suif.h"

class FlattenScopeStatementsPass : public PipelinablePass {
public:
  FlattenScopeStatementsPass( SuifEnv* suif_env );
  virtual ~FlattenScopeStatementsPass() {} 

  virtual Module* clone() const { return(Module*)this; }
  virtual void do_procedure_definition(ProcedureDefinition *proc_def);
};

#endif
