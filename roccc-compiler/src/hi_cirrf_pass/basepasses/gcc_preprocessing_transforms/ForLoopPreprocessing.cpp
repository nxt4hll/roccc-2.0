// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*
  The purpose of this pass is to undo some of the things gcc does when 
   generating code.  Namely, the for loop moves the before position outside
   the loop and we need to move it back into the SUIF structure.  
*/

#include <cassert>

#include <suifkernel/utilities.h>
#include <basicnodes/basic.h>
#include <basicnodes/basic_factory.h>

#include "ForLoopPreprocessing.h"
#include "roccc_utils/warning_utils.h"
#include "roccc_utils/roccc2.0_utils.h"

ForLoopPreprocessingPass::ForLoopPreprocessingPass(SuifEnv* pEnv) :
  PipelinablePass(pEnv, "ForLoopPreprocessingPass") 
{
  theEnv = pEnv ;
  procDef = NULL ;
}

void ForLoopPreprocessingPass::do_procedure_definition(ProcedureDefinition* proc_def)
{
  procDef = proc_def ;
  assert(procDef != NULL) ;

  OutputInformation("For Loop Preprocessing begins") ;
  
  list<CForStatement*>* allFors = 
    collect_objects<CForStatement>(procDef->get_body()) ;

  list<CForStatement*>::iterator forIter = allFors->begin() ;
  while (forIter != allFors->end())
  {
    HandleComparison(*forIter) ;
    HandleBefore(*forIter) ;

    // Move on
    ++forIter ;
  }

  delete allFors ;

  OutputInformation("For Loop Preprocessing ends") ;
}

void ForLoopPreprocessingPass::HandleBefore(CForStatement* c)
{
  // For loops should always be located inside of statement lists
  StatementList* forParent = 
    dynamic_cast<StatementList*>(c->get_parent()) ;
  assert(forParent != NULL) ;

  // Get the position of the C for statement
  int forPosition = DeterminePosition(forParent, c) ;
  assert(forPosition > 0) ; // There should always be a statement before
    
  Statement* before = forParent->get_statement(forPosition - 1) ;

  // Check the current "before."  If it is a blank statement list
  //  then we need to make this transformation.
  Statement* currentBefore = c->get_before() ;
  StatementList* blankList = dynamic_cast<StatementList*>(currentBefore) ;
  if (blankList != NULL && blankList->get_statement_count() == 0)
  {
    // Replace the original statement that was before with a blank
    //  statement list.  This will be removed in the next pass when
    //  we flatten statement lists.
    forParent->replace(before, create_statement_list(theEnv)) ;
    c->set_before(before) ;
  }
}

void ForLoopPreprocessingPass::HandleComparison(CForStatement* c)
{
  Expression* test = c->get_test() ;
  BinaryExpression* binTest = dynamic_cast<BinaryExpression*>(test) ;
  IntConstant* testConstant = dynamic_cast<IntConstant*>(test) ;
  if (testConstant != NULL)
  {
    return ;
  }
  assert(binTest != NULL && "Improper test") ;
  IntConstant* rightInt = dynamic_cast<IntConstant*>(binTest->get_source2()) ;
  LoadVariableExpression* leftLoadVar = 
    dynamic_cast<LoadVariableExpression*>(binTest->get_source1()) ;
  VariableSymbol* leftVar = NULL ;
  IntegerType* leftType = NULL ;
  
  if (leftLoadVar != NULL)
  {
    leftVar = leftLoadVar->get_source() ;
    leftType = 
      dynamic_cast<IntegerType*>(leftVar->get_type()->get_base_type());
  }
  
  LString opcode = binTest->get_opcode() ;
  if (opcode == LString("is_equal_to") && rightInt != NULL && 
      rightInt->get_value().c_int() == 0 &&
      leftVar != NULL && leftType != NULL && !leftType->get_is_signed())
  {
    // Replace with a < 1
    binTest->set_opcode(LString("is_less_than")) ;
    rightInt->set_value(IInteger(1)) ;
  }
}

