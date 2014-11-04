// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include "scalar_renaming_pass2.h"

#include <suifnodes/suif_factory.h>
#include <suifkernel/utilities.h>
#include <utils/symbol_utils.h>
#include "roccc_utils/symbol_utils.h"
#include "roccc_utils/warning_utils.h"

ScalarRenamingPass2::ScalarRenamingPass2(SuifEnv *pEnv) : 
  PipelinablePass(pEnv, "ScalarRenamingPass2") 
{
  theEnv = pEnv ;
  procDef = NULL ;
}

void ScalarRenamingPass2::do_procedure_definition(ProcedureDefinition* proc_def)
{
  if (proc_def == NULL)
  {
    OutputWarning("Scalar renaming was passed a NULL procedure") ;
    return ;
  }
  procDef = proc_def ;

  OutputInformation("Scalar Renaming 2.0 begins") ;
  
  // We need to identify all the store variable statements and the
  //  call statements.
  //  ProcessStores() ;
  ProcessCalls() ;

  OutputInformation("Scalar Renaming 2.0 ends") ;
}

// In order to process stores we find the destination and replace all uses
//  with a new scalar variable.
void ScalarRenamingPass2::ProcessStores()
{
  assert(procDef != NULL) ;
  assert(theEnv != NULL) ;

  list<StoreVariableStatement*>* allStores = 
    collect_objects<StoreVariableStatement>(procDef->get_body()); 
  assert(allStores != NULL) ;
  
  list<StoreVariableStatement*>::iterator storeIter ;
  storeIter = allStores->begin() ;
  while (storeIter != allStores->end())
  {
    VariableSymbol* dest = (*storeIter)->get_destination() ;
    VariableSymbol* replacementVar = 
      new_anonymous_variable(theEnv, dest->get_symbol_table(), 
			     dest->get_type()) ;

    name_variable(replacementVar) ;

    // Find all the uses of this variable replace them
    BrickAnnote* allUses = 
      to<BrickAnnote>((*storeIter)->lookup_annote_by_name("reached_uses")) ;
    assert(allUses != NULL) ;
    Iter<SuifBrick*> useIter = allUses->get_brick_iterator() ;
    while (useIter.is_valid())
    {
      SuifObjectBrick* currentObject = to<SuifObjectBrick>(useIter.current()) ;
      assert(currentObject != NULL) ;

      // The uses should be load variable expressions
      LoadVariableExpression* nextUse = 
	dynamic_cast<LoadVariableExpression*>(currentObject->get_object()) ;

      assert(nextUse != NULL) ;
      nextUse->set_source(replacementVar) ;

      useIter.next() ;
    }
    
    // Now replace the original destination
    (*storeIter)->set_destination(replacementVar) ;

    // Now delete the original variable
    delete dest ;

    ++storeIter ;
  }
  
  delete allStores ;
}

// Now for calls, we have to create a new temporary for every destination
//  and every output argument
void ScalarRenamingPass2::ProcessCalls()
{
  assert(procDef != NULL) ;
  assert(theEnv != NULL) ;
  
  list<CallStatement*>* allCalls = 
    collect_objects<CallStatement>(procDef->get_body()) ;
  assert(allCalls != NULL) ;

  list<CallStatement*>::iterator callIter ;
  callIter = allCalls->begin() ;

  while (callIter != allCalls->end())
  {

    // Get the reached uses annotation for this call statement
    BrickAnnote* reachedUses = 
      to<BrickAnnote>((*callIter)->lookup_annote_by_name("reached_uses")) ;
    assert(reachedUses != NULL) ;

    // Handle the actual destination separately
    VariableSymbol* dest = (*callIter)->get_destination() ;
    if (dest != NULL)
    {
      // Create a replacement
      VariableSymbol* replacement = 
	new_anonymous_variable(theEnv, dest->get_symbol_table(),
			       dest->get_type()) ;
      name_variable(replacement) ;

      // Go through all of the reachedUses and replace any instances
      Iter<SuifBrick*> useIter = reachedUses->get_brick_iterator() ;
      while (useIter.is_valid())
      {
	SuifObjectBrick* sob = 
	  dynamic_cast<SuifObjectBrick*>(useIter.current());
	assert(sob != NULL) ;
	LoadVariableExpression* nextUse = 
	  dynamic_cast<LoadVariableExpression*>(sob->get_object()) ;
	assert(nextUse != NULL) ; 
	VariableSymbol* toReplace = nextUse->get_source() ;
	if (toReplace == dest)
	{
	  nextUse->set_source(replacement) ;
	}
      }
      // Now remove the original.
      delete dest ;
    }

    // Collect all the destinations from the arguments and put them in a list
    list<VariableSymbol*> allDestinations ;    
    for (unsigned int i = 0 ; i < (*callIter)->get_argument_count() ; ++i)
    {
      if (dynamic_cast<SymbolAddressExpression*>((*callIter)->get_argument(i)))
      {
	Symbol* nextSym = dynamic_cast<SymbolAddressExpression*>((*callIter)->get_argument(i))->get_addressed_symbol() ;
	VariableSymbol* nextVarSym = dynamic_cast<VariableSymbol*>(nextSym) ;
	if (nextVarSym != NULL)
	{
	  allDestinations.push_back(nextVarSym) ;
	}
      }
    }

    // Now, create a replacement variable for each of the destinations
    list<VariableSymbol*> allReplacements ;
    
    list<VariableSymbol*>::iterator destIter ;
    destIter =  allDestinations.begin() ;
    
    // Suif starts numbering arguments with 1
    int currentArgument = 1 ;
    while (destIter != allDestinations.end())
    {
      VariableSymbol* nextReplace = 
	new_anonymous_variable(theEnv, (*destIter)->get_symbol_table(),
			       (*destIter)->get_type());
      name_variable(nextReplace) ;
      allReplacements.push_back(nextReplace) ;

      // Also, replace the current variable.
      (*callIter)->replace_argument(
       currentArgument,
       create_load_variable_expression(theEnv,
				       nextReplace->get_type()->get_base_type(), 
				       nextReplace)) ;
      
      ++currentArgument ;
      ++destIter ;
    }
    

    // Go through all the uses.  For each one, replace with the
    //  appropriate replacement variable
    
    Iter<SuifBrick*> useIter = reachedUses->get_brick_iterator() ;
    while (useIter.is_valid())
    {
      SuifObjectBrick* sob = dynamic_cast<SuifObjectBrick*>(useIter.current());
      assert(sob != NULL) ;
      LoadVariableExpression* nextUse = 
	dynamic_cast<LoadVariableExpression*>(sob->get_object()) ;
      assert(nextUse != NULL) ; 

      VariableSymbol* toReplace = nextUse->get_source() ;

      list<VariableSymbol*>::iterator dstIter ;
      list<VariableSymbol*>::iterator replaceIter ;
      destIter = allDestinations.begin() ;
      replaceIter = allReplacements.begin() ;
      while(destIter != allDestinations.end())
      {
	if ((*destIter) == toReplace)
	{
	  nextUse->set_source(*replaceIter) ;
	  break ;
	}
	++destIter ;
	++replaceIter ;
      }
      if (destIter == allDestinations.end())
      {
	std::cerr << "Trying to replace:" ;
	FormattedText tmpText ;
	toReplace->print(tmpText) ;
	std::cerr << tmpText.get_value() << std::endl ;
      }
      assert(destIter != allDestinations.end()) ;

      useIter.next() ;
    }
    
    // Go through and delete all of the original variables


    // End loop
    ++callIter ;
  }

  delete allCalls ;
}
