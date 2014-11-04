// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

#include <suifkernel/utilities.h>
#include <basicnodes/basic_factory.h>
#include <suifnodes/suif.h>
#include <suifnodes/suif_factory.h>
#include <cfenodes/cfe.h>

#include <cassert>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>

#include <cfenodes/cfe.h>

#include "roccc_utils/warning_utils.h"
#include "roccc_utils/roccc2.0_utils.h"

#include "verifyPass.h"

// These are the possible types of ROCCC 2.0 systems we are creating
//static const int MODULE = 1 ;
//static const int SYSTEM = 2 ;
//static const int COMPOSABLE_SYSTEM = 3 ;

VerifyPass::VerifyPass(SuifEnv* pEnv) : PipelinablePass(pEnv, "VerifyPass")
{
  env = pEnv ;
  procDef = NULL ;
  myType = -1 ;
}

VerifyPass::~VerifyPass()
{
  ;
}

void VerifyPass::do_procedure_definition(ProcedureDefinition* p)
{
  procDef = p ;
  assert(procDef != NULL) ;

  OutputInformation("Verify pass starts") ;

  // First, determine if we are compiling a module or system
  myType = DetermineType() ;
  
  // Now, make all the appropriate checks
  VerifyRocccStatus() ;

  OutputInformation("Verify pass ends") ;
}

void VerifyPass::VerifyRocccStatus()
{
  assert(myType == MODULE || myType == SYSTEM || myType == COMPOSED_SYSTEM) ;

  // Generic checks
  VerifySizes() ;
  VerifyShifts() ;
  VerifyShortCircuit() ;
  VerifyLUTWrites() ;

  // Checks specific modules
  if (myType == MODULE)
  {
    VerifyModuleParameters() ;
    VerifyModuleDestinations() ;
    VerifyModuleSources() ;
    VerifyModuleLoop() ;
    VerifyModuleNames() ;
    VerifyModuleOutputWrites() ;
    VerifyModuleFeedback() ;
  }

  // Checks specific to systems
  if (myType == SYSTEM)
  {
    VerifyCLoop() ;
    VerifyArrayAccesses() ;
    VerifySystemOutputWrites() ;
    VerifyInputOutputUniqueness() ;
  }

  // Checks specific to composable systems
  if (myType == COMPOSED_SYSTEM)
  {
  }
}

// What this function does is grab the parameter that is passed in.  It 
//  should be a structure because we are inside a module.  We then check
//  to make sure both inputs and outputs are declared.  We don't have to go
//  through the implementation code because if a variable isn't used it isn't
//  put in the structure definition.
void VerifyPass::VerifyModuleParameters()
{
  assert(procDef != NULL) ;
  ProcedureSymbol* procSym = procDef->get_procedure_symbol() ;
  ProcedureType* procType = procSym->get_type() ;
  CProcedureType* cProcType = dynamic_cast<CProcedureType*>(procType) ;

  assert(cProcType != NULL) ;

  // There should only be one parameter passed to a module
  if (cProcType->get_argument_count() != 1)
  {
    std::cerr << "ERROR: " 
	      << "Module implementations should only have one struct argument"
	      << std::endl ;
    assert(0) ;
  }
  
  StructType* paramType = 
    dynamic_cast<StructType*>(cProcType->get_argument(0)->get_base_type()) ;

  assert(paramType != NULL) ;

  // Check to see the return type is equivalent to the same as the passed
  //  parameter
  if (dynamic_cast<StructType*>(cProcType->get_result_type()) == NULL)
  {
    std::cerr << "ERROR: "
	      << "Module implementations must return a struct value"
	      << std::endl ;
    assert(0) ;
  }
  if (cProcType->get_result_type() != 
      cProcType->get_argument(0)->get_base_type())
  {
    std::cerr << "ERROR: "
	      << "Return type must have the same type as the passed parameter"
	      << std::endl ;
    assert(0) ;
  }

  bool hasInputs = false ;
  bool hasOutputs = false ;

  SymbolTable* internalTable = paramType->get_group_symbol_table() ;
  assert(internalTable != NULL) ;

  for (int i = 0 ; i < internalTable->get_symbol_table_object_count() ; ++i)
  {
    SymbolTableObject* curSym = internalTable->get_symbol_table_object(i) ;
    LString currentName = curSym->get_name() ;

    if (strstr(currentName.c_str(), "_in") != NULL)
    {
      hasInputs = true ;
    }
    if (strstr(currentName.c_str(), "_out") != NULL)
    {
      hasOutputs = true ;
    }
    // Also, we need to verify that the type of each one is not
    //  an array or a struct.  This MIGHT be extended in the future.
    VariableSymbol* curVar = dynamic_cast<VariableSymbol*>(curSym) ;
    if (curVar != NULL)
    {
      DataType* curType = curVar->get_type()->get_base_type() ;
      // Array verification check - Currently disabled
      /*
      if (dynamic_cast<ArrayType*>(curType) != NULL)
      {
	std::cerr << "ERROR: " ;
	std::cerr << "Module code cannot currently contain arrays inside "
		  << "the interface struct!" << std::endl ;
	assert(0) ;
      } 
      */     
      if (dynamic_cast<GroupType*>(curType) != NULL)
      {
	std::cerr << "ERROR: " ;
	std::cerr << "Module code cannot contain unions or structs inside"
		  << " the interface struct!" << std::endl ;
	assert(0) ;
      }
    }
  }

  if (!(hasInputs && hasOutputs))
  {
    std::cerr << "ERROR: " ;
    std::cerr << "Module code must use both inputs and outputs "
	      << "in the implementation!" << std::endl ;
    assert(0) ;
  }
}

void VerifyPass::VerifySizes()
{
  assert(procDef != NULL) ;
  SymbolTable* symTab = procDef->get_symbol_table() ;
  
  VerifySizesWorkhorse(symTab) ;
}

void VerifyPass::VerifySizesWorkhorse(SymbolTable* symTab)
{
  assert(symTab != NULL) ;
  
  for (int i = 0 ; i < symTab->get_symbol_table_object_count() ; ++i)
  {
    SymbolTableObject* nextSym = symTab->get_symbol_table_object(i) ;
    
    if (dynamic_cast<VariableSymbol*>(nextSym) != NULL)
    {
      DataType* nextType = 
	dynamic_cast<VariableSymbol*>(nextSym)->get_type()->get_base_type() ;
      assert(nextType != NULL) ;
    
      if (nextType->get_bit_size().c_int() < 1)
      {
	std::cerr << "Cannot declare a variable with a bit size less than 1!"
		  << std::endl ;
	std::cerr << "Bit size:" << nextType->get_bit_size().c_int() 
		  << std::endl ;
	std::cerr << "Variable symbol: " 
		  << dynamic_cast<VariableSymbol*>(nextSym)->get_name() 
		  << std::endl ;
	assert(0) ;
      }

      // If this happens to be a group type, we have to go verify all of the
      //  internal elements.  
      if (dynamic_cast<GroupType*>(nextType) != NULL)
      {
	VerifySizesWorkhorse(dynamic_cast<GroupType*>(nextType)->get_group_symbol_table()) ;
      }

    }    
  }
}

void VerifyPass::VerifyShifts()
{
  assert(procDef != NULL) ;

  list<BinaryExpression*>* allBinExps = 
    collect_objects<BinaryExpression>(procDef->get_body()) ;
  
  list<BinaryExpression*>::iterator binIter = allBinExps->begin() ;
  while (binIter != allBinExps->end())
  {
    if ((*binIter)->get_opcode() == LString("left_shift") ||
	(*binIter)->get_opcode() == LString("right_shift"))
    {
      Expression* rightHandSide = (*binIter)->get_source2() ;
      if (dynamic_cast<IntConstant*>(rightHandSide) == NULL)
      {
	OutputError("ERROR:  Cannot shift by a variable amount!") ;
	assert(0) ;
      }
    }

    ++binIter ;
  }
 
  delete allBinExps ;

}

void VerifyPass::VerifyShortCircuit()
{
  assert(procDef != NULL) ;
  
  list<BinaryExpression*>* allBinExps = 
    collect_objects<BinaryExpression>(procDef->get_body()) ;
  list<BinaryExpression*>::iterator binIter = allBinExps->begin() ;
  while (binIter != allBinExps->end())
  {
    if ((*binIter)->get_opcode() == LString("logical_and") ||
	(*binIter)->get_opcode() == LString("logical_or")) 
    {
      OutputError("Operators which perform short circuit evaluation"
		  " are currently not supported") ;
      assert(0) ;
    }
    ++binIter ;
  }
  delete allBinExps ;
}

void VerifyPass::VerifyLUTWrites()
{
  assert(procDef != NULL) ;

  list<VariableSymbol*> previouslySeen ;

  list<CallStatement*>* allCalls = 
    collect_objects<CallStatement>(procDef->get_body()) ;
  list<CallStatement*>::iterator callIter = allCalls->begin() ;
  while (callIter != allCalls->end())
  {
    // Check the name of the function
    SymbolAddressExpression* functionAddress = 
     dynamic_cast<SymbolAddressExpression*>((*callIter)->get_callee_address());
    assert(functionAddress != NULL) ;
    std::string functionName = 
      functionAddress->get_addressed_symbol()->get_name().c_str() ;

    if (functionName.find("ROCCCLUTStore") == std::string::npos)
    {
      ++callIter ;
      continue ;
    }
    
    // Get the first argument, which should be a load variable expression
    //  to the variable of the LUT
    LoadVariableExpression* lutArg = 
      dynamic_cast<LoadVariableExpression*>((*callIter)->get_argument(0)) ;
    assert(lutArg != NULL) ;
    VariableSymbol* lutVar = lutArg->get_source() ;   

    list<VariableSymbol*>::iterator lutIter = previouslySeen.begin() ;
    while (lutIter != previouslySeen.end())
    {
      if (lutVar == (*lutIter))
      {
	std::cerr << "ERROR: We currenlty only support one write to "
		  << "a lookup table.  Multiple reads are supported."
		  << std::endl ;
	assert(0) ;
      }
      ++lutIter ;
    }

    previouslySeen.push_back(lutVar) ;

    ++callIter ;
  }
  delete allCalls ;
}

void VerifyPass::VerifyCLoop()
{
  if (!HasCForLoop())
  {
    std::cerr << "ERROR: " ;
    std::cerr << "System code must have a loop and Module code must "
	      << "have inputs and outputs!" << std::endl ;
    assert(0) ;
  }
  // Also make sure there is only one for loop
  
  if (!OnlyOneCForLoopNest())
  {
    std::cerr << "ERROR: " ;
    std::cerr << "System code currently must only have one loop nest!  " 
	      << "Did you unroll a nonconstant loop or unroll a loop"
	      << " by a nondivisible amount?" << std::endl ;
    assert(0) ;
  }

  // Also, make sure that the loops are perfectly nested
  
  if (!PerfectlyNestedLoops())
  {
    std::cerr << "ERROR: " ;
    std::cerr << "System code currently can only contain perfectly nested" 
	      << " loops!" << std::endl 
	      << "Multiple loops must not have any code in between them!"
	      << std::endl ;
    assert(0) ;
  }
  
  // We currently only support for loops that have a "less than" or "less
  //  than or equal" comparisons or ar infinite
  if (!ComparisonCorrect())
  {
    std::cerr << "ERROR: " ;
    std::cerr << "We currently only support less than, less than or"
	      << " equal comparisons, and the integer constant 1 in for loops!"
	      << std::endl ;
    std::cerr << "For nested loops, only the outermost loop can be infinite."
	      << std::endl ;
    assert(0) ;
  }   
}

void VerifyPass::VerifyArrayAccesses()
{
  assert(procDef != NULL) ;

  ClearIndicies() ;
  CollectIndicies() ;
  
  // Go through each of the variables in the symbol table and
  //  determine which ones are input and output fifos.
  SymbolTable* symTab = procDef->get_symbol_table() ;
  for (int i = 0 ; i < symTab->get_symbol_table_object_count() ; ++i)
  {
    SymbolTableObject* nextObj = symTab->get_symbol_table_object(i) ;
    VariableSymbol* nextVarSym = dynamic_cast<VariableSymbol*>(nextObj) ;
    if (nextVarSym == NULL)
    {
      continue ;
    }
    if (nextVarSym->lookup_annote_by_name("InputFifo") != NULL)
    {
      BrickAnnote* inputFifoBrick = dynamic_cast<BrickAnnote*>
	(nextVarSym->lookup_annote_by_name("InputFifo")) ;
      // We also need the dimensionality
      BrickAnnote* dimAnnote = dynamic_cast<BrickAnnote*>
	(nextVarSym->lookup_annote_by_name("DimensionAnnote")) ;
      assert(inputFifoBrick != NULL) ;
      assert(dimAnnote != NULL) ;
      IntegerBrick* dimValue = to<IntegerBrick>(dimAnnote->get_brick(0)) ;
      assert(dimValue != NULL) ;
      int dimensionality = dimValue->get_value().c_int() ;
      list<VariableSymbol*> alreadyUsedIndicies ;
      for (int j = 0 ; j < dimensionality ; ++j)
      {
	SuifObjectBrick* indexBrick = 
	  dynamic_cast<SuifObjectBrick*>(inputFifoBrick->get_brick(j)) ;
	assert(indexBrick != NULL) ;
	VariableSymbol* indexVariable = 
	  dynamic_cast<VariableSymbol*>(indexBrick->get_object()) ;
	assert(indexVariable != NULL) ;
	if (!IsIndex(indexVariable))
	{
	  std::cerr << "ERROR: " ;
	  std::cerr << "Indexing an array off of a non loop induction "
		    << "variable!" << std::endl ;
	  assert(0) ;
	}
	if (InList(alreadyUsedIndicies, indexVariable))
	{
	  std::cerr << "Error: " ;
	  std::cerr << "Cannot index a multidimensional array with the same "
		    << "index in multiple dimensions!" 
		    << std::endl ;
	  assert(0) ;
	}
	alreadyUsedIndicies.push_back(indexVariable) ;
      }
    }
    else if (nextVarSym->lookup_annote_by_name("OutputFifo") != NULL)
    {
      BrickAnnote* outputFifoBrick = dynamic_cast<BrickAnnote*>
	(nextVarSym->lookup_annote_by_name("OutputFifo")) ;
      // We also need the dimensionality
      BrickAnnote* dimAnnote = dynamic_cast<BrickAnnote*>
	(nextVarSym->lookup_annote_by_name("DimensionAnnote")) ;
      assert(outputFifoBrick != NULL) ;
      assert(dimAnnote != NULL) ;
      IntegerBrick* dimValue = to<IntegerBrick>(dimAnnote->get_brick(0)) ;
      assert(dimValue != NULL) ;
      int dimensionality = dimValue->get_value().c_int() ;
      list<VariableSymbol*> alreadyUsedIndicies ;
      for (int j = 0 ; j < dimensionality ; ++j)
      {
	SuifObjectBrick* indexBrick = 
	  dynamic_cast<SuifObjectBrick*>(outputFifoBrick->get_brick(j)) ;
	assert(indexBrick != NULL) ;
	VariableSymbol* indexVariable = 
	  dynamic_cast<VariableSymbol*>(indexBrick->get_object()) ;
	assert(indexVariable != NULL) ;
	if (!IsIndex(indexVariable))
	{
	  std::cerr << "ERROR: " ;
	  std::cerr << "Indexing an array off of a non loop induction "
		    << "variable!" << std::endl ;
	  assert(0) ;
	}
	if (InList(alreadyUsedIndicies, indexVariable))
	{
	  std::cerr << "Error: " ;
	  std::cerr << "Cannot index a multidimensional array with the same "
		    << "index in multiple dimensions!" 
		    << std::endl ;
	  assert(0) ;
	}
	alreadyUsedIndicies.push_back(indexVariable) ;
      }
    }
  }

}

void VerifyPass::ClearIndicies()
{
  // Don't delete these pointers, but clear out the list
  while (allIndicies.size() > 0)
  {
    allIndicies.pop_front() ;
  }
}

void VerifyPass::CollectIndicies()
{
  list<CForStatement*>* allFors = 
    collect_objects<CForStatement>(procDef->get_body()) ;
  list<CForStatement*>::iterator forIter = allFors->begin() ;
  while (forIter != allFors->end())
  {
    Statement* before = (*forIter)->get_before() ;
    assert(before != NULL) ;
    StoreVariableStatement* storeBefore = 
      dynamic_cast<StoreVariableStatement*>(before) ;
    assert(storeBefore != NULL) ;
    allIndicies.push_back(storeBefore->get_destination()) ;
    ++forIter ;
  }
  delete allFors ;  
}

bool VerifyPass::IsIndex(VariableSymbol* v)
{
  list<VariableSymbol*>::iterator indexIter = allIndicies.begin() ;
  while (indexIter != allIndicies.end())
  {
    if ((*indexIter)->get_name() == v->get_name())
    {
      return true ;
    }
    ++indexIter ;
  }
  return false ;
}

int VerifyPass::DetermineType()
{
  assert(procDef != NULL) ;
  
  Annote* functionAnnote = procDef->lookup_annote_by_name("FunctionType") ;
  BrickAnnote* functionBrickAnnote = 
    dynamic_cast<BrickAnnote*>(functionAnnote) ;
  assert(functionBrickAnnote != NULL) ;
  IntegerBrick* valueBrick = 
    dynamic_cast<IntegerBrick*>(functionBrickAnnote->get_brick(0)) ;
  assert(valueBrick != NULL) ;

  return valueBrick->get_value().c_int() ;
}

// This function will make sure no input variable is written to.
//  This only works with modules as it checks for field accesses.
void VerifyPass::VerifyModuleDestinations()
{
  assert(procDef != NULL) ;

  list<StoreStatement*>* allStores = 
    collect_objects<StoreStatement>(procDef->get_body()) ;

  list<StoreStatement*>::iterator storeIter = allStores->begin() ;
  while (storeIter != allStores->end())
  {
    Expression* destAddr = (*storeIter)->get_destination_address() ;

    if (dynamic_cast<FieldAccessExpression*>(destAddr) != NULL)
    {
      Symbol* curSym = dynamic_cast<FieldAccessExpression*>(destAddr)->
	get_field() ;

      LString currentName = curSym->get_name() ;

      // Check that this is not an input by looking at the last three
      //  characters to see if they are "_in"

      std::string curName = currentName.c_str() ;
      if (curName.find("_in", curName.size() - 3) != std::string::npos) 
      {
	std::cerr << "ERROR: " ;
	std::cerr << "You cannot write to the variable: " << currentName 
		  << "!" << std::endl ;
	assert(0) ;
      }
    }
    ++storeIter ;
  }
  delete allStores ;

  list<CallStatement*>* allCalls = 
    collect_objects<CallStatement>(procDef->get_body()) ;
  list<CallStatement*>::iterator callIter = allCalls->begin() ;
  while (callIter != allCalls->end())
  {
    list<VariableSymbol*> writtenVariables = AllDefinedVariables(*callIter) ;
    list<VariableSymbol*>::iterator varIter = writtenVariables.begin() ;
    while (varIter != writtenVariables.end())
    {
      // Check that this is not an input by looking at the last three
      //  characters to see if they are "_in"

      std::string curName = (*varIter)->get_name().c_str()  ;
      if (curName.find("_in", curName.size() - 3) != std::string::npos) 
      {
	std::cerr << "ERROR: " ;
	std::cerr << "You cannot write to the variable: " 
		  << (*varIter)->get_name()
		  << "!" << std::endl ;
	assert(0) ;
      }      
      ++varIter ;
    }
    ++callIter ;
  }
  delete allCalls ;

}

// This verifies that we are never reading from an output variable.
void VerifyPass::VerifyModuleSources()
{
  assert(procDef != NULL) ;

  // This picks up all of the field expressions that are embedded in 
  //  binary expressions and the like.

  list<LoadExpression*>* allLoads = 
    collect_objects<LoadExpression>(procDef->get_body()) ;
  list<LoadExpression*>::iterator loadIter = allLoads->begin() ;
  while (loadIter != allLoads->end())
  {    
    Expression* sourceAddress = (*loadIter)->get_source_address() ;
    if (dynamic_cast<FieldAccessExpression*>(sourceAddress) != NULL)
    {
      Symbol* curSym = dynamic_cast<FieldAccessExpression*>(sourceAddress)->
	get_field() ;

      LString currentName = curSym->get_name() ;

      if (strstr(currentName.c_str(), "_out") != NULL)
      {
	std::cerr << "ERROR: " ;
	std::cerr << "You cannot read from the variable: " << currentName 
		  << "!" << std::endl ;
	assert(0) ;
      }      
    }
    
    ++loadIter ;
  }
  
  delete allLoads ;

  // Collect all of the ones in hanging out by themselves
  list<StoreVariableStatement*>* allStoreVars = 
    collect_objects<StoreVariableStatement>(procDef->get_body()) ;
  list<StoreVariableStatement*>::iterator storeVarIter = allStoreVars->begin();
  while (storeVarIter != allStoreVars->end())
  {
    Expression* value = (*storeVarIter)->get_value() ;
    if (dynamic_cast<FieldAccessExpression*>(value) != NULL)
    {
      Symbol* curSym =dynamic_cast<FieldAccessExpression*>(value)->get_field();

      LString currentName = curSym->get_name() ;

      if (strstr(currentName.c_str(), "_out") != NULL)
      {
	std::cerr << "ERROR: " ;
	std::cerr << "You cannot read from the variable: " << currentName 
		  << "!" << std::endl ;
	assert(0) ;
      }      
    }
    ++storeVarIter ;
  }
  delete allStoreVars ;

  list<StoreStatement*>* allStores = 
    collect_objects<StoreStatement>(procDef->get_body()) ;
  list<StoreStatement*>::iterator storeIter = allStores->begin() ;
  while (storeIter != allStores->end())
  {
    Expression* value = (*storeIter)->get_value() ;
    if (dynamic_cast<FieldAccessExpression*>(value) != NULL)
    {
      Symbol* curSym =dynamic_cast<FieldAccessExpression*>(value)->get_field();

      LString currentName = curSym->get_name() ;

      if (strstr(currentName.c_str(), "_out") != NULL)
      {
	std::cerr << "ERROR: " ;
	std::cerr << "You cannot read from the variable: " << currentName 
		  << "!" << std::endl ;
	assert(0) ;
      }      
    }
    ++storeIter ;
  }
  delete allStores ;
}

void VerifyPass::VerifyModuleLoop()
{
  if (HasCForLoop())
  {
    std::cerr << "ERROR: " ;
    std::cerr << "Modules currently may not have a loop unless it is fully "
	      << "unrolled!" << std::endl ;
    assert(0) ;
  }
}

void VerifyPass::VerifyModuleNames()
{
  // Collect all of the names in the main function
  assert(procDef != NULL) ;
  list<LString> mainNames ;
  SymbolTable* symTab = procDef->get_symbol_table() ;

  for (int i = 0 ; i < symTab->get_symbol_table_object_count() ; ++i)
  {
    mainNames.push_back(symTab->get_symbol_table_object(i)->get_name()) ;
  }

  // Now get the struct and make sure none of the names are in the 
  //  list of variables we have defined in the main body
  ProcedureSymbol* procSym = procDef->get_procedure_symbol() ;
  ProcedureType* procType = procSym->get_type() ;
  CProcedureType* cProcType = dynamic_cast<CProcedureType*>(procType) ;

  StructType* moduleType =
    dynamic_cast<StructType*>(cProcType->get_argument(0)->get_base_type()) ;
  assert(moduleType != NULL) ;
  
  SymbolTable* moduleSymTab = moduleType->get_group_symbol_table() ;
  for (int i = 0 ; i < moduleSymTab->get_symbol_table_object_count() ; ++i)
  {
    if (InList(mainNames, 
	       moduleSymTab->get_symbol_table_object(i)->get_name()))
    {
      OutputError("ERROR:") ;
      OutputError("You cannot have a variable in the interface with "
		  "the same name as a variable in the implementation!") ;
      assert(0) ;
    }
  }
}

void VerifyPass::VerifyModuleOutputWrites()
{
  assert(procDef != NULL) ;
  
  // For modules, we need to check all of the store statements
  //  that go to field access expressions.
  list<StoreStatement*>* allStores = 
    collect_objects<StoreStatement>(procDef->get_body()) ;
  list<StoreStatement*>::iterator storeIter = allStores->begin() ;
  while (storeIter != allStores->end())
  {
    FieldAccessExpression* currentField =
      dynamic_cast<FieldAccessExpression*>((*storeIter)->get_destination_address()) ;    
    if (currentField != NULL)
    {
      FieldSymbol* currentDest = currentField->get_field() ;
      if (currentDest->lookup_annote_by_name("AlreadyWrittenTo") != NULL)
      {
	std::string errMsg = "We do not support writing to the same output (" ;
	errMsg += currentDest->get_name().c_str() ;
	errMsg += ") multiple times!" ;
	OutputError(errMsg.c_str()) ;
	assert(0) ;
      }
      else
      {
	currentDest->append_annote(create_brick_annote(env, 
						       "AlreadyWrittenTo")) ;
      }
    }
    ++storeIter ;
  }
  delete allStores ;

  // Perform the same check on call statements just in case boolean selects
  //  are used.
  list<CallStatement*>* allCalls = 
    collect_objects<CallStatement>(procDef->get_body()) ;
  list<CallStatement*>::iterator callIter = allCalls->begin() ;
  while (callIter != allCalls->end())
  {
    FieldSymbol* currentDest = 
      dynamic_cast<FieldSymbol*>((*callIter)->get_destination()) ;
    if (currentDest != NULL) 
    {
      if (currentDest->lookup_annote_by_name("AlreadyWrittenTo") != NULL)
      {
	std::string errMsg = "We do not support writing to the same output (" ;
	errMsg += currentDest->get_name().c_str() ;
	errMsg += ") multiple times!" ;
	OutputError(errMsg.c_str()) ;
	assert(0) ;
      }
      else
      {
	currentDest->append_annote(create_brick_annote(env, 
						       "AlreadyWrittenTo")) ;
      }
    }
    ++callIter ;
  }
  delete allCalls ;
}

void VerifyPass::VerifyModuleFeedback() 
{
  assert(procDef != NULL) ;
  // Modules should not have any ROCCC summations
  if (procDef->lookup_annote_by_name("TransformedModule") != NULL)
  {
    // Chack all variables
    SymbolTable* symTab = procDef->get_symbol_table() ;
    for (int i = 0 ; i < symTab->get_symbol_table_object_count() ; ++i)
    {
      SymbolTableObject* currentObj = symTab->get_symbol_table_object(i) ;
      if (currentObj->lookup_annote_by_name("FeedbackVariable") != NULL)
      {
	OutputError("Error: You cannot have any feedback variables in modules!"
		    "  This includes summations!");
	assert(0) ;
      }
    }

    list<CallStatement*>* allCalls = 
      collect_objects<CallStatement>(procDef->get_body()) ;
    list<CallStatement*>::iterator callIter = allCalls->begin() ;
    while (callIter != allCalls->end())
    {
      SymbolAddressExpression* procSymExpr =
	dynamic_cast<SymbolAddressExpression*>((*callIter)->get_callee_address()) ;
      assert(procSymExpr != NULL) ;
      ProcedureSymbol* procSym = 
	dynamic_cast<ProcedureSymbol*>(procSymExpr->get_addressed_symbol()) ;
      assert(procSym != NULL) ;
      std::string procName = procSym->get_name().c_str() ;
      if (procName.find("ROCCCSummation") != std::string::npos)
      {
	OutputError("Error: You cannot have any feedback variables in modules!"
		    "  This includes summations!");
	assert(0) ;
      }
      ++callIter ;
    }
    delete allCalls ;
  }
}

void VerifyPass::VerifySystemOutputWrites()
{
  assert(procDef != NULL) ;
  StatementList* innermost = InnermostList(procDef) ;
  assert(innermost != NULL) ;
  
  // Check all of the store variables
  list<StoreVariableStatement*>* allStoreVars = 
    collect_objects<StoreVariableStatement>(innermost) ;
  list<StoreVariableStatement*>::iterator storeVarIter = allStoreVars->begin();
  while (storeVarIter != allStoreVars->end())
  {
    VariableSymbol* currentDest = (*storeVarIter)->get_destination() ;

    if (currentDest->lookup_annote_by_name("ScalarReplacedVariable") != NULL)
    {
      if (currentDest->lookup_annote_by_name("AlreadyWrittenTo") != NULL)
      {
	std::string errMsg = "We do not support writing to the same output (" ;
	errMsg += currentDest->get_name().c_str() ;
	errMsg += ") multiple times!" ;
	OutputError(errMsg.c_str()) ;
	assert(0) ;
      }
      else
      {
	currentDest->append_annote(create_brick_annote(env, 
						       "AlreadyWrittenTo")) ;
      }
    }
    ++storeVarIter ;
  }
  delete allStoreVars ;
}

void VerifyPass::VerifyInputOutputUniqueness()
{
  // Collect all input and output fifos.
  SymbolTable* symTab = procDef->get_symbol_table() ;
  assert(symTab != NULL) ;

  list<VariableSymbol*> allFifos ;

  for (int i = 0 ; i < symTab->get_symbol_table_object_count() ; ++i)
  {
    SymbolTableObject* currentObj = symTab->get_symbol_table_object(i) ;
    if (currentObj->lookup_annote_by_name("InputFifo") != NULL ||
	currentObj->lookup_annote_by_name("OutputFifo") != NULL)
    {
      VariableSymbol* toInsert = dynamic_cast<VariableSymbol*>(currentObj) ;
      assert(toInsert != NULL) ;
      allFifos.push_back(toInsert) ;
    }
  }

  list<VariableSymbol*> usedVars ;
  
  // Now, go through all of the scalar replaced variables and make sure
  //  they aren't used more than once
  list<VariableSymbol*>::iterator fifoIter = allFifos.begin() ;
  while (fifoIter != allFifos.end())
  {
    // For each fifo, determine the dimensionality
    int dimensionality = GetDimensionality(*fifoIter) ;
    assert(dimensionality > 0) ;

    Annote* indexAnnote = (*fifoIter)->lookup_annote_by_name("IndexAnnote") ;
    BrickAnnote* indexBrick = dynamic_cast<BrickAnnote*>(indexAnnote) ;
    assert(indexBrick != NULL) ;
    for (int i = 0 ; i < indexBrick->get_brick_count() ; i+=1 + dimensionality)
    {
      SuifObjectBrick* scalarReplacedBrick = 
	dynamic_cast<SuifObjectBrick*>(indexBrick->get_brick(i)) ;
      assert(scalarReplacedBrick != NULL) ;
      VariableSymbol* scalarReplacedVar =
	dynamic_cast<VariableSymbol*>(scalarReplacedBrick->get_object()) ;
      assert(scalarReplacedVar != NULL) ;
      
      if (InList(usedVars, scalarReplacedVar))
      {
	OutputError("Cannot both read from and write to the same location!") ;
	assert(0) ;
      }
      else
      {
	usedVars.push_back(scalarReplacedVar) ;
      }
      
    }

    ++fifoIter ;
  }

}

// This really should be templatized
bool VerifyPass::InList(list<LString>& names, LString toCheck)
{
  list<LString>::iterator nameIter = names.begin() ;
  while (nameIter != names.end())
  {
    if ((*nameIter) == toCheck)
    {
      return true ;
    }
    ++nameIter ;
  }
  return false ;
}

bool VerifyPass::InList(list<VariableSymbol*>& vars, VariableSymbol* v)
{
  list<VariableSymbol*>::iterator varIter = vars.begin() ;
  while (varIter != vars.end())
  {
    if ((*varIter) == v)
    {
      return true ;
    }
    ++varIter ;
  }
  return false ;
}

bool VerifyPass::HasCForLoop()
{
  assert(procDef != NULL) ;

  Iter<CForStatement> forIter = 
    object_iterator<CForStatement>(procDef->get_body()) ;
  
  return forIter.is_valid() ;
}

bool VerifyPass::OnlyOneCForLoopNest()
{
  // There should be at maximum one loop nest, but maybe more than one
  //  loop.  

  assert(procDef != NULL) ;
  list<CForStatement*>* allFors = 
    collect_objects<CForStatement>(procDef->get_body()) ;
  assert(allFors != NULL) ;

  if (allFors->size() == 0)
  {
    delete allFors ;
    return false ;
  }

  // Find the innermost loop, and count backwards to see how many loops
  //  are contained in that nest.  If it doesn't match the total number
  //  of for loops then there must be more than one nest.
  CForStatement* innermost = InnermostLoop(procDef) ;
  assert(innermost != NULL) ;
  SuifObject* parent = innermost->get_parent() ;
  int numLoops = 1 ;
  int totalNumLoops = allFors->size() ;
  while (parent != NULL)
  {
    if (dynamic_cast<CForStatement*>(parent) != NULL)
    {
      ++numLoops ;
    }
    parent = parent->get_parent() ;
  }
  
  delete allFors ;
  return numLoops == totalNumLoops ;
}

bool VerifyPass::ComparisonCorrect()
{
  assert(procDef != NULL) ;

  bool valid = true ;
  
  list<CForStatement*>* allFors = 
    collect_objects<CForStatement>(procDef->get_body()) ;
  assert(allFors != NULL) ;

  list<CForStatement*>::iterator forIter = allFors->begin() ;
  while (forIter != allFors->end())
  {
    Expression* test = (*forIter)->get_test() ;

    if (dynamic_cast<BinaryExpression*>(test) != NULL)
    {
      BinaryExpression* binaryTest = dynamic_cast<BinaryExpression*>(test) ;
      LString opcode = binaryTest->get_opcode() ;
      if (opcode != LString("is_less_than") &&
	  opcode != LString("is_less_than_or_equal_to"))
      {
	valid = false ;
      }
    }
    else if (dynamic_cast<IntConstant*>(test) != NULL)
    {
      int value = dynamic_cast<IntConstant*>(test)->get_value().c_int() ;
      if (value != 1 || forIter != allFors->begin())
      {	
	valid = false ;
      }
    }
    else
    {
      valid = false ;
    }
    
    ++forIter ;
  }
  
  delete allFors ;
  return valid ;
}

bool VerifyPass::PerfectlyNestedLoops()
{
  list<CForStatement*>* allFors = 
    collect_objects<CForStatement>(procDef->get_body()) ;
  list<CForStatement*>::iterator forIter = allFors->begin() ;
  while (forIter != allFors->end())
  {
    if (!IsInnermostLoop(*forIter))
    {
      Statement* inner = (*forIter)->get_body() ;
      CForStatement* innerLoop = dynamic_cast<CForStatement*>(inner) ;
      StatementList* innerList = dynamic_cast<StatementList*>(inner) ;
      if (innerList != NULL)
      {
	innerLoop = dynamic_cast<CForStatement*>(innerList->get_statement(0)) ;
      }
      if (innerLoop == NULL)
      {
	return false ;
      }
    }
    ++forIter ;
  }

  delete allFors ;
  return true ;
}
