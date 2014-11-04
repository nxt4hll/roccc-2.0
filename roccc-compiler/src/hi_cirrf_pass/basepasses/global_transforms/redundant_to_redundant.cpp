#include <cassert>
#include <string>

#include <suifkernel/utilities.h>

#include "roccc_utils/warning_utils.h"

#include "redundant_to_redundant.h"

RedundantToRedundantPass::RedundantToRedundantPass(SuifEnv* pEnv) :
  PipelinablePass(pEnv, "RedundantToRedundantPass")
{
  theEnv = pEnv ;
  procDef = NULL ;
}

void RedundantToRedundantPass::do_procedure_definition(ProcedureDefinition* proc_def)
{
  procDef = proc_def ;
  assert(procDef != NULL) ;
  
  OutputInformation("RedundantToRedundantPass begins") ;
  
  list<CallStatement*>* allCalls = 
    collect_objects<CallStatement>(procDef->get_body()) ;

  list<CallStatement*>::iterator callIter = allCalls->begin() ;
  while (callIter != allCalls->end())
  {    
    if ((*callIter)->lookup_annote_by_name("RedundantCall") != NULL)
    {
      ProcessCall(*callIter) ;
    }
    ++callIter ;
  }
  
  delete allCalls ;

  OutputInformation("RedundantToRedundantPass ends") ;
}

void RedundantToRedundantPass::ProcessCall(CallStatement* c)
{
  assert(c != NULL) ;
  BrickAnnote* redundantBrick = 
    dynamic_cast<BrickAnnote*>(c->lookup_annote_by_name("RedundantCall")) ;
  assert(redundantBrick != NULL) ;
  IntegerBrick* replacementBrick = 
    dynamic_cast<IntegerBrick*>(redundantBrick->get_brick(0)) ;
  assert(replacementBrick != NULL) ;

  int replacementNumber = replacementBrick->get_value().c_int() ;

  if (replacementNumber == 1)
  {
    return ; // The first one does not need to be replaced with a copy
  }
  
  // For each input, find the reaching definition and replace it with the 
  //  appropriate replacement
  
  for (int i = 0 ; i < c->get_argument_count() ; ++i)
  {
    Expression* currentArg = c->get_argument(i) ;
    LoadVariableExpression* currentLoad = 
      dynamic_cast<LoadVariableExpression*>(currentArg) ;
    if (currentLoad == NULL)
    {
      continue ;
    }
    BrickAnnote* reachingBrick = 
      dynamic_cast<BrickAnnote*>(currentLoad->lookup_annote_by_name("reaching_defs")) ;
    assert(reachingBrick != NULL) ;
    if (reachingBrick->get_brick_count() == 0)
    {
      continue ;
    }
    SuifBrick* brick = reachingBrick->get_brick(0) ;
    SuifObjectBrick* sob = dynamic_cast<SuifObjectBrick*>(brick) ;
    assert(sob != NULL) ;
    Statement* definition = dynamic_cast<Statement*>(sob->get_object()) ;
    assert(definition != NULL) ;
    CallStatement* voteCall = dynamic_cast<CallStatement*>(definition) ;
    if (voteCall == NULL)
    {
      continue ;
    }
    SymbolAddressExpression* voteAddress = 
      dynamic_cast<SymbolAddressExpression*>(voteCall->get_callee_address()) ;
    assert(voteAddress != NULL) ;
    LString voteName = voteAddress->get_addressed_symbol()->get_name() ;
    std::string stdVoteName = voteName.c_str() ;
    if (stdVoteName.find("ROCCCTripleVote") != std::string::npos)
    { 
      // Get the argument that corresponds with this replacement
      //  skipping over the first four arguments (0-3) because we know
      //  what those are.
      Expression* replacementArg = voteCall->get_argument(3+replacementNumber);
      SymbolAddressExpression* outputArg = 
	dynamic_cast<SymbolAddressExpression*>(replacementArg) ;
      assert(outputArg != NULL) ;
      
      Symbol* sym = outputArg->get_addressed_symbol() ;
      assert(sym != NULL) ;
      VariableSymbol* newSource =
	dynamic_cast<VariableSymbol*>(sym) ;
      assert(newSource != NULL) ;
      currentLoad->set_source(newSource) ;
    }
  }

}
