#include <cassert>

#include <sstream>

#include <suifkernel/utilities.h>

#include "remove_unused_variables.h"

#include "roccc_utils/warning_utils.h"
#include "roccc_utils/roccc2.0_utils.h"

RemoveUnusedVariables::RemoveUnusedVariables(SuifEnv* pEnv) : 
  PipelinablePass(pEnv, "RemoveUnusedVariables")
{
  theEnv = pEnv ;
  procDef = NULL ;

  allRefs = NULL ;

  allLoads = NULL ;
  allStores = NULL ;
  allCalls = NULL ;
}

RemoveUnusedVariables::~RemoveUnusedVariables()
{
  if (allRefs != NULL)
  {
    delete allRefs ;
  }
  if (allLoads != NULL)
  {
    delete allLoads ;
  }
  if (allStores != NULL)
  {
    delete allStores ;
  }
  if (allCalls != NULL)
  {
    delete allCalls ;
  }
}

void RemoveUnusedVariables::do_procedure_definition(ProcedureDefinition* proc_def)
{
  procDef = proc_def ;
  assert(procDef != NULL) ;

  OutputInformation("Remove Unused Variables Pass Begins") ;

  // Collect all of the information
  allRefs = collect_objects<ArrayReferenceExpression>(procDef->get_body()) ;

  allLoads = collect_objects<LoadVariableExpression>(procDef->get_body()) ;
  allStores = collect_objects<StoreVariableStatement>(procDef->get_body()) ;
  allCalls = collect_objects<CallStatement>(procDef->get_body()) ;

  // Go through the symbol table and collect all of the variables that
  //  need to be removed
  list<VariableSymbol*> toBeRemoved ;

  SymbolTable* procSymTab = procDef->get_symbol_table() ;
  assert(procSymTab != NULL) ;

  for (int i = 0 ; i < procSymTab->get_symbol_table_object_count() ; ++i)
  {
    VariableSymbol* nextSym = 
      dynamic_cast<VariableSymbol*>(procSymTab->get_symbol_table_object(i)) ;
    
    if (nextSym != NULL)
    {
      // Don't get rid of arrays I might have created to replace parameters
      if (nextSym->lookup_annote_by_name("InputFifo") != NULL ||
	  nextSym->lookup_annote_by_name("OutputFifo") != NULL)
      {
	continue ;
      }

      if (!HasUse(nextSym) && !HasDefinition(nextSym) 
	  && dynamic_cast<StructType*>(nextSym->get_type()->get_base_type()) ==NULL
	  && nextSym->lookup_annote_by_name("TemporalFeedback") == NULL
	  && nextSym->lookup_annote_by_name("FeedbackVariable") == NULL)
      {	
	toBeRemoved.push_back(nextSym) ;
      }
    }
  }

  list<VariableSymbol*>::iterator removeIter = toBeRemoved.begin() ;
  while (removeIter != toBeRemoved.end())
  {
    std::stringstream removeStream ;
    removeStream << "Removing a variable:" << (*removeIter)->get_name() ;
    OutputInformation(removeStream.str().c_str()) ;

    procDef->get_symbol_table()->remove_symbol_table_object(*removeIter) ;
    ++removeIter ;
  }

  delete allRefs ;  
  delete allLoads ;
  delete allStores ;
  delete allCalls ;

  allRefs   = NULL ;
  allLoads  = NULL ;
  allStores = NULL ;
  allCalls  = NULL ;

  OutputInformation("Remove Unused Variables Pass Ends") ;
}

bool RemoveUnusedVariables::HasUse(VariableSymbol* v)
{
  assert(allLoads != NULL) ;

  list<LoadVariableExpression*>::iterator loadIter = allLoads->begin() ;
  while (loadIter != allLoads->end())
  {
    if ((*loadIter)->get_source() == v)
    {
      return true ;
    }
    ++loadIter ;
  }
  
  list<ArrayReferenceExpression*>::iterator refIter = allRefs->begin() ;
  while (refIter != allRefs->end())
  {
    if (GetArrayVariable(*refIter) == v)
    {
      return true ;
    }
    ++refIter ;
  }

  return false ;
}

bool RemoveUnusedVariables::HasDefinition(VariableSymbol* v)
{
  assert(allStores != NULL) ;
  assert(allCalls != NULL) ;

  list<StoreVariableStatement*>::iterator storeIter = allStores->begin() ;
  while(storeIter != allStores->end())
  {
    if ((*storeIter)->get_destination() == v && 
	(*storeIter)->lookup_annote_by_name("NonPrintable") == NULL)
    {
      return true ;
    }
    ++storeIter ;
  }

  list<CallStatement*>::iterator callIter = allCalls->begin() ;
  while (callIter != allCalls->end())
  {
    if (IsDefinition(*callIter, v))
    {
      return true ;
    }	
    ++callIter ;
  }

  return false ;
}
