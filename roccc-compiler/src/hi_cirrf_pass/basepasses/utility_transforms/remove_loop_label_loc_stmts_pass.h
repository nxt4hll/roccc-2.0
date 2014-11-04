// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#ifndef REMOVE_LOOP_LABEL_LOC_STMTS_PASS_H 
#define REMOVE_LOOP_LABEL_LOC_STMTS_PASS_H

#include "suifpasses/passes.h"
#include "suifnodes/suif.h"

class RemoveLoopLabelLocStmtsPass : public PipelinablePass {
public:
  RemoveLoopLabelLocStmtsPass( SuifEnv* suif_env );
  virtual ~RemoveLoopLabelLocStmtsPass() {} 

  virtual Module* clone() const { return(Module*)this; }
  virtual void do_procedure_definition(ProcedureDefinition *proc_def);
};

#endif
