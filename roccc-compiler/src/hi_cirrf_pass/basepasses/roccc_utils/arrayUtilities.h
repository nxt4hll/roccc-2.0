
#ifndef ARRAY_UTILITIES_DOT_H
#define ARRAY_UTILITIES_DOT_H

#include <basicnodes/basic.h>
#include <suifnodes/suif.h>

bool IsLookupTable(VariableSymbol* v) ;

VariableSymbol* GetArrayVariable(ArrayReferenceExpression* a) ;
QualifiedType* GetQualifiedTypeOfElement(ArrayReferenceExpression* ref) ;
QualifiedType* GetQualifiedTypeOfElement(VariableSymbol* v) ;
int GetDimensionality(ArrayReferenceExpression* a) ;
int GetDimensionality(VariableSymbol* v) ;

list<VariableSymbol*>* UsedIndicies(ArrayReferenceExpression* a) ;

#endif
