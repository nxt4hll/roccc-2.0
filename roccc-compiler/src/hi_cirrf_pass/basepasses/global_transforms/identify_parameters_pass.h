
#ifndef IDENTIFY_PARAMETERS_PASS_DOT_H
#define IDENTIFY_PARAMETERS_PASS_DOT_H

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>

class IdentifyParametersPass : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;

  void MarkParameter(ParameterSymbol* p) ;

 public:
  IdentifyParametersPass(SuifEnv* pEnv) ;
  Module* clone() const { return (Module*) this ; }
  void do_procedure_definition(ProcedureDefinition* p) ;
} ;

#endif
