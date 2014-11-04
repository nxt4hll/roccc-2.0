// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#ifndef DFA_STATE_TABLE_EXPANSION_PASS_H
#define DFA_STATE_TABLE_EXPANSION_PASS_H

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"

class DFA_StateTableExpansionPass : public PipelinablePass {
public:
  DFA_StateTableExpansionPass(SuifEnv *pEnv);
  Module* clone() const { return (Module*)this; }
  void do_procedure_definition(ProcedureDefinition *proc_def);
};

#endif
