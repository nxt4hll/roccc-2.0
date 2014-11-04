
#ifndef INTRINSIC_PASS_DOT_H
#define INTRINSIC_PASS_DOT_H

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>

class IntrinsicPass : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;

  String name ;

 public:
  IntrinsicPass(SuifEnv* pEnv) ;
  Module* clone() const { return (Module*) this ; }
  void do_procedure_definition(ProcedureDefinition* proc_def) ;
  void initialize() ;
} ;

#endif
