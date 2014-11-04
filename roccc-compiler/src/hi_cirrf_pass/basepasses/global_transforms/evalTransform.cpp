// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

#include <cassert>

#include <basicnodes/basic.h>
#include <suifnodes/suif_factory.h>
#include <suifkernel/utilities.h>
#include <cfenodes/cfe.h>

#include "evalTransform.h"

#include "roccc_utils/warning_utils.h"

EvalTransformPass::EvalTransformPass(SuifEnv* pEnv) :
  PipelinablePass(pEnv, "EvalTransformPass")
{
  theEnv = pEnv ;
  procDef = NULL ;
}

void EvalTransformPass::do_procedure_definition(ProcedureDefinition* p)
{
  procDef = p ;
  assert(procDef != NULL) ;

  OutputInformation("Eval Transform pass begins") ;
  
  list<EvalStatement*>* allEvals = 
    collect_objects<EvalStatement>(procDef->get_body()) ;
  list<EvalStatement*>::iterator evalIter = allEvals->begin() ;
  while (evalIter != allEvals->end())
  {
    ProcessEval(*evalIter) ;
    ++evalIter ;
  }
  delete allEvals ;

  OutputInformation("Eval Transform pass ends") ;
}

void EvalTransformPass::ProcessEval(EvalStatement* e)
{
  assert((e->get_expression_count() == 1) && "Unsupported eval statement") ;  
  Expression* currentExp = e->get_expression(0) ;

  if (dynamic_cast<CallExpression*>(currentExp) != NULL)
  {
    CallExpression* currentCall = dynamic_cast<CallExpression*>(currentExp) ;
    Expression* calleeAddress = currentCall->get_callee_address() ;

    // Disconnect the address from the expression
    calleeAddress->set_parent(NULL) ;
    
    CallStatement* replacement = 
      create_call_statement(theEnv, NULL, calleeAddress) ;

    // Now, add all of the parameters
    Iter<Expression*> argIter = currentCall->get_argument_iterator() ;
    while (argIter.is_valid())
    {
      argIter.current()->set_parent(NULL) ;
      replacement->append_argument(argIter.current()) ;
      argIter.next() ;
    }
    
    e->get_parent()->replace(e, replacement) ;

    // Go through and detach these expressions from the original expression
    while(e->get_expression_count() != 0)
    {
      e->remove_expression(0) ;
    }
    // Finally, clean up the memory associated with the eval statement
    delete e ;
  }
  else
  {
    assert(0 && "Unsupported eval statement") ;
  }
  
}
