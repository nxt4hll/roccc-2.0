// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details. 

/*
  This is the original ROCCC 1.0 version of feedback load elimination. It is
   still called when creating a systolic array.

  This pass must be called after scalar replacement and flatten statement 
   lists, but more importantly must be called while the control/data flow 
   solve passes are still valid (no new instructions are added).

*/

#ifndef FEEDBACK_LOAD_ELIMINATION_PASS_H
#define FEEDBACK_LOAD_ELIMINATION_PASS_H

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"
#include "common/suif_map.h"

class FeedbackLoadEliminationPass : public PipelinablePass 
{
public:
  FeedbackLoadEliminationPass(SuifEnv *pEnv);
  Module* clone() const { return (Module*)this; }
  void do_procedure_definition(ProcedureDefinition *proc_def);
};

#endif
