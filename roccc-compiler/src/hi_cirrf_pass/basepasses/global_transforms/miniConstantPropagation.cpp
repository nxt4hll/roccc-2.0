
#include <cassert>
#include <map>
#include <sstream>

#include <basicnodes/basic.h>
#include <basicnodes/basic_factory.h>
#include <suifnodes/suif.h>
#include <suifnodes/suif_factory.h>
#include <suifkernel/utilities.h>

#include "roccc_utils/warning_utils.h"
#include "roccc_utils/roccc2.0_utils.h"

#include "miniConstantPropagation.h"

MiniConstantPropagationPass::MiniConstantPropagationPass(SuifEnv* pEnv) :
  PipelinablePass(pEnv, "MiniConstantPropagationPass")
{
  theEnv = pEnv ;
  procDef = NULL ;
}

void MiniConstantPropagationPass::do_procedure_definition(ProcedureDefinition* p)
{
  procDef = p ;
  assert(procDef != NULL) ;
  OutputInformation("Mini Constant Propagation Pass begins") ;

  StatementList* innermostList = InnermostList(procDef) ;
  assert(innermostList != NULL) ;

  list<StatementList*>* allStatementLists = 
    collect_objects<StatementList>(innermostList) ;
  
  list<StatementList*>::iterator listIter = allStatementLists->begin() ;
  while (listIter != allStatementLists->end())
  {
    ProcessList(*listIter) ;
    ++listIter ;
  }

  delete allStatementLists ;

  OutputInformation("Mini Constant Propagation Pass ends") ;
}

void MiniConstantPropagationPass::ProcessList(StatementList* s)
{
  assert(s != NULL) ;
  bool changed ;
  do
  {
    changed = false ;
    // Go through each statement until we either hit the end or we
    //  hit a branch point.  Keep track of all constants we have identified
    //  and propagate them as appropriate
    std::map<VariableSymbol*, Constant*> propagateConstants ;
    for (int i = 0 ; i < s->get_statement_count() ; ++i)
    {
      Statement* currentStatement = s->get_statement(i) ;
      // Check to see if we are at a break point
      if (dynamic_cast<StoreStatement*>(currentStatement) == NULL &&
	  dynamic_cast<StoreVariableStatement*>(currentStatement) == NULL &&
	  dynamic_cast<CallStatement*>(currentStatement) == NULL &&
	  dynamic_cast<LabelLocationStatement*>(currentStatement) == NULL)
      {
	break ;
      }
      // Make any replacements as necessary
      list<LoadVariableExpression*>* allLoadVars = 
	collect_objects<LoadVariableExpression>(currentStatement) ;
      list<LoadVariableExpression*>::iterator loadVarIter = 
	allLoadVars->begin() ;
      while (loadVarIter != allLoadVars->end())
      {
	if (propagateConstants[(*loadVarIter)->get_source()] != NULL)
	{
	  (*loadVarIter)->get_parent()->replace((*loadVarIter), 
						dynamic_cast<Expression*>(propagateConstants[(*loadVarIter)->get_source()]->deep_clone())) ;
	  changed = true ;
	}
	++loadVarIter ;
      }
      delete allLoadVars ;
      // Adjust the map depending on the variable stored if applicable
      if (dynamic_cast<StoreVariableStatement*>(currentStatement) != NULL)
      {
	StoreVariableStatement* currentStoreVar = 
	  dynamic_cast<StoreVariableStatement*>(currentStatement) ;
	if (dynamic_cast<Constant*>(currentStoreVar->get_value()) != NULL)
	{
	  propagateConstants[currentStoreVar->get_destination()] = 
	    dynamic_cast<Constant*>(currentStoreVar->get_value()) ;
	}
	else
	{
	  propagateConstants[currentStoreVar->get_destination()] = NULL ;
	}
      }      
    }
    changed |= MiniFold(s) ;
  }
  while (changed == true) ;
}

bool MiniConstantPropagationPass::MiniFold(StatementList* s)
{
  assert(s != NULL) ;
  bool changed = false ;
  list<BinaryExpression*>* allBinExp = collect_objects<BinaryExpression>(s) ;
  list<BinaryExpression*>::iterator binIter = allBinExp->begin() ;
  while (binIter != allBinExp->end())
  {
    Constant* x = dynamic_cast<Constant*>((*binIter)->get_source1()) ;
    Constant* y = dynamic_cast<Constant*>((*binIter)->get_source2()) ;
    Constant* replacement = Merge(x, y, (*binIter)->get_opcode()) ;
    if (replacement != NULL)
    {
      (*binIter)->get_parent()->replace(*binIter, replacement) ;
      changed = true ;
    }
    ++binIter ;
  }
  delete allBinExp ;
  return changed ;
}

Constant* MiniConstantPropagationPass::Merge(Constant* x, Constant* y, 
					     LString opcode)
{
  assert(theEnv != NULL) ;

  if (x == NULL || y == NULL)
  {
    return NULL ;
  }

  IntConstant* xInt = dynamic_cast<IntConstant*>(x) ;
  IntConstant* yInt = dynamic_cast<IntConstant*>(y) ;
  FloatConstant* xFloat = dynamic_cast<FloatConstant*>(x) ;
  FloatConstant* yFloat = dynamic_cast<FloatConstant*>(y) ;

  // This mini propagation only cares for addition and subtraction.  Other
  //  propagations will be handled in a later pass.
  
  if (opcode == LString("add"))
  {
    if (xInt != NULL && yInt != NULL)
    {
      int mergedValue = xInt->get_value().c_int() + yInt->get_value().c_int() ;
      return create_int_constant(theEnv, xInt->get_result_type(),
				 IInteger(mergedValue)) ;
    }
    if (xFloat != NULL && yFloat != NULL)
    {
      float mergedValue = StringToFloat(xFloat->get_value()) +
	                  StringToFloat(yFloat->get_value()) ;
      return create_float_constant(theEnv, xFloat->get_result_type(),
				   FloatToString(mergedValue)) ;
    }
    if (xInt != NULL && yFloat != NULL)
    {
      float mergedValue = xInt->get_value().c_int() + 
 	                  StringToFloat(yFloat->get_value()) ;
      return create_float_constant(theEnv, yFloat->get_result_type(),
				   FloatToString(mergedValue)) ;
    }
    if (xFloat != NULL && yInt != NULL)
    {
      float mergedValue = StringToFloat(xFloat->get_value()) +
 	                  yInt->get_value().c_int() ;
      return create_float_constant(theEnv, xFloat->get_result_type(),
				   FloatToString(mergedValue)) ;
    }
  }

  if (opcode == LString("subtract"))
  {
    if (xInt != NULL && yInt != NULL)
    {
      int mergedValue = xInt->get_value().c_int() - yInt->get_value().c_int() ;
      return create_int_constant(theEnv, xInt->get_result_type(),
				 IInteger(mergedValue)) ;
    }
    if (xFloat != NULL && yFloat != NULL)
    {
      float mergedValue = StringToFloat(xFloat->get_value()) - 
                          StringToFloat(yFloat->get_value()) ;
      return create_float_constant(theEnv, xFloat->get_result_type(),
				   FloatToString(mergedValue)) ;
    }
    if (xInt != NULL && yFloat != NULL)
    {
      float mergedValue = xInt->get_value().c_int() - 
 	                  StringToFloat(yFloat->get_value()) ;
      return create_float_constant(theEnv, yFloat->get_result_type(),
				   FloatToString(mergedValue)) ;
    }
    if (xFloat != NULL && yInt != NULL)
    {
      float mergedValue = StringToFloat(xFloat->get_value()) -
                          yInt->get_value().c_int() ;
      return create_float_constant(theEnv, xFloat->get_result_type(),
				   FloatToString(mergedValue)) ;
    }
  }

  return NULL ;

}

float MiniConstantPropagationPass::StringToFloat(String x)
{
  float toReturn ;
  std::stringstream convert ;
  convert << x ;
  convert >> toReturn ;
  return toReturn ;
}

String MiniConstantPropagationPass::FloatToString(float x)
{
  String toReturn ;
  std::stringstream convert ;
  convert << x ;
  toReturn = convert.str().c_str() ;
  return toReturn ;
}
