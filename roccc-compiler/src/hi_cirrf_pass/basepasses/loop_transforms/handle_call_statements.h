// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

#ifndef HANDLE_CALL_STATEMENTS_DOT_H
#define HANDLE_CALL_STATEMENTS_DOT_H

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>

class HandleCallStatements : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;  

  void HandleCalls(StatementList* bodyList) ;
  int ReplaceAllUsesWith(VariableSymbol* original,
			 VariableSymbol* newSym,
			 StatementList* containingList,
			 int position) ;
  VariableSymbol* CreateDuplicate(VariableSymbol* original) ;

  int numLoopBodies ;
  int originalStatementLength ;

 public:
  HandleCallStatements(SuifEnv* pEnv) ;
  Module* clone() const { return (Module*) this ; }
  void do_procedure_definition(ProcedureDefinition* proc_def) ;
} ;

#endif
