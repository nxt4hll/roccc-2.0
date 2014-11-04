
#include "miscUtilities.h"

LString TempName(LString baseName)
{
  static int counter = 0 ; 
  LString toReturn = baseName ;
  toReturn = toReturn + String(counter) ;
  ++counter ;
  return toReturn ;
}
