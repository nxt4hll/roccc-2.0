
#ifndef USES_AND_DEFS_UTILITIES_DOT_H
#define USES_AND_DEFS_UTILITIES_DOT_H

#include <basicnodes/basic.h>
#include <suifnodes/suif.h>

bool HasUses(StatementList* s, int position, VariableSymbol* v) ;
bool HasUses(StatementList* s, int position , Expression* dest) ;
bool HasUses(Statement* s, VariableSymbol* v) ;
bool HasUses(Statement* s, Expression* dest) ;
bool NoUses(Statement* s, VariableSymbol* v) ;
bool NoUses(Statement* s, Expression* dest) ;
bool HasPreviousUses(StatementList* s, int position, VariableSymbol* v) ;
bool HasPreviousUses(StatementList* s, int position, Expression* dest) ;

bool IsDefinition(Statement* possibleDef, VariableSymbol* var) ;
bool IsPossibleDefinition(Statement* s) ;

bool IsUsedBeforeDefined(StatementList* s, int position, VariableSymbol* v) ;
bool IsUsedBeforeDefined(StatementList* s, int poisition, Expression* dest) ;

bool IsUsed(Type* t, CProcedureType* p) ;

bool HasDefs(StatementList* s, int position, VariableSymbol* v) ;
bool HasDefs(StatementList* s, int position, Expression* dest) ;
bool HasPreviousDefs(StatementList* s, int position, VariableSymbol* v) ;
bool HasPreviousDefs(StatementList* s, int position, Expression* dest) ;

#endif
