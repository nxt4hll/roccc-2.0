// The ROCCC Compiler Infrastructure

#ifndef SUMMATION_PASS_DOT_H
#define SUMMATION_PASS_DOT_H

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>

class SummationPass : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;

  void ProcessStoreStatement(StoreStatement* s, StatementList* p, int pos) ;
  void ProcessStoreVariableStatement(StoreVariableStatement* s, 
				     StatementList* p, int pos) ;

  Expression* RemoveValue(Expression* original, VariableSymbol* value) ;

  bool IsSummation(Expression* e, VariableSymbol* v) ;
  bool OnlySummation(Expression* e, VariableSymbol* v) ;

 public:
  SummationPass(SuifEnv* pEnv) ;
  ~SummationPass() ;
  Module* clone() const { return (Module*) this ; }
  void do_procedure_definition(ProcedureDefinition* p) ;
} ;

#endif
