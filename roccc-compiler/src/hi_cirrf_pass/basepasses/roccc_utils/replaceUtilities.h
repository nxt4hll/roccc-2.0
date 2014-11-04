
#ifndef REPLACE_UTILITIES_DOT_H
#define REPLACE_UTILITIES_DOT_H

#include <basicnodes/basic.h>
#include <suifnodes/suif.h>

int ReplaceAllUsesWith(VariableSymbol* original, 
		       VariableSymbol* newSym,
		       StatementList* containingList,
		       int position) ;
int ReplaceAllDefsWith(VariableSymbol* original,
		       VariableSymbol* newSym,
		       StatementList* containingList,
		       int position) ;

// All of the previous replace child expressions functions 
//  have been removed and calls to the virtual function replace() have 
//  been put in their place.

void ReplaceOutputVariable(Expression* argument, VariableSymbol* v) ;
void ReplaceOutputVariable(CallStatement* call, VariableSymbol* original,
			   VariableSymbol* replacement) ;

#endif
