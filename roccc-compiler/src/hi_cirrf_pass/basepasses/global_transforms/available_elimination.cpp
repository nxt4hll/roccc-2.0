// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

#include <cassert>

#include <cfenodes/cfe.h>

#include "roccc_utils/warning_utils.h"
#include "roccc_utils/roccc2.0_utils.h"

#include "available_elimination.h"

AvailableCodeEliminationPass::AvailableCodeEliminationPass(SuifEnv* pEnv) :
  PipelinablePass(pEnv, "AvailableCodeEliminationPass")
{
  theEnv = pEnv ;
  procDef = NULL ;
}

void AvailableCodeEliminationPass::do_procedure_definition(ProcedureDefinition* p)
{
  procDef = p ;
  assert(procDef != NULL) ;

  OutputInformation("Available Code Elimination 2.0 Pass Begins") ;

  StatementList* innermost = NULL ;
  CForStatement* innermostFor = InnermostLoop(procDef) ;
  if (innermostFor != NULL)
  {
    innermost = dynamic_cast<StatementList*>(innermostFor->get_body()) ;
  }
  else
  {
    innermost = dynamic_cast<StatementList*>(procDef->get_body()) ;
  }
  assert(innermost != NULL) ;

  bool change ;
  do
  {
    change = false ;
    change |= ProcessStatementList(innermost) ;

  } while (change == true) ;  

  OutputInformation("Available Code Elimination 2.0 Pass Ends") ;
}

bool AvailableCodeEliminationPass::ProcessStatementList(StatementList* s)
{
  assert(s != NULL) ;
  bool changed = false ;
  list<int> statementsToRemove ;
  
  for (int i = 0 ; i < s->get_statement_count() ; ++i)
  {
    Statement* currentStatement = s->get_statement(i) ;
    assert(currentStatement != NULL) ;

    // Collect all output variables
    list<VariableSymbol*> currentOutputs = 
      AllDefinedVariables(currentStatement);

    // See if there are any uses and or definitions
    bool hasUses = false ;
    bool hasDefs = false ;
    list<VariableSymbol*>::iterator varIter = currentOutputs.begin() ;
    while (varIter != currentOutputs.end())
    {
      hasUses |= IsUsedBeforeDefined(s, i+1, *varIter) ;
      hasDefs |= HasDefs(s, i+1, *varIter) ;
      ++varIter ;
    }

    // If this statement happens to be a store statement, call the 
    //  appropriate functions for this as well.
    StoreStatement* currentStore = 
      dynamic_cast<StoreStatement*>(currentStatement) ;
    if (currentStore != NULL)
    {
      hasUses |= HasUses(s, i+1, currentStore->get_destination_address()) ;
      hasDefs |= HasDefs(s, i+1, currentStore->get_destination_address()) ;
    }

    // If no uses, but definitions, mark this statement for removal
    if (hasUses == false && hasDefs == true)
    {
      // Put them in reverse order so when we remove them we don't mess
      //  up the numbering
      statementsToRemove.push_front(i) ;
      changed = true ;
    }
  }

  // Now actually remove all statements that are marked as available
  list<int>::iterator removeIter = statementsToRemove.begin() ;
  while (removeIter != statementsToRemove.end())
  {
    // Delete?
    delete s->remove_statement(*removeIter) ;
    ++removeIter ;
  }

  return changed ;
}
