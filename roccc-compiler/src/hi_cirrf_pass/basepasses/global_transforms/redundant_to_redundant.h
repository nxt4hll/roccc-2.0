
#ifndef REDUNDANT_TO_REDUNDANT_PASS_DOT_H
#define REDUNDANT_TO_REDUNDANT_PASS_DOT_H

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>

class RedundantToRedundantPass : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;
  
  void ProcessCall(CallStatement* c) ;
 public:
  RedundantToRedundantPass(SuifEnv* pEnv) ;
  Module* clone() const { return (Module*)this ; }
  void do_procedure_definition(ProcedureDefinition* proc_def) ;
} ;

#endif
