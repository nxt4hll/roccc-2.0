
#ifndef INNERMOST_UTILITIES_DOT_H
#define INNERMOST_UTILITIES_DOT_H

#include <suifnodes/suif.h>
#include <basicnodes/basic.h>
#include <cfenodes/cfe.h>

bool IsInnermostLoop(CForStatement* c) ;
bool IsOutermostLoop(CForStatement* c) ;
CForStatement* InnermostLoop(ProcedureDefinition* procDef) ;
StatementList* InnermostList(ProcedureDefinition* procDef) ;

#endif
