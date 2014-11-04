
#include <cassert>

#include <suifkernel/utilities.h>
#include <basicnodes/basic.h>
#include <basicnodes/basic_factory.h>
#include <suifnodes/suif.h>
#include <suifnodes/suif_factory.h>

#include "roccc_utils/warning_utils.h"
#include "roccc_utils/roccc2.0_utils.h"

#include "scalar_replacement_pass2.h"

ScalarReplacementPass2::ScalarReplacementPass2(SuifEnv* pEnv) :
  PipelinablePass(pEnv, "ScalarReplacementPass2")
{
  theEnv = pEnv ;
  procDef = NULL ;
}

ScalarReplacementPass2::~ScalarReplacementPass2()
{
  ; // Nothing to delete yet
}

void ScalarReplacementPass2::do_procedure_definition(ProcedureDefinition* p)
{
  procDef = p ;
  assert(procDef != NULL) ;

  OutputInformation("Scalar replacement pass 2.0 begins") ;

  VerifyArrayReferences() ;
  CollectArrayReferences() ;

  ProcessLoads() ;
  ProcessStores() ;

  PrependLoads() ;
  AppendStores() ;

  OutputInformation("Scalar replacement pass 2.0 ends") ;
}

void ScalarReplacementPass2::ClearArrayIndicies()
{
  std::map<VariableSymbol*, list<VariableSymbol*>* >::iterator indexIter =
    arrayIndicies.begin() ;
  while (indexIter != arrayIndicies.end())
  {
    delete (*indexIter).second ;
    ++indexIter ;
  }
  arrayIndicies.clear() ;
}

void ScalarReplacementPass2::VerifyArrayReferences()
{
  assert(procDef != NULL) ;
  ClearArrayIndicies() ;
  list<ArrayReferenceExpression*>* allRefs = 
    collect_objects<ArrayReferenceExpression>(procDef->get_body()) ;
  list<ArrayReferenceExpression*>::iterator refIter = allRefs->begin() ;
  while (refIter != allRefs->end())
  {
    // If not the topmost array reference, skip it
    if (dynamic_cast<ArrayReferenceExpression*>((*refIter)->get_parent()) != NULL)
    {
      ++refIter ;
      continue ;
    }
    VariableSymbol* currentArrayVar = GetArrayVariable(*refIter) ;
    // Don't process lookup tables
    if (IsLookupTable(currentArrayVar))
    {
      ++refIter ;
      continue ;
    }
    if (arrayIndicies[currentArrayVar] == NULL)
    {
      arrayIndicies[currentArrayVar] = UsedIndicies(*refIter) ;
    }
    else
    {
      list<VariableSymbol*>* approvedIndicies = arrayIndicies[currentArrayVar];
      assert(approvedIndicies != NULL) ;
      list<VariableSymbol*>* usedIndicies = UsedIndicies(*refIter) ;
      list<VariableSymbol*>::iterator usedIter = usedIndicies->begin() ;
      while (usedIter != usedIndicies->end())
      {
	bool approved = false ;
	list<VariableSymbol*>::iterator approvedIter = 
	  approvedIndicies->begin() ;
	while (approvedIter != approvedIndicies->end())
	{
	  if (*approvedIter == *usedIter)
	  {
	    approved = true ;
	    break ;
	  }
	  ++approvedIter ;
	}
	if (approved == false)
	{
	  OutputError("Error: You cannot access a stream with different index"
		      " variables!") ;
	  assert(0) ;
	}
	++usedIter ;
      }
      delete usedIndicies ;
    }
    ++refIter ;
  }
  delete allRefs ;
  ClearArrayIndicies() ;
}

void ScalarReplacementPass2::CollectArrayReferences()
{
  assert(procDef != NULL) ;
  list<ArrayReferenceExpression*>* allRefs = 
    collect_objects<ArrayReferenceExpression>(procDef->get_body()) ;
  list<ArrayReferenceExpression*>::iterator refIter = allRefs->begin() ;
  while (refIter != allRefs->end())
  {
    // If this is not the topmost array reference, skip it.
    if (dynamic_cast<ArrayReferenceExpression*>((*refIter)->get_parent()) !=
	NULL)
    {
      ++refIter ;
      continue ;
    }
    
    // Also, skip lookup tables
    if (IsLookupTable(GetArrayVariable(*refIter)))
    {
      ++refIter ;
      continue ;
    }

    bool found = false ;
    std::pair<Expression*, VariableSymbol*> toAdd ;
    list<std::pair<Expression*, VariableSymbol*> >::iterator identIter = 
      Identified.begin() ;
    while (identIter != Identified.end())
    {
      if (EquivalentExpressions((*identIter).first, *refIter))
      {
	found = true ;
	toAdd = (*identIter) ;
	break ;
      }
      ++identIter ;
    }

    if (!found)
    {
      VariableSymbol* replacement =
	create_variable_symbol(theEnv, 
			       GetQualifiedTypeOfElement(*refIter),
			       TempName(LString("suifTmp"))) ;	
      replacement->append_annote(create_brick_annote(theEnv, 
						  "ScalarReplacedVariable")) ;
      procDef->get_symbol_table()->append_symbol_table_object(replacement) ;

      toAdd.first = (*refIter) ;
      toAdd.second = replacement ;
      Identified.push_back(toAdd) ;
    }
    if (dynamic_cast<LoadExpression*>((*refIter)->get_parent()) != NULL)
    {
      found = false ;
      identIter = IdentifiedLoads.begin() ;
      while (identIter != IdentifiedLoads.end())
      {
	if (EquivalentExpressions((*identIter).first, *refIter))
	  {
	    found = true ;
	    break ;
	  }
	++identIter ;
      }
      if (!found)
      {
	IdentifiedLoads.push_back(toAdd) ;
      }      
    }
    else if (dynamic_cast<StoreStatement*>((*refIter)->get_parent()) != NULL)
    {
      found = false ;
      identIter = IdentifiedStores.begin() ;
      while (identIter != IdentifiedStores.end())
      {
	if (EquivalentExpressions((*identIter).first, *refIter))
	{
	  found = true ;
	  break ;
	}
	++identIter ;
      }
      if (!found)
      {
	IdentifiedStores.push_back(toAdd) ;
      }
    }
    else 
    {
      assert(0 && "Improperly formatted array reference!") ;
    }  
    ++refIter ;
  }
  delete allRefs ;
}

void ScalarReplacementPass2::ProcessLoads()
{
  assert(procDef != NULL) ;
  list<LoadExpression*>* allLoads = 
    collect_objects<LoadExpression>(procDef->get_body()) ;
  list<LoadExpression*>::iterator loadIter = allLoads->begin() ;
  while (loadIter != allLoads->end())
  {
    ProcessLoad(*loadIter) ;
    ++loadIter ;
  }
  delete allLoads ;
}

void ScalarReplacementPass2::ProcessLoad(LoadExpression* e) 
{
  assert(e != NULL) ;
  Expression* innerExp = e->get_source_address() ;
  ArrayReferenceExpression* innerRef = 
    dynamic_cast<ArrayReferenceExpression*>(innerExp) ;
  if (innerRef == NULL)
  {
    return ;
  }

  // Again, don't process lookup tables
  if (IsLookupTable(GetArrayVariable(innerRef)))
  {
    return ;
  }

  VariableSymbol* replacement = NULL ;
  list<std::pair<Expression*, VariableSymbol*> >::iterator identIter = 
    Identified.begin() ;
  while (identIter != Identified.end())
  {
    if (EquivalentExpressions((*identIter).first, innerRef))
    {
      replacement = (*identIter).second ;
      break ;
    }
    ++identIter ;
  }
  assert(replacement != NULL) ;

  LoadVariableExpression* loadVar = 
    create_load_variable_expression(theEnv, 
				    replacement->get_type()->get_base_type(),
				    replacement) ;
  e->get_parent()->replace(e, loadVar) ;
}

void ScalarReplacementPass2::ProcessStores()
{
  assert(procDef != NULL) ;
  list<StoreStatement*>* allStores = 
    collect_objects<StoreStatement>(procDef->get_body()) ;
  list<StoreStatement*>::iterator storeIter = allStores->begin() ;
  while (storeIter != allStores->end())
  {
    ProcessStore(*storeIter) ;
    ++storeIter ;
  }
  delete allStores ;
}

void ScalarReplacementPass2::ProcessStore(StoreStatement* s)
{
  assert(s != NULL) ;
  Expression* value = s->get_value() ;
  Expression* dest = s->get_destination_address() ;
  ArrayReferenceExpression* destRef = 
    dynamic_cast<ArrayReferenceExpression*>(dest) ;
  if (destRef == NULL)
  {
    return ;
  }
  
  // Even here we skip lookup tables.
  if (IsLookupTable(GetArrayVariable(destRef)))
  {
    return ;
  }

  VariableSymbol* replacementVar = NULL ;
  list<std::pair<Expression*, VariableSymbol*> >::iterator identIter = 
    Identified.begin() ;
  while (identIter != Identified.end())
  {
    if (EquivalentExpressions((*identIter).first, destRef))
    {
      replacementVar = (*identIter).second ;
      break ;
    }
    ++identIter ;
  }
  assert(replacementVar != NULL) ;

  s->set_value(NULL) ;

  StoreVariableStatement* replacementStore = 
    create_store_variable_statement(theEnv, 
				    replacementVar,
				    value) ;

  s->get_parent()->replace(s, replacementStore) ;
}

void ScalarReplacementPass2::PrependLoads()
{
  assert(procDef != NULL) ;
  StatementList* innermost = InnermostList(procDef) ;
  assert(innermost != NULL) ;
  list<std::pair<Expression*, VariableSymbol*> >::iterator loadIter = 
    IdentifiedLoads.begin() ;
  while (loadIter != IdentifiedLoads.end())
  {
    (*loadIter).first->set_parent(NULL) ;
    LoadExpression* loadTmp = 
      create_load_expression(theEnv, 
			     (*loadIter).first->get_result_type(),
			     (*loadIter).first) ;
    StoreVariableStatement* prependedLoad = 
      create_store_variable_statement(theEnv, 
				      (*loadIter).second,
				      loadTmp) ;
    innermost->insert_statement(0, prependedLoad) ;
    ++loadIter ;
  }
}

void ScalarReplacementPass2::AppendStores()
{
  assert(procDef != NULL) ;
  StatementList* innermost = InnermostList(procDef) ;
  assert(innermost != NULL) ;
  list<std::pair<Expression*, VariableSymbol*> >::iterator storeIter = 
    IdentifiedStores.begin() ;
  while (storeIter != IdentifiedStores.end())
  {
    (*storeIter).first->set_parent(NULL) ;
    LoadVariableExpression* loadTmp = 
      create_load_variable_expression(theEnv,
			   (*storeIter).second->get_type()->get_base_type(),
				      (*storeIter).second) ;
    StoreStatement* postStore = 
      create_store_statement(theEnv, 
			     loadTmp,
    			     (*storeIter).first) ;
    innermost->append_statement(postStore) ;
    ++storeIter ;
  }
}
