// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

#include "solve_feedback_calls.h"
#include "roccc_utils/warning_utils.h"
#include "roccc_utils/roccc2.0_utils.h"

#include <cfenodes/cfe.h>
#include <suifkernel/utilities.h>
#include <basicnodes/basic.h>
#include <suifnodes/suif.h>
#include <basicnodes/basic_factory.h>
#include <suifnodes/suif_factory.h>

#include <cassert>
#include <cstring>
#include <iostream>

SolveFeedbackCalls::SolveFeedbackCalls(SuifEnv* pEnv) :
  PipelinablePass(pEnv, "SolveFeedbackCalls")
{
  theEnv = pEnv ;
  procDef = NULL ;
}

void SolveFeedbackCalls::do_procedure_definition(ProcedureDefinition* proc_def)
{
  procDef = proc_def ;
  assert(procDef != NULL) ;
  
  OutputInformation("Solve Feedback Calls Pass Begins") ;

  CForStatement* innermostLoop = InnermostLoop(procDef) ;
  if (innermostLoop == NULL)
  {
    OutputInformation("Solve Feedback Calls Pass Ends") ;
    return ;
  }

  Statement* body = innermostLoop->get_body() ;
  StatementList* bodyList = dynamic_cast<StatementList*>(body) ;
  assert(bodyList != NULL) ;

  // Handle intra-module feedback
  list<CallStatement*>* allCalls = 
    collect_objects<CallStatement>(innermostLoop->get_body()) ;

  list<CallStatement*>::iterator callIter = allCalls->begin() ;
  while (callIter != allCalls->end())
  {
    if (!IsABoolSelect(*callIter))
      HandleSingleCall(*callIter) ;
    ++callIter ;
  }
  
  delete allCalls ;

  // Handle cross-module feedback
  HandleCalls(bodyList) ;

  // TODO: Handle module/normal statement feedback
  
  OutputInformation("Solve Feedback Calls Pass Ends") ;
}

void SolveFeedbackCalls::HandleCalls(StatementList* bodyList)
{
  // Go through each call statement in order and determine if any of the
  //  input variables we read are written to in a later call.
  for (int i = 0 ; i < bodyList->get_statement_count() ; ++i)
  {
    Statement* currentStatement = bodyList->get_statement(i) ;
    
    list<CallStatement*>* allCalls = 
      collect_objects<CallStatement>(currentStatement) ;
    list<CallStatement*>::iterator callIter = allCalls->begin() ;
    int counter = 0 ;
    
    while (callIter != allCalls->end())
    {      
      CallStatement* currentCall = (*callIter) ;
      // For all input arguments
      
      for (unsigned int j = 0 ; j < currentCall->get_argument_count() ; ++j)
      {
	Expression* currentArg = currentCall->get_argument(j) ;
	LoadVariableExpression* inputArg = 
	  dynamic_cast<LoadVariableExpression*>(currentArg) ;
	
	if (inputArg != NULL)
	{	  
	  CallStatement* definingStatement = 
	    DefinedByACall(inputArg->get_source(),
			   bodyList,
			   i+1) ;	  
	  if (definingStatement != NULL)
	  {
	    // Create a new variable and two new statements
	    VariableSymbol* replacement = 
	      create_variable_symbol(theEnv,
				     inputArg->get_source()->get_type(),
				     TempName(inputArg->get_source()->get_name())) ;
	    LoadVariableExpression* beforeLoad = 
	      create_load_variable_expression(theEnv, 
					      replacement->get_type()->get_base_type(),
					      replacement) ;
	    LoadVariableExpression* afterLoad = 
	      create_load_variable_expression(theEnv,
					      inputArg->get_source()->get_type()->get_base_type(),
					      inputArg->get_source()) ;
	    
	    StoreVariableStatement* beforeStatement =
	      create_store_variable_statement(theEnv,
					      inputArg->get_source(),
					      beforeLoad) ;
	    
	    StoreVariableStatement* afterStatement =
	      create_store_variable_statement(theEnv,
					      replacement,
					      afterLoad) ;

	    
	    procDef->get_symbol_table()->append_symbol_table_object(replacement);
	    bodyList->insert_statement(i, beforeStatement) ;
	    bodyList->append_statement(afterStatement) ;
	    ++i ; // Here to move past the statement that we just processed
	  }	
	}      
      }
      ++callIter ;
    }
    delete allCalls ;
  }
}

void SolveFeedbackCalls::HandleSingleCall(CallStatement* c)
{
  // Go through all arguments and determine if any of them are also
  //  definitions in the same call statement.

  for (unsigned int i = 0 ; i < c->get_argument_count() ; ++i)
  {
    Expression* nextArg = c->get_argument(i) ;

    // We are only interested in variables
    LoadVariableExpression* nextLoadVariableExpression = 
      dynamic_cast<LoadVariableExpression*>(nextArg) ;
    if (nextLoadVariableExpression != NULL) 
    {
      VariableSymbol* nextLoadVariable = 
	nextLoadVariableExpression->get_source() ;
      if (IsDefinition(c, nextLoadVariable))
      {
	// We need two replacement variables, one for before, one for
	//  after.
	VariableSymbol* replacementBeforeVariable = 
	  create_variable_symbol(theEnv,
				 nextLoadVariable->get_type(),
				 TempName(nextLoadVariable->get_name())) ;
	
	// Here we have to make the new value.
	VariableSymbol* replacementVariable = 
	  create_variable_symbol(theEnv,
				 nextLoadVariable->get_type(),
				 TempName(nextLoadVariable->get_name())) ;
	nextLoadVariableExpression->set_source(replacementBeforeVariable) ;

	// Add the replacement variables to the correct symbol table
	procDef->get_symbol_table()->append_symbol_table_object(replacementVariable) ;
	procDef->get_symbol_table()->append_symbol_table_object(replacementBeforeVariable) ;

	LoadVariableExpression* newValue = 
	  create_load_variable_expression(theEnv,
					  nextLoadVariable->get_type()->get_base_type(),
					  nextLoadVariable) ;

	LoadVariableExpression* olderValue =
	  create_load_variable_expression(theEnv,
					  replacementVariable->get_type()->get_base_type(),
					  replacementVariable) ;

	// Now add the copies
	StoreVariableStatement* beforeStatement = 
	  create_store_variable_statement(theEnv,
					  replacementBeforeVariable,
					  olderValue) ;


	StoreVariableStatement* followStatement = 
	  create_store_variable_statement(theEnv,
					  replacementVariable,
					  newValue) ;

	// Now put it right before and right after the call statement.  
	//  There has to be a statement list somewhere, so find the
	//  location of this call statement in the statement list.

	SuifObject* parent = c->get_parent() ;
	while (dynamic_cast<StatementList*>(parent) == NULL && parent != NULL)
	{
	  parent = parent->get_parent() ;
	}
	assert(parent != NULL) ;

	StatementList* containingList =
	  dynamic_cast<StatementList*>(parent) ;
	assert(containingList != NULL) ;
	
	int position = 0 ;
	for (position = 0 ; position < containingList->get_statement_count() ;
	     ++position)
	{
	  if (containingList->get_statement(position) == c)
	  {
	    break ;
	  }
	}
	// If this assert goes off, the call statement is somehow not
	//  inside a statement list
	assert(position != containingList->get_statement_count()) ;

	containingList->insert_statement(position + 1, followStatement);
	containingList->insert_statement(position, beforeStatement) ;

      }
    }
  }
}


CallStatement* SolveFeedbackCalls::DefinedByACall(VariableSymbol* var,
						  StatementList* theList,
						  int position)
{
  for (int i = position ; i < theList->get_statement_count() ; ++i)
  {
    Statement* currentStatement = theList->get_statement(i) ;
    list<CallStatement*>* allCalls = 
      collect_objects<CallStatement>(currentStatement) ;

    list<CallStatement*>::iterator callIter = allCalls->begin() ;
    while (callIter != allCalls->end())
    {
      CallStatement* currentCall = (*callIter) ;
      for (unsigned int j = 0 ; j < currentCall->get_argument_count() ; ++j)
      {
	Expression* currentArg = currentCall->get_argument(j) ;
	SymbolAddressExpression* outputArg = 
	  dynamic_cast<SymbolAddressExpression*>(currentArg) ;
	if (outputArg != NULL)
	{
	  if (outputArg->get_addressed_symbol() == var)
	  {
	    delete allCalls ;
	    return currentCall ;
	  }
	}
      }      
      ++callIter ;
    }
    delete allCalls ;
  }
  return NULL ;
}

bool SolveFeedbackCalls::IsABoolSelect(CallStatement* c)
{
  Expression* address = c->get_callee_address() ;
  SymbolAddressExpression* symAddr = 
    dynamic_cast<SymbolAddressExpression*>(address) ;
  assert(symAddr != NULL) ;
  Symbol* procSym = 
    symAddr->get_addressed_symbol() ;
  assert(procSym != NULL) ;
  
  if (strstr(procSym->get_name().c_str(), "boolsel") != 0)
  {
    return true ; 
  }
  else
  {
    return false ;
  }

}
