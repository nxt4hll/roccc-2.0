// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#ifndef FLATTEN_STATEMENT_LISTS_PASS_H 
#define FLATTEN_STATEMENT_LISTS_PASS_H

#include "suifpasses/passes.h"
#include "suifnodes/suif.h"

class FlattenStatementListsPass : public PipelinablePass {
public:
  FlattenStatementListsPass( SuifEnv* suif_env );
  virtual ~FlattenStatementListsPass() {} 

  virtual Module* clone() const { return(Module*)this; }
  virtual void do_procedure_definition(ProcedureDefinition *proc_def);
};

#endif
