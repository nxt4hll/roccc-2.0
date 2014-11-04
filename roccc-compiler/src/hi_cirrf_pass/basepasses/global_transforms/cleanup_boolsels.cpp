
#include <cassert>

#include <basicnodes/basic_factory.h>
#include <suifnodes/suif_factory.h>
#include <suifkernel/utilities.h>

#include "roccc_utils/warning_utils.h"

#include "cleanup_boolsels.h"

static const int MODULE = 1 ;
static const int SYSTEM = 2 ;
static const int COMPOSABLE_SYSTEM = 3 ;

CleanupBoolSelsPass::CleanupBoolSelsPass(SuifEnv* pEnv) : 
  PipelinablePass(pEnv, "CleanupBoolSelsPass")
{
  theEnv = pEnv ;
  procDef = NULL ;
}

void CleanupBoolSelsPass::do_procedure_definition(ProcedureDefinition* p)
{
  procDef = p ;
  assert(procDef != NULL) ;

  OutputInformation("Cleanup Boolean Select Pass Begins") ;
  
  // Find all of the locations that have been marked as "UndefinedPath"
  Annote* functionAnnote = procDef->lookup_annote_by_name("FunctionType") ;
  BrickAnnote* functionBrickAnnote = 
    dynamic_cast<BrickAnnote*>(functionAnnote) ;
  assert(functionBrickAnnote != NULL) ;
  IntegerBrick* valueBrick = 
    dynamic_cast<IntegerBrick*>(functionBrickAnnote->get_brick(0)) ;
  assert(valueBrick != NULL) ;
  
  int myType = valueBrick->get_value().c_int() ;

  if (myType == MODULE)
  {
    // Find all of the expressions marked "UndefinedPath" and replace with
    //  the integer constant zero.
    list<Expression*>* allExpr = 
      collect_objects<Expression>(procDef->get_body()) ;
    list<Expression*>::iterator exprIter = allExpr->begin() ;
    while (exprIter != allExpr->end())
    {
      if ((*exprIter)->lookup_annote_by_name("UndefinedPath") != NULL)
      {
	Expression* zero = 
	  create_int_constant(theEnv,
			      (*exprIter)->get_result_type(),
			      IInteger(0)) ;
	(*exprIter)->get_parent()->replace((*exprIter), zero) ; 
      }
      ++exprIter ;
    }
    delete allExpr ;
  }

  OutputInformation("Cleanup Boolean Select Pass Ends") ;
}
