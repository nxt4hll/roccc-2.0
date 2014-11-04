
#include "statementUtilities.h"

// This function should only be called if we know a next statement exists,
//  such as when we are finding the statement after a label
Statement* NextStatement(Statement* s)
{
  assert(s != NULL) ;
  StatementList* containingList = 
    dynamic_cast<StatementList*>(s->get_parent()) ;
  assert(containingList != NULL) ;
  
  for (int i = 0 ; i < containingList->get_statement_count() ; ++i)
  {
    Statement* current = containingList->get_statement(i) ;
    if (current == s)
    {
      assert(i != containingList->get_statement_count() - 1) ;
      return containingList->get_statement(i+1) ;
    }
  }
  return NULL ;
}

Statement* PreviousStatement(Statement* s)
{
  assert(s != NULL) ;
  StatementList* containingList = 
    dynamic_cast<StatementList*>(s->get_parent()) ;
  assert(containingList != NULL) ;
  
  Statement* previous = NULL ;

  for (int i = 0 ; i < containingList->get_statement_count() ; ++i)
  {
    Statement* current = containingList->get_statement(i) ;
    if (current == s)
    {
      return previous ;
    }
    previous = current ;
  }

  return NULL ;  
}
