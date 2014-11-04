// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include "common/system_specific.h"
#include <common/suif_copyright.h>

#include <cassert>

#include <basicnodes/basic.h>
#include <basicnodes/basic_factory.h>
#include <suifnodes/suif.h>
#include <suifnodes/suif_factory.h>

#include <suifkernel/utilities.h>

#include "roccc_utils/annote_utils.h"
#include "roccc_utils/warning_utils.h"
#include "strip_annotes_pass.h"

using namespace std ;

StripAnnotesPass::StripAnnotesPass( SuifEnv* suif_env) : 
  PipelinablePass(suif_env, "StripAnnotesPass" ) 
{
  theEnv = suif_env ;
  procDef = NULL ;
}

void StripAnnotesPass::do_procedure_definition(ProcedureDefinition *p)
{
  procDef = p ;
  assert(procDef != NULL) ;
  
  OutputInformation("Removing annotations starts") ;
  if (procDef->lookup_annote_by_name("bit_vector_map"))
  {
    delete (procDef->remove_annote_by_name("bit_vector_map")) ;
  }

  list<Statement*>* allStatements = 
    collect_objects<Statement>(procDef->get_body()) ;
  list<Statement*>::iterator statementIter = allStatements->begin(); 
  while (statementIter != allStatements->end())
  {
    if ((*statementIter)->lookup_annote_by_name("in_stmts") != NULL)
    {
      BrickAnnote* ba = 
	dynamic_cast<BrickAnnote*>((*statementIter)->lookup_annote_by_name("in_stmts")) ;
      assert(ba != NULL) ;
      empty(ba) ;
      delete (*statementIter)->remove_annote_by_name("in_stmts") ;
    }
    if ((*statementIter)->lookup_annote_by_name("out_stmts") != NULL)
    {
      BrickAnnote* ba = 
	dynamic_cast<BrickAnnote*>((*statementIter)->lookup_annote_by_name("out_stmts")) ;
      assert(ba != NULL) ;
      empty(ba) ;
      delete (*statementIter)->remove_annote_by_name("out_stmts") ;
    }
    if ((*statementIter)->lookup_annote_by_name("killed_stmts") != NULL)
    {
      BrickAnnote* ba = 
	dynamic_cast<BrickAnnote*>((*statementIter)->lookup_annote_by_name("killed_stmts")) ;
      assert(ba != NULL) ;
      empty(ba) ;
      delete (*statementIter)->remove_annote_by_name("killed_stmts") ;
    }
    if ((*statementIter)->lookup_annote_by_name("predecessors") != NULL)
    {
      BrickAnnote* ba = 
	dynamic_cast<BrickAnnote*>((*statementIter)->lookup_annote_by_name("predecessors")) ;
      assert(ba != NULL) ;
      empty(ba) ;
      delete (*statementIter)->remove_annote_by_name("predecessors") ;
    }
    if ((*statementIter)->lookup_annote_by_name("reached_uses") != NULL)
    {
      BrickAnnote* ba = 
	dynamic_cast<BrickAnnote*>((*statementIter)->lookup_annote_by_name("reached_uses")) ;
      assert(ba != NULL) ;
      empty(ba) ;
      delete (*statementIter)->remove_annote_by_name("reached_uses") ;
    }
    if ((*statementIter)->lookup_annote_by_name("successors") != NULL)
    {
      BrickAnnote* ba = 
	dynamic_cast<BrickAnnote*>((*statementIter)->lookup_annote_by_name("successors")) ;
      assert(ba != NULL) ;
      empty(ba) ;
      delete (*statementIter)->remove_annote_by_name("successors") ;
    }
    if ((*statementIter)->lookup_annote_by_name("reaching_defs") != NULL)
    {
      BrickAnnote* ba = 
	dynamic_cast<BrickAnnote*>((*statementIter)->lookup_annote_by_name("reaching_defs")) ;
      assert(ba != NULL) ;
      empty(ba) ;
      delete (*statementIter)->remove_annote_by_name("reaching_defs") ;
    }
    if ((*statementIter)->lookup_annote_by_name("in_available_exprs") != NULL)
    {
      BrickAnnote* ba = 
	dynamic_cast<BrickAnnote*>((*statementIter)->lookup_annote_by_name("in_available_exprs")) ;
      assert(ba != NULL) ;
      empty(ba) ;
      delete (*statementIter)->remove_annote_by_name("in_available_exprs") ;
    }
    if ((*statementIter)->lookup_annote_by_name("out_available_exprs") != NULL)
    {
      BrickAnnote* ba = 
	dynamic_cast<BrickAnnote*>((*statementIter)->lookup_annote_by_name("out_available_exprs")) ;
      assert(ba != NULL) ;
      empty(ba) ;
      delete (*statementIter)->remove_annote_by_name("out_available_exprs") ;
    }
    if ((*statementIter)->lookup_annote_by_name("reached_available_exprs") != NULL)
    {
      BrickAnnote* ba = 
	dynamic_cast<BrickAnnote*>((*statementIter)->lookup_annote_by_name("reached_available_exprs")) ;
      assert(ba != NULL) ;
      empty(ba) ;
      delete (*statementIter)->remove_annote_by_name("reached_available_exprs") ;
    }
    if ((*statementIter)->lookup_annote_by_name("array_ref_info") != NULL)
    {
      BrickAnnote* ba = 
	dynamic_cast<BrickAnnote*>((*statementIter)->lookup_annote_by_name("array_ref_info")) ;
      assert(ba != NULL) ;
      empty(ba) ;
      delete (*statementIter)->remove_annote_by_name("array_ref_info") ;
    }
    ++statementIter ;
  }
  delete allStatements ;

  list<Expression*>* allExpressions = 
    collect_objects<Expression>(procDef->get_body()) ;
  list<Expression*>::iterator exprIter = allExpressions->begin() ;
  while (exprIter != allExpressions->end())
  {
    if ((*exprIter)->lookup_annote_by_name("in_stmts") != NULL)
    {
      BrickAnnote* ba = 
	dynamic_cast<BrickAnnote*>((*exprIter)->lookup_annote_by_name("in_stmts")) ;
      assert(ba != NULL) ;
      empty(ba) ;
      delete (*exprIter)->remove_annote_by_name("in_stmts") ;
    }
    if ((*exprIter)->lookup_annote_by_name("out_stmts") != NULL)
    {
      BrickAnnote* ba = 
	dynamic_cast<BrickAnnote*>((*exprIter)->lookup_annote_by_name("out_stmts")) ;
      assert(ba != NULL) ;
      empty(ba) ;
      delete (*exprIter)->remove_annote_by_name("out_stmts") ;
    }
    if ((*exprIter)->lookup_annote_by_name("killed_stmts") != NULL)
    {
      BrickAnnote* ba = 
	dynamic_cast<BrickAnnote*>((*exprIter)->lookup_annote_by_name("killed_stmts")) ;
      assert(ba != NULL) ;
      empty(ba) ;
      delete (*exprIter)->remove_annote_by_name("killed_stmts") ;
    }
    if ((*exprIter)->lookup_annote_by_name("predecessors") != NULL)
    {
      BrickAnnote* ba = 
	dynamic_cast<BrickAnnote*>((*exprIter)->lookup_annote_by_name("predecessors")) ;
      assert(ba != NULL) ;
      empty(ba) ;
      delete (*exprIter)->remove_annote_by_name("predecessors") ;
    }
    if ((*exprIter)->lookup_annote_by_name("reached_uses") != NULL)
    {
      BrickAnnote* ba = 
	dynamic_cast<BrickAnnote*>((*exprIter)->lookup_annote_by_name("reached_uses")) ;
      assert(ba != NULL) ;
      empty(ba) ;
      delete (*exprIter)->remove_annote_by_name("reached_uses") ;
    }
    if ((*exprIter)->lookup_annote_by_name("successors") != NULL)
    {
      BrickAnnote* ba = 
	dynamic_cast<BrickAnnote*>((*exprIter)->lookup_annote_by_name("successors")) ;
      assert(ba != NULL) ;
      empty(ba) ;
      delete (*exprIter)->remove_annote_by_name("successors") ;
    }
    if ((*exprIter)->lookup_annote_by_name("reaching_defs") != NULL)
    {
      BrickAnnote* ba = 
	dynamic_cast<BrickAnnote*>((*exprIter)->lookup_annote_by_name("reaching_defs")) ;
      assert(ba != NULL) ;
      empty(ba) ;
      delete (*exprIter)->remove_annote_by_name("reaching_defs") ;
    }
    if ((*exprIter)->lookup_annote_by_name("in_available_exprs") != NULL)
    {
      BrickAnnote* ba = 
	dynamic_cast<BrickAnnote*>((*exprIter)->lookup_annote_by_name("in_available_exprs")) ;
      assert(ba != NULL) ;
      empty(ba) ;
      delete (*exprIter)->remove_annote_by_name("in_available_exprs") ;
    }
    if ((*exprIter)->lookup_annote_by_name("out_available_exprs") != NULL)
    {
      BrickAnnote* ba = 
	dynamic_cast<BrickAnnote*>((*exprIter)->lookup_annote_by_name("out_available_exprs")) ;
      assert(ba != NULL) ;
      empty(ba) ;
      delete (*exprIter)->remove_annote_by_name("out_available_exprs") ;
    }
    if ((*exprIter)->lookup_annote_by_name("reached_available_exprs") != NULL)
    {
      BrickAnnote* ba = 
	dynamic_cast<BrickAnnote*>((*exprIter)->lookup_annote_by_name("reached_available_exprs")) ;
      assert(ba != NULL) ;
      empty(ba) ;
      delete (*exprIter)->remove_annote_by_name("reached_available_exprs") ;
    }
    if ((*exprIter)->lookup_annote_by_name("array_ref_info") != NULL)
    {
      BrickAnnote* ba = 
	dynamic_cast<BrickAnnote*>((*exprIter)->lookup_annote_by_name("array_ref_info")) ;
      assert(ba != NULL) ;
      empty(ba) ;
      delete (*exprIter)->remove_annote_by_name("array_ref_info") ;
    }
    ++exprIter ;
  }
  delete allExpressions ;

  OutputInformation("Removing annotations ends") ;
}


