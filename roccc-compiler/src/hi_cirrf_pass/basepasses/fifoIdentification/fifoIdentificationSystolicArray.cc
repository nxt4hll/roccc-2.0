// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
/*

  This pass is responsible for detecting the one fifo in systolic
   array generation and setting up the input and output copies.

*/

#include <cassert>
#include <sstream>
#include <iostream>

#include <suifkernel/utilities.h>
#include <basicnodes/basic.h>
#include <basicnodes/basic_factory.h>
#include <suifnodes/suif_factory.h>
#include <typebuilder/type_builder.h>

#include "fifoIdentificationSystolicArray.h"
#include "roccc_utils/loop_utils.h"
#include "roccc_utils/warning_utils.h"

FifoIdentificationSystolicArrayPass::FifoIdentificationSystolicArrayPass(SuifEnv* pEnv) :
  PipelinablePass(pEnv, "FifoIdentificationSystolicArray")
{
  theEnv = pEnv ;
  procDef = NULL ;

  inputArray = NULL ;
  outputArray = NULL ;

  refExpr = NULL ;
  lastDest = NULL ;
}

void FifoIdentificationSystolicArrayPass::do_procedure_definition(ProcedureDefinition* proc_def)
{
  procDef = proc_def ;
  assert(procDef != NULL) ;

  OutputInformation("Fifo identification for systolic arrays begins") ;
  
  // The trouble with systolic array fifos is that there is only one of them
  //  and it is terrible.

  CForStatement* innermostLoop = FindInnermostLoop() ;

  if (innermostLoop == NULL)
  {
    OutputWarning("Cannot identify fifos when no loop exists!") ;
    return ;
  }

  CollectInformation(innermostLoop) ;
  CreateDuplicates() ;
  Replace(innermostLoop) ;

  OutputInformation("Fifo identification for systolic array ends") ;
}

// Note: This code does not work if there is more than one loop nest.  It
//  will return 'a' innermost loop, but it is unknown which one.
CForStatement* FifoIdentificationSystolicArrayPass::FindInnermostLoop()
{
  assert(theEnv != NULL) ;
  assert(procDef != NULL) ;

  CForStatement* innermost = NULL ;

  list<CForStatement*>* allFors = 
    collect_objects<CForStatement>(procDef->get_body()) ;
  assert(allFors != NULL) ;
  list<CForStatement*>::iterator forIter = allFors->begin() ;
  while (forIter != allFors->end())
  {
    Statement* body = (*forIter)->get_body() ;
    Iter<CForStatement> iter = object_iterator<CForStatement>(body) ;
    if (!iter.is_valid())
    {
      innermost = (*forIter) ;
    }
    ++forIter ;
  }
 
  delete allFors ;
  
  return innermost ;
}

// This function is responsible for collecting the load and store instructions
//  that need to change as well as an array reference expression that will be 
//  used to create the new variable symbols.
//
// I have to be careful as well, there should only be one two-dimensional
//  array, but there might be other one-dimensional arrays, so I have to 
//  find those.
void FifoIdentificationSystolicArrayPass::CollectInformation(CForStatement* c)
{

  // Keep the last variable that is stored as reference for later on.

  list<Statement*>* allStatements = 
    collect_objects<Statement>(c->get_body()) ;
  assert(allStatements != NULL) ;
  
  list<Statement*>::iterator findIter = allStatements->begin() ;
  list<Statement*>::iterator follow = allStatements->end() ;
  while(findIter != allStatements->end())
  {
    if (dynamic_cast<CallStatement*>(*findIter) != NULL)
    {      
      follow = findIter ;
    }
    else if (dynamic_cast<StoreVariableStatement*>(*findIter) != NULL)
    {
      // We need a further check that we are not storing to a feedback
      //  variable.
      if (dynamic_cast<StoreVariableStatement*>(*findIter)->get_destination()->lookup_annote_by_name("FeedbackVariable") == NULL)
      {
	follow = findIter ;
      }
    }
    else if (dynamic_cast<StoreStatement*>(*findIter) != NULL)
    {
      follow = findIter ;
    }
    ++findIter ;
  }

  assert(follow != allStatements->end()) ;

  if (dynamic_cast<StoreVariableStatement*>(*follow) != NULL)
  {
    lastDest = dynamic_cast<StoreVariableStatement*>(*follow)->get_destination();
  }
  else if (dynamic_cast<StoreStatement*>(*follow) != NULL)
  {
    Expression* value = dynamic_cast<StoreStatement*>(*follow)->get_value() ;
    LoadVariableExpression* loadValue = 
      dynamic_cast<LoadVariableExpression*>(value) ;
    assert(loadValue != NULL) ;
    lastDest = loadValue->get_source() ;
  }
  else
  {
    OutputError("Call statement should not be the last statement!") ;
    assert(0) ;
    lastDest = dynamic_cast<CallStatement*>(*follow)->get_destination() ;
  }

  delete allStatements ;

  // Now the meat of the function.
  //  First, collect the store variable statements
  list<StoreVariableStatement*>* allStoreVars = 
    collect_objects<StoreVariableStatement>(c) ;
  assert(allStoreVars != NULL) ;

  list<StoreVariableStatement*>::iterator storeVarIter = allStoreVars->begin();
  while (storeVarIter != allStoreVars->end())
  {
    // The statement that we are looking for will NOT have the
    //  "NonPrintable" annotation.
    if ((*storeVarIter)->lookup_annote_by_name("NonPrintable") == NULL)
    {
      // There's more than just an annotation we need to look for, though.
      //  We also have to make sure that the right hand side is an array
      //  reference (or a multi-dimensional array reference).
      Expression* rightHandSide = (*storeVarIter)->get_value() ;
      LoadExpression* rightHandLoad = 
	dynamic_cast<LoadExpression*>(rightHandSide) ;
      
      if (rightHandLoad != NULL)
      {
	// We are almost sure that this is the instruction we are
	//  looking for.  If the destination of this store variable instruction
	//  happens to be a source in the first line in the datapath, we
	//  want to add it.  If not we just want to remove it (by adding
	//  the nonPrintable annote).
	//  Lookup reached_uses and see if it is the first element in the
	//  datapath...

	BrickAnnote* reachedAnnotes = to<BrickAnnote>((*storeVarIter)->lookup_annote_by_name("reached_uses")) ;
	assert(reachedAnnotes != NULL) ;

	bool addValue = true;
	
	// Here I check to see if I am adding a one dimensional or two
	//  dimensional array.  If it is a two-dimensional array go ahead.
	Expression* sourceAddr = rightHandLoad->get_source_address() ;
	ArrayReferenceExpression* arrayAddr = 
	  dynamic_cast<ArrayReferenceExpression*>(sourceAddr) ;
	assert(arrayAddr != NULL) ;

	if (dynamic_cast<ArrayReferenceExpression*>(arrayAddr->get_base_array_address()) == NULL)
	{
	  addValue = false; 
	}

	if (addValue)
	{
	  // This is the instruction we have been looking for, 
	  //  Go ahead and get the array reference expression and store it
	  //  as well as the store statement and it's index
	  Expression* sourceAddr = rightHandLoad->get_source_address() ;
	  assert(dynamic_cast<ArrayReferenceExpression*>(sourceAddr) != NULL) ;
	  refExpr = dynamic_cast<ArrayReferenceExpression*>(sourceAddr) ;
	  allStoreVariablesToReplace.push_back(*storeVarIter) ;
	  allInputIndicies.push_back(refExpr->get_index()) ;	
	}
	else
	{
	  // This store statement is either going to become an input
	  //  scalar or should be printed as a normal array.  This is
	  //  an access to a one dimensional array other than
	  //  the feedback array.  So determine if the index is all 
	  //  constants or not.
	  
	  if (AllConstants(arrayAddr->get_index()) == true)
	  {
	    // Mark as input scalar
	    (*storeVarIter)->get_destination()->append_annote(create_brick_annote(theEnv, "InputScalar")) ;
	    // mark as unprintable
	  (*storeVarIter)->append_annote(create_brick_annote(theEnv,
							     "NonPrintable")) ;
	  }
	  else
	  {
	    // Don't do anything, we'll pick this up in the next pass
	  }	  
	}
      }

    }
    ++storeVarIter ;
  }

  delete allStoreVars ;

  // Now, find the appropriate Store instruction for the output fifo
  list<StoreStatement*>* allStores = 
    collect_objects<StoreStatement>(c) ;
  assert(allStores != NULL) ;

  list<StoreStatement*>::iterator storeIter = allStores->begin() ;
  while (storeIter != allStores->end())
  {
    // Once again, we are looking for a statement that does
    //  not have the "NonPrintable" annotation.  
    if ((*storeIter)->lookup_annote_by_name("NonPrintable") == NULL)
    {
      // There should be only one.  Go ahead and store the information
      //  that we require.
      ArrayReferenceExpression* dest = dynamic_cast<ArrayReferenceExpression*>((*storeIter)->get_destination_address()) ;
      assert(dest != NULL) ;
      allStoresToReplace.push_back(*storeIter) ;
      allOutputIndicies.push_back(dest->get_index()) ;
    }
    ++storeIter ;
  }

  delete allStores ;

}

// The create duplicates function relies on us identifying an array 
//  reference expression based upon the two-dimensional array.
void FifoIdentificationSystolicArrayPass::CreateDuplicates()
{
  assert(refExpr != NULL) ;

  // First thing is I have to get the Variable symbol.  This should be
  //  a two-dimensional array, so I need to go through a couple of 
  //  layers before I hit the variable symbol

  Expression* baseAddress = refExpr->get_base_array_address() ;
  int dimensionality = 1 ;

  while (dynamic_cast<ArrayReferenceExpression*>(baseAddress) != NULL)
  {
    baseAddress = dynamic_cast<ArrayReferenceExpression*>(baseAddress)->get_base_array_address() ;
    ++dimensionality ;
  }

  assert(dimensionality == 2) ;

  SymbolAddressExpression* symbolAddress = 
    dynamic_cast<SymbolAddressExpression*>(baseAddress) ;

  assert(symbolAddress != NULL) ;

  Symbol* baseSymbol = symbolAddress->get_addressed_symbol() ;
  VariableSymbol* arraySymbol = dynamic_cast<VariableSymbol*>(baseSymbol) ;

  assert(arraySymbol != NULL) ;

  // We are going to be removing this symbol and replacing it with two
  //  equivalent arrays (one for input and one for output).  I will
  //  have to mark the original symbol as removed.
  Annote* removedAnnote = 
    arraySymbol->lookup_annote_by_name("RemovedVariable") ;
  if (removedAnnote == NULL)
  {
    arraySymbol->append_annote(create_brick_annote(theEnv, "RemovedVariable"));
  }

  // All right, now I have to get the type (a two-dimensional array) and
  //  create an equivalent one dimensional array based off of the 
  //  last dimension...
  ArrayType* twoDimType = 
    dynamic_cast<ArrayType*>(arraySymbol->get_type()->get_base_type()) ;
  assert(twoDimType != NULL) ;
  
  QualifiedType* oneDimType = twoDimType->get_element_type() ;

  assert(oneDimType != NULL) ;

  LString inputName = arraySymbol->get_name() + "_input" ;
  LString outputName = arraySymbol->get_name() + "_output" ;

  inputArray = create_variable_symbol(theEnv,
				      oneDimType,
				      inputName,
				      true) ;

  outputArray = create_variable_symbol(theEnv,
				       oneDimType,
				       outputName,
				       true) ;

  // Add these variables to the procedure symbol table
  procDef->get_symbol_table()->append_symbol_table_object(inputArray) ;
  procDef->get_symbol_table()->append_symbol_table_object(outputArray) ;
}

// This function is responsible for replacing all of the loads with
//  the appropriate array and all of the stores with the appropriate array
void FifoIdentificationSystolicArrayPass::Replace(CForStatement* c)
{
  assert(inputArray != NULL) ;
  assert(outputArray != NULL) ;

  assert(allStoreVariablesToReplace.size() == allInputIndicies.size()) ;
  assert(allStoresToReplace.size() == allOutputIndicies.size()) ;

  // First replace the store variable instructionsr
  list<StoreVariableStatement*>::iterator storeVarIter ;
  storeVarIter = allStoreVariablesToReplace.begin() ;
  list<Expression*>::iterator inputIndexIter ;
  inputIndexIter = allInputIndicies.begin() ;
  // Result type should be the same for both stores and store variables.
  DataType* resultType ;
  while (storeVarIter != allStoreVariablesToReplace.end())
  {
    StoreVariableStatement* storeVariableToReplace = (*storeVarIter) ;

    // In order to replace the store we must construct a load expression.  
    //  The result type of the load expression should be same as the 
    //  destination of the store
    resultType = 
      storeVariableToReplace->get_destination()->get_type()->get_base_type() ;

    // The Address expression must be created with a little bit of care.

    SymbolAddressExpression* baseArrayAddress =
      create_symbol_address_expression(theEnv,
				       inputArray->get_type()->get_base_type(),
				       inputArray) ;

    ArrayReferenceExpression* addrExpression = 
      create_array_reference_expression(theEnv,
					resultType,
					baseArrayAddress,
					dynamic_cast<Expression*>((*inputIndexIter)->deep_clone())) ;
  
    LoadExpression* replacementLoad = create_load_expression(theEnv,
							     resultType,
							     addrExpression) ;

    storeVariableToReplace->replace(storeVariableToReplace->get_value(),
				    replacementLoad) ;

    ++storeVarIter ;
    ++inputIndexIter ;
  }

  // We should have replaced at least one!
  assert(resultType != NULL) ;

  // Now replace the store instructions
  list<StoreStatement*>::iterator storeIter = allStoresToReplace.begin() ;
  list<Expression*>::iterator outputIndexIter = allOutputIndicies.begin() ;
  int storeCount = 0 ;
  while (storeIter != allStoresToReplace.end())
  {
    StoreStatement* storeToReplace = (*storeIter) ;
    // In order to replace the store we need to replace the destination
    //  with our own array reference expression.
    SymbolAddressExpression* baseArrayAddress =
      create_symbol_address_expression(theEnv,
			       outputArray->get_type()->get_base_type(),
			       outputArray) ;  

    ArrayReferenceExpression* addrExpression = 
      create_array_reference_expression(theEnv,
					resultType,
					baseArrayAddress,
					dynamic_cast<Expression*>((*outputIndexIter)->deep_clone())) ;

    storeToReplace->replace(storeToReplace->get_destination_address(),
			    addrExpression) ;

    ++storeIter ;
    ++outputIndexIter ;
    ++storeCount ;
  }
  
  if (storeCount == 0)
  {
    // We must have at least one output store, otherwise the hi-cirrf will
    //  be incorrect...

    // The value that gets output must be dropped out of the bottom.  Luckily
    //  we've saved this value for just this occasion.
    
    assert(lastDest != NULL) ;

    // Create a new store instruction, and append it after
    //  the last store variable statement.

    // Create an array reference expression for the destination of the 
    //  store statement
    SymbolAddressExpression* baseArrayAddress =
      create_symbol_address_expression(theEnv,
			       outputArray->get_type()->get_base_type(),
			       outputArray) ;  

    LoadVariableExpression* index = 
      create_load_variable_expression(theEnv,
				      resultType,
				      get_c_for_basic_induction_variable(c)) ;
    IntConstant* zero = 
      create_int_constant(theEnv,
			  resultType,
			  0) ;

    // I need a binary expression with the index + the constant 0.
    BinaryExpression* indexPlusZero = 
      create_binary_expression(theEnv, 
			       resultType,
			       "add",
			       index,
			       zero) ;

    ArrayReferenceExpression* addrExpression = 
      create_array_reference_expression(theEnv,
					resultType,
					baseArrayAddress,
					indexPlusZero) ;

    // Create a load variable expression based off the destination of the
    //  last store variable statement.
    LoadVariableExpression* loadLastVarExp = 
      create_load_variable_expression(theEnv,
				      lastDest->get_type()->get_base_type(),
				      lastDest) ;

    StoreStatement* fallthrough = create_store_statement(theEnv,
							 loadLastVarExp,
							 addrExpression) ;

    Statement* body = c->get_body() ;
    StatementList* bodyList = dynamic_cast<StatementList*>(body) ;
    assert(bodyList != NULL) ;
    bodyList->append_statement(fallthrough) ;    

  }  
}

bool FifoIdentificationSystolicArrayPass::AllConstants(Expression* i)
{
  if (dynamic_cast<Constant*>(i) != NULL)
  {
    return true ;
  }
  if (dynamic_cast<BinaryExpression*>(i) != NULL)
  {
    BinaryExpression* binExp = dynamic_cast<BinaryExpression*>(i) ;
    return AllConstants(binExp->get_source1()) && 
      AllConstants(binExp->get_source2()) ;
  }
  return false ;
}
