
#ifndef CLEANUP_LOAD_PASS_DOT_H
#define CLEANUP_LOAD_PASS_DOT_H

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>

class CleanupLoadPass : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;
 public:
  CleanupLoadPass(SuifEnv* pEnv) ;
  ~CleanupLoadPass() ;
  Module* clone() const { return (Module*) this ;}
  void do_procedure_definition(ProcedureDefinition* p) ;
} ;

#endif
