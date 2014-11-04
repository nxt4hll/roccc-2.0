// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#ifndef PEEL_PASS_H
#define PEEL_PASS_H

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"

class PeelPass : public PipelinablePass {
public:
  PeelPass(SuifEnv *pEnv);
  void initialize(); 
  Module* clone() const { return (Module*)this; }
  void do_procedure_definition(ProcedureDefinition *proc_def);
private:
  String loop_label_argument;
  int first_iter_peel_count_argument;
  int last_iter_peel_count_argument;
};

#endif
