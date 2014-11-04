
#include <cassert>

#include <cfenodes/cfe.h>
#include <suifkernel/utilities.h>

#include "identificationUtilities.h"

// Legacy modules take and return an instance of a struct type.  That struct
//  will be the only interaction with the outside world.
bool isLegacyModule(ProcedureDefinition* p)
{
  if (p == NULL)
  {
    return false ;
  }
  ProcedureSymbol* procSym = p->get_procedure_symbol() ;
  CProcedureType* cProcType = 
    dynamic_cast<CProcedureType*>(procSym->get_type()) ;
  assert(cProcType != NULL) ;

  // This expression is reliant on short circuit evaluation
  return ((cProcType->get_result_type() != NULL) &&
	  (cProcType->get_argument_count() == 1) &&
	  (dynamic_cast<StructType*>(cProcType->get_argument(0)->get_base_type()) != NULL) &&
	  (cProcType->get_result_type() == cProcType->get_argument(0)->get_base_type())) ;

}

// A Legacy system takes no parameters and returns no parameters
bool isLegacySystem(ProcedureDefinition* p) 
{
  if (p == NULL)
  {
    return false ;
  }
  ProcedureSymbol* procSym = p->get_procedure_symbol() ;
  CProcedureType* cProcType = 
    dynamic_cast<CProcedureType*>(procSym->get_type()) ;
  assert(cProcType != NULL) ;


  // The return type could be "void", so I need to check for that as well
  bool returnsVoid = ((cProcType->get_result_type() == NULL) ||
		      (dynamic_cast<VoidType*>(cProcType->get_result_type()) != NULL)) ;
  bool noArguments = (cProcType->get_argument_count() == 0) ;

  return (returnsVoid && noArguments) ;
}

// Current modules will have input and output arguments, but none of
//  them should be pointers
bool isModule(ProcedureDefinition* p)
{
  if (p == NULL)
  {
    return false ;
  }
  ProcedureSymbol* procSym = p->get_procedure_symbol() ;
  CProcedureType* cProcType = 
    dynamic_cast<CProcedureType*>(procSym->get_type()) ;
  assert(cProcType != NULL) ;

  if ((cProcType->get_result_type() != NULL) &&
      (dynamic_cast<VoidType*>(cProcType->get_result_type()) == NULL)) 
  {
    return false ;
  }
  
  if (cProcType->get_argument_count() == 0)
  {
    return false ;
  }

  for (int i = 0 ; i < cProcType->get_argument_count() ; ++i)
  {
    if (dynamic_cast<PointerType*>(cProcType->get_argument(i)->get_base_type()) != NULL) 
    {
      return false ;
    }
  }

  return true ;
}

// Current systems take parameters and one of them has to be a pointer
bool isSystem(ProcedureDefinition* p) 
{
  if (p == NULL)
  {
    return false ;
  }
  ProcedureSymbol* procSym = p->get_procedure_symbol() ;
  CProcedureType* cProcType = 
    dynamic_cast<CProcedureType*>(procSym->get_type()) ;
  assert(cProcType != NULL) ;
  
  if ((cProcType->get_result_type() != NULL) &&
      (dynamic_cast<VoidType*>(cProcType->get_result_type()) == NULL))
  {
    return false ;
  }

  if (cProcType->get_argument_count() == 0)
  {
    return false ;
  }

  for (int i = 0 ; i < cProcType->get_argument_count() ; ++i)
  {
    if (dynamic_cast<PointerType*>(cProcType->get_argument(i)->get_base_type()) != NULL)
    {
      return true ;
    }
  }  
  return false ;
}

bool isComposedSystem(ProcedureDefinition* p)
{
  if (p == NULL)
  {
    return false ;
  }

  // A composed system will have multiple function calls, and the individual
  //  function calls will take pointers, not scalar values.

  list<CallStatement*>* allCalls = 
    collect_objects<CallStatement>(p->get_body()) ;

  int numValidFunctions = 0 ;

  list<CallStatement*>::iterator callIter = allCalls->begin() ;
  while (callIter != allCalls->end())
  {
    bool validFunction = false ;
    // Go through all of the parameters and identify if any are pointers
    for (int i = 0 ; i < (*callIter)->get_argument_count() ; ++i)
    {
      Expression* currentArg = (*callIter)->get_argument(i) ;
      LoadVariableExpression* currentInputArg = 
	dynamic_cast<LoadVariableExpression*>(currentArg) ;
      if (currentInputArg != NULL)
      {
	if (dynamic_cast<PointerType*>(currentInputArg->get_source()->
				       get_type()->get_base_type()) != NULL)
	{
	  validFunction = true ;
	  break ;
	}
      }
    }
    if (validFunction)
    {
      ++numValidFunctions ;
    }
    ++callIter ;
  }
  delete allCalls ;
  
  list<CallExpression*>* allCallExprs = 
    collect_objects<CallExpression>(p->get_body()) ;
  list<CallExpression*>::iterator callExprIter = allCallExprs->begin() ;
  while (callExprIter != allCallExprs->end())
  {
    bool validFunction = false ;
    for (int i = 0 ; i < (*callExprIter)->get_argument_count() ; ++i)
    {
      Expression* currentArg = (*callExprIter)->get_argument(i) ;
      LoadVariableExpression* currentInputArg = 
	dynamic_cast<LoadVariableExpression*>(currentArg) ;
      if (currentInputArg != NULL)
      {
	if (dynamic_cast<PointerType*>(currentInputArg->get_source()->
				       get_type()->get_base_type()) != NULL)
	{
	  validFunction = true ;
	  break ;
	}
      }
    }
    if (validFunction)
    {
      ++numValidFunctions ;
    }
    ++callExprIter ;
  }
  delete allCallExprs ;

  return (numValidFunctions >= 2) ;
}

int CodeType(ProcedureDefinition* p)
{
  if (isLegacyModule(p) || isModule(p))
  {
    return MODULE ;
  }
  if (isLegacySystem(p) || isSystem(p))
  {
    return SYSTEM ;
  }
  if (isComposedSystem(p))
  {
    return COMPOSED_SYSTEM ;
  }
  return UNDETERMINABLE ;
}

bool isLegacy(ProcedureDefinition* p)
{
  return isLegacyModule(p) || isLegacySystem(p) ;
}
