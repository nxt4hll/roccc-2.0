
#include <cassert>

#include <cfenodes/cfe.h>
#include <basicnodes/basic_factory.h>

#include "handle_call_statements.h"
#include "roccc_utils/warning_utils.h"
#include "roccc_utils/roccc2.0_utils.h"

HandleCallStatements::HandleCallStatements(SuifEnv* pEnv) :
  PipelinablePass(pEnv, "HandleCallStatements")
{
  theEnv = pEnv ;
  procDef = NULL ;
  numLoopBodies = -1 ;
  originalStatementLength = -1 ;
}

void HandleCallStatements::do_procedure_definition(ProcedureDefinition* proc_def)
{
  procDef = proc_def ;
  assert(procDef != NULL) ;
  OutputInformation("Handle Call Statements Begins") ;

  CForStatement* innermost = InnermostLoop(procDef) ;
  StatementList* bodyList ;
  if (innermost != NULL)
  {
    bodyList = dynamic_cast<StatementList*>(innermost->get_body()) ;
  }
  else
  {
    bodyList = dynamic_cast<StatementList*>(procDef->get_body()) ;
  }
  assert(bodyList != NULL) ;

  Annote* callAnnote = procDef->lookup_annote_by_name("UnrolledWithCalls") ;
  if (callAnnote != NULL)
  {
    BrickAnnote* unrollAnnote = 
      dynamic_cast<BrickAnnote*>(callAnnote) ;
    assert(unrollAnnote != NULL) ;
    numLoopBodies = 
      dynamic_cast<IntegerBrick*>(unrollAnnote->get_brick(0))->get_value().c_int() ;
    originalStatementLength = 
      dynamic_cast<IntegerBrick*>(unrollAnnote->get_brick(1))->get_value().c_int() ;
    // Do I need to delete the integer bricks first?
    delete procDef->remove_annote_by_name("UnrolledWithCalls") ;
    HandleCalls(bodyList) ;
  }
  // Otherwise don't do anything.

  OutputInformation("Handle Call Statements Ends") ;
}

void HandleCallStatements::HandleCalls(StatementList* bodyList)
{
  assert(theEnv != NULL) ;
  assert(procDef != NULL) ;
  assert(bodyList != NULL) ;
  
  int position = 0 ;
  // Go through each statement individually
  for (int i = 0 ; i < bodyList->get_statement_count() ; ++i)
  {
    // For each statement, collect each call.  I do this because of
    //  if statements and just random blocks of statements that the
    //  user could create.

    list<CallStatement*>* allCalls =
      collect_objects<CallStatement>(bodyList->get_statement(i)) ;
    // For each call
    list<CallStatement*>::iterator callIter = allCalls->begin() ;
    while (callIter != allCalls->end())
    {

      // Bool selects have a destination, which is the output variable
      //  while all the arguments are just uses
      if ((*callIter)->get_destination() != NULL)
      {
	VariableSymbol* dest = (*callIter)->get_destination() ;

	VariableSymbol* dupe = CreateDuplicate(dest) ;
	dupe->append_annote(create_brick_annote(theEnv,
						"NeedsFake")) ;
	int replacedUses =
	  ReplaceAllUsesWith(dest, dupe, bodyList, position + 1) ;
	// Also replace the original write, unless we are the last
	//  one, so we can handle feedback correctly in a later stage
	// This should be only if there are no replaced uses and we are
	//  in the last iteration.
	if (replacedUses > 0 || 
	    (i < (bodyList->get_statement_count()-originalStatementLength)))
	{
	  (*callIter)->set_destination(dupe) ;
	  if (dest->lookup_annote_by_name("NeedsFake") == NULL)
	  {
	    dest->append_annote(create_brick_annote(theEnv,
						    "NeedsFake"));
	  }
	}

	++callIter ; 
	continue ;
      }

      // For each argument, check if it is an output
      for (unsigned int j = 0 ; j < (*callIter)->get_argument_count() ; ++j)
      {
	Expression* currentArg = (*callIter)->get_argument(j) ;
        SymbolAddressExpression* outputArg =
          dynamic_cast<SymbolAddressExpression*>(currentArg) ;
        VariableSymbol* outputVar = NULL ;
        if (outputArg != NULL)
	{
	  Symbol* argSym = outputArg->get_addressed_symbol() ;
	  outputVar = dynamic_cast<VariableSymbol*>(argSym) ;
	}
        if (outputVar != NULL)
	{
	  VariableSymbol* dupe = CreateDuplicate(outputVar) ;
	  dupe->append_annote(create_brick_annote(theEnv,
						  "NeedsFake")) ;
          int replacedUses =
            ReplaceAllUsesWith(outputVar, dupe, bodyList, position + 1) ;
          // Also replace the original write, unless we are the last
          //  one, so we can handle feedback correctly in a later stage
	  // This should be only if there are no replaced uses and we are
	  //  in the last iteration.
	  if (replacedUses > 0 || 
	      (i < (bodyList->get_statement_count()-originalStatementLength)))
	  {
	    outputArg->set_addressed_symbol(dupe) ;
	    if (outputVar->lookup_annote_by_name("NeedsFake") == NULL)
	    {
	      outputVar->append_annote(create_brick_annote(theEnv,
							   "NeedsFake"));
	    }
	  }
	}
      }      
      ++callIter ;
    } 
    delete allCalls ;
    ++position ;
  }
}

// This function goes from position until the next definition and replaces
//  all uses up to that point
int HandleCallStatements::ReplaceAllUsesWith(VariableSymbol* original,
					     VariableSymbol* newSym,
					     StatementList* containingList,
					     int position)
{
  int replacedUses = 0 ;
  for (int i = position ; i < containingList->get_statement_count() ; ++i)
  {
    list<LoadVariableExpression*>* allLoads =
      collect_objects<LoadVariableExpression>(containingList->get_statement(i));
    
    list<LoadVariableExpression*>::iterator loadIter = allLoads->begin() ;
    while (loadIter != allLoads->end())
    {
      if ((*loadIter)->get_source() == original)
      {
	(*loadIter)->set_source(newSym) ;
	++replacedUses ;
      }
      ++loadIter ;
    }
    delete allLoads ;
    if (IsDefinition(containingList->get_statement(i), original))
    {
      break ;
    }
  }
  return replacedUses ;
}

// This function creates a duplicate and places it in the proper symbol table
VariableSymbol* HandleCallStatements::CreateDuplicate(VariableSymbol* original)
{
  assert(theEnv != NULL) ;
  assert(procDef != NULL) ;
  VariableSymbol* dupe =
    create_variable_symbol(theEnv,
                           original->get_type(),
                           TempName(original->get_name())) ;
  procDef->get_symbol_table()->append_symbol_table_object(dupe) ;
  return dupe ;
}


