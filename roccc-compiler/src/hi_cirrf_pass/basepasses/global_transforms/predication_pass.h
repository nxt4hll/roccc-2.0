// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*

  This pass is responsible for changing nested If statements into predicated
   statements ready for conversion into boolsels.

*/

#ifndef __PREDICATION_PASS_DOT_H__
#define __PREDICATION_PASS_DOT_H__

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"

class PredicationPass : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;

  void PreprocessIfs() ;
  void PreprocessIf(IfStatement* i) ;
  
  void ProcessIfs() ;
  bool ProcessIf(IfStatement* i) ;

  void DetermineUndefinedVariables() ;

  void PullPredicatesUp() ;

  bool IsInnermostIf(IfStatement* i) ;

  // These put the fall through statements as the else portion
  void PostprocessIfs() ;
  void PostprocessIf(IfStatement* i) ;
  
  bool InList(list<VariableSymbol*>& toCheck, VariableSymbol* inThere) ;

  list<IfStatement*> CreatePredicates(Statement* toPredicate,
				      Expression* condition) ;

  list<VariableSymbol*> FindDefinedVariables(Statement* def) ;

  void FlattenIfs() ;
  bool FlattenIf(IfStatement* i) ;

  void FlattenStatementLists(IfStatement* i) ;
  StatementList* FlattenedList(StatementList* s) ;

  // After predication, we need to deal with function calls that may define
  //  multiple variables.  I'll do this by creating temporary variables and
  //  switching off of those.
  //  All of the functions here are associated with that task.
  void ConvertCalls() ;
  bool AppropriateCall(Statement* s) ;
  

 public:
  PredicationPass(SuifEnv* pEnv) ;
  Module* clone() const { return (Module*) this ; }
  void do_procedure_definition(ProcedureDefinition* p) ;

} ;

#endif 
