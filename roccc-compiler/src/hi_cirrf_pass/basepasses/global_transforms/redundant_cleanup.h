/*
  This file makes sure that all error variables are different in all of the
   redundant functions.

*/

#ifndef REDUNDANT_CLEANUP_DOT_H
#define REDUNDANT_CLEANUP_DOT_H

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>

#include <list>

class CleanupRedundantVotes : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;

  std::list<VariableSymbol*> usedVariables ;

  void ProcessCall(CallStatement* c) ;

  bool InList(VariableSymbol* v) ;

 public:

  CleanupRedundantVotes(SuifEnv* pEnv) ;
  Module* clone() const { return (Module*)this ; }
  void do_procedure_definition(ProcedureDefinition* proc_def) ;
} ;

#endif
