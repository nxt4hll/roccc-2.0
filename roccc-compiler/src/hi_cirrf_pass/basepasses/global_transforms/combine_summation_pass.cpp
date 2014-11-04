#include <cassert>
#include <map>

#include <suifnodes/suif.h>
#include <suifnodes/suif_factory.h>

#include "roccc_utils/warning_utils.h"
#include "roccc_utils/roccc2.0_utils.h"

#include "combine_summation_pass.h"

CombineSummationPass::CombineSummationPass(SuifEnv* pEnv) :
  PipelinablePass(pEnv, "CombineSummationPass")
{
  theEnv = pEnv ;
  procDef = NULL ;
}

CombineSummationPass::~CombineSummationPass()
{
  ; // Nothing to delete yet
}

void CombineSummationPass::do_procedure_definition(ProcedureDefinition* p)
{
  procDef = p ;
  assert(procDef != NULL) ;

  OutputInformation("Combine summation pass begins") ;

  StatementList* innermost = InnermostList(procDef) ;
  assert(innermost != NULL) ;

  bool change = false ;
  do
  {
    // Find the first summation
    StoreVariableStatement* firstStatement = NULL ;
    StoreVariableStatement* secondStatement = NULL ;
    change = false ;
    int i ;
    int firstStatementPosition = -1 ;
    i = 0 ;
    while (i < innermost->get_statement_count())
    {
      StoreVariableStatement* currentStoreVariable =
	dynamic_cast<StoreVariableStatement*>(innermost->get_statement(i)) ;
      if (currentStoreVariable != NULL && IsSummation(currentStoreVariable))
      {
	firstStatement = currentStoreVariable ;
	firstStatementPosition = i ;
	break ;
      }
	++i ;
    }
    
    if (firstStatement != NULL)
    {
      VariableSymbol* firstDest = firstStatement->get_destination() ;
      for (int j = i+1 ; j < innermost->get_statement_count() ; ++j)
      {
	StoreVariableStatement* nextStoreVar = 
	  dynamic_cast<StoreVariableStatement*>(innermost->get_statement(j));
	if (nextStoreVar != NULL && IsSummation(nextStoreVar, firstDest))
	{
	  secondStatement = nextStoreVar ;
	  break ;
	}
	if (IsDefinition(innermost->get_statement(j), firstDest) ||
	    HasUses(innermost->get_statement(j), firstDest))
	{
	  break ;
	}						
      }
    }
    if (secondStatement != NULL)
    {
      // Go through each of the variables used in the first statement and
      //  make sure there are no definitions to any of them.
      //  I only have to worry about variables and not array accesses because
      //  we don't allow them to read and write to array values.
      int originalPosition = DeterminePosition(innermost, firstStatement) ;
      assert(originalPosition >= 0) ;
      list<VariableSymbol*> usedVars = 
	AllUsedVariablesBut(firstStatement, firstStatement->get_destination());
      bool goodPath = true ;
      for (int j = originalPosition ; 
	   j < innermost->get_statement_count() && 
	     innermost->get_statement(j) != secondStatement ; 
	   ++j)
      {
	list<VariableSymbol*>::iterator usedIter = usedVars.begin() ;
	while (usedIter != usedVars.end())
	{
	  if (IsOutputVariable((*usedIter), innermost->get_statement(j)))
	  {
	    goodPath = false ;
	    break ;
	  }
	  ++usedIter ;
	}
	if (!goodPath) 
	{
	  break ;
	}
      }
      if (!goodPath)
      {
	continue ;
      }
      // Actually do the combining here
      change = true ;
      Expression* remains = RemoveValue(firstStatement) ;
      Expression* secondRemains = RemoveValue(secondStatement) ;
      // Create two binary expressions
      BinaryExpression* remainsSum = 
	create_binary_expression(theEnv,
				 remains->get_result_type(),
				 LString("add"),
				 remains,
				 secondRemains) ;
      LoadVariableExpression* loadDest = 
	create_load_variable_expression(theEnv,
	    secondStatement->get_destination()->get_type()->get_base_type(),
					secondStatement->get_destination()) ;
      BinaryExpression* finalSum =
	create_binary_expression(theEnv,
				 remainsSum->get_result_type(),
				 LString("add"),
				 remainsSum,
				 loadDest) ;

      secondStatement->set_value(finalSum) ;
      // Delete?
      innermost->remove_statement(firstStatementPosition) ;
    }
     
  } while (change == true) ;  

  OutputInformation("Combine summation pass ends") ;
}

bool CombineSummationPass::IsSummation(StoreVariableStatement* s)
{
  assert(s != NULL) ;
  return IsSummation(s->get_value(), s->get_destination()) ;
}

bool CombineSummationPass::IsSummation(StoreVariableStatement* s, 
				       VariableSymbol* v)
{
  assert(s != NULL) ;
  assert(v != NULL) ;
  return (s->get_destination() == v) && IsSummation(s->get_value(), v) ;
}

bool CombineSummationPass::IsSummation(Expression* e, VariableSymbol* v)
{
  assert(e != NULL) ;
  assert(v != NULL) ;

  BinaryExpression* binExp = dynamic_cast<BinaryExpression*>(e) ;
  if (binExp == NULL)
  {
    return false ;
  }

  if (binExp->get_opcode() != LString("add"))
  {
    return false ;
  }

  Expression* leftSide = binExp->get_source1() ;
  Expression* rightSide = binExp->get_source2() ;

  LoadVariableExpression* leftLoadVar = 
    dynamic_cast<LoadVariableExpression*>(leftSide) ;
  LoadVariableExpression* rightLoadVar = 
    dynamic_cast<LoadVariableExpression*>(rightSide) ;

  if (leftLoadVar != NULL && leftLoadVar->get_source() == v)
  {
    return true ;
  }
  
  if (rightLoadVar != NULL && rightLoadVar->get_source() == v)
  {
    return true ;
  }

  return IsSummation(leftSide, v) || IsSummation(rightSide, v) ;
}

bool CombineSummationPass::HasIntermediateUse(StatementList* s,
					      VariableSymbol* v,
					      int startPos,
					      int endPos)
{
  assert(s != NULL) ;
  assert(v != NULL) ;
  assert(endPos < s->get_statement_count()) ;
  for (int i = startPos ; i < endPos ; ++i)
  {
    if (HasUses(s->get_statement(i), v))
    {
      return true ;
    }
  }
  return false ;
}

bool CombineSummationPass::HasIntermediateDef(StatementList* s,
					      VariableSymbol* v,
					      int startPos,
					      int endPos)
{
  assert(s != NULL) ;
  assert(v != NULL) ;
  assert(endPos < s->get_statement_count()) ;
  for (int i = startPos ; i < endPos ; ++i)
  {
    if (IsDefinition(s->get_statement(i), v))
    {
      return true ;
    }
  }
  return false ;
}

Expression* CombineSummationPass::RemoveValue(StoreVariableStatement* s)
{
  assert(s != NULL) ;
  return RemoveValue(s->get_value(), s->get_destination()) ;  
}

Expression* CombineSummationPass::RemoveValue(Expression* original,
					      VariableSymbol* v)
{
  assert(original != NULL) ;
  assert(v != NULL) ;

  BinaryExpression* binExp = dynamic_cast<BinaryExpression*>(original) ;
  if (binExp == NULL)
  {
    return NULL ;
  }

  Expression* leftSide = binExp->get_source1() ;
  Expression* rightSide = binExp->get_source2() ;

  // Check to see if we are the correct format
  LoadVariableExpression* leftVar = 
    dynamic_cast<LoadVariableExpression*>(leftSide) ;
  LoadVariableExpression* rightVar = 
    dynamic_cast<LoadVariableExpression*>(rightSide) ;
  if (leftVar != NULL && leftVar->get_source() == v)
  {
    binExp->set_source2(NULL) ;
    return rightSide ;
  }
  if (rightVar != NULL && rightVar->get_source() == v)
  {
    binExp->set_source1(NULL) ;
    return leftSide ;
  }
  // Recursively go through and find the value
  Expression* leftSideReplacement = RemoveValue(leftSide, v) ;
  if (leftSideReplacement != NULL)
  {
    binExp->set_source2(leftSideReplacement) ;
    return binExp ;
  }
  Expression* rightSideReplacement = RemoveValue(rightSide, v) ;
  if (rightSideReplacement != NULL)
  {
    binExp->set_source2(rightSideReplacement) ;
    return binExp ;
  }
  return NULL ;
}
