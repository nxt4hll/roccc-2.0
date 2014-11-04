// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*
  The functions in this unit are responsible for dealing with statements
   including their relative positions and relationship to each other.
*/

#ifndef STATEMENT_UTILITIES_DOT_H
#define STATEMENT_UTILITIES_DOT_H

#include <basicnodes/basic.h>

int DeterminePosition(StatementList* containingList, Statement* toFind) ;
Statement* NextStatement(Statement* s) ;
Statement* PreviousStatement(Statement* s) ;

#endif
