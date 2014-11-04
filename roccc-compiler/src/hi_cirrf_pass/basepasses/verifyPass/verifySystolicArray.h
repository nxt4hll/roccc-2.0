
#ifndef VERIFY_SYSTOLIC_DOT_H
#define VERIFY_SYSTOLIC_DOT_H

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>

class VerifySystolicPass : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;

  bool VerifyType() ;
  bool VerifyLoopNest() ;
  bool VerifyLoopBounds() ;
  bool VerifyArrays() ;

 public:
  VerifySystolicPass(SuifEnv* pEnv) ;
  ~VerifySystolicPass() ;
  Module* clone() const { return (Module*) this ; }
  void do_procedure_definition(ProcedureDefinition* p) ;
} ;

#endif
