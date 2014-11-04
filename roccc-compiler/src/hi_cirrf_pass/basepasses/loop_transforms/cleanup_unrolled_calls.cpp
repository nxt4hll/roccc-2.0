/*

  This must be called directly after loop unrolling and control/data flow
   analysis.

*/

// Standard C++ headers
#include <cassert>
#include <sstream>
#include <map>
// Suif headers
#include <suifkernel/utilities.h>
#include <basicnodes/basic_factory.h>
// ROCCC headers
#include <roccc_utils/warning_utils.h>
#include <roccc_utils/roccc2.0_utils.h>
// Local headers
#include "cleanup_unrolled_calls.h"

CleanupUnrolledCalls::CleanupUnrolledCalls(SuifEnv* pEnv) :
  PipelinablePass(pEnv, "CleanupUnrolledCalls")
{
  theEnv = pEnv ;
  procDef = NULL ;
  nameCounter = 0 ;
}

void CleanupUnrolledCalls::do_procedure_definition(ProcedureDefinition* proc_def) 
{
  procDef = proc_def ;
  assert(procDef != NULL) ;

  OutputInformation("Cleaning up of unrolled calls begins") ;

  StatementList* innermost = InnermostList(procDef) ;
  assert(innermost != NULL) ;
  for (int i = 0 ; i < innermost->get_statement_count() ; ++i)
  {
    CallStatement* currentCall = 
      dynamic_cast<CallStatement*>(innermost->get_statement(i)) ;
    if (currentCall != NULL && 
	currentCall->lookup_annote_by_name("UnrolledCall") != NULL)
    {
      ProcessCall(currentCall, innermost, i) ;
    }
  }
  OutputInformation("Cleaning up of unrolled calls ends") ;
}

void CleanupUnrolledCalls::ProcessCall(CallStatement* c, 
				       StatementList* parentList,
				       int position)
{
  assert(c != NULL) ;
  assert(procDef != NULL) ;
  list<VariableSymbol*> defined = AllDefinedVariables(c) ;

  // Create replacements for each variable
  list<VariableSymbol*>::iterator defIter = defined.begin() ;
  while (defIter != defined.end())
  {
    VariableSymbol* dupe = 
      create_variable_symbol(theEnv, 
			     (*defIter)->get_type(), 
			     TempName((*defIter)->get_name())) ;
    procDef->get_symbol_table()->append_symbol_table_object(dupe) ;
    dupe->append_annote(create_brick_annote(theEnv, "NeedsFake")) ;
    ReplaceAllUsesWith((*defIter), dupe, parentList, position+1) ;
    ReplaceAllDefsWith((*defIter), dupe, parentList, position) ;
    ++defIter ;
  }
}

void CleanupUnrolledCalls::ProcessStatement(Statement* p)
{
  StatementList* parentList = dynamic_cast<StatementList*>(p) ;
  assert(parentList != NULL) ;

  for (int i = 0 ; i < parentList->get_statement_count() ; ++i)
  {
    
  }


  // Collect all call statements
  list<CallStatement*>* allCalls =
    collect_objects<CallStatement>(p) ;
  assert(allCalls != NULL) ;
  
  list<CallStatement*>::iterator callIter = allCalls->begin() ;
  while (callIter != allCalls->end())
  {
    // Go through all the arguments and create replacement variables
    //  for each output variable that we encounter.
    std::map< VariableSymbol*, VariableSymbol* > originalToReplacement ;
    for (unsigned int i = 0 ; i < (*callIter)->get_argument_count() ; ++i)
    {
      Expression* nextArg = (*callIter)->get_argument(i) ;
      if (dynamic_cast<SymbolAddressExpression*>(nextArg) != NULL)
      {
	SymbolAddressExpression* outputExp = 
	  dynamic_cast<SymbolAddressExpression*>(nextArg) ;
	Symbol* outputSymbol = outputExp->get_addressed_symbol() ;
	VariableSymbol* outputVariable = 
	  dynamic_cast<VariableSymbol*>(outputSymbol) ;
	assert(outputVariable != NULL) ;
	
	// Create a replacement
	LString replacementName = outputVariable->get_name() ;
	replacementName = replacementName + LString("_unrollTmp") ;
	VariableSymbol* replacementVariable = 
	  create_variable_symbol(theEnv,
				 outputVariable->get_type(),
				 TempName(replacementName)) ;
	originalToReplacement[outputVariable] = replacementVariable ;

	// Add the replacement variable to the symbol table
	procDef->get_symbol_table()->append_symbol_table_object(replacementVariable) ;

	outputExp->set_addressed_symbol(replacementVariable) ;
      }
    }

    // For all call statements, go through all of the reached uses.
    //  For each one in this scope (the parent is the same as the statement
    //  passed in to this function), replace with a new variable.
    
    BrickAnnote* reachedUses = 
      to<BrickAnnote>((*callIter)->lookup_annote_by_name("reached_uses"));    

    Iter<SuifBrick*> useIter = reachedUses->get_brick_iterator();
    while(useIter.is_valid())
    {
      SuifBrick* currentBrick = useIter.current() ;
      SuifObjectBrick* sob = dynamic_cast<SuifObjectBrick*>(currentBrick) ;
      assert(sob != NULL) ;
      SuifObject* nextObj = sob->get_object() ;
      assert(nextObj != NULL) ;
      // A use has to be a load variable expression.
      LoadVariableExpression* nextUse = 
	dynamic_cast<LoadVariableExpression*>(nextObj) ;
      assert(nextUse != NULL) ;

      bool inScope = false ;

      SuifObject* parent = nextUse->get_parent() ;
      while (parent != NULL)
      {
	if (parent == p)
	{
	  inScope = true ;
	  break ;
	}
	parent = parent->get_parent() ;
      }

      bool inCallStatement = false ;
      parent = nextUse->get_parent() ;
      inCallStatement = (dynamic_cast<CallStatement*>(parent) != NULL) ;

      if (inScope && inCallStatement)
      {
	// go through all the arguments until we find the one
	CallStatement* useCall = dynamic_cast<CallStatement*>(parent) ;
	for(unsigned int j = 0 ; j < useCall->get_argument_count() ; ++j)
	{
	  Expression* nextUseExp = useCall->get_argument(j) ;
	  LoadVariableExpression* nextUseLoad = 
	    dynamic_cast<LoadVariableExpression*>(nextUseExp) ;
	  if (nextUseLoad != NULL)
	  {
	    if (originalToReplacement[nextUseLoad->get_source()] != NULL)
	    {
	      nextUseLoad->set_source(originalToReplacement[nextUseLoad->get_source()]) ;
	    }
	  }
	}
      }
      if (inScope && !inCallStatement)
      {
	// I need to figure out why this is not working the way I think it
	//  should.  It does work, but why?
	if (originalToReplacement[nextUse->get_source()] == NULL)
	{
	  std::stringstream warningStream ;
	  warningStream << "Cannot find " << nextUse->get_source()->get_name();
	  warningStream << " in the map!" ;
	  OutputWarning(warningStream.str().c_str()) ;
	}
	else
	{
	  assert(originalToReplacement[nextUse->get_source()] != NULL) ;
	  nextUse->set_source(originalToReplacement[nextUse->get_source()]) ;
	}
      }
      
      useIter.next() ;
    }
    
    ++callIter ;
  }

  delete allCalls ;
}

