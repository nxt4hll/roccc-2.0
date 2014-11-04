
#include "innermostUtilities.h"

#include <suifkernel/utilities.h>

bool IsInnermostLoop(CForStatement* c)
{
  bool innermost ;
  list<CForStatement*>* allFors = 
    collect_objects<CForStatement>(c->get_body()) ;
  innermost = (allFors->size() == 0) ;
  delete allFors ;
  return innermost ;
}

bool IsOutermostLoop(CForStatement* c)
{
  SuifObject* parent = c->get_parent() ;
  while (parent != NULL)
  {
    if (dynamic_cast<CForStatement*>(parent) != NULL)
    {
      return false ;
    }
    parent = parent->get_parent() ;
  }
  return true ;
}

CForStatement* InnermostLoop(ProcedureDefinition* procDef)
{
  assert(procDef != NULL) ;
  CForStatement* innermost = NULL ;

  list<CForStatement*>* allFors = 
    collect_objects<CForStatement>(procDef->get_body()) ;
  list<CForStatement*>::iterator forIter = allFors->begin() ;
  while (forIter != allFors->end())
  {
    Statement* body = (*forIter)->get_body() ;
    Iter<CForStatement> tmpIter = 
      object_iterator<CForStatement>(body) ;
    if (!tmpIter.is_valid())
    {
      innermost = (*forIter) ;
    }
    ++forIter ;
  }
  delete allFors ;
  return innermost ;
}

StatementList* InnermostList(ProcedureDefinition* procDef)
{
  CForStatement* innermostLoop = InnermostLoop(procDef) ;
  if (innermostLoop != NULL)
  {
    return dynamic_cast<StatementList*>(innermostLoop->get_body()) ;
  }
  else
  {
    return dynamic_cast<StatementList*>(procDef->get_body()) ;
  }
}
