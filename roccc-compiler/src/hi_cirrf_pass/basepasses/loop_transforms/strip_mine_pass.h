// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#ifndef STRIP_MINE_PASS_H
#define STRIP_MINE_PASS_H

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"

class StripMinePass : public PipelinablePass {
public:
  StripMinePass(SuifEnv *pEnv);
  void initialize();
  Module* clone() const { return (Module*)this; }
  void do_procedure_definition(ProcedureDefinition *proc_def);
private:
  String loop_label_argument;	
  int strip_size_argument;	// size of the strip
};

#endif
