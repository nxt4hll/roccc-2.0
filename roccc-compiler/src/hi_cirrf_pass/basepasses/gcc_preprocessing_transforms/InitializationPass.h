// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*

  The purpose of this pass is to add a statement to the beginning of each
   statement list for variable initializations.

*/

#ifndef INITIALIZATION_PASS_DOT_H
#define INITIALIZATION_PASS_DOT_H

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>

class InitializationPass : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;

  void AddInitialization(VariableSymbol* v) ;

 public:
  InitializationPass(SuifEnv* pEnv) ;
  ~InitializationPass() ;
  void do_procedure_definition(ProcedureDefinition* p) ;
  Module* clone() const { return (Module*) this ; } 
} ;

#endif
