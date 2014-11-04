// The ROCCC Compiler Infrastructure 
//  This file is distributed under the University of California Open Source 
//  License.  See ROCCCLICENSE.TXT for details.

/*
   The purpose of this pass is to change for loops from the form of:
   i = 0 ; 
   for ( ; i < N ; ++i)
   { 
   }

   into the form of:

   {
   }
   for (i = 0 ; i < N ; ++i)
   {
   }

   This is an artifact of the way gcc creates for loops in the abstract
    syntax tree.

  Also, this handles the case where an unsigned value is compared with 0.
 */
#ifndef FOR_LOOP_PREPROCESSING_DOT_H
#define FOR_LOOP_PREPROCESSING_DOT_H

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>
#include <cfenodes/cfe.h>

class ForLoopPreprocessingPass : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;  

  void HandleComparison(CForStatement* c) ;
  void HandleBefore(CForStatement* c) ;

 public:
  ForLoopPreprocessingPass(SuifEnv* pEnv) ;
  Module* clone() const { return (Module*)this ; }
  void do_procedure_definition(ProcedureDefinition* proc_def) ;
} ;

#endif
