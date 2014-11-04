// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
/*

  This pass is responsible for finding all variables used in the innermost
   for loop of a function and determining which ones have definitions
   but no uses.  It then annotes them with the an annotation called
   "OutputVariable" so they can be printed out in the output pass.

  This pass also determines which variables have uses but no definitions,
   and annotes them with "InputVariable".
 
*/

#include <cassert>
#include <iostream>

#include <suifkernel/utilities.h>
#include <basicnodes/basic_factory.h>

#include "roccc_utils/warning_utils.h"
#include "roccc_utils/roccc2.0_utils.h"
#include "output_identification_pass.h"

bool InList(list<VariableSymbol*>& theList, VariableSymbol* var)
{
  list<VariableSymbol*>::iterator findIter = theList.begin() ;
  while (findIter != theList.end())
  {
    if ((*findIter) == var)
    {
      return true ;
    }
    ++findIter ;
  }
  return false ;
}

void RemoveFromList(list<VariableSymbol*>& theList, VariableSymbol* var)
{
  list<VariableSymbol*>::iterator varIter = theList.begin() ;
  int position = 0 ;
  while (varIter != theList.end())
  {
    if ((*varIter) == var)
    {
      theList.erase(position) ;
    }
    ++position ;
    ++varIter ;
  }
}

OutputIdentificationPass::OutputIdentificationPass(SuifEnv* pEnv) :
  PipelinablePass(pEnv, "OutputIdentificationPass")
{
  theEnv = pEnv ;
  procDef = NULL ;
}

void OutputIdentificationPass::do_procedure_definition(ProcedureDefinition* p)
{
  procDef = p ;

  assert(procDef != NULL) ;

  OutputInformation("Output identification begins") ;

  if (procDef->lookup_annote_by_name("ComposedSystem") != NULL)
  {
    IdentifyComposedSystem() ;
    OutputInformation("Output identification ends") ;
    return ;
  }

  CForStatement* innermostLoop = InnermostLoop(procDef) ;
  
  if (innermostLoop != NULL)
  {
    StatementList* loopBodyList = 
      dynamic_cast<StatementList*>(innermostLoop->get_body()) ;
    assert(loopBodyList != NULL) ;
    ProcessLoop(loopBodyList) ;

    // Special case.  The loop induction variables must not be an input scalar
    //  or output scalar.
    CleanupInductionVariables(innermostLoop) ;
  }
  else
  {
    StatementList* procDefList = 
      dynamic_cast<StatementList*>(procDef->get_body()) ;
    assert(procDefList != NULL) ;
    ProcessLoop(procDefList) ;
  }

  OutputInformation("Output identification ends") ;
}

void OutputIdentificationPass::ProcessLoop(StatementList* innermost)
{
  // Pointers should not be input nor output scalars
  ProcessOutputs(innermost) ;
  ProcessInputs(innermost) ;
}

void OutputIdentificationPass::ProcessOutputs(StatementList* innermost)
{
  assert(theEnv != NULL) ;
  assert(procDef != NULL) ;
  assert(innermost != NULL) ;

  // Output scalars are anything that has a definition with no use
  
  list<VariableSymbol*> currentlyAlive ;

  StatementList* bodyList = innermost ;
  assert(bodyList != NULL) ;

  for (int i = 0 ; i < bodyList->get_statement_count() ; ++i)
  {
    Statement* currentStatement = bodyList->get_statement(i) ;
    
    // Temporal common subexpression elimination might leave NULL statements
    //  hanging around.  If we find them, or a non printable statement,
    //  just skip over them.
    if (currentStatement == NULL ||
	currentStatement->lookup_annote_by_name("NonPrintable") != NULL)
    {
      continue ;
    }

    RemoveUses(currentlyAlive, currentStatement) ;
    AddDefinitions(currentlyAlive, currentStatement) ;

  }

  // Go through all the currently alive variables (feedback variables are o.k.)
  //  and make them output variables.
  list<VariableSymbol*>::iterator aliveIter = currentlyAlive.begin() ;
  while (aliveIter != currentlyAlive.end())
  {
    if ((*aliveIter)->lookup_annote_by_name("LoopIndex") == NULL)
    {
      DataType* aliveType = (*aliveIter)->get_type()->get_base_type() ;
      if (dynamic_cast<PointerType*>(aliveType) != NULL)
      {
	(*aliveIter)->append_annote(create_brick_annote(theEnv,
							"OutputFifo")) ;
      }
      else
      {
	(*aliveIter)->append_annote(create_brick_annote(theEnv,
							"OutputVariable")) ;
      }

    }
    ++aliveIter ;
  }
}

void OutputIdentificationPass::ProcessInputs(StatementList* innermost)
{
  assert(theEnv != NULL) ;
  assert(procDef != NULL) ;
  assert(innermost != NULL) ;

  // What we do here is go through all of the statements in the statement list
  //  inside the for statement and keep track of every variable that has
  //  been defined.
  
  StatementList* bodyList = innermost ;
  assert(bodyList != NULL) ;

  list<VariableSymbol*> definedSymbols ;
  
  for (int i = 0 ; i < bodyList->get_statement_count() ; ++i)
  {
    Statement* currentStatement = bodyList->get_statement(i) ;

    // Find all load variables in the current statement and see
    //  if they have been defined yet.
    list<LoadVariableExpression*>* allLoads = 
      collect_objects<LoadVariableExpression>(currentStatement) ;
    list<LoadVariableExpression*>::iterator loadIter = allLoads->begin() ;
    while (loadIter != allLoads->end())
    {
      VariableSymbol* loadedVar = (*loadIter)->get_source() ;

      if (!InList(definedSymbols, loadedVar))
      {
	// Feedback variables should not be input scalars
	if (loadedVar->lookup_annote_by_name("InputScalar") == NULL && 
	    loadedVar->lookup_annote_by_name("FeedbackVariable") == NULL &&
	    loadedVar->lookup_annote_by_name("TemporalFeedback") == NULL &&
	    loadedVar->lookup_annote_by_name("NonInputScalar") == NULL &&
	    loadedVar->lookup_annote_by_name("NormalFeedback") == NULL)
	{
	  DataType* varType = loadedVar->get_type()->get_base_type() ;
	  if (dynamic_cast<PointerType*>(varType) != NULL)
	  {
	    assert(0) ;
	    //	    loadedVar->append_annote(create_brick_annote(theEnv, "InputFifo"));
	  }
	  else
	  {
	    loadedVar->append_annote(create_brick_annote(theEnv, 
							 "InputScalar"));
	  }

	  // Add needs fake for the case we are compiling a module and we
	  //  find an input scalar that isn't listed in the struct
	  //  declaration.
	  loadedVar->append_annote(create_brick_annote(theEnv, "NeedsFake")) ;
	}
      }
      ++loadIter ;
    }
    delete allLoads ;
    
    // Add all definitions    
    StoreVariableStatement* currentStore = 
      dynamic_cast<StoreVariableStatement*>(currentStatement) ;
    CallStatement* currentCall = 
      dynamic_cast<CallStatement*>(currentStatement) ;
    if (currentStore != NULL)
    {
      definedSymbols.push_back(currentStore->get_destination()) ;
    }
    if (currentCall != NULL)
    {
      // Find all variables that are defined by the module
      if (currentCall->get_destination() != NULL)
      {
	definedSymbols.push_back(currentCall->get_destination()) ;
      }
      for (unsigned int j = 0 ; j < currentCall->get_argument_count() ; ++j)
      {
	Expression* nextArgument = currentCall->get_argument(j) ;
	if (dynamic_cast<SymbolAddressExpression*>(nextArgument) != NULL)
	{
	  SymbolAddressExpression* outputArgument = 
	    dynamic_cast<SymbolAddressExpression*>(nextArgument) ;
	  VariableSymbol* toPushBack = 
	    dynamic_cast<VariableSymbol*>(outputArgument->get_addressed_symbol()) ;
	  assert(toPushBack != NULL) ;
	  definedSymbols.push_back(toPushBack) ;
	}
      }
    }
  }
}

void OutputIdentificationPass::CleanupInductionVariables(CForStatement* innermost)
{
  // Starting from the innermost, remove any input and output scalar 
  //  annotations from any loop induction variables.  I'm locating these
  //  variables based upon the step.

  while (innermost != NULL)
  {
    Statement* step = innermost->get_step() ;
    StoreVariableStatement* stepStore = 
      dynamic_cast<StoreVariableStatement*>(step) ;
    assert(stepStore != NULL) ;
    VariableSymbol* stepVar = stepStore->get_destination() ;
    Annote* inputAnnote = stepVar->remove_annote_by_name("InputScalar") ;
    if (inputAnnote != NULL)
    {
      delete inputAnnote ;
    }
    Annote* outputAnnote = stepVar->remove_annote_by_name("OutputScalar") ;
    if (outputAnnote != NULL)
    {
      delete outputAnnote ;
    }
    
    // Find the encompassing CForStatement
    SuifObject* parent = innermost->get_parent() ;
    while (parent != NULL && dynamic_cast<CForStatement*>(parent) == NULL)
    {
      parent = parent->get_parent() ;
    }
    innermost = dynamic_cast<CForStatement*>(parent) ;
  }
}

void OutputIdentificationPass::RemoveUses(list<VariableSymbol*>& alive,
					  Statement* s)
{ 
  list<LoadVariableExpression*>* allUses = 
    collect_objects<LoadVariableExpression>(s) ;

  list<LoadVariableExpression*>::iterator useIter = allUses->begin() ;
  while (useIter != allUses->end())
  {
    RemoveFromList(alive, (*useIter)->get_source()) ;
    ++useIter ;
  }
  delete allUses ;
}

// Definitions are either in store variable statements or call statements
void OutputIdentificationPass::AddDefinitions(list<VariableSymbol*>& alive,
					      Statement* s)
{
  list<StoreVariableStatement*>* allStores = 
    collect_objects<StoreVariableStatement>(s) ;
  list<StoreVariableStatement*>::iterator storeIter = allStores->begin() ;
  while (storeIter != allStores->end())
  {
    if ((*storeIter)->lookup_annote_by_name("NonPrintable") == NULL)
    {
      alive.push_back((*storeIter)->get_destination()) ;
    }
    ++storeIter ;
  }
  delete allStores ;

  list<CallStatement*>* allCalls = 
    collect_objects<CallStatement>(s) ;
  list<CallStatement*>::iterator callIter = allCalls->begin() ;
  while (callIter != allCalls->end())
  {
    if ((*callIter)->get_destination() != NULL)
    {
      alive.push_back((*callIter)->get_destination()) ;
    }
      
    for(unsigned int j = 0 ; j < (*callIter)->get_argument_count() ; ++j)
    {
      Expression* currentArg = (*callIter)->get_argument(j) ;
      if (dynamic_cast<SymbolAddressExpression*>(currentArg) != NULL)
      {
	SymbolAddressExpression* outputExp =
	  dynamic_cast<SymbolAddressExpression*>(currentArg) ;
	Symbol* outputSym = outputExp->get_addressed_symbol() ;
	VariableSymbol* outputVar = 
	  dynamic_cast<VariableSymbol*>(outputSym) ;
	assert(outputVar != NULL) ;
	alive.push_back(outputVar) ;
      }
    }
    ++callIter ;
  }
  delete allCalls ;      
}

void OutputIdentificationPass::IdentifyComposedSystem()
{
  assert(procDef != NULL) ;

  // All input and output variables are only parameter symbols.
  list<ParameterSymbol*> allParameters ;
  
  SymbolTable* symTab = procDef->get_symbol_table() ;
  for (int i = 0 ; i < symTab->get_symbol_table_object_count() ; ++i)
  {
    SymbolTableObject* currentObject = symTab->get_symbol_table_object(i) ;
    ParameterSymbol* currentParam = 
      dynamic_cast<ParameterSymbol*>(currentObject) ;
    if (currentParam != NULL)
    {
      allParameters.push_back(currentParam) ;
    }
  }

  // I have no idea how to identify which parameters are input or output...
  

}
