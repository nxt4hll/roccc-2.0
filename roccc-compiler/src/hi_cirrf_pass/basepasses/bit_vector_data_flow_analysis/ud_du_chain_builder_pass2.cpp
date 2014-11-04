// The ROCCC Compiler Infrastructure 
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*

  This file contains the definitions of the UD/DU chain builder class.

*/

#include <iostream>
#include <suifnodes/suif.h>
#include <basicnodes/basic.h>
#include <basicnodes/basic_factory.h>
#include <suifkernel/utilities.h>
#include <utils/expression_utils.h>
#include <cassert>

#include "ud_du_chain_builder_pass2.h"
#include "roccc_utils/bitVector2.h"
#include "roccc_utils/warning_utils.h"

UD_DU_ChainBuilderPass2::UD_DU_ChainBuilderPass2(SuifEnv *pEnv) : 
  PipelinablePass(pEnv, "UD_DU_ChainBuilderPass2") 
{
  theEnv = pEnv ;
  procDef = NULL ;
}

UD_DU_ChainBuilderPass2::~UD_DU_ChainBuilderPass2()
{
  ; // Nothing yet
}

void UD_DU_ChainBuilderPass2::ClearMap()
{
  reverseMap.clear() ;
}

void UD_DU_ChainBuilderPass2::do_procedure_definition(ProcedureDefinition* proc_def)
{
  procDef = proc_def ;
  assert(procDef != NULL) ;

  theEnv = get_suif_env() ;

  OutputInformation("UD DU chain builder 2.0 begins") ;

  InitializeMap() ;

   // For all possible definitions, set up the "reached_uses" annotation
   // For all possible uses, set up the "reaching_defs" annotation
   // Also, for each variable symbol in the symbol table, we have
   //  a reached_uses annotation

   SetupAnnotations() ;
   
   BuildChains() ;

   OutputInformation("UD DU chain builder 2.0 ends") ;
}

// This function performs the same thing as the initialization pass 
//  done in the data flow solve pass, which should have been run immediately
//  previous to this pass.  Since no optimizations will have taken place
//  I can assume the order in which I find things is the same.
void UD_DU_ChainBuilderPass2::InitializeMap()
{

  if (reverseMap.empty() != true)
  {
    ClearMap() ;
  }
  assert(reverseMap.empty()) ;

  assert(procDef != NULL) ;

  int totalDefinitions = 0 ;
  
  // I did store variable statements first, followed by call statements.
  
  list<StoreVariableStatement*>* allStores = 
    collect_objects<StoreVariableStatement>(procDef->get_body()) ;

  assert(allStores != NULL) ;

  list<StoreVariableStatement*>::iterator storeIter =
    allStores->begin() ;
  while(storeIter != allStores->end())
  {
    reverseMap[totalDefinitions] = (*storeIter) ;
    ++totalDefinitions ;

    ++storeIter ;
  }

  delete allStores ;

  // Now call statements
  list<CallStatement*>* allCalls = 
    collect_objects<CallStatement>(procDef->get_body()) ;
  assert (allCalls != NULL) ;

  list<CallStatement*>::iterator callIter = allCalls->begin() ;
  while (callIter != allCalls->end())
  {
    // Check destination first
    VariableSymbol* dest = (*callIter)->get_destination() ;
    if (dest != NULL)
    {
      reverseMap[totalDefinitions] = (*callIter) ;
      ++totalDefinitions ;
    }
    // Now check all arguments
    for (unsigned int i = 0 ; i < (*callIter)->get_argument_count() ; ++i)
    {
      Expression* nextArg = (*callIter)->get_argument(i) ;
      if (dynamic_cast<SymbolAddressExpression*>(nextArg) != NULL)
      {
	reverseMap[totalDefinitions] = (*callIter) ;
	++totalDefinitions ;
      }
    }
    ++callIter ;
  }

  delete allCalls ;
}

void UD_DU_ChainBuilderPass2::SetupAnnotations()
{
  assert(procDef != NULL) ;
  assert(theEnv != NULL) ;

  list<StoreVariableStatement*>* allStores = 
    collect_objects<StoreVariableStatement>(procDef->get_body()) ;

  list<StoreVariableStatement*>::iterator storeIter ;
  storeIter = allStores->begin() ;
  while (storeIter != allStores->end())
  {
    if ((*storeIter)->lookup_annote_by_name("reached_uses") != NULL)
    {
      delete ((*storeIter)->remove_annote_by_name("reached_uses")) ;
    }

    BrickAnnote* reachedUses = create_brick_annote(theEnv, "reached_uses") ;
    (*storeIter)->append_annote(reachedUses) ;

    ++storeIter ;
  }

  delete allStores ;

  list<CallStatement*>* allCalls = 
    collect_objects<CallStatement>(procDef->get_body()) ;

  list<CallStatement*>::iterator callIter = allCalls->begin() ;
  while (callIter != allCalls->end())
  {
    if ((*callIter)->lookup_annote_by_name("reached_uses") != NULL)
    {
      delete ((*callIter)->remove_annote_by_name("reached_uses")) ;
    }

    BrickAnnote* reachedUses = create_brick_annote(theEnv, "reached_uses") ;
    (*callIter)->append_annote(reachedUses) ;    

    ++callIter ;
  }
  
  delete allCalls ;

  list<LoadVariableExpression*>* allLoads = 
    collect_objects<LoadVariableExpression>(procDef->get_body()) ;
  
  list<LoadVariableExpression*>::iterator loadIter = allLoads->begin() ;
  while (loadIter != allLoads->end())
  {
    if ((*loadIter)->lookup_annote_by_name("reaching_defs") != NULL)
    {
      delete (*loadIter)->remove_annote_by_name("reaching_defs") ;
    }
    BrickAnnote* reachingDefs = create_brick_annote(theEnv, "reaching_defs") ;
    (*loadIter)->append_annote(reachingDefs) ;

    ++loadIter ;
  }

  delete allLoads ;

  SymbolTable* symTab = procDef->get_symbol_table() ;
  Iter<SymbolTableObject*> symIter =
    symTab->get_symbol_table_object_iterator() ;
  while (symIter.is_valid())
  {
    if (is_a<VariableSymbol>(symIter.current()))
    {
      VariableSymbol* nextVar = to<VariableSymbol>(symIter.current()) ;
      assert(nextVar != NULL) ;
      if (nextVar->lookup_annote_by_name("reached_uses") != NULL)
      {
	delete (nextVar->remove_annote_by_name("reached_uses")) ;
      }
      
      BrickAnnote* reachedUses = create_brick_annote(theEnv, "reached_uses");
      nextVar->append_annote(reachedUses) ;
    }
    symIter.next() ;
  }
}

void UD_DU_ChainBuilderPass2::BuildChains()
{
  assert(procDef != NULL) ;
  assert(theEnv != NULL) ;
  
  // All the annotations are set up and empty, so let's go through
  //  and fill them up.  To do this, all we have to do is visit every 
  //  load variable expression and set things up that way (this is where
  //  the revese map would have come in...)

  list<LoadVariableExpression*>* allLoads = 
    collect_objects<LoadVariableExpression>(procDef->get_body()) ;

  list<LoadVariableExpression*>::iterator loadIter ;
  loadIter = allLoads->begin() ;

  while (loadIter != allLoads->end())
  {
    // Find the Brick annote for "reaching_defs" that we will be adding to
    BrickAnnote* reachingDefs = 
      to<BrickAnnote>((*loadIter)->lookup_annote_by_name("reaching_defs")) ;
    assert(reachingDefs != NULL) ;

    // Get the bit vector associated with the in_stmts for this load
    //  variable expression.  This bit vector should tell us the definitions
    //  that reach this spot.  The bit vector, however, is associated with
    //  the parent statement and not the expression.

    Statement* parentStmt = get_expression_owner(*loadIter) ;
    assert(parentStmt != NULL) ;
    BrickAnnote* inStmtAnnote = 
      to<BrickAnnote>(parentStmt->lookup_annote_by_name("in_stmts")) ;
    assert(inStmtAnnote != NULL) ;
    SuifObjectBrick* sob = to<SuifObjectBrick>(inStmtAnnote->get_brick(0)) ;
    assert(sob != NULL) ;
    BitVector2* currentBitVector = 
      dynamic_cast<BitVector2*>(sob->get_object()) ;
    assert(currentBitVector != NULL) ;
    assert(currentBitVector->size() == reverseMap.size()) ;

    // Now, go through each element in the bit vector and make annotations
    //  based upon both the map and the reverse map.
    
    // I have to do a little more here than I previously thought.  Just 
    //  because a definition reaches this point I shouldn't mark
    //  it as a reaching definition unless I am using the variable defined
    //  in that reaching definition.
    VariableSymbol* currentSymbol = (*loadIter)->get_source() ;

    for (unsigned int location = 0 ; 
	 location < currentBitVector->size() ;
	 ++location)
    {
      if (currentBitVector->isMarked(location))
      {
	bool doAppend = false ;
	Statement* reverseStatement = reverseMap[location] ;
	assert(reverseStatement != NULL) ;
	if (dynamic_cast<StoreVariableStatement*>(reverseStatement) != NULL)
	{
	  if (dynamic_cast<StoreVariableStatement*>(reverseStatement)->get_destination() == currentSymbol)
	  {
	    doAppend = true ;
	  }
	}
	else if (dynamic_cast<CallStatement*>(reverseStatement) != NULL)
	{
	  // Check the destination and all the arguments
	  CallStatement* reverseCall = dynamic_cast<CallStatement*>(reverseStatement) ;
	  if (reverseCall->get_destination() == currentSymbol)
	  {
	    doAppend = true ;
	  }

	  for (unsigned int j = 0 ; 
	       j < reverseCall->get_argument_count() ;
	       ++j)
	  {
	    Expression* nextExp = reverseCall->get_argument(j) ;
	    if (dynamic_cast<SymbolAddressExpression*>(nextExp) != NULL)
	    {
	      if (dynamic_cast<SymbolAddressExpression*>(nextExp)->get_addressed_symbol() == currentSymbol)
	      {
		doAppend = true ;
	      }
	    }
	  }

	}
	else
	{
	  assert(0 && "Unknown definition!") ;
	}

	if (doAppend == true)
	{
	  // Append the reaching definition to this load variable expression
	  reachingDefs->
	    append_brick(create_suif_object_brick(theEnv, reverseMap[location]));
	  // Also append a reached use to the load
	  BrickAnnote* reachedUses = 
	    to<BrickAnnote>(reverseMap[location]->lookup_annote_by_name("reached_uses")) ;
	  assert(reachedUses != NULL) ;
	  reachedUses->append_brick(create_suif_object_brick(theEnv,
							   (*loadIter))) ;
	  }
      }
    }
    
    ++loadIter ;
  }
  
  delete allLoads ;

}

void UD_DU_ChainBuilderPass2::DumpChains()
{
  assert(procDef != NULL) ;
  assert(theEnv != NULL) ;

  // Just as a test, we output all of the chain information we discovered
  //  to compare with the information Betul generated

  list<StoreVariableStatement*>* allStores = 
    collect_objects<StoreVariableStatement>(procDef->get_body()) ;

  assert(allStores != NULL) ;
 
  list<StoreVariableStatement*>::iterator storeIter = allStores->begin() ;
  int counter = 1 ;
  int useCount ;
  while (storeIter != allStores->end())
  {
    BrickAnnote* reachedUses = 
      to<BrickAnnote>((*storeIter)->lookup_annote_by_name("reached_uses")) ;
    assert(reachedUses != NULL) ;
    useCount = reachedUses->get_brick_count() ;

    std::cerr << "Store Variable Statement " << counter 
	      << " Has " << useCount << " Uses." << std::endl ;
    ++storeIter ;
  }
 
  delete allStores ;

}
