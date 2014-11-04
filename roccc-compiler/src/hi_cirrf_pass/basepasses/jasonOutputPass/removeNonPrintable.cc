// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
/*

  This pass is responsible for locating all statements marked as non 
   printable and removing them.  Note, currently this pass does not delete 
   the statements and causes a temporary memory leak.

*/

#include "removeNonPrintable.h"

#include "roccc_utils/warning_utils.h"

#include "suifkernel/utilities.h"

RemoveNonPrintablePass::RemoveNonPrintablePass(SuifEnv* pEnv) : 
  PipelinablePass(pEnv, "RemoveNonPrintablePass")
{
  theEnv = pEnv ;
}

void RemoveNonPrintablePass::do_procedure_definition(ProcedureDefinition* p)
{
  OutputInformation("Remove non printable begins") ;

  // Go through every statement.  If it is non printable, then remove it
  list<Statement*>* allStatements = 
    collect_objects<Statement>(p->get_body()) ;
  assert(allStatements != NULL) ;

  list<Statement*>::iterator statementIter = allStatements->begin() ;
  while (statementIter != allStatements->end())
  {
    // The only non printables we want to remove are store variable
    //  statements or store statements.
    if (dynamic_cast<StoreStatement*>(*statementIter) == NULL &&
	dynamic_cast<StoreVariableStatement*>(*statementIter) == NULL)
    {
      ;  // Don't do anything 
    }
    else if ((*statementIter)->lookup_annote_by_name("NonPrintable") != NULL)
    {
      delete (*statementIter)->remove_annote_by_name("NonPrintable") ;

      (*statementIter)->get_parent()->replace((*statementIter),
					      NULL) ;

    }
    ++statementIter ;
  }

  delete allStatements ;

  OutputInformation("Remove non printable ends") ;
}
