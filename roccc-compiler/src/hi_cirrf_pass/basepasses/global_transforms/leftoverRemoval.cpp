// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include <cassert>

#include "cfenodes/cfe.h"

#include "roccc_utils/warning_utils.h"
#include "leftoverRemoval.h"

LeftoverRemovalPass::LeftoverRemovalPass(SuifEnv* pEnv) : 
  PipelinablePass(pEnv, "LeftoverRemovalPass")
{
  theEnv = pEnv ;
  procDef = NULL ;
}

void LeftoverRemovalPass::do_procedure_definition(ProcedureDefinition* proc_def)
{
  procDef = proc_def ;
  assert(procDef != NULL) ;

  OutputInformation("Leftover removal pass begins") ;

  // All I need to do is go through each statement in the body and remove it
  //  if it is not a for loop.  Hopefully this doesn't remove j = 0...

  StatementList* fullList = 
    dynamic_cast<StatementList*>(procDef->get_body()) ;
  assert(fullList != NULL) ;

  int pos = 0 ;
  Iter<Statement*> statementIter = fullList->get_statement_iterator() ;
  while (statementIter.is_valid())
  {
    // When we remove, reset.  Otherwise just move on
    if (dynamic_cast<CForStatement*>(statementIter.current()) == NULL)
    {
      fullList->remove_statement(pos) ;
      statementIter = fullList->get_statement_iterator() ;
      pos = 0 ;
    }
    else
    {
      statementIter.next() ;
      ++pos ;
    }
  }

  OutputInformation("Leftover removal pass ends") ;
}
