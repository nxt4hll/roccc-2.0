
#ifndef PREFERENCE_PASS_DOT_H
#define PREFERENCE_PASS_DOT_H

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>

class PreferencePass : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ; 

  String versionString ;

 public:
  PreferencePass(SuifEnv* pEnv) ;
  Module* clone() const { return (Module*) this ; }
  void do_procedure_definition(ProcedureDefinition* proc_def) ;
  void initialize() ;
} ;

#endif
