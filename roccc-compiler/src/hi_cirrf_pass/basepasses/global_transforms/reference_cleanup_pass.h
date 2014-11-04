// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details

/*
   The purpose of this pass is to convert any reference variables into 
    symbol addresses.  This is neccessary right now for passing references
    into and out of child function processes.
*/

#ifndef REFERENCE_CLEANUP_PASS_DOT_H
#define REFERENCE_CLEANUP_PASS_DOT_H

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>

class ReferenceCleanupPass : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;

  void CleanupCalls() ;
  void CleanupCall(CallStatement* c) ;

  void CleanupArrayStores() ;
  void CleanupArrayStore(StoreStatement* s) ;

  VariableSymbol* FindVariable(Expression* e) ;

 public:
  ReferenceCleanupPass(SuifEnv* pEnv) ;
  ~ReferenceCleanupPass() ;
  void do_procedure_definition(ProcedureDefinition* p) ;
  Module* clone() const { return (Module*) this ; } 
} ;

#endif
