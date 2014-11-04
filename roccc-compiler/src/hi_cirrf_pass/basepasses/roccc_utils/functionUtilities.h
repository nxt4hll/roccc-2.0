// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
/*

  All the functions in this unit deal with call statements.

*/

#ifndef FUNCTION_UTILITIES_DOT_H
#define FUNCTION_UTILITIES_DOT_H

#include "suifnodes/suif.h"

bool RedundantVoter(CallStatement* c) ;
bool IsBoolSel(CallStatement* c) ;
bool IsBuiltIn(CallStatement* c) ;

#endif
