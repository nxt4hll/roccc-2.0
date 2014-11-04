// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*
  This file represents the ROCCC 2.0 loop unrolling pass.  The differences
   between this pass and the original are that we no longer worry about
   the remainder loop and we have to unroll module calls differently than
   the original.

   In unrolling I tend to make things SSA to make it easier to identify
    feedback later on.  This is handled both in the "HandleCalls" pass and
    the "HandleCopyStatements"

   There will be hooks to unroll either as a pipeline or as parallel code.

*/

#ifndef UNROLL_PASS_TWO_DOT_H
#define UNROLL_PASS_TWO_DOT_H

#include <string>

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"
#include "cfenodes/cfe.h"

class UnrollPass2 : public PipelinablePass 
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;
  int originalStepValue ;
  VariableSymbol* index ;
  bool fullyUnrolled ;
  int unrollAmount ;

  bool ProcessLoop(CForStatement* c) ;

  // These functions work on the copied statements and adjust accordingly
  // These work when we unroll in parallel
  void ProcessLoadsParallel(Statement* n, int loopCopy) ;

  // This function is responsible for determining the original step
  //  value and the index of the loop for use in other functions.
  void CollectInformation(CForStatement* c) ;

  // These are the workhorse functions for collecting information
  void CollectStep(CForStatement* c) ;
  void CollectIndex(CForStatement* c) ;
  void CollectUnrollAmount(CForStatement* c) ;

  // This function is responsible for either replacing the entire for loop
  //  if we have completely unrolled it
  void AdjustOuterLoop(CForStatement* c) ;

  // This is responsible for changing the step from whatever it was
  //  before to what it should be.  Although we only print out i = i + 1
  //  in the hi-cirrf, this is still necessary to determine if we are
  //  outputting a smart buffer or a fifo.
  void AdjustStep(CForStatement* c) ;

  // Checks to see if the loop is the one we are looking for
  bool LabelledLoop(CForStatement* c) ;

  // Annotates the loop index as a loop index, so if it gets removed
  //  later I can make sure it is not marked as an output scalar
  void AnnotateIndex() ;

public:
  UnrollPass2(SuifEnv *pEnv);
  Module* clone() const { return (Module*)this; }
  void initialize(); 
  void do_procedure_definition(ProcedureDefinition *proc_def);

private:
  String loop_label_argument;
  int unroll_count_argument;
};

#endif
