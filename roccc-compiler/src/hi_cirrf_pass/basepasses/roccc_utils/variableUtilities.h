// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
/*
  This file deals with utility functions that return some fact about 
   a variable symbol
*/

#ifndef VARIABLE_UTILITIES_DOT_H
#define VARIABLE_UTILITIES_DOT_H

#include <basicnodes/basic.h>
#include <suifnodes/suif.h>

bool isConst(VariableSymbol* v) ;
bool isOutput(VariableSymbol* v) ;
bool isInput(VariableSymbol* v) ;

list<VariableSymbol*> AllDefinedVariables(Statement* s) ;
VariableSymbol* GetDefinedVariable(Statement* def, int location) ;
VariableSymbol* GetOutputVariable(Expression* argument) ;
bool IsOutputVariable(Expression* argument) ;
bool IsOutputVariable(VariableSymbol* v, Statement* s) ;

bool IsStream(Expression* argument) ;
bool IsStream(VariableSymbol* v) ;

list<VariableSymbol*> AllUsedVariables(Statement* s) ;
list<VariableSymbol*> AllUsedVariablesBut(Statement* s, VariableSymbol* v) ;

VariableSymbol* ScalarReplacedVariable(VariableSymbol* array, 
				       list<int> dimOffsets) ;

#endif
