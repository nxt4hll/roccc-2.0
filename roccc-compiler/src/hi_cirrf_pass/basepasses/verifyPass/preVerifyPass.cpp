
#include <cassert>

#include <suifkernel/utilities.h>

#include "roccc_utils/warning_utils.h"
#include "roccc_utils/roccc2.0_utils.h"

#include "preVerifyPass.h"

PreVerifyPass::PreVerifyPass(SuifEnv* pEnv) : 
  PipelinablePass(pEnv, "PreVerifyPass") 
{
  theEnv = pEnv ;
  procDef = NULL ;
}

PreVerifyPass::~PreVerifyPass()
{
  ; // Nothing to delete yet
}

void PreVerifyPass::do_procedure_definition(ProcedureDefinition* p)
{
  procDef = p ;
  assert(procDef != NULL) ;

  OutputInformation("PreVerify pass begins") ;
  if (EmptyFunction())
  {
    OutputError("Error: No code to translate!") ;
    assert(0) ;
  }
  if (!ProperStoreVariables())
  {
    OutputError("We do not support writes to loop indicies!") ;
    assert(0) ;
  }
  if (!ProperLoopSteps())
  {
    OutputError("We only support loop increments that are a positive constant "
		"and only based on the loop induction variable") ;
    assert(0) ;
  }

  OutputInformation("PreVerify pass ends") ;
}

bool PreVerifyPass::EmptyFunction()
{
  assert(procDef != NULL) ;
  StatementList* innermost = InnermostList(procDef) ;
  if (innermost == NULL ||
      innermost->get_statement_count() == 0)
  {
    return true ;
  }
  
  return false ;
}

bool PreVerifyPass::ProperStoreVariables()
{
  assert(procDef != NULL) ;
  // First, collect all indicies
  CollectIndicies() ;

  StatementList* innermost = InnermostList(procDef) ;
  assert(innermost != NULL) ;
  list<StoreVariableStatement*>* allStoreVars = 
    collect_objects<StoreVariableStatement>(innermost) ;
  list<StoreVariableStatement*>::iterator storeIter = allStoreVars->begin() ;
  while (storeIter != allStoreVars->end())
  {
    if (IsIndex((*storeIter)->get_destination()))
    {
      delete allStoreVars ;
      return false ;
    }
    ++storeIter ;
  }
  delete allStoreVars ;

  return true ;
}

void PreVerifyPass::CollectIndicies()
{
  assert(procDef != NULL) ;
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

bool PreVerifyPass::IsIndex(VariableSymbol* v)
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

bool PreVerifyPass::ProperLoopSteps()
{
  assert(procDef != NULL) ;
  list<CForStatement*>* allFors = 
    collect_objects<CForStatement>(procDef->get_body()) ;
  list<CForStatement*>::iterator forIter = allFors->begin() ;
  while (forIter != allFors->end())
  {
    Statement* step = (*forIter)->get_step() ;
    assert(step != NULL) ;
    StoreVariableStatement* storeStep =
      dynamic_cast<StoreVariableStatement*>(step) ;
    assert(storeStep != NULL) ;
    BinaryExpression* value = 
      dynamic_cast<BinaryExpression*>(storeStep->get_value()) ;
    assert(value != NULL) ;
    if (value->get_opcode() != LString("add"))
    {
      delete allFors ;
      return false ;
    }
    // Check that one of the binary expression's values is an integer constant
    if (dynamic_cast<Constant*>(value->get_source1()) == NULL &&
	dynamic_cast<Constant*>(value->get_source2()) == NULL)
    {
      delete allFors ;
      return false ;
    }
    // Check that the variable being modified is the same as the variable 
    //  set in the before.
    Statement* before = (*forIter)->get_before() ;
    StoreVariableStatement* beforeStore = 
      dynamic_cast<StoreVariableStatement*>(before) ;
    assert(beforeStore != NULL) ;
    if (beforeStore->get_destination() != storeStep->get_destination())
    {
      delete allFors ;
      return false ;
    }

    ++forIter ;
  }
  delete allFors ;
  return true ;
}

