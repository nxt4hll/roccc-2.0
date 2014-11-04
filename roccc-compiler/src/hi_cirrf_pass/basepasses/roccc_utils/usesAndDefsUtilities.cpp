
#include <suifkernel/utilities.h>
#include <cfenodes/cfe.h>

#include "usesAndDefsUtilities.h"
#include "equivalentUtilities.h"

bool HasUses(StatementList* s, int position, VariableSymbol* v)
{
  for (int i = position ; i < s->get_statement_count() ; ++i)
  {
    Statement* currentStatement = s->get_statement(i) ;
    if (!NoUses(currentStatement, v))
    {
      return true ;
    }
  }
  return false ;
}

bool HasUses(StatementList* s, int position, Expression* dest)
{
  for (int i = position ; i < s->get_statement_count() ; ++i)
  {
    Statement* currentStatement = s->get_statement(i) ;
    if (!NoUses(currentStatement, dest))
    {
      return true ;
    }
  }
  return false ;
}

bool HasUses(Statement* s, VariableSymbol* v)
{
  return !NoUses(s, v) ;
}

bool HasUses(Statement* s, Expression* dest)
{
  return !NoUses(s, dest) ;
}

bool NoUses(Statement* s, VariableSymbol* v)
{
  assert(s != NULL) ;
  assert(v != NULL) ;
  // Search for uses
  list<LoadVariableExpression*>* allLoads = 
    collect_objects<LoadVariableExpression>(s) ;
  list<LoadVariableExpression*>::iterator loadIter = allLoads->begin() ;
  while (loadIter != allLoads->end())
  {
    if ((*loadIter)->get_source()->get_name() == v->get_name())
    {
      delete allLoads ;
      return false ;
    }
    ++loadIter ;
  }
  delete allLoads ;

  // Look for arrays as well
  list<SymbolAddressExpression*>* allSymAddr = 
    collect_objects<SymbolAddressExpression>(s) ;
  list<SymbolAddressExpression*>::iterator symIter = allSymAddr->begin() ;
  while (symIter != allSymAddr->end())
  {
    VariableSymbol* toCheck = 
      dynamic_cast<VariableSymbol*>((*symIter)->get_addressed_symbol()) ;
    if (toCheck != NULL)
    {
      if (toCheck->get_name() == v->get_name())
      {
	delete allSymAddr ;
	return false ;
      }
    }
    ++symIter ;
  }
  delete allSymAddr ;

  return true ;
}

bool NoUses(Statement* s, Expression* dest)
{
  bool foundUse = false ;
  list<LoadExpression*>* allLoads = collect_objects<LoadExpression>(s) ;
  list<LoadExpression*>::iterator loadIter = allLoads->begin() ;
  while (loadIter != allLoads->end())
  {
    foundUse |= EquivalentExpressions(dest, (*loadIter)->get_source_address());
    ++loadIter ;
  }
  delete allLoads ;
  return !foundUse ;
}

bool HasPreviousUses(StatementList* s, int position, VariableSymbol* v)
{
  for (int i = position ; i >= 0 ; --i)
  {
    Statement* currentStatement = s->get_statement(i) ;
    if (!NoUses(currentStatement, v))
    {
      return true ;
    }
  }
  return false ;
}

bool HasPreviousUses(StatementList* s, int position, Expression* dest)
{
  for (int i = position ; i >= 0 ; --i)
  {
    Statement* currentStatement = s->get_statement(i) ;
    if (!NoUses(currentStatement, dest))
    {
      return true ;
    }
  }
  return false ;
}

bool IsDefinition(Statement* possibleDef, VariableSymbol* var)
{
  StatementList* defList = 
    dynamic_cast<StatementList*>(possibleDef)  ;
  StoreVariableStatement* storeDef = 
    dynamic_cast<StoreVariableStatement*>(possibleDef) ;
  CallStatement* callDef = 
    dynamic_cast<CallStatement*>(possibleDef) ;
  IfStatement* ifDef = 
    dynamic_cast<IfStatement*>(possibleDef) ;
  CForStatement* forDef = 
    dynamic_cast<CForStatement*>(possibleDef) ;

  if (defList != NULL)
  {
    for (int i = 0 ; i < defList->get_statement_count() ; ++i)
    {
      if (IsDefinition(defList->get_statement(i), var))
      {
	return true ;
      }
    }
    return false ;
  }

  if (storeDef != NULL)
  {
    return (storeDef->get_destination() == var) ;
  }

  if (callDef != NULL)
  {
    if (callDef->get_destination() == var)
    {
      return true ;
    }

    // I should replace this with a function...

    Expression* nextExp ;
    for (unsigned int i = 0 ; i < callDef->get_argument_count() ; ++i)
    {
      nextExp = callDef->get_argument(i) ;
      if (dynamic_cast<SymbolAddressExpression*>(nextExp) != NULL)
      {
	SymbolAddressExpression* outputExpression = 
	  dynamic_cast<SymbolAddressExpression*>(nextExp) ;
	VariableSymbol* outputVariable =
	  dynamic_cast<VariableSymbol*>(outputExpression->get_addressed_symbol()) ;
	if (outputVariable == var) 
	{
	  return true ;
	}
      }
    }
    return false ;
  }

  if (ifDef != NULL)
  {
    return IsDefinition(ifDef->get_then_part(), var) ||
           IsDefinition(ifDef->get_else_part(), var) ;
  }

  if (forDef != NULL)
  {
    return IsDefinition(forDef->get_before(), var) ||
           IsDefinition(forDef->get_body(), var) ;
  }
  return false ;
}

bool IsPossibleDefinition(Statement* s)
{
  return (dynamic_cast<StoreVariableStatement*>(s) != NULL) ||
         (dynamic_cast<CallStatement*>(s) != NULL) ;
}

bool IsUsedBeforeDefined(StatementList* s, int position, VariableSymbol* v)
{
  for (int i = position ; i < s->get_statement_count() ; ++i)
  {
    Statement* currentStatement = s->get_statement(i) ;
    if (!NoUses(currentStatement, v))
    {
      return true ;
    }
    if (IsDefinition(currentStatement, v))
    {
      return false ;
    }
  }
  // No uses means there is no use before defined
  return false ;
}

bool IsUsedBeforeDefined(StatementList* s, int position, Expression* dest)
{
  for (int i = position ; i < s->get_statement_count() ; ++i)
  {
    Statement* currentStatement = s->get_statement(i) ;
    if (!NoUses(currentStatement, dest))
    {
      return true ;
    }
    StoreStatement* currentStore = 
      dynamic_cast<StoreStatement*>(currentStatement) ;
    if (currentStore != NULL &&
	EquivalentExpressions(currentStore->get_destination_address(), dest))
    {
      return false ;
    }
  }
  return false ;
}

bool IsUsed(Type* t, CProcedureType* p)
{
  DataType* d = dynamic_cast<DataType*>(t) ;
  QualifiedType* q = dynamic_cast<QualifiedType*>(t) ;

  if (d != NULL)
  {
    return (p->get_result_type() == d) ;
  }
  if (q != NULL)
  {
    for (int i = 0 ; i < p->get_argument_count() ; ++i)
    {
      if (q == p->get_argument(i))
      {
	return true ;
      }
    }
  }
  return false ;
}

bool HasDefs(StatementList* s, int position, VariableSymbol* v)
{
  for (int i = position ; i < s->get_statement_count() ; ++i)
  {
    Statement* currentStatement = s->get_statement(i) ;
    if (IsDefinition(currentStatement, v))
    {
      return true ;
    }
  }
  return false ;
}

bool HasDefs(StatementList* s, int position, Expression* dest)
{
  for (int i = position ; i < s->get_statement_count() ; ++i)
  {
    StoreStatement* currentStore =
      dynamic_cast<StoreStatement*>(s->get_statement(i)) ;
    if (currentStore != NULL &&
	EquivalentExpressions(currentStore->get_destination_address(), dest))
    {
      return true ;
    }
  }
  return false ;
}

bool HasPreviousDefs(StatementList* s, int position, VariableSymbol* v)
{
  for (int i = position ; i >= 0 ; --i)
  {
    if (IsDefinition(s->get_statement(i), v))
    {
      return true ;
    }
  }
  return false ;
}

bool HasPreviousDefs(StatementList* s, int position, Expression* dest)
{
  for (int i = position ; i >= 0 ; --i)
  {
    StoreStatement* currentStore = 
      dynamic_cast<StoreStatement*>(s->get_statement(i)) ;
    if (currentStore != NULL &&
	EquivalentExpressions(currentStore->get_destination_address(), dest))
    {
      return true ;
    }
  }
  return false ;
}
