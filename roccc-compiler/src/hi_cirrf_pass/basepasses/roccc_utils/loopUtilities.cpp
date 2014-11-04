
#include "loopUtilities.h"

#include <suifkernel/utilities.h>
#include <cfenodes/cfe.h>

bool hasInfiniteLoop(ProcedureDefinition* procDef)
{
  bool infinite = false ;
  list<CForStatement*>* allFors = 
    collect_objects<CForStatement>(procDef->get_body()) ;
  list<CForStatement*>::iterator forIter = allFors->begin() ;
  while (forIter != allFors->end()) 
  {
    if (dynamic_cast<IntConstant*>((*forIter)->get_test()) != NULL)
    {
      infinite = true ;
    }
    ++forIter ;
  }
  delete allFors ;  
  return infinite ;
}
