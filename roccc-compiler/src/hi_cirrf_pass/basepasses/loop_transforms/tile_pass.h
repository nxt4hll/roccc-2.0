// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#ifndef TILE_PASS_H
#define TILE_PASS_H

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"

class TilePass : public PipelinablePass {
public:
  TilePass(SuifEnv *pEnv);
  void initialize();
  Module* clone() const { return (Module*)this; }
  void do_procedure_definition(ProcedureDefinition *proc_def);
private:
  String loop_label_argument1;
  String loop_label_argument2;
  int tile_size_argument1;	
  int tile_size_argument2;
};

#endif
