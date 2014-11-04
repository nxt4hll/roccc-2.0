// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
/*
  This pass is responsible for renaming scalar variables to expose
   more parallelism.  It eliminates all Anti- and Output dependencies
   between scalar variables.

   Note: You must run UD/DU chain builder immediately before this pass.

*/

#ifndef SCALAR_RENAMING_PASS_TWO_DOT_H
#define SCALAR_RENAMING_PASS_TWO_DOT_H

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"
#include "basicnodes/basic.h"

class ScalarRenamingPass2 : public PipelinablePass 
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;

  list<StoreVariableStatement*> renamedList ;
  list<CallStatement*> renamedCallStmtList ;

  void ProcessStores() ;
  void ProcessCalls() ;

 public:
  ScalarRenamingPass2(SuifEnv *pEnv);
  Module* clone() const { return (Module*)this; }
  void do_procedure_definition(ProcedureDefinition *proc_def);
} ;

#endif
