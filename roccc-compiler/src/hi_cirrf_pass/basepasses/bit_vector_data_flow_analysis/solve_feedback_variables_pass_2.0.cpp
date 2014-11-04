// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*

  Variables are identified as feedback if there is a load before a store.
 
*/

#include <cassert>

#include "solve_feedback_variables_pass_2.0.h"
#include <suifnodes/suif.h>
#include <basicnodes/basic.h>
#include <cfenodes/cfe.h>
#include "suifkernel/utilities.h"
#include <basicnodes/basic_factory.h>
#include "roccc_utils/warning_utils.h"

SolveFeedbackVariablesPass2::SolveFeedbackVariablesPass2(SuifEnv *pEnv) : 
  PipelinablePass(pEnv, "SolveFeedbackVariablesPass2") 
{
  procDef = NULL ;
  theEnv = pEnv ;
}

void SolveFeedbackVariablesPass2::do_procedure_definition(ProcedureDefinition* proc_def)
{
  procDef = proc_def ;

  assert(procDef != NULL) ;

  OutputInformation("Solve feedback variables 2.0 pass begins") ;

  // For each Variable Symbol, see if there is both a store and 
  //  a load for each one.  If there is, then annotate it.  So first, 
  //  collect all stores and loads
  list<StoreVariableStatement*>* allStores = 
    collect_objects<StoreVariableStatement>(procDef->get_body()) ;
  list<LoadVariableExpression*>* allLoads = 
    collect_objects<LoadVariableExpression>(procDef->get_body()) ;

  assert(allStores != NULL && allLoads != NULL) ;
  
  SymbolTable* symTab = procDef->get_symbol_table() ;
  for (int i = 0 ; i < symTab->get_symbol_table_object_count() ; ++i)
  {
    SymbolTableObject* nextObj = symTab->get_symbol_table_object(i) ;
    VariableSymbol* currentVar = dynamic_cast<VariableSymbol*>(nextObj) ;
    if (currentVar != NULL)
    {
      // See if there is a store statement and a load expression for
      //  this variable.  If so, then annotate

      VariableSymbol* sourceVar = NULL ;
      list<StoreVariableStatement*>::iterator storeIter = allStores->begin() ;
      while(storeIter != allStores->end())
      {
	if ((*storeIter)->get_destination() == currentVar)
	{	
	  // Find the source and setup the sourceVar
	  Expression* sourceExp = (*storeIter)->get_value() ;
	  if (dynamic_cast<LoadVariableExpression*>(sourceExp) != NULL)
	  {
	    sourceVar = 
	      dynamic_cast<LoadVariableExpression*>(sourceExp)->get_source() ;
	    // Also, mark the store variable statement as not printable
	    (*storeIter)->append_annote(create_brick_annote(theEnv, 
	    						    "NonPrintable")) ;
	  }	  
	}
	++storeIter ;
      }

      VariableSymbol* destVar = NULL ;
      list<LoadVariableExpression*>::iterator loadIter = allLoads->begin() ;
      while (loadIter != allLoads->end())
      {
	if ((*loadIter)->get_source() == currentVar)
	{
	  // Find the parent and see if the destination
	  Statement* destStmt = 
	    dynamic_cast<Statement*>((*loadIter)->get_parent()) ;
	  if (destStmt != NULL)
	  {
	    if (dynamic_cast<StoreVariableStatement*>(destStmt) != NULL)
	    {
	      destVar = dynamic_cast<StoreVariableStatement*>(destStmt)->
		get_destination() ;
	      // Also mark the statement as non printable
	      destStmt->append_annote(create_brick_annote(theEnv,
							  "NonPrintable")) ;
	    }
	  }
	}
	++loadIter ;
      }

      if (sourceVar != NULL && destVar != NULL)
      {
	SetupAnnotations(currentVar, sourceVar, destVar) ;
	// Also, add the source to the list of sources so we can deal
	//  with store statements later.
	storeVars.push_back(sourceVar) ;
      }
    }
  }

  ProcessStores() ;

  delete allStores ;
  delete allLoads ;

  OutputInformation("Solve feedback variables 2.0 pass ends") ;
}

void SolveFeedbackVariablesPass2::SetupAnnotations(VariableSymbol* toAnnote,
						   VariableSymbol* source,
						   VariableSymbol* destination)
{

  // Clean up any annotation that previously existed
  if (toAnnote->lookup_annote_by_name("FeedbackVariable") != NULL)
  {
    // Do I need to remove the bricks associated with this as well, or
    //  does the destructor handle that?
    delete toAnnote->remove_annote_by_name("FeedbackVariable") ;
  }

  // Now create the annotation we will use
  BrickAnnote* feedbackAnnote = 
    to<BrickAnnote>(create_brick_annote(theEnv, "FeedbackVariable")) ;

  // Create the bricks and add them to the annotation
  feedbackAnnote->append_brick(create_suif_object_brick(theEnv, destination)) ;
  feedbackAnnote->append_brick(create_suif_object_brick(theEnv, source)) ;

  // And finally attach the annotation to the variable
  toAnnote->append_annote(feedbackAnnote) ;  
}

void SolveFeedbackVariablesPass2::ProcessStores()
{
  assert(procDef != NULL) ;
  list<StoreStatement*>* allStores = 
    collect_objects<StoreStatement>(procDef->get_body()) ;
  assert(allStores != NULL) ;

  list<StoreStatement*>::iterator storeIter = allStores->begin() ;
  while(storeIter != allStores->end())
  {
    Expression* rightHandSide = (*storeIter)->get_value() ;
    if (dynamic_cast<LoadVariableExpression*>(rightHandSide) != NULL)
    {
      VariableSymbol* rhVar = 
	dynamic_cast<LoadVariableExpression*>(rightHandSide)->get_source() ;
      // The right hand variable wouldn't be a feedback variable itself, 
      //  but it would be the source of another feedback variable.
      if (isStoreVar(rhVar))
      {
	(*storeIter)->append_annote(create_brick_annote(theEnv,
							"NonPrintable")) ;
      }

    }
    ++storeIter ;
  }

  delete allStores ;
}

bool SolveFeedbackVariablesPass2::isStoreVar(VariableSymbol* v)
{
  list<VariableSymbol*>::iterator varIter = storeVars.begin() ;
  while (varIter != storeVars.end())
  {
    if ((*varIter) == v)
    {
      return true ;
    }
    ++varIter ;
  }
  return false ;
}
