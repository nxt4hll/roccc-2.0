
#ifndef STATEMENT_LIST_UTILITIES_DOT_H
#define STATEMENT_LIST_UTILITIES_DOT_H

#include <basicnodes/basic.h>

int DeterminePosition(StatementList* containingList, Statement* toFind) ;

void AddChildStatementBefore(StatementList* parent, Statement* child, 
			     Statement* after) ;
void AddChildStatementAfter(StatementList* parent, Statement* child,
			    Statement* before) ;

#endif
