// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

#ifndef PRE_VERIFY_PASS_DOT_H
#define PRE_VERIFY_PASS_DOT_H

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>

class PreVerifyPass : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;

  list<VariableSymbol*> allIndicies ;
  void CollectIndicies() ;
  bool IsIndex(VariableSymbol* v) ;

  bool EmptyFunction() ;
  bool ProperStoreVariables() ;
  bool ProperLoopSteps() ;
  
 public:
  PreVerifyPass(SuifEnv* pEnv) ;
  ~PreVerifyPass() ;
  Module* clone() const { return (Module*) this ; }
  void do_procedure_definition(ProcedureDefinition* p) ;
} ;

#endif
