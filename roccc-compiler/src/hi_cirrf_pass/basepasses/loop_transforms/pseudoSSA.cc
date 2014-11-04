// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

#include <cassert>
#include <string>

#include <suifnodes/suif.h>
#include <suifnodes/suif_factory.h>
#include <basicnodes/basic_factory.h>
#include <suifkernel/utilities.h>

#include <roccc_utils/warning_utils.h>
#include <roccc_utils/roccc2.0_utils.h>

#include "pseudoSSA.h"

PseudoSSA::PseudoSSA(SuifEnv* pEnv) : PipelinablePass(pEnv, "PseudoSSA") 
{
  theEnv = pEnv ;
  procDef = NULL ;
  numPhis = 0 ;
}

void PseudoSSA::do_procedure_definition(ProcedureDefinition* proc_def)
{
  procDef = proc_def ;
  assert(procDef != NULL) ;

  OutputInformation("Pseudo SSA phase begins") ;

  if (procDef->lookup_annote_by_name("ComposedSystem") != NULL)
  {
    OutputInformation("Pseudo SSA not done on compsed systems") ;
    return ;
  }

  StatementList* bodyList = InnermostList(procDef) ;
  assert(bodyList != NULL) ;
  
  // First, collect all of the variables with uses before definitions.
  //  Only do feedbacks for code that has a loop...
  if (InnermostLoop(procDef) != NULL)
  {
    CollectPotentialFeedbackVariables(bodyList) ;
  }
  
  // Add a mark statement for every feedback variable and put a phi 
  //  placeholder for it at the beginning of the loop.
  CreatePhis(bodyList) ;

  // Perform the replacements and update each phi placeholder
  //  These function calls also perform the SSA in the normal case.
  HandleBuiltInStatements(bodyList) ;
  HandleCopyStatements(bodyList) ;
  HandleCallStatements(bodyList) ;

  CreateFeedbackVariables(bodyList) ;
  /*
  SymbolTable* symTab = procDef->get_symbol_table() ;
  assert(symTab != NULL) ;
  for (unsigned int i = 0 ; i < symTab->get_symbol_table_object_count() ; ++i)
  {
    if (dynamic_cast<VariableSymbol*>(symTab->get_symbol_table_object(i)) != NULL)
    {
      std::cout << (symTab->get_symbol_table_object(i))->get_name() 
		<< std::endl ;
    }
  }
  */
  OutputInformation("Pseudo SSA phase ends") ;
}

void PseudoSSA::CollectPotentialFeedbackVariables(StatementList* bodyList)
{
  assert(bodyList != NULL) ;
  
  list<VariableSymbol*> allDefined ;
  
  // Go through the statements one by one and add any variable that is used
  //  before it is defined to the list of potential feedback variables
  for (int i = 0 ; i < bodyList->get_statement_count() ; ++i)
  {
    // All uses should be load variable expressions
    list<LoadVariableExpression*>* allLoads = 
      collect_objects<LoadVariableExpression>(bodyList->get_statement(i)) ;
    list<LoadVariableExpression*>::iterator loadIter = allLoads->begin() ;
    while (loadIter != allLoads->end())
    {
      VariableSymbol* currentVariable = (*loadIter)->get_source() ;
      if (!InList(allDefined, currentVariable) && 
	  !InList(potentialFeedbackVariables, currentVariable))
      {
	potentialFeedbackVariables.push_back(currentVariable) ;
      }
      ++loadIter ;
    }
    delete allLoads ;

    list<VariableSymbol*> definedInStatement = 
      AllDefinedVariables(bodyList->get_statement(i)) ;
    list<VariableSymbol*>::iterator definedIter = definedInStatement.begin() ;
    while (definedIter != definedInStatement.end())
    {
      allDefined.push_back(*definedIter) ;
      ++definedIter ;
    }
  }

  // Go through the entire list of potential feedback variables.  If 
  //  any of them are not defined in the loop they are straight input
  //  scalars and should be removed.

  list<VariableSymbol*>::iterator potentialIter =
    potentialFeedbackVariables.begin() ;
  while (potentialIter != potentialFeedbackVariables.end())
  {
    if (InList(allDefined, (*potentialIter)))
    {
      ++potentialIter ;
    }
    else
    {
      potentialIter = potentialFeedbackVariables.erase(potentialIter) ;
    }
  }
}

bool PseudoSSA::InList(list<VariableSymbol*> l, VariableSymbol* v)
{
  list<VariableSymbol*>::iterator varIter = l.begin() ;
  while(varIter != l.end())
  {
    if ((*varIter) == v)
    {
      return true ;
    }
    ++varIter ;
  }
  return false ;
}

void PseudoSSA::CreatePhis(StatementList* bodyList)
{
  numPhis = 0 ;
  list<VariableSymbol*>::iterator potentialIter = 
    potentialFeedbackVariables.begin() ;
  while (potentialIter != potentialFeedbackVariables.end())
  {
    MarkStatement* nextPhi =
      create_mark_statement(theEnv) ;
    // Mark statements are used as phi nodes thanks to the annotations
    BrickAnnote* phiAnnote = 
      create_brick_annote(theEnv, "PHI") ;

    SuifObjectBrick* fromAbove = 
      create_suif_object_brick(theEnv, (*potentialIter)) ;
    SuifObjectBrick* fromBelow =
      create_suif_object_brick(theEnv, (*potentialIter)) ;

    (*potentialIter)->append_annote(create_brick_annote(theEnv, "NormalFeedback")) ;

    phiAnnote->append_brick(fromAbove) ;
    phiAnnote->append_brick(fromBelow) ;

    nextPhi->append_annote(phiAnnote) ;
    bodyList->insert_statement(0, nextPhi) ;
    ++numPhis ;

    // Associate the variable with the phi node
    correspondingPhis[(*potentialIter)] = nextPhi ;

    ++potentialIter ;
  }
}

void PseudoSSA::HandleBuiltInStatements(StatementList* bodyList)
{
  assert(bodyList != NULL) ;
  assert(procDef != NULL) ;

  for (int i = 0 ; i < bodyList->get_statement_count() ; ++i)
  {
    Statement* currentStatement = bodyList->get_statement(i) ;
    CallStatement* currentCall = 
      dynamic_cast<CallStatement*>(currentStatement) ;
    if (currentCall != NULL && IsBuiltIn(currentCall))
    {
      VariableSymbol* outputVar = currentCall->get_destination() ;
      if (outputVar != NULL &&
	  outputVar->lookup_annote_by_name("TemporalFeedback") == NULL)
      {
	VariableSymbol* dupe = CreateDuplicate(outputVar) ;
	dupe->append_annote(create_brick_annote(theEnv, "NeedsFake")) ;
	// Regardless?
	if (dynamic_cast<FieldSymbol*>(outputVar) == NULL)
	{
	  (currentCall)->set_destination(dupe) ;
	  // Update Phi
	  MarkStatement* toChange = correspondingPhis[outputVar] ;
	  if (toChange != NULL)
	  {
	    correspondingPhis[dupe] = toChange ;
	    
	    BrickAnnote* phiAnnote = 
	      dynamic_cast<BrickAnnote*>(toChange->lookup_annote_by_name("PHI"));
	    assert(phiAnnote != NULL) ;
	    SuifObjectBrick* secondBrick = 
	      dynamic_cast<SuifObjectBrick*>(phiAnnote->get_brick(1)) ;
	    assert(secondBrick != NULL) ;
	    secondBrick->set_object(dupe) ;
	  }
	}

	// Replace all uses
	int replaced = ReplaceAllUsesWith(outputVar, dupe, bodyList, i + 1) ;
	//if (replaced > 0)
	{
	  ReplaceAllStoresWith(outputVar, dupe, bodyList, i + 1) ;
	  
	  // Propagate output scalars if necessary...
	  Annote* outputAnnote = 
	    outputVar->lookup_annote_by_name("OutputScalar") ;
	  if (outputAnnote != NULL)
	  {
	    delete outputAnnote->remove_annote_by_name("OutputScalar") ;
	    dupe->append_annote(create_brick_annote(theEnv, "OutputScalar")) ;
	  }
	}
      }
    }
  }
}

void PseudoSSA::HandleCopyStatements(StatementList* bodyList) 
{
  assert(bodyList != NULL) ;
  for (int i = 0 ; i < bodyList->get_statement_count() ; ++i)
  {
    // For each statement, collect each store variable statement.
    //  In most cases there will be one or zero.
    list<StoreVariableStatement*>* allStores =
      collect_objects<StoreVariableStatement>(bodyList->get_statement(i)) ;
    list<StoreVariableStatement*>::iterator storeIter = allStores->begin() ;
    while (storeIter != allStores->end())
    {
      VariableSymbol* outputVar = (*storeIter)->get_destination() ;
      if (outputVar != NULL && 
	  outputVar->lookup_annote_by_name("TemporalFeedback") == NULL) 
      {
	// Create a duplicate
	VariableSymbol* dupe = CreateDuplicate(outputVar) ;
	dupe->append_annote(create_brick_annote(theEnv, "NeedsFake")) ;
	// Regardless?
	if (dynamic_cast<FieldSymbol*>(outputVar) == NULL)
	{
	  (*storeIter)->set_destination(dupe) ;
	  // Update Phi
	  MarkStatement* toChange = correspondingPhis[outputVar] ;
	  if (toChange != NULL)
	  {
	    correspondingPhis[dupe] = toChange ;
	    
	    BrickAnnote* phiAnnote = 
	      dynamic_cast<BrickAnnote*>(toChange->lookup_annote_by_name("PHI"));
	    assert(phiAnnote != NULL) ;
	    SuifObjectBrick* secondBrick = 
	      dynamic_cast<SuifObjectBrick*>(phiAnnote->get_brick(1)) ;
	    assert(secondBrick != NULL) ;
	    secondBrick->set_object(dupe) ;
	  }
	}

	// Replace all uses
	int replaced = ReplaceAllUsesWith(outputVar, dupe, bodyList, i + 1) ;
	if (replaced > 0)
	{
	  ReplaceAllStoresWith(outputVar, dupe, bodyList, i + 1) ;
	  
	  // Propagate output scalars if necessary...
	  Annote* outputAnnote = 
	    outputVar->lookup_annote_by_name("OutputScalar") ;
	  if (outputAnnote != NULL)
	  {
	    delete outputAnnote->remove_annote_by_name("OutputScalar") ;
	    dupe->append_annote(create_brick_annote(theEnv, "OutputScalar")) ;
	  }

	  // Should I do this regardless?
	  /*
	  if (dynamic_cast<FieldSymbol*>(outputVar) == NULL)
	  {
	    (*storeIter)->set_destination(dupe) ;
	    // Update Phi
	    MarkStatement* toChange = correspondingPhis[outputVar] ;
	    if (toChange != NULL)
	    {
	      correspondingPhis[dupe] = toChange ;
	      
	      BrickAnnote* phiAnnote = 
		dynamic_cast<BrickAnnote*>(toChange->lookup_annote_by_name("PHI"));
	      assert(phiAnnote != NULL) ;
	      SuifObjectBrick* secondBrick = 
		dynamic_cast<SuifObjectBrick*>(phiAnnote->get_brick(1)) ;
	      assert(secondBrick != NULL) ;
	      secondBrick->set_object(dupe) ;
	    }
	  }
	  */
	}
      }
      ++storeIter ;
    }
    delete allStores ;
  }
}

void PseudoSSA::HandleCallStatements(StatementList* bodyList) 
{
  assert(bodyList != NULL) ;
  assert(theEnv != NULL) ;
  assert(procDef != NULL) ;

  for (int i = 0 ; i < bodyList->get_statement_count() ; ++i)
  {
    list<CallStatement*>* allCalls = 
      collect_objects<CallStatement>(bodyList->get_statement(i)) ;
    list<CallStatement*>::iterator callIter = allCalls->begin() ;
    while (callIter != allCalls->end())
    {
      //if ((*callIter)->get_destination() != NULL)
      if (!IsBuiltIn(*callIter))
      {
	// Find all of the output arguments
	for (unsigned int j = 0 ; j < (*callIter)->get_argument_count() ; ++j)
	{
	  Expression* currentArg = (*callIter)->get_argument(j) ;
	  VariableSymbol* outputVar = GetOutputVariable(currentArg) ;
	  if (outputVar != NULL &&
	      outputVar->lookup_annote_by_name("TemporalFeedback") == NULL) 
	  {
	    VariableSymbol* dupe = CreateDuplicate(outputVar, (*callIter)) ;
	    dupe->append_annote(create_brick_annote(theEnv, "NeedsFake")) ;
	    ReplaceAllUsesWith(outputVar, dupe, bodyList, i+1) ;
	    ReplaceAllStoresWith(outputVar, dupe, bodyList, i+1) ;
	    
	    Annote* outputAnnote = 
	      outputVar->lookup_annote_by_name("OutputScalar") ;
	    if (outputAnnote != NULL)
	    {
	      delete outputVar->remove_annote_by_name("OutputScalar") ;
	      dupe->append_annote(create_brick_annote(theEnv, "OutputScalar"));
	    }

	    // If we are not looking at a field symbol, we need to 
	    //  update the phis
	    if (dynamic_cast<FieldSymbol*>(outputVar) == NULL)
	    {
	      ReplaceOutputVariable(currentArg, dupe) ;
	      MarkStatement* toChange = correspondingPhis[outputVar] ;
	      if (toChange != NULL)
	      {
		correspondingPhis[dupe] = toChange ;
		
		BrickAnnote* phiAnnote = 
	    dynamic_cast<BrickAnnote*>(toChange->lookup_annote_by_name("PHI"));
		assert(phiAnnote != NULL) ;
		SuifObjectBrick* secondBrick = 
		  dynamic_cast<SuifObjectBrick*>(phiAnnote->get_brick(1)) ;
		assert(secondBrick != NULL) ;
		secondBrick->set_object(dupe) ;
	      }
	    }
	  }
	}
      }
      ++callIter ;
    }
    delete allCalls ;
  }
}

VariableSymbol* PseudoSSA::CreateDuplicate(VariableSymbol* original, 
					   CallStatement* c)
{
  assert(theEnv != NULL) ;
  assert(procDef != NULL) ;

  LString dupeTmpName = BaseName(original->get_name()) ;
  if (c != NULL && !IsBuiltIn(c))
  {
    dupeTmpName = dupeTmpName + "_moduleOut" ;
  }
  else
  {
    dupeTmpName = dupeTmpName + "_ssaTmp" ;
  }
  
  VariableSymbol* dupe = 
    create_variable_symbol(theEnv,
			   original->get_type(),
			   TempName(dupeTmpName)) ;
  procDef->get_symbol_table()->append_symbol_table_object(dupe) ;

  if (original->lookup_annote_by_name("Dummy") != NULL)
  {
    dupe->append_annote(create_brick_annote(theEnv, "Dummy")) ;
  }

  if (original->lookup_annote_by_name("ParameterOrder") != NULL)
  {
    BrickAnnote* originalOrder = 
      dynamic_cast<BrickAnnote*>(original->lookup_annote_by_name("ParameterOrder")) ;
    assert(originalOrder != NULL) ;
    IntegerBrick* originalIntBrick = 
      dynamic_cast<IntegerBrick*>(originalOrder->get_brick(0)) ;
    assert(originalIntBrick != NULL) ;

    BrickAnnote* copyOrder = 
      create_brick_annote(theEnv, "ParameterOrder") ;
    IntegerBrick* copyBrick = 
      create_integer_brick(theEnv, originalIntBrick->get_value()) ;
    copyOrder->append_brick(copyBrick) ;
    dupe->append_annote(copyOrder) ;
  }

  return dupe ;
}

int PseudoSSA::ReplaceAllUsesWith(VariableSymbol* original, 
				  VariableSymbol* newSym,
				  StatementList* containingList,
				  int position)
{
  assert(containingList != NULL) ;
  assert(original != NULL) ;
  assert(newSym != NULL) ;
  int replacedUses = 0 ;

  for (int i = position ; i < containingList->get_statement_count()  ; ++i)
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

void PseudoSSA::ReplaceAllStoresWith(VariableSymbol* original,
				     VariableSymbol* newSym,
				     StatementList* containingList,
				     int position)
{
  assert(containingList != NULL) ;
  assert(original != NULL) ;
  assert(newSym != NULL) ;
  
  // When dealing with modules, we don't want to mess with any of the 
  //  field symbols...
  if (dynamic_cast<FieldSymbol*>(original) != NULL)
  {
    return ;
  }

  for (int i = position ; i < containingList->get_statement_count() ; ++i)
  {
    StoreVariableStatement* currentStore = 
      dynamic_cast<StoreVariableStatement*>(containingList->get_statement(i)) ;
    CallStatement* currentCall = 
      dynamic_cast<CallStatement*>(containingList->get_statement(i)) ;
    if (currentStore != NULL)
    {
      if (currentStore->get_destination() == original)
      {
	currentStore->set_destination(newSym) ;
      }
    }
    if (currentCall != NULL)
    {
      if (currentCall->get_destination() == original)
      {
	currentCall->set_destination(newSym) ;
      }
      // Go through all of the arguments and change any of the output variables
      //  if they happen to be writing to them.
      for (int j = 0 ; j < currentCall->get_argument_count() ; ++j)
      {
	SymbolAddressExpression* outputArg =
	  dynamic_cast<SymbolAddressExpression*>(currentCall->get_argument(j));
	if (outputArg != NULL)
	{
	  if (outputArg->get_addressed_symbol() == original)
	  {
	    outputArg->set_addressed_symbol(newSym) ;
	  }
	}
      }
    }
  }

}

void PseudoSSA::CreateFeedbackVariables(StatementList* bodyList)
{
  // For every phi statement, create a feedback variable and add it to 
  //  the symbol table with the appropriate annotations
  for (int i = 0 ; i < bodyList->get_statement_count() ; ++i)
  {
    MarkStatement* currentPhi = 
      dynamic_cast<MarkStatement*>(bodyList->get_statement(i)) ;
    if (currentPhi == NULL)
    {
      continue ;
    }
    Annote* phiAnnote = currentPhi->lookup_annote_by_name("PHI") ;
    if (phiAnnote == NULL)
    {
      continue ;
    }
    BrickAnnote* phiBrick = dynamic_cast<BrickAnnote*>(phiAnnote) ;
    assert(phiBrick != NULL) ;
    SuifObjectBrick* destBrick = 
      dynamic_cast<SuifObjectBrick*>(phiBrick->get_brick(0)) ;
    SuifObjectBrick* sourceBrick = 
      dynamic_cast<SuifObjectBrick*>(phiBrick->get_brick(1)) ;
   
    VariableSymbol* destination =
      dynamic_cast<VariableSymbol*>(destBrick->get_object()) ;
    VariableSymbol* source =
      dynamic_cast<VariableSymbol*>(sourceBrick->get_object()) ;

    source->append_annote(create_brick_annote(theEnv, "FeedbackSource")) ;

    VariableSymbol* feedbackVariable = 
      create_variable_symbol(theEnv,
			     destination->get_type(),
			     TempName(LString("feedbackTmp"))) ;
    procDef->get_symbol_table()->append_symbol_table_object(feedbackVariable) ;

    feedbackVariable->append_annote(create_brick_annote(theEnv, "NeedsFake")) ;
    BrickAnnote* feedbackAnnote = 
      create_brick_annote(theEnv, "FeedbackVariable") ;
    feedbackAnnote->append_brick(create_suif_object_brick(theEnv, destination));
    feedbackAnnote->append_brick(create_suif_object_brick(theEnv, source)) ;
    feedbackVariable->append_annote(feedbackAnnote) ;
    feedbackVariable->append_annote(create_brick_annote(theEnv, "NonSystolic"));

  }
}

LString PseudoSSA::BaseName(LString x)
{
  std::string copy = x.c_str() ;
  int position = copy.find("_ssaTmp") ;
  if (position != std::string::npos)
  {
    std::string toReturn = copy.substr(0, position) ;
    return LString(toReturn.c_str()) ;
  }

  position = copy.find("_moduleOut") ;
  if (position != std::string::npos)
  {
    std::string toReturn = copy.substr(0, position) ;
    return LString(toReturn.c_str()) ;
  }

  return x ;

}
