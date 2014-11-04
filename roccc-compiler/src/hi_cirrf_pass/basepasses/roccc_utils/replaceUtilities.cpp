
#include <suifkernel/utilities.h>

#include "replaceUtilities.h"

int ReplaceAllUsesWith(VariableSymbol* original, 
		       VariableSymbol* newSym,
		       StatementList* containingList,
		       int position)
{
  assert(original != NULL) ;
  assert(newSym != NULL) ;
  assert(containingList != NULL) ;
  
  int replacedUses = 0 ;

  for (int i = position ; i < containingList->get_statement_count() ; ++i)
  {
    list<LoadVariableExpression*>* allUses =
      collect_objects<LoadVariableExpression>(containingList->get_statement(i));
    list<LoadVariableExpression*>::iterator useIter = allUses->begin() ;
    while (useIter != allUses->end())
    {
      if ((*useIter)->get_source() == original)
      {
	(*useIter)->set_source(newSym) ;
	++replacedUses ;
      }
      ++useIter ;
    }
    delete allUses ;
  }
  return replacedUses ;
}

int ReplaceAllDefsWith(VariableSymbol* original,
		       VariableSymbol* newSym,
		       StatementList* containingList,
		       int position)
{
  assert(original != NULL) ;
  assert(newSym != NULL) ;
  assert(containingList != NULL) ;
  
  int replacedDefs = 0 ;
  
  // Should I do the check for field symbol here?

  for (int i = position ; i < containingList->get_statement_count() ; ++i)
  {
    StoreVariableStatement* currentStore = 
      dynamic_cast<StoreVariableStatement*>(containingList->get_statement(i)) ;
    CallStatement* currentCall =
      dynamic_cast<CallStatement*>(containingList->get_statement(i)) ;
    if (currentStore != NULL && currentStore->get_destination() == original)
    {
      currentStore->set_destination(newSym) ;
      ++replacedDefs ;
    }
    if (currentCall != NULL)
    {
      ReplaceOutputVariable(currentCall, original, newSym) ;
    }
  }
  return replacedDefs ;
}

void ReplaceOutputVariable(Expression* argument, VariableSymbol* v)
{
  SymbolAddressExpression* outputSym = 
    dynamic_cast<SymbolAddressExpression*>(argument) ;
  if (outputSym != NULL)
  {
    outputSym->set_addressed_symbol(v) ;
    return ;
  }
  assert(0 && "Trying to set a non output variable!") ;
}

void ReplaceOutputVariable(CallStatement* call, VariableSymbol* original,
			   VariableSymbol* replacement)
{
  assert(call != NULL) ;
  assert(original != NULL) ;
  assert(replacement != NULL) ;

  if (call->get_destination() == original) 
  {
    call->set_destination(replacement) ;   
  }
  
  for (unsigned int i = 0 ; i < call->get_argument_count() ; ++i)
  {
    Expression* currentArg = call->get_argument(i) ;
    SymbolAddressExpression* nextOutput = 
      dynamic_cast<SymbolAddressExpression*>(currentArg) ;
    if (nextOutput != NULL)
    {
      VariableSymbol* outputVar = 
	dynamic_cast<VariableSymbol*>(nextOutput->get_addressed_symbol()) ;
      if (outputVar == original)
      {
	nextOutput->set_addressed_symbol(replacement) ;
      }
    }
  }
}
