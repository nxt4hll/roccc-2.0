
#ifndef CLEANUP_BOOLSELS_DOT_H
#define CLEANUP_BOOLSELS_DOT_H

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>

class CleanupBoolSelsPass : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;

 public:
  CleanupBoolSelsPass(SuifEnv* pEnv) ;
  Module* clone() const { return (Module*) this ; }
  void do_procedure_definition(ProcedureDefinition* p) ;
} ;

#endif 
