
#ifndef MULTIPLY_BY_CONST_DOT_H
#define MULTIPLY_BY_CONST_DOT_H

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>

class MultiplyByConstEliminationPass2 : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;

  list<int> ComputePowerSeries(int n) ;

  void ProcessBinaryExpression(BinaryExpression* b) ;
 public:
  MultiplyByConstEliminationPass2(SuifEnv* pEnv) ;
  Module* clone() const { return (Module*) this ; }
  void do_procedure_definition(ProcedureDefinition* p) ;
} ;

#endif
