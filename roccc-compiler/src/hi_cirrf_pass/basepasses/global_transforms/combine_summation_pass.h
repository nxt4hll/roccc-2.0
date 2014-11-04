#ifndef COMBINE_SUMMATION_PASS_DOT_H
#define COMBINE_SUMMATION_PASS_DOT_H

/*
  The purpose of this pass is to do the same thing that the summation pass
   can do, but also combines all of the statements that are summations
   into one statement.
*/

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>

class CombineSummationPass : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;

  Expression* RemoveValue(StoreVariableStatement* s) ;
  Expression* RemoveValue(Expression* original, VariableSymbol* v) ;

  bool IsSummation(Expression* e, VariableSymbol* v) ;
  bool IsSummation(StoreVariableStatement* s, VariableSymbol* v) ;
  bool IsSummation(StoreVariableStatement* s) ;

  bool HasIntermediateUse(StatementList* s, VariableSymbol* v,
			  int startPos, int endPos) ;
  bool HasIntermediateDef(StatementList* s, VariableSymbol* v,
			  int startPos, int endPos) ;

 public:
  CombineSummationPass(SuifEnv* pEnv) ;
  ~CombineSummationPass() ;
  Module* clone() const { return (Module*) this ; } 
  void do_procedure_definition(ProcedureDefinition* p) ;
} ;

#endif 
