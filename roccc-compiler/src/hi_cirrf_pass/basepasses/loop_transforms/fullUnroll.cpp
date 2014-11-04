
#include <cassert>

#include <utils/expression_utils.h>
#include <suifkernel/utilities.h>
#include <basicnodes/basic_factory.h>
#include <suifnodes/suif_factory.h>

#include "roccc_utils/warning_utils.h"
#include "roccc_utils/verifier.h"

#include "../control_flow_analysis/solve_pass.h"
#include "../bit_vector_data_flow_analysis/solve_pass2.h"
#include "../bit_vector_data_flow_analysis/ud_du_chain_builder_pass2.h"
#include "../global_transforms/constant_propagation_n_folding_pass2.h"

#include "fullUnroll.h"

const int MAX_ITERATIONS = 10000 ;

FullUnrollPass::FullUnrollPass(SuifEnv* pEnv) :
  PipelinablePass(pEnv, "FullUnrollPass")
{
  theEnv = pEnv ;
  procDef = NULL ;

  stepValue = 0 ;
  unrollAmount = 0 ;
  index = NULL ;
}

FullUnrollPass::~FullUnrollPass()
{
  ; // Nothing to delete yet...
}

void FullUnrollPass::ResetValues()
{
  stepValue = 0 ;
  unrollAmount = 0 ;
  index = NULL ;
}

void FullUnrollPass::do_procedure_definition(ProcedureDefinition* proc_def)
{
  procDef = proc_def ;
  assert(procDef != NULL) ;
  
  OutputInformation("Full Unroll Pass begins") ;

  list<CForStatement*>* allForLoops =
    collect_objects<CForStatement>(procDef->get_body()) ;
  list<CForStatement*>::iterator forIter = allForLoops->begin() ;
  int iterationCount = 0 ;
  while (forIter != allForLoops->end() && iterationCount < MAX_ITERATIONS)
  {
    ResetValues() ;
    ProcessLoop(*forIter) ;
    delete allForLoops ;

    ++iterationCount ;
    
    ControlFlowSolvePass controlFlow(theEnv) ;
    controlFlow.do_procedure_definition(procDef) ;
    DataFlowSolvePass2 dataFlow(theEnv) ;
    dataFlow.do_procedure_definition(procDef) ;
    UD_DU_ChainBuilderPass2 chainBuilder(theEnv) ;
    chainBuilder.do_procedure_definition(procDef) ;
    ConstantPropagationAndFoldingPass2 constantProp(theEnv) ;
    constantProp.do_procedure_definition(procDef) ;
    AnnotationCleaner cleanup(theEnv) ;
    cleanup.Clean(procDef) ;
    
    allForLoops = collect_objects<CForStatement>(procDef->get_body()) ;
    forIter = allForLoops->begin() ;    
  }
  delete allForLoops ;

  if (iterationCount >= MAX_ITERATIONS)
  {
    OutputError("Could not unroll all loops!  Try individually unrolling loops.") ;
    assert(0) ;    
  }
  
  OutputInformation("Full Unroll Pass ends") ;  
}

void FullUnrollPass::ProcessLoop(CForStatement* loop)
{
  assert(loop != NULL) ;

  CollectInformation(loop) ;

  StatementList* totalList = create_statement_list(theEnv) ;
  Statement* originalBody = loop->get_body() ;
  assert(originalBody != NULL) ;
  loop->set_body(NULL) ;
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
    BrickAnnote* unrolledBrick = create_brick_annote(theEnv,
						     "UnrolledLoopBody") ;
    unrolledBrick->append_brick(create_integer_brick(theEnv, IInteger(i))) ;
    nextBody->append_annote(unrolledBrick) ;
    
    // Calls are handled after the loop has finished unrolling.
    //  This function just adjusts as array accesses
    ProcessLoadsParallel(nextBody, i) ;
    totalList->append_statement(nextBody) ;
  }

  loop->set_body(totalList) ;
  AdjustOuterLoop(loop) ;

}

void FullUnrollPass::CollectInformation(CForStatement* loop)
{
  CollectStep(loop) ;
  CollectIndex(loop) ;
  CollectUnrollAmount(loop) ;
}

void FullUnrollPass::CollectStep(CForStatement* loop) 
{
  assert(loop != NULL) ;
  Statement* step = loop->get_step() ;
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
    stepValue = dynamic_cast<IntConstant*>(rightHandBinary->get_source1())->get_value().c_int() ;
  }
  else if (dynamic_cast<IntConstant*>(rightHandBinary->get_source2()) != NULL)
  {
    stepValue = dynamic_cast<IntConstant*>(rightHandBinary->get_source2())->get_value().c_int() ;
  }
  else
  {
    OutputError("Tryinf to unroll a for loop with a nonstandard step!") ;
    assert(0) ;
  }
  assert(stepValue != 0) ;
}

void FullUnrollPass::CollectIndex(CForStatement* loop)
{
  assert(loop != NULL) ;
  Statement* originalStep = loop->get_step() ;
  assert(originalStep != NULL) ;
  StoreVariableStatement* stepStoreVar = 
    dynamic_cast<StoreVariableStatement*>(originalStep) ;
  if (stepStoreVar == NULL)
  {
    OutputError("Trying to unroll a for loop with a nonstandard step!") ;
    assert(0) ;
  }
  index = stepStoreVar->get_destination() ;
  assert(index != NULL) ;
}

void FullUnrollPass::CollectUnrollAmount(CForStatement* loop)
{
  assert(loop != NULL) ;

  int startValue ;
  int endValue ;
  
  Statement* before = loop->get_before() ;
  assert(before != NULL) ;
  StoreVariableStatement* storeBefore =
    dynamic_cast<StoreVariableStatement*>(before) ;
  assert(storeBefore != NULL) ;

  Expression* beforeValue = storeBefore->get_value() ;
  assert(beforeValue != NULL) ;
  IntConstant* beforeConstant = dynamic_cast<IntConstant*>(beforeValue) ;
  if (beforeConstant == NULL)
  {
    return ;
  }
  assert(beforeConstant != NULL) ;
  startValue = beforeConstant->get_value().c_int() ;

  Expression* test = loop->get_test() ;
  BinaryExpression* binTest = dynamic_cast<BinaryExpression*>(test) ;
  if (binTest == NULL)
  {
    OutputError("For loop test incorrectly formatted!") ;
    assert(0) ;
  }
  if (dynamic_cast<IntConstant*>(binTest->get_source2()) == NULL)
  {
    OutputError("Cannot fully unroll a loop with variable bounds!") ;
    assert(0) ;
  }

  endValue = dynamic_cast<IntConstant*>(binTest->get_source2())->get_value().c_int() ;

  if (binTest->get_opcode() == LString("is_less_than"))
  {
    unrollAmount = (endValue - startValue) / stepValue ;
    if (((endValue - startValue) % stepValue) != 0)
    {
      ++unrollAmount ;
    }
  }
  else if (binTest->get_opcode() == LString("is_less_than_or_equal_to"))
  {
    unrollAmount = (endValue - startValue + 1) / stepValue ;
    if (((endValue - startValue + 1) % stepValue) != 0)
    {
      ++unrollAmount ;
    }
  }
  else 
  {
    OutputError("Incorrectly formatted for loop!") ;
    assert(0) ;
  }
}

void FullUnrollPass::AdjustOuterLoop(CForStatement* loop)
{
  AnnotateIndex() ;

  Statement* body = loop->get_body() ;
  Statement* before = loop->get_before() ;
  StatementList* both = create_statement_list(theEnv) ;

  if (before != NULL)
  {
    loop->set_before(NULL) ;
    before->set_parent(NULL) ;
    both->append_statement(before) ;
    loop->set_body(NULL) ;
    body->set_parent(NULL) ;
    both->append_statement(body) ;
    loop->get_parent()->replace(loop, both) ;
  }
  else
  {
    loop->set_body(NULL) ;
    body->set_parent(NULL) ;
    loop->get_parent()->replace(loop, body) ;
  }

}

void FullUnrollPass::AnnotateIndex()
{
  assert(index != NULL) ;
  index->append_annote(create_brick_annote(theEnv, "LoopIndex")) ;
}

void FullUnrollPass::ProcessLoadsParallel(Statement* n, int loopCopy)
{
  assert(procDef != NULL) ;
  assert(stepValue != 0) ;

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
					     IInteger(stepValue * loopCopy)) ;
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

AnnotationCleaner::AnnotationCleaner(SuifEnv* pEnv)
{
  theEnv = pEnv ;
  procDef = NULL ;
}

void AnnotationCleaner::CleanObject(AnnotableObject* o)
{
  assert(o != NULL) ;
  RemoveAnnotations(o, "predecessors") ;
  RemoveAnnotations(o, "successors") ;
  RemoveAnnotations(o, "in_stmts") ;
  RemoveAnnotations(o, "out_stmts") ;
  RemoveAnnotations(o, "killed_stmts") ;
  RemoveAnnotations(o, "reached_uses") ;
  RemoveAnnotations(o, "reaching_defs") ;
}

void AnnotationCleaner::RemoveAnnotations(AnnotableObject* o, 
					  const char* annoteName)
{
  assert(o != NULL) ;
  if (o->lookup_annote_by_name(annoteName) != NULL)
  {
    BrickAnnote* brick = 
      dynamic_cast<BrickAnnote*>(o->lookup_annote_by_name(annoteName)) ;
    if (brick != NULL)
    {
      while (brick->get_brick_count() > 0)
      {
	delete brick->remove_brick(0) ;
      }
    }
    delete o->remove_annote_by_name(annoteName) ;
  }
}

void AnnotationCleaner::Clean(ProcedureDefinition* p)
{
  assert(p != NULL) ;
  assert(p->get_body() != NULL) ;

  list<AnnotableObject*>* allObjects = 
    collect_objects<AnnotableObject>(p->get_body()) ;
  
  list<AnnotableObject*>::iterator objectIter = allObjects->begin() ;
  while (objectIter != allObjects->end())
  {
    CleanObject(*objectIter) ;
    ++objectIter ;
  }
  delete allObjects ;
}
