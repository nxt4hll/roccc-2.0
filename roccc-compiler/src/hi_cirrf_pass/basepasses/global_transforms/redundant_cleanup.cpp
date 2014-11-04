
#include <cassert>

#include <suifkernel/utilities.h>
#include <basicnodes/basic.h>
#include <basicnodes/basic_factory.h>

#include "roccc_utils/warning_utils.h"
#include "roccc_utils/roccc2.0_utils.h"

#include "redundant_cleanup.h"

CleanupRedundantVotes::CleanupRedundantVotes(SuifEnv* pEnv) : 
  PipelinablePass(pEnv, "CleanupRedundantVotes")
{
  theEnv = pEnv ;
  procDef = NULL ;
}

void CleanupRedundantVotes::do_procedure_definition(ProcedureDefinition* proc_def)
{
  procDef = proc_def ;
  assert(procDef != NULL) ;
  OutputInformation("Cleanup Redundant Votes Begins") ;
  
  list<CallStatement*>* allCalls = 
    collect_objects<CallStatement>(procDef->get_body()) ;
  list<CallStatement*>::iterator callIter = allCalls->begin() ;
  while (callIter != allCalls->end())
  {
    ProcessCall(*callIter) ;
    ++callIter ;
  }
  delete allCalls ;

  OutputInformation("Cleanup Redundant Votes Ends") ;
}

void CleanupRedundantVotes::ProcessCall(CallStatement* c)
{
  assert(c != NULL) ;
  
  SymbolAddressExpression* symAddress = 
    dynamic_cast<SymbolAddressExpression*>(c->get_callee_address()) ;
  assert(symAddress != NULL) ;
  
  Symbol* sym = symAddress->get_addressed_symbol() ;
  assert(sym != NULL) ;

  if (sym->get_name() == LString("ROCCCTripleVote") || 
      sym->get_name() == LString("ROCCCDoubleVote") )
  {
    LoadVariableExpression* errorVariableExpression = 
      dynamic_cast<LoadVariableExpression*>(c->get_argument(0)) ;
    assert(errorVariableExpression != NULL) ;
    VariableSymbol* currentError = errorVariableExpression->get_source() ;
    assert(currentError != NULL) ;
    if (InList(currentError))
    {
      // Create a new variable
      VariableSymbol* errorDupe = 
	create_variable_symbol(theEnv,
			       currentError->get_type(),
			       TempName(LString("UnrolledRedundantError"))) ;
      errorDupe->append_annote(create_brick_annote(theEnv, "DebugRegister")) ;
      procDef->get_symbol_table()->append_symbol_table_object(errorDupe) ;
      usedVariables.push_back(errorDupe) ;
      errorVariableExpression->set_source(errorDupe) ;
    }
    else
    {
      usedVariables.push_back(currentError) ;
    }
  }

}

bool CleanupRedundantVotes::InList(VariableSymbol* v)
{
  std::list<VariableSymbol*>::iterator varIter = usedVariables.begin() ;
  while (varIter != usedVariables.end())
  {
    if ((*varIter) == v)
    {
      return true ;
    }
    ++varIter ;
  }
  return false ;
}
