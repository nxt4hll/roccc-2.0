// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

// This unrolling algorithm no longer assumes normalized steps.

#include <cassert>
#include <sstream>

#include <utils/expression_utils.h>
#include <suifkernel/utilities.h>
#include <suifkernel/command_line_parsing.h>
#include <basicnodes/basic.h>
#include <suifnodes/suif.h>
#include <basicnodes/basic_factory.h>
#include <suifnodes/suif_factory.h>

#include "roccc_utils/warning_utils.h"
#include "roccc_utils/roccc2.0_utils.h"
#include "unroll_pass2.h"

UnrollPass2::UnrollPass2(SuifEnv *pEnv) : PipelinablePass(pEnv, "UnrollPass2") 
{
  theEnv = pEnv ;
  procDef = NULL ;

  originalStepValue = 0 ;
  index = NULL ;
  fullyUnrolled = false ;
  unrollAmount = 0 ;
}

void UnrollPass2::initialize()
{
  PipelinablePass::initialize();
  _command_line->set_description("Partially unrolls CForStatements");
  OptionString *option_loop_label = new OptionString("Loop label",  &loop_label_argument);
  OptionInt *option_unroll_count = new OptionInt("Unroll count", &unroll_count_argument);
  OptionList *unroll_pass_arguments = new OptionList();
  unroll_pass_arguments->add(option_loop_label);
  unroll_pass_arguments->add(option_unroll_count);
  _command_line->add(unroll_pass_arguments);
}

void UnrollPass2::do_procedure_definition(ProcedureDefinition* proc_def)
{
  procDef = proc_def ;
  assert(procDef != NULL) ;

  std::stringstream informationStream ;
  informationStream << "Unrolling " << loop_label_argument << " by "
		    << unroll_count_argument << "." ;
  OutputInformation(informationStream.str().c_str()) ;

  bool unrolled = false ;

  list<CForStatement*>* allFors = 
    collect_objects<CForStatement>(procDef->get_body()) ;
  assert(allFors != NULL) ;
  list<CForStatement*>::iterator forIter = allFors->begin() ;
  while (forIter != allFors->end())
  {
    unrolled |= ProcessLoop(*forIter) ;
    ++forIter ;
  }
  delete allFors ;

  if (!unrolled)
  {
    std::stringstream errorStream ;
    errorStream << "Could not find the loop " 
		<< loop_label_argument
		<< " to unroll!" 
		<< std::endl ;
    OutputError(errorStream.str().c_str()) ;
  }

  OutputInformation("Loop unroll pass ends") ;
}

bool UnrollPass2::ProcessLoop(CForStatement* c)
{
  assert(c != NULL) ;
  if (!LabelledLoop(c))
  {
    return false ;
  }

  // We are in the correct loop, so we can begin.
  CollectInformation(c) ;
   
  // Create a statement list that will contain all of the unrolled loop
  //  bodies 
  StatementList* totalList = create_statement_list(theEnv) ;

  // Replace the original body
  Statement* originalBody = c->get_body() ;
  c->set_body(NULL) ;
  totalList->append_statement(originalBody) ;

  // If we are unrolling a body that contains a call statement, we will have
  //  to handle the call statements in the next pass, so annotate the
  //  procedure definition so we know.

  Iter<CallStatement> callIter = 
    object_iterator<CallStatement>(originalBody) ;
  if (callIter.is_valid())
  {
    StatementList* bodyList = 
      dynamic_cast<StatementList*>(originalBody) ;
    assert(bodyList != NULL) ;
    
    BrickAnnote* unrolledAnnote = 
      create_brick_annote(theEnv, "UnrolledWithCalls") ;
    unrolledAnnote->append_brick(create_integer_brick(theEnv,
						      IInteger(unrollAmount+1)));
    unrolledAnnote->append_brick(create_integer_brick(theEnv,
						      IInteger(bodyList->get_statement_count()))) ;
			      
    procDef->append_annote(unrolledAnnote) ;
    
  }
  else
  {
    StatementList* bodyList = 
      dynamic_cast<StatementList*>(originalBody) ;
    assert(bodyList != NULL) ;
    
    BrickAnnote* unrolledAnnote = 
      create_brick_annote(theEnv, "UnrolledNoCalls") ;

    unrolledAnnote->append_brick(create_integer_brick(theEnv,
						      IInteger(bodyList->get_statement_count()))) ;
			      
    procDef->append_annote(unrolledAnnote) ;
  }

  // Start at one so we don't multiply by zero
  for (int i = 1 ; i < unrollAmount ; ++i)
  {
    Statement* nextBody = dynamic_cast<Statement*>(originalBody->deep_clone());
    assert(nextBody != NULL) ;
    
    list<CallStatement*>* allCalls = 
      collect_objects<CallStatement>(nextBody) ;
    list<CallStatement*>::iterator callIter = allCalls->begin() ;
    while (callIter != allCalls->end())
    {
      (*callIter)->append_annote(create_brick_annote(theEnv, "UnrolledCall")) ;
      ++callIter ;
    }
    delete allCalls ;
    
    BrickAnnote* unrolledBrick = create_brick_annote(theEnv,
						     "UnrolledLoopBody") ;
    unrolledBrick->append_brick(create_integer_brick(theEnv, IInteger(i))) ;
    nextBody->append_annote(unrolledBrick) ;

    // Calls are handled after the loop has finished unrolling.
    //  This function just adjusts any array accesses
    ProcessLoadsParallel(nextBody, i) ;

    totalList->append_statement(nextBody) ;
  }
  c->set_body(totalList) ;

  // Call statements will be handled in another pass, after the 
  //  result of this pass is flattened.

  // Finally, we must transform the outer loop.  This means
  //  both adjusting the step and changing the body to be a list of lists
  //  We must make sure to call FlattenStatementListsPass after this 
  //  pass.
  AdjustOuterLoop(c) ;

  return true ;
}

bool UnrollPass2::LabelledLoop(CForStatement* c)
{
  Statement* body = c->get_body() ;
  if (body == NULL)
  {
    return false ;
  }
  BrickAnnote* labelAnnote = 
    to<BrickAnnote>(c->lookup_annote_by_name("c_for_label")) ;
  if (labelAnnote == NULL)
  {
    return false ;
  }
  StringBrick* labelBrick = 
    to<StringBrick>(labelAnnote->get_brick(0)) ;
  String currentLabel = labelBrick->get_value() ;
  if (currentLabel != loop_label_argument)
  {
    return false ;
  }
  return true ;
}

// This function is responsible for collecting the original step, the
//  index used in the loop, and the maximum amount to unroll.
void UnrollPass2::CollectInformation(CForStatement* c) 
{
  CollectStep(c) ;
  CollectIndex(c) ;
  CollectUnrollAmount(c) ;
}

void UnrollPass2::CollectStep(CForStatement* c)
{
  assert(c != NULL) ;
  Statement* step = c->get_step() ;
  assert(step != NULL) ;
  StoreVariableStatement* stepStoreVar = 
    dynamic_cast<StoreVariableStatement*>(step) ;
  if (stepStoreVar == NULL)
  {
    OutputError("Trying to unroll a for loop with a nonstandard step!") ;
    assert(0) ;
  }
  Expression* rightHandSide = stepStoreVar->get_value() ;
  assert(rightHandSide != NULL) ;
  BinaryExpression* rightHandBinary = 
    dynamic_cast<BinaryExpression*>(rightHandSide) ;
  if (rightHandBinary == NULL)
  {
    OutputError("Trying to unroll a for loop with a nonstandard step!") ;
    assert(0) ;
  }
  // One of the binary expressions sources should be an int constant
  if (dynamic_cast<IntConstant*>(rightHandBinary->get_source1()) != NULL)
  {
    originalStepValue = dynamic_cast<IntConstant*>(rightHandBinary->get_source1())->get_value().c_int() ;
  }
  else if (dynamic_cast<IntConstant*>(rightHandBinary->get_source2()) != NULL)
  {
    originalStepValue = dynamic_cast<IntConstant*>(rightHandBinary->get_source2())->get_value().c_int() ;
  }
  else
  {
    OutputError("Trying to unroll a for loop with a nonstandard step!") ;
    assert(0) ;
  }
  assert(originalStepValue != 0) ;
}

void UnrollPass2::CollectIndex(CForStatement* c) 
{
  assert(c != NULL) ;

  assert(c != NULL) ;
  Statement* originalStep = c->get_step() ;
  assert(originalStep != NULL) ;
  StoreVariableStatement* stepStoreVar = 
    dynamic_cast<StoreVariableStatement*>(originalStep) ;
  if (stepStoreVar == NULL)
  {
    OutputError("Trying to unroll a for loop with a nonstandard step!") ;
    assert(0) ;
  }

  // Now that we have the step we have access to the index, so store it
  //  away.
  index = stepStoreVar->get_destination() ;
  assert(index != NULL) ;  
}

// The unroll amount depends upon the the start, the end, and the step
void UnrollPass2::CollectUnrollAmount(CForStatement* c) 
{
  assert(c != NULL) ;
  Expression* test = c->get_test() ;
  int endValue ;

  int startValue ;
  Statement* before = c->get_before() ;
  assert(before != NULL) ;

  // The before should be a store variable statement
  StoreVariableStatement* storeBefore = 
    dynamic_cast<StoreVariableStatement*>(before) ;
  assert(storeBefore != NULL) ;

  Expression* beforeValue = storeBefore->get_value() ;
  assert(beforeValue != NULL) ;
  IntConstant* beforeConstant = dynamic_cast<IntConstant*>(beforeValue) ;
  assert(beforeConstant != NULL) ;
  startValue = beforeConstant->get_value().c_int() ;

  BinaryExpression* binTest = dynamic_cast<BinaryExpression*>(test) ;
  if (dynamic_cast<IntConstant*>(test) != NULL)
  {
    assert((unroll_count_argument > 0) && 
	   "Cannot fully unroll an infinite loop!") ;
    unrollAmount = unroll_count_argument ;
    fullyUnrolled = false ;
    return ;
  }
  else if (binTest == NULL)
  {
    OutputError("For loop test incorrectly formatted!") ;
    assert(0) ;
  }
  
  
  if (dynamic_cast<IntConstant*>(binTest->get_source2()) != NULL)
  {
    endValue = dynamic_cast<IntConstant*>(binTest->get_source2())->get_value().c_int() ;
  }
  else
  {
    assert((unroll_count_argument > 0) && 
	   "Cannot fully unroll a loop with variable bounds!") ;
    // This is not a constant, so just unroll as normal
    unrollAmount = unroll_count_argument ;
    fullyUnrolled = false ;
    return ;
  }

  int numIterations ;
  if (binTest->get_opcode() == LString("is_less_than"))
  {
    numIterations = (endValue - startValue) / originalStepValue ;
    if (((endValue - startValue) % originalStepValue) != 0)
    {
      ++numIterations ;
    }
  }
  else if (binTest->get_opcode() == LString("is_less_than_or_equal_to"))
  {
    numIterations = (endValue - startValue + 1) / originalStepValue ;
    if (((endValue - startValue + 1) % originalStepValue) != 0)
    {
      ++numIterations ;
    }
  }
  else
  {
    OutputError("Incorrectly formatted for loop!") ;
    assert(0) ;
  }

  if (unroll_count_argument >= numIterations || unroll_count_argument == -1)
  {
    unrollAmount = numIterations ;
    fullyUnrolled = true ;
  }
  else
  {
    unrollAmount = unroll_count_argument ;
    fullyUnrolled = false ;
  }
}

void UnrollPass2::AdjustOuterLoop(CForStatement* c)
{
  AnnotateIndex() ;

  if (!fullyUnrolled)
  {
    AdjustStep(c) ;
    return ;
  }
  
  // This was fully unrolled, so we have to replace this
  Statement* body = c->get_body() ;
  Statement* before = c->get_before() ;
  StatementList* both = create_statement_list(theEnv) ;

  if (before != NULL)
  {
    c->set_before(NULL) ;
    before->set_parent(NULL) ;
    // Don't print the before statement
    before->append_annote(create_brick_annote(theEnv, "NonPrintable")) ;
    both->append_statement(before) ;
    c->set_body(NULL) ;
    body->set_parent(NULL) ;
    both->append_statement(body) ;
    c->get_parent()->replace(c, both) ;
    FormattedText tmpText ;
    before->print(tmpText) ;
  }
  else
  {
    c->set_body(NULL) ;
    body->set_parent(NULL) ;
    c->get_parent()->replace(c, body) ;
  }
}

void UnrollPass2::AnnotateIndex()
{
  assert(index != NULL) ;
  index->append_annote(create_brick_annote(theEnv,
					   "LoopIndex")) ;
}

void UnrollPass2::AdjustStep(CForStatement* c)
{
  assert(c != NULL) ;
  Statement* originalStep = c->get_step() ;
  assert(originalStep != NULL) ;

  // The step should be a store variable statement
  StoreVariableStatement* stepStoreVar = 
    dynamic_cast<StoreVariableStatement*>(originalStep) ;
  if (stepStoreVar == NULL)
  {
    OutputError("Trying to unroll a for loop with a nonstandard step!") ;
    assert(0) ;
  }

  index = stepStoreVar->get_destination() ;
  
  Expression* rightHandSide = stepStoreVar->get_value() ;
  assert(rightHandSide != NULL) ;
  BinaryExpression* rightHandBinary = 
    dynamic_cast<BinaryExpression*>(rightHandSide) ;
  if (rightHandBinary == NULL)
  {
    OutputError("Trying to unroll a for loop with a nonstandard step!") ;
    assert(0) ;
  }

  // One of the binary expressions sources should be an int constant
  if (dynamic_cast<IntConstant*>(rightHandBinary->get_source1()) != NULL)
  {
    originalStepValue = dynamic_cast<IntConstant*>(rightHandBinary->get_source1())->get_value().c_int() ;
    int adjustedStep = originalStepValue * (unroll_count_argument) ;
    dynamic_cast<IntConstant*>(rightHandBinary->get_source1())->set_value(IInteger(adjustedStep)) ;
  }
  else if (dynamic_cast<IntConstant*>(rightHandBinary->get_source2()) != NULL)
  {
    originalStepValue = dynamic_cast<IntConstant*>(rightHandBinary->get_source2())->get_value().c_int() ;
    int adjustedStep = originalStepValue * (unroll_count_argument) ;
    dynamic_cast<IntConstant*>(rightHandBinary->get_source2())->set_value(IInteger(adjustedStep)) ;
  }
  else
  {
    OutputError("Trying to unroll a for loop with a nonstandard step!") ;
    assert(0) ;
  }
}

// What I have to do here is find every instance where we are loading the
//  index associated with this loop and replace it with an addition to
//  a constant.  This will be folded away later.
void UnrollPass2::ProcessLoadsParallel(Statement* n, int loopCopy)
{
  assert(procDef != NULL) ;
  assert(originalStepValue != 0) ;

  list<LoadVariableExpression*>* allLoads = 
    collect_objects<LoadVariableExpression>(n) ;
  assert(allLoads != NULL) ;

  list<LoadVariableExpression*>::iterator loadIter = allLoads->begin() ;
  while (loadIter != allLoads->end())
  {
    if (index == (*loadIter)->get_source())
    {
      Expression* src1 = clone_expression(*loadIter) ;
      Expression* src2 = create_int_constant(theEnv,
					     IInteger(originalStepValue * loopCopy)) ;
      Expression* replacement = create_binary_expression(theEnv,
							 (*loadIter)->get_result_type(),
							 "add", 
							 src1,
							 src2) ;
      (*loadIter)->get_parent()->replace((*loadIter), replacement) ;
    }
    ++loadIter ;
  }

  delete allLoads ;

}
