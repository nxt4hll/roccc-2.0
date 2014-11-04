// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

#ifndef PSEUDO_SSA_DOT_H
#define PSEUDO_SSA_DOT_H

#include <map>

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>

// This should perform all of the SSA steps I do, including the
//  handling of copy statements and the handling of call statements.

// This must be called before scalar replacement.

// This pass can expand to find all input, output, and feedback variables!

class PseudoSSA : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;

  int numPhis ;

  std::map<VariableSymbol*, MarkStatement*> correspondingPhis ;

  list<VariableSymbol*> potentialFeedbackVariables ;

  void CollectPotentialFeedbackVariables(StatementList* bodyList) ;
  void CreatePhis(StatementList* bodyList) ;
  void HandleBuiltInStatements(StatementList* bodyList) ;
  void HandleCopyStatements(StatementList* bodyList) ;
  void HandleCallStatements(StatementList* bodyList) ;
  void CreateFeedbackVariables(StatementList* bodyList) ;

  int ReplaceAllUsesWith(VariableSymbol* original, 
			 VariableSymbol* newSym,
			 StatementList* containingList,
			 int position) ;

  void ReplaceAllStoresWith(VariableSymbol* original,
			    VariableSymbol* newSym,
			    StatementList* containingList,
			    int position) ;

  VariableSymbol* CreateDuplicate(VariableSymbol* original,
				  CallStatement* c = NULL) ;

  bool InList(list<VariableSymbol*> l, VariableSymbol* v) ;

  LString BaseName(LString x) ;

 public:
  PseudoSSA(SuifEnv* pEnv) ;
  Module* clone() const { return (Module*) this ; }
  void do_procedure_definition(ProcedureDefinition* proc_def) ;
} ;

#endif
