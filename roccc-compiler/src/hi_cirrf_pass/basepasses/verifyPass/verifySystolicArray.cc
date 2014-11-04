
#include <cassert>
#include <iostream>

#include <suifkernel/utilities.h>
#include <cfenodes/cfe.h>

#include "roccc_utils/warning_utils.h"
#include "roccc_utils/roccc2.0_utils.h"

#include "verifySystolicArray.h"

//static const int MODULE = 1 ;
//static const int SYSTEM = 2 ;
//static const int COMPOSABLE_SYSTEM = 3 ;

VerifySystolicPass::VerifySystolicPass(SuifEnv* pEnv) : 
  PipelinablePass(pEnv, "VerifySystolicPass")
{
  theEnv = pEnv ;
  procDef = NULL ;
}

VerifySystolicPass::~VerifySystolicPass()
{
  // Nothing to clean up yet...
}

void VerifySystolicPass::do_procedure_definition(ProcedureDefinition* p)
{
  procDef = p ;
  assert(procDef != NULL) ;

  OutputInformation("Verify Systolic Array Pass Begins") ;

  if (!VerifyType())
  {
    std::cerr << "ERROR: "
	      << "Systolic Array Generation can only be performed on"
	      << " system code!" << std::endl ;
    assert(0) ;
  }

  if (!VerifyLoopNest())
  {
    std::cerr << "ERROR: " 
	      << "Systolic Array Generation only works on a perfectly nested"
	      << " double loop nest!" << std::endl ;
    assert(0) ;
  }

  if (!VerifyLoopBounds())
  {
    std::cerr << "ERROR: "
	      << "Systolic Array Generation loop outer loop bound must be"
	      << " a constant!" << std::endl ;
    assert(0) ;
  }

  if (!VerifyArrays())
  {
    std::cerr << "ERROR: "
	      << "Systolic Array Generation requires a two-dimensional array"
	      << " that is both read and written from!" 
	      << std::endl ;
    assert(0) ;
  }

  OutputInformation("Verify Systolic Array Pass Ends") ;
}

bool VerifySystolicPass::VerifyType()
{
  assert(procDef != NULL) ;
  Annote* functionAnnote = procDef->lookup_annote_by_name("FunctionType") ;
  BrickAnnote* functionBrickAnnote =
    dynamic_cast<BrickAnnote*>(functionAnnote) ;
  assert(functionBrickAnnote != NULL) ;
  IntegerBrick* valueBrick = 
    dynamic_cast<IntegerBrick*>(functionBrickAnnote->get_brick(0)) ;
  assert(valueBrick != NULL) ;

  return valueBrick->get_value().c_int() == SYSTEM ;
}

bool VerifySystolicPass::VerifyLoopNest()
{
  assert(procDef != NULL) ;

  list<CForStatement*>* allFors = 
    collect_objects<CForStatement>(procDef->get_body()) ;
  if (allFors->size() != 2)
  {
    delete allFors ;
    return false ;
  }

  // The loops should be perfectly nested
  list<CForStatement*>::iterator forIter = allFors->begin() ;
  while (forIter != allFors->end())
  {
    if (!IsInnermostLoop(*forIter))
    {
      Statement* inner = (*forIter)->get_body() ;
      assert(inner != NULL) ;

      CForStatement* innerLoop = dynamic_cast<CForStatement*>(inner) ;
      StatementList* innerList = dynamic_cast<StatementList*>(inner) ;      
      if (innerList != NULL)
      {
	Statement* firstStatement = innerList->get_statement(0) ;
	if (dynamic_cast<LabelLocationStatement*>(firstStatement) != NULL)
	{
	  innerLoop = dynamic_cast<CForStatement*>(innerList->get_statement(1)) ;
	}
	else
	{
	  innerLoop = dynamic_cast<CForStatement*>(innerList->get_statement(0)) ;
	}
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

bool VerifySystolicPass::VerifyLoopBounds()
{
  assert(procDef != NULL) ;
  list<CForStatement*>* allFors = 
    collect_objects<CForStatement>(procDef->get_body()) ;
  list<CForStatement*>::iterator forIter = allFors->begin() ;
  while (forIter != allFors->end())
  {
    if (!IsInnermostLoop(*forIter))
    {
      // The boundary must be a constant
      Expression* test = (*forIter)->get_test() ;
      BinaryExpression* binTest = dynamic_cast<BinaryExpression*>(test) ;
      assert(binTest != NULL) ;
      LString opcode = binTest->get_opcode() ;
      bool correctComparison = 
	(opcode == LString("is_less_than_or_equal_to") ||
	 opcode == LString("is_less_than")) ;

      if (!correctComparison ||
	  dynamic_cast<Constant*>(binTest->get_source2()) == NULL)
      {
	delete allFors ;
	return false ;
      }
    }
    ++forIter ;
  }
  delete allFors ;
  return true ;
}

bool VerifySystolicPass::VerifyArrays()
{
  assert(procDef != NULL) ;

  // First, verify that we access one and only one two-dimensional array
  //  in the innermost loop
  CForStatement* innermost = InnermostLoop(procDef) ;
  assert(innermost != NULL) ;

  VariableSymbol* wavefrontArray= NULL ;
  list<ArrayReferenceExpression*>* allRefs = 
    collect_objects<ArrayReferenceExpression>(innermost) ;
  list<ArrayReferenceExpression*>::iterator refIter = allRefs->begin() ;
  while (refIter != allRefs->end())
  {
    if (GetDimensionality(*refIter) == 2)
    {      
      if (wavefrontArray == NULL)
      {
	wavefrontArray = GetArrayVariable(*refIter) ;
      }
      else
      {
	if (wavefrontArray != GetArrayVariable(*refIter))
	{
	  delete allRefs ;
	  return false ;
	}
      }
    }
    ++refIter ;
  }
  delete allRefs ;

  if (wavefrontArray == NULL)
  {
    return false ;
  }

  // Now check to make sure there is at least one read from this array
  //  and one write to this location.
  bool hasUse = false ;
  bool hasDefinition = false ;

  list<LoadExpression*>* allLoads = 
    collect_objects<LoadExpression>(innermost) ;
  list<LoadExpression*>::iterator loadIter = allLoads->begin() ;
  while (loadIter != allLoads->end())
  {
    Expression* sourceAddr = (*loadIter)->get_source_address() ;
    hasUse |= 
      (wavefrontArray == 
       GetArrayVariable(dynamic_cast<ArrayReferenceExpression*>(sourceAddr))) ;
    ++loadIter ;
  }
  delete allLoads ;
  
  list<StoreStatement*>* allStores = 
    collect_objects<StoreStatement>(innermost) ;
  list<StoreStatement*>::iterator storeIter = allStores->begin() ;
  while (storeIter != allStores->end())
  {
    Expression* destAddress = (*storeIter)->get_destination_address() ;
    hasDefinition |= 
      (wavefrontArray == 
       GetArrayVariable(dynamic_cast<ArrayReferenceExpression*>(destAddress)));
    ++storeIter ;
  }
  delete allStores ;
  
  return (hasUse && hasDefinition) ;
}
