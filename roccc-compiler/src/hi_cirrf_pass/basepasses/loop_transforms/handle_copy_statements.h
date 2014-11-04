
#ifndef HANDLE_COPY_STATEMENTS_DOT_H
#define HANDLE_COPY_STATEMENTS_DOT_H

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>

class HandleCopyStatements : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;

  void HandleIf(IfStatement* s, StatementList* containingList, int position) ;

  void HandleCopies(StatementList* bodyList) ;
  VariableSymbol* CreateDuplicate(VariableSymbol* original) ;
  int ReplaceAllUsesWith(VariableSymbol* original, VariableSymbol* newSym,
			 StatementList* containingList, int position) ;

 public:
  HandleCopyStatements(SuifEnv* pEnv) ;
  Module* clone() const { return (Module*) this ; }
  void do_procedure_definition(ProcedureDefinition* proc_def) ;
} ;

#endif
