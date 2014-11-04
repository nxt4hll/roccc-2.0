
#ifndef MARK_SYSTEM_TO_SYSTEM_DOT_H
#define MARK_SYSTEM_TO_SYSTEM_DOT_H

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>

class MarkSystemToSystemPass : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;
 public:
  MarkSystemToSystemPass(SuifEnv* pEnv) ;
  ~MarkSystemToSystemPass() ;
  Module* clone() const { return (Module*) this ; }
  void do_procedure_definition(ProcedureDefinition* p) ;
} ;

#endif
