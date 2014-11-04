// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*
  The functions described here deal with the the procedure as a whole and
    if it is a module or system.
*/

#ifndef IDENTIFICATION_UTILITIES_DOT_H
#define IDENTIFICATION_UTILITIES_DOT_H

#include <suifnodes/suif.h>

const int MODULE = 1 ;
const int SYSTEM = 2 ;
const int COMPOSED_SYSTEM = 3 ;
const int UNDETERMINABLE = 99 ;

bool isLegacyModule(ProcedureDefinition* p) ;
bool isLegacySystem(ProcedureDefinition* p) ;
bool isModule(ProcedureDefinition* p) ;
bool isSystem(ProcedureDefinition* p) ;
bool isComposedSystem(ProcedureDefinition* p) ;
int CodeType(ProcedureDefinition* p) ;
bool isLegacy(ProcedureDefinition* p) ;

#endif 
