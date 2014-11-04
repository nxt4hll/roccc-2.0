
#include <cassert>

#include <cfenodes/cfe.h>
#include <basicnodes/basic_factory.h>

#include "roccc_utils/warning_utils.h"
#include "roccc_utils/roccc2.0_utils.h"

#include "remove_unsupported_statements.h"

RemoveUnsupportedStatements::RemoveUnsupportedStatements(SuifEnv* pEnv) : 
  PipelinablePass(pEnv, "RemoveUnsupportedStatements")
{
  theEnv = pEnv ;
  procDef = NULL ;
}

RemoveUnsupportedStatements::~RemoveUnsupportedStatements()
{
  ; // Nothing to remove
}

void RemoveUnsupportedStatements::do_procedure_definition(ProcedureDefinition* p)
{
  procDef = p ;
  assert(procDef != NULL) ;

  OutputInformation("Remove Unsupported statements begins") ;

  CForStatement* innermost = InnermostLoop(procDef) ;
  if (innermost == NULL)
  {
    OutputInformation("Remove Unsupported statements ends") ;
    return ;
  }

  StatementList* parentList = 
    dynamic_cast<StatementList*>(innermost->get_parent()) ;
  CForStatement* outerLoops = 
    dynamic_cast<CForStatement*>(innermost->get_parent()) ;
  CForStatement* outermostLoop = innermost ;
  while (parentList == NULL && outerLoops != NULL)
  {
    parentList = dynamic_cast<StatementList*>(outerLoops->get_parent()) ;
    outermostLoop = outerLoops ;
    outerLoops = dynamic_cast<CForStatement*>(outerLoops->get_parent()) ;
  }
  assert(parentList != NULL) ;
  assert(outermostLoop != NULL) ;

  int position = DeterminePosition(parentList, outermostLoop) ;
  assert(position >= 0) ;
  
  for (int i = 0 ; i < position ; ++i)
  {
    StatementList* blankList = create_statement_list(theEnv) ;
    Statement* toReplace = parentList->get_statement(i) ;
    parentList->replace(toReplace, blankList) ;
    // Will this delete kill me?
    //delete toReplace ;
  }
  
  for (int i = position + 1 ; i < parentList->get_statement_count() ; ++i)
  {
    StatementList* blankList = create_statement_list(theEnv) ;
    Statement* toReplace = parentList->get_statement(i) ;
    parentList->replace(toReplace, blankList) ;
    // Will this delete kill me?... yep!  or not...
    //delete toReplace ;
  }
  
  OutputInformation("Remove Unsupported statments ends") ;
}
