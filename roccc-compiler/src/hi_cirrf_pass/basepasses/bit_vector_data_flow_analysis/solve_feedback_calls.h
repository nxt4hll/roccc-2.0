/*

  The purpose of this pass is to find all calls to modules that
   both read and write to the same variable and split this into
   two separate variables.  

  Note: This is only relevant in 

*/

#ifndef SOLVE_FEEDBACK_CALLS_DOT_H
#define SOLVE_FEEDBACK_CALLS_DOT_H

#include <suifnodes/suif.h>
#include <suifpasses/suifpasses.h>

class SolveFeedbackCalls : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;

  void HandleCalls(StatementList* bodyList) ;

  void HandleSingleCall(CallStatement* c) ;

  CallStatement* DefinedByACall(VariableSymbol* var, StatementList* theList, 
				int position) ;

  bool IsABoolSelect(CallStatement* c) ;

 public:
  SolveFeedbackCalls(SuifEnv* pEnv) ;
  Module* clone() const { return (Module*) this ; }
  void do_procedure_definition(ProcedureDefinition* proc_def) ;
} ;

#endif
