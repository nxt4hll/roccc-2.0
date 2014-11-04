#ifndef _UTILS_PRINT_UTILS_H_
#define _UTILS_PRINT_UTILS_H_

/**
  * @file
  * Utilities for printing objects.
  */

#include "common/MString.h"
#include "iokernel/object.h"



/** Get a short unique identification string for a SuifObject.
  * @param obj the object.
  * @return a string that uniquely identify \a obj.
  *
  * The id string returned has format \<Type\>_\<address\>, e.g.
  * "SymbolTable_0xf34745".  Except for named symbol objects,
  * they will have their name as prefix,
  * e.g. "\<count\>_VariableSymbol_0xf2235".
  *
  */
extern String to_id_string(const Object *obj);

#endif  // _UTILS_PRINT_UTILS_H_
