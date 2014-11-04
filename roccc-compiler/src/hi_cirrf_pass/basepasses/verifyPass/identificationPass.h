
#ifndef IDENTIFICATION_PASS_DOT_H
#define IDENTIFICATION_PASS_DOT_H

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>

class IdentificationPass : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;

  int DetermineType() ;

  bool IsModule() ;
  bool IsComposedSystem() ;

 public:
  IdentificationPass(SuifEnv* pEnv) ;
  ~IdentificationPass() ;
  Module* clone() const { return (Module*) this ; }
  void do_procedure_definition(ProcedureDefinition* p) ; 
} ;

#endif
