// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

#include <cfenodes/cfe.h>
#include <basicnodes/basic_factory.h>
#include <suifkernel/utilities.h>

#include "roccc_utils/warning_utils.h"

#include "normalize_statement_lists.h"

NormalizeStatementListsPass::NormalizeStatementListsPass(SuifEnv* pEnv) :
  PipelinablePass(pEnv, "NormalizeStatementListsPass")
{
  theEnv = pEnv ;
  procDef = NULL ;
}

void NormalizeStatementListsPass::do_procedure_definition(ProcedureDefinition* proc_def)
{
  procDef = proc_def ;
  assert(procDef != NULL) ;
  assert(theEnv != NULL) ;

  OutputInformation("Normalize Statement Lists Pass Begins") ;

  NormalizeForLoops() ;
  NormalizeIfStatements() ;
  NormalizeProcDef() ;

  OutputInformation("Normalize Statement Lists Pass Ends") ;
  
}

void NormalizeStatementListsPass::NormalizeForLoops()
{
  assert(procDef != NULL) ;

  list<CForStatement*>* allFors = 
    collect_objects<CForStatement>(procDef->get_body()) ;

  list<CForStatement*>::iterator forIter = allFors->begin() ;
  while (forIter != allFors->end()) 
  {
    Statement* currentBody = (*forIter)->get_body() ;
    StatementList* bodyList = 
      dynamic_cast<StatementList*>(currentBody) ;
    if (bodyList == NULL)
    {
      bodyList = create_statement_list(theEnv) ;
      (*forIter)->set_body(NULL) ;
      bodyList->append_statement(currentBody) ;
      (*forIter)->set_body(bodyList) ;
    }
    // Apparently, I also have to do something about the step.
    StatementList* stepList = 
      dynamic_cast<StatementList*>((*forIter)->get_step()) ;
    if (stepList != NULL)
    {
      assert(stepList->get_statement_count() == 1 && 
	     "Incorrectly formed step!") ;
      (*forIter)->set_step(stepList->remove_statement(0)) ;
      delete stepList ;
    }
    ++forIter ;
  }

  delete allFors ;

}

void NormalizeStatementListsPass::NormalizeIfStatements()
{
  assert(procDef != NULL) ;

  list<IfStatement*>* allIfs = 
    collect_objects<IfStatement>(procDef->get_body()) ;

  list<IfStatement*>::iterator ifIter = allIfs->begin() ;
  while (ifIter != allIfs->end())
  {
    // Check both the "then" and "else" part
    Statement* thenPart = (*ifIter)->get_then_part() ;
    Statement* elsePart = (*ifIter)->get_else_part() ;

    StatementList* thenList = dynamic_cast<StatementList*>(thenPart) ;
    StatementList* elseList = dynamic_cast<StatementList*>(elsePart) ;
    if (thenList == NULL && thenPart != NULL)
    {
      thenList = create_statement_list(theEnv) ;
      (*ifIter)->set_then_part(NULL) ;
      (*ifIter)->set_then_part(thenList) ;
      thenList->append_statement(thenPart) ;
    }

    if (elseList == NULL && elsePart != NULL)
    {
      elseList = create_statement_list(theEnv) ;
      (*ifIter)->set_else_part(NULL) ;
      (*ifIter)->set_else_part(elseList) ;
      elseList->append_statement(elsePart) ;
    }

    ++ifIter ;
  }

  delete allIfs ;

}

void NormalizeStatementListsPass::NormalizeProcDef()
{
  assert(procDef != NULL) ;
  assert(theEnv != NULL) ;

  Statement* body = dynamic_cast<Statement*>(procDef->get_body()) ;
  assert(body != NULL) ;
  StatementList* bodyList = dynamic_cast<StatementList*>(body) ;

  if (bodyList == NULL)
  {
    bodyList = create_statement_list(theEnv) ;
    procDef->set_body(bodyList) ;
    bodyList->append_statement(body) ;
  }
  
}
