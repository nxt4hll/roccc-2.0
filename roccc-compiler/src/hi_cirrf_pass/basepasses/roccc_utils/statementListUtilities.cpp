
#include "statementListUtilities.h"

int DeterminePosition(StatementList* containingList, Statement* toFind) 
{
  assert(containingList != NULL) ;
  assert(toFind != NULL) ;
  for (int i = 0 ; i < containingList->get_statement_count() ; ++i)
  {
    if (containingList->get_statement(i) == toFind)
    {
      return i ;
    }
  }
  return -1 ;
}

void AddChildStatementBefore(StatementList* parent, Statement* child, 
			     Statement* after)
{
  int position = DeterminePosition(parent, after) ;
  assert(position != -1) ;
  parent->insert_statement(position, child) ;
}

void AddChildStatementAfter(StatementList* parent, Statement* child,
			    Statement* before)
{
  int position = DeterminePosition(parent, before) ;
  assert(position != -1) ;
  parent->insert_statement(position + 1, child) ;
}
