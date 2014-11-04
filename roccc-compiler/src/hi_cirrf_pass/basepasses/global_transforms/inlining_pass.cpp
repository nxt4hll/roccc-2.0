// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

#include <cassert>
#include <fstream>

#include <basicnodes/basic.h>
#include <basicnodes/basic_factory.h>
#include <suifnodes/suif.h>
#include <suifnodes/suif_factory.h>

#include <suifkernel/utilities.h>
#include <suifkernel/command_line_parsing.h>

#include "roccc_utils/warning_utils.h"
#include "roccc_utils/roccc2.0_utils.h"

#include "inlining_pass.h"
#include "../gcc_preprocessing_transforms/ForLoopPreprocessing.h"

InliningPass::InliningPass(SuifEnv* pEnv) : 
  PipelinablePass(pEnv, "InliningPass") 
{
  theEnv = pEnv ;
  procDef = NULL ;
}

InliningPass::~InliningPass()
{
  ; // Nothing to delete yet
}

void InliningPass::initialize()
{
  PipelinablePass::initialize() ;
  _command_line->set_description("Inlines all functions with the given name") ;
  OptionString* optionFunctionName = new OptionString("Function Name", 
						      &functionName) ;
  OptionString* optionMapFileName = new OptionString("Map file name", 
						     &mapFileName) ;
  OptionList* inliningPassArguments = new OptionList() ;
  inliningPassArguments->add(optionFunctionName) ;
  inliningPassArguments->add(optionMapFileName) ;
  _command_line->add(inliningPassArguments) ;
}

void InliningPass::do_procedure_definition(ProcedureDefinition* p)
{
  procDef = p ;
  assert(procDef != NULL) ;

  OutputInformation("Inlining Pass begins") ;

  list<CallStatement*>* allCalls = 
    collect_objects<CallStatement>(procDef->get_body()) ;
  list<CallStatement*>::iterator callIter = allCalls->begin() ;
  while (callIter != allCalls->end())
  {
    ProcessCall(*callIter) ;
    ++callIter ;
  }

  // Perform a for loop preprocessing pass to clean up any loops that were
  //  inlined
  ForLoopPreprocessingPass forPreprocess(theEnv) ;
  forPreprocess.do_procedure_definition(procDef) ;

  OutputInformation("Inlining Pass ends") ;
}

void InliningPass::ProcessCall(CallStatement* c)
{
  assert(c != NULL) ;
  SymbolAddressExpression* address = 
    dynamic_cast<SymbolAddressExpression*>(c->get_callee_address()) ;
  assert(address != NULL) ;

  ProcedureSymbol* functionVar = 
    dynamic_cast<ProcedureSymbol*>(address->get_addressed_symbol()) ;
  assert(functionVar != NULL) ;

  if (functionName != functionVar->get_name())
  {
    return ;
  }

  std::string functionFile = LookupFile(functionName) ;
  FileSetBlock* functionFSB = theEnv->read_more(functionFile.c_str()) ;
  StatementList* replacementList = NULL ; 
 
  // Now, replace the call statement with the code from the function file
  // Go through each of the statements in the list and replace all 
  //  parameters with the actual values
  list<ProcedureDefinition*>* allProcs = 
    collect_objects<ProcedureDefinition>(functionFSB) ;
  assert(allProcs->size() == 1) ;
  list<ProcedureDefinition*>::iterator procIter = allProcs->begin() ;
  while (procIter != allProcs->end())
  {
    VerifyProcedure(*procIter) ;
    CreateMappings(*procIter, c) ;
    CopyValueBlocks(*procIter) ;
    ReplaceSymbolAddresses(*procIter) ;
    ReplaceLoadVariables(*procIter) ;
    ReplaceStoreVariables(*procIter) ;
    replacementList = ConstructReplacement(*procIter) ;
    
    ++procIter ;
  }
  delete allProcs ;
  // Don't delete the inlined file set block, parts of it are in use in the
  //  main function now.
  //delete functionFSB ;
  assert(replacementList != NULL) ;
  c->get_parent()->replace(c, replacementList) ;
}

std::string InliningPass::LookupFile(LString functionName)
{
  // Open up the file that contains all of the mappings
  std::ifstream mapIn ;
  mapIn.open(mapFileName.c_str()) ;
  if (!mapIn)
  {
    assert(0 && "Cannot open Map file!") ;
  }
  // Find the suif file that corresponds to this function name
  std::string nextFunction ;
  std::string nextFileName ;
  while (!mapIn.eof())
  {
    mapIn >> std::ws >> nextFunction >> std::ws >> nextFileName ;
    if (nextFunction == std::string(functionName.c_str()))
    {
      return nextFileName ;
    }
  }
  OutputError("Error: Cannot find C code for the specified inline function!"
	      "  Please make sure the specified function was compiled"
	      " with ROCCC.") ;
  assert(0) ;
}

Statement* InliningPass::CopyStatement(Statement* s) 
{
  StoreVariableStatement* storeVar = dynamic_cast<StoreVariableStatement*>(s) ;
  if (storeVar != NULL) 
  {
    storeVar->get_value()->set_parent(NULL) ;
    return create_store_variable_statement(theEnv, 
					   storeVar->get_destination(),
					   storeVar->get_value()) ;
  }
  return NULL ;
}

void InliningPass::ClearMappings()
{
  // Don't actually delete any of the values pointed to, they are still used
  //  in the actual tree.
  parameterMapping.clear() ;
  localVarMapping.clear() ;
}

// This function is responsible for creating a mapping between the 
//  parameters that are passed in a call statement and the variables that
//  are used in the function to be inlined.  Also, we create a local 
//  variable for any variable declared inside the function to be inlined
void InliningPass::CreateMappings(ProcedureDefinition* p, CallStatement* c)
{
  assert(p != NULL) ;
  assert(c != NULL) ;

  ClearMappings() ;

  SymbolTable* symTab = p->get_symbol_table() ;
  for (int i = 0 ; i < symTab->get_symbol_table_object_count() ; ++i)
  {
    SymbolTableObject* currentObject = symTab->get_symbol_table_object(i) ;
    ParameterSymbol* currentParam =
      dynamic_cast<ParameterSymbol*>(currentObject) ;
    VariableSymbol* currentVar = 
      dynamic_cast<VariableSymbol*>(currentObject) ;
    if (currentParam != NULL)
    {
      Annote* orderAnnote = 
	currentParam->lookup_annote_by_name("ParameterOrder") ;
      BrickAnnote* orderBrickAnnote = 
	dynamic_cast<BrickAnnote*>(orderAnnote) ;
      assert(orderBrickAnnote != NULL) ;
      IntegerBrick* orderBrick = 
	dynamic_cast<IntegerBrick*>(orderBrickAnnote->get_brick(0)) ;
      assert(orderBrick != NULL) ;
      int order = orderBrick->get_value().c_int() ;
      assert(c->get_argument_count() > order) ;
      parameterMapping[dynamic_cast<ParameterSymbol*>(currentObject)] =
	c->get_argument(order) ;
    }
    else if (currentVar != NULL)
    {
      LString copyName = currentVar->get_name() ;
      copyName = copyName + "_inlined" ;
      VariableSymbol* varCopy = 
	create_variable_symbol(theEnv,
			       currentVar->get_type(),
			       TempName(copyName)) ;
      localVarMapping[currentVar] = varCopy ;
      // Also, add this variable to the local symbol table
      procDef->get_symbol_table()->append_symbol_table_object(varCopy) ;
    }
  }
}

// The purpose of this function is to make sure that any constant variables
//  or arrays are copied into the new function from the inlined function.
void InliningPass::CopyValueBlocks(ProcedureDefinition* p)
{
  assert(p != NULL) ;
  DefinitionBlock* defBlock = p->get_definition_block() ;
  assert(defBlock != NULL) ;
  while (defBlock->get_variable_definition_count() > 0)
  {
    VariableDefinition* currentDef = defBlock->remove_variable_definition(0) ;
    VariableSymbol* definedVar = currentDef->get_variable_symbol() ;
    ParameterSymbol* definedParam = dynamic_cast<ParameterSymbol*>(definedVar);
    if (definedParam != NULL)
    {
      assert(0 && "Currently not supporting parameters with default values") ;
    }
    else
    {
      assert(definedVar != NULL) ;
      VariableSymbol* mappedVar = localVarMapping[definedVar] ;
      assert(mappedVar != NULL) ;
      currentDef->set_variable_symbol(mappedVar) ;
      procDef->get_definition_block()->append_variable_definition(currentDef) ;
    }
  }

}

// What this function does is go through the inlined function and replace 
//  all instances of variable usage with expressions that reference the
//  argument that was passed into the call statement.

void InliningPass::ReplaceLoadVariables(ProcedureDefinition* p)
{
  assert(p != NULL) ;
  // The mappings have already been made
  list<LoadVariableExpression*>* allLoadVars =
    collect_objects<LoadVariableExpression>(p->get_body()) ;
  
  list<LoadVariableExpression*>::iterator loadIter = allLoadVars->begin() ;
  while (loadIter != allLoadVars->end())
  {
    VariableSymbol* currentVariable = (*loadIter)->get_source() ;
    ParameterSymbol* currentParam = 
      dynamic_cast<ParameterSymbol*>(currentVariable) ;
      
    if (currentParam != NULL)
    {
      Expression* replacementExpr = parameterMapping[currentParam] ;
      assert(replacementExpr != NULL) ;
      Expression* cloneExpr = 
	dynamic_cast<Expression*>(replacementExpr->deep_clone()) ;
      assert(cloneExpr != NULL) ;
      (*loadIter)->get_parent()->replace((*loadIter), cloneExpr) ;
    }
    else
    {
      assert(currentVariable != NULL) ;
      VariableSymbol* localCopy = localVarMapping[currentVariable] ;
      if (localCopy == NULL)
      {
	LString constructedName = currentVariable->get_name() ;
	constructedName = constructedName + "_inlined" ;
	localCopy = create_variable_symbol(theEnv, 
					   currentVariable->get_type(),
					   TempName(constructedName)) ;
	procDef->get_symbol_table()->append_symbol_table_object(localCopy) ;
      }
      assert(localCopy != NULL) ;
      (*loadIter)->set_source(localCopy) ;
    }
    ++loadIter ;
  }
  delete allLoadVars ;  
}

void InliningPass::ReplaceStoreVariables(ProcedureDefinition* p)
{
  assert(p != NULL) ;
  list<StoreVariableStatement*>* allStoreVars = 
    collect_objects<StoreVariableStatement>(p->get_body()) ;
  list<StoreVariableStatement*>::iterator storeVarIter = allStoreVars->begin();
  while (storeVarIter != allStoreVars->end())
  {
    VariableSymbol* dest = (*storeVarIter)->get_destination() ;
    ParameterSymbol* destParam = dynamic_cast<ParameterSymbol*>(dest) ;
    if (destParam != NULL)
    {
      Expression* mappedParameter = parameterMapping[destParam] ;
      LoadVariableExpression* loadParam = 
	dynamic_cast<LoadVariableExpression*>(mappedParameter) ;
      assert(loadParam != NULL && 
	     "Cannot pass an arbitrary expression to an output variable!") ;
      (*storeVarIter)->set_destination(loadParam->get_source()) ;
    }
    else
    {
      (*storeVarIter)->set_destination(localVarMapping[dest]) ;
    }
    ++storeVarIter ;
  }
  delete allStoreVars ;
}

// This should get all instances of arrays inside the inlined function
void InliningPass::ReplaceSymbolAddresses(ProcedureDefinition* p) 
{
  assert(p != NULL) ;
  list<SymbolAddressExpression*>* allSymAddr = 
    collect_objects<SymbolAddressExpression>(p->get_body()) ;
  list<SymbolAddressExpression*>::iterator symAddrIter = allSymAddr->begin() ;
  while (symAddrIter != allSymAddr->end())
  {
    Symbol* addressedSym = (*symAddrIter)->get_addressed_symbol() ;
    VariableSymbol* varSym = dynamic_cast<VariableSymbol*>(addressedSym) ;
    ParameterSymbol* parmSym = dynamic_cast<ParameterSymbol*>(addressedSym) ;
    if (parmSym != NULL)
    {
      assert(0 && "Currently you cannot inline a function that takes the address of a parameter") ;
    }
    else if (varSym != NULL) 
    {
      VariableSymbol* mapped = localVarMapping[varSym] ;
      if (mapped == NULL)
      {
	std::cout << "Could not find mapping for: " << varSym->get_name()
		  << std::endl ;
      }
      assert(mapped != NULL) ;
      (*symAddrIter)->set_addressed_symbol(mapped) ;
    }
    ++symAddrIter ;
  }
  delete allSymAddr ;
}

StatementList* InliningPass::ConstructReplacement(ProcedureDefinition* p)
{
  assert(p != NULL) ;
  StatementList* replacementList = create_statement_list(theEnv) ;

  StatementList* functionList = dynamic_cast<StatementList*>(p->get_body()) ;
  assert(functionList != NULL) ;
  while (functionList->get_statement_count() > 0)
  {
    Statement* currentStatement = functionList->remove_statement(0) ;      
    replacementList->append_statement(currentStatement) ;
  }
  return replacementList ;
}

void InliningPass::VerifyProcedure(ProcedureDefinition* p)
{
  assert(p != NULL) ;
  ProcedureSymbol* inlinedSymbol = p->get_procedure_symbol() ;
  assert(inlinedSymbol != NULL) ;
  CProcedureType* inlinedType = 
    dynamic_cast<CProcedureType*>(inlinedSymbol->get_type()) ;
  assert(inlinedType != NULL) ;
  
  // Check to make sure that the arguments are not structs
  for (int i = 0 ; i < inlinedType->get_argument_count() ; ++i)
  {
    QualifiedType* currentArgType = inlinedType->get_argument(i) ;
    DataType* baseArgType = currentArgType->get_base_type() ;
    if (dynamic_cast<StructType*>(baseArgType) != NULL)
    {
      OutputError("Inlining not supported for old module types!") ;
      assert(0) ;
    }
  }
}
