// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#ifndef UNSWITCH_PASS_H
#define UNSWITCH_PASS_H

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"

class UnswitchPass : public PipelinablePass {
public:
  UnswitchPass(SuifEnv *pEnv);
  void initialize();
  Module* clone() const { return (Module*)this; }
  void do_procedure_definition(ProcedureDefinition *proc_def);
private:
  String loop_label_argument;
};

#endif
