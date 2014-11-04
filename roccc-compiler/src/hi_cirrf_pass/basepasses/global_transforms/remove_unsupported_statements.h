
#ifndef REMOVE_UNSUPPORTED_STATEMENTS_DOT_H
#define REMOVE_UNSUPPORTED_STATEMENTS_DOT_H

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>

class RemoveUnsupportedStatements : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;
 public:
  RemoveUnsupportedStatements(SuifEnv* pEnv) ;
  ~RemoveUnsupportedStatements() ;
  Module* clone() const { return (Module*) this ; } 
  void do_procedure_definition(ProcedureDefinition* p) ;
} ;

#endif
