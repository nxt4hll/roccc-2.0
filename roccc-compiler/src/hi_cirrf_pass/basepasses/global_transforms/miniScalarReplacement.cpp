// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

#include <cassert>
#include <map>

#include <basicnodes/basic.h>
#include <basicnodes/basic_factory.h>
#include <suifnodes/suif.h>
#include <suifnodes/suif_factory.h>
#include <suifkernel/utilities.h>

#include "roccc_utils/warning_utils.h"
#include "roccc_utils/roccc2.0_utils.h"

#include "miniScalarReplacement.h"

// Flatten statement lists should be performed after this pass.

MiniScalarReplacementPass::MiniScalarReplacementPass(SuifEnv* pEnv) :
  PipelinablePass(pEnv, "MiniScalarReplacementPass")
{
  theEnv = pEnv ;
  procDef = NULL ;
}

MiniScalarReplacementPass::~MiniScalarReplacementPass()
{
  ; // Nothing to delete yet
}

void MiniScalarReplacementPass::do_procedure_definition(ProcedureDefinition* p)
{
  procDef = p ;
  assert(procDef != NULL) ;
  OutputInformation("Mini scalar replacement pass begins") ;
  
  list<IfStatement*>* allIfs = 
    collect_objects<IfStatement>(procDef->get_body()) ;

  list<IfStatement*>::iterator ifIter = allIfs->begin() ;
  while(ifIter != allIfs->end())
  {
    ProcessIf(*ifIter) ;
    ++ifIter ;
  }

  delete allIfs ;

  OutputInformation("Mini scalar replacement pass ends") ;
}

void MiniScalarReplacementPass::ProcessIf(IfStatement* i)
{
  assert(i != NULL) ;

  // All of the places we have to look are in the store statements
  //  contained inside this if statement.

  list< std::pair<ArrayReferenceExpression*, VariableSymbol*> > 
    foundReferences ;

  list<StoreStatement*>* allStores = collect_objects<StoreStatement>(i) ;
  list<StoreStatement*>::iterator storeIter = allStores->begin() ;
  while (storeIter != allStores->end())
  {
    // Replace each store with a store variable statement
    Expression* originalValue = (*storeIter)->get_value() ;
    Expression* destAddress = (*storeIter)->get_destination_address() ;
    ArrayReferenceExpression* destRef = 
      dynamic_cast<ArrayReferenceExpression*>(destAddress) ;
    if (destRef == NULL)
    {
      // It must be a field access expression, so do not do anything
      ++storeIter ;
      continue ;
    }
    
    if (dynamic_cast<Constant*>(destRef->get_index()) != NULL)
    {
      // This will be transformed into a register in a different pass 
      ++storeIter ;
      continue ;
    }

    if (IsLookupTable(GetArrayVariable(destRef)))
    {
      // Lookup tables should not have mini scalar replacement performed
      //  on it 
      ++storeIter ;
      continue ;
    }
    
    // Get the variable symbol associated with this expression, 
    //  or create one if none exist.
    VariableSymbol* replacementVar = NULL ;

    list< std::pair<ArrayReferenceExpression*, VariableSymbol*> >::iterator
      foundIter = foundReferences.begin() ;
    while (foundIter != foundReferences.end())
    {
      if (EquivalentExpressions((*foundIter).first, destRef))
      {
	replacementVar = (*foundIter).second ;
	break ;
      }
      ++foundIter ;
    }

    if (replacementVar == NULL)
    {
      QualifiedType* elementType = GetQualifiedTypeOfElement(destRef) ;
      replacementVar = create_variable_symbol(theEnv,
					      elementType,
					      TempName("miniTmp")) ;
      procDef->get_symbol_table()->append_symbol_table_object(replacementVar) ;
      replacementVar->append_annote(create_brick_annote(theEnv, "NeedsFake")) ;
      replacementVar->append_annote(create_brick_annote(theEnv, "MiniReplaced")) ;

      std::pair<ArrayReferenceExpression*, VariableSymbol*> foundRef ;
      foundRef.first = destRef ;
      foundRef.second = replacementVar ;
      foundReferences.push_back(foundRef) ;
    }
    assert(replacementVar != NULL) ;

    (*storeIter)->set_value(NULL) ;
    originalValue->set_parent(NULL) ;

    StoreVariableStatement* replacement =
      create_store_variable_statement(theEnv,
				      replacementVar,
				      originalValue) ;
    (*storeIter)->get_parent()->replace((*storeIter), replacement) ;
    // Will this delete cause problems?
    //delete (*storeIter) ;
    ++storeIter ;
  }  
  delete allStores ;

  StatementList* replacementList = create_statement_list(theEnv) ;
  assert(replacementList != NULL) ; 

  // Create store statements for all replaced instances
  list< std::pair<ArrayReferenceExpression*, VariableSymbol*> >::iterator
    foundIter = foundReferences.begin() ;
  while (foundIter != foundReferences.end())
  {
    LoadVariableExpression* nextValue = 
      create_load_variable_expression(theEnv, 
			      (*foundIter).second->get_type()->get_base_type(),
			      (*foundIter).second) ;
    (*foundIter).first->set_parent(NULL) ;
    StoreStatement* nextStore =
      create_store_statement(theEnv, 
			     nextValue,
			     (*foundIter).first) ;
    replacementList->append_statement(nextStore) ;
    ++foundIter ;
  }
  StatementList* parent = dynamic_cast<StatementList*>(i->get_parent()) ;
  assert(parent != NULL) ;
  
  AddChildStatementAfter(parent, replacementList, i) ;  
}
