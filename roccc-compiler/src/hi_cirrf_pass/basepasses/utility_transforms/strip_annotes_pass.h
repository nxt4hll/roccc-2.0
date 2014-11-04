// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#ifndef STRIP_ANNOTES_PASS_H 
#define STRIP_ANNOTES_PASS_H

#include "suifpasses/passes.h"
#include "suifnodes/suif.h"

class StripAnnotesPass : public PipelinablePass 
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;
 public:
  StripAnnotesPass( SuifEnv* suif_env );
  virtual ~StripAnnotesPass() {} 

  virtual Module* clone() const { return(Module*)this; }
  virtual void do_procedure_definition(ProcedureDefinition *p);
};

#endif

