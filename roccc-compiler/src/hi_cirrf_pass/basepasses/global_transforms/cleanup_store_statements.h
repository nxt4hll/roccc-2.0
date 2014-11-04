
#ifndef CLEANUP_STORE_STATEMENTS_DOT_H
#define CLEANUP_STORE_STATEMENTS_DOT_H

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>

class CleanupStoreStatementsPass : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;
 public:
  CleanupStoreStatementsPass(SuifEnv* pEnv) ;
  Module* clone() const { return (Module*) this ; } 
  void do_procedure_definition(ProcedureDefinition* proc_def) ;
} ;

#endif
