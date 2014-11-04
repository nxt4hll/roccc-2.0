// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*

  The purpose of this pass is to identify all of the function calls and
   labels used in this function and report this information back to 
   the GUI (which is written by Roby).

*/

#ifndef ROBY_PREPROCESSING_PASS_DOT_H
#define ROBY_PREPROCESSING_PASS_DOT_H

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>

class RobyPreprocessingPass : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;
 public:
  RobyPreprocessingPass(SuifEnv* pEnv) ;
  ~RobyPreprocessingPass() ;
  void do_procedure_definition(ProcedureDefinition* p) ;
  Module* clone() const { return (Module*) this ; }
} ;

#endif
