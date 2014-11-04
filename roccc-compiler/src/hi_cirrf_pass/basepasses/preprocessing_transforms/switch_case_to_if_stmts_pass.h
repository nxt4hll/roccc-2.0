// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#ifndef SWITCH_CASE_TO_IF_STMTS_PASS_H
#define SWITCH_CASE_TO_IF_STMTS_PASS_H

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"

class SwitchCaseToIfStmtsPass : public PipelinablePass {
public:
  SwitchCaseToIfStmtsPass(SuifEnv *pEnv);
  Module* clone() const { return (Module*)this; }
  void do_procedure_definition(ProcedureDefinition *proc_def);
};

#endif
