// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*
  The functions in this file deal with determining if two trees are equivalent.
*/

#ifndef EQUIVALENT_UTILITIES_DOT_H
#define EQUIVALENT_UTILITIES_DOT_H

#include <basicnodes/basic.h>
#include <suifnodes/suif.h>

bool EquivalentExpressions(Expression* x, Expression* y) ;
//bool EquivalentStatements(Statement* x, Statement* y) ;
bool EquivalentTypes(DataType* x, DataType* y) ;
bool EquivalentTypes(QualifiedType* x, QualifiedType* y) ;

#endif
