
#include <cassert>

#include <suifkernel/utilities.h>
#include <basicnodes/basic.h>
#include <basicnodes/basic_factory.h>
#include <suifnodes/suif.h>
#include <suifnodes/suif_factory.h>

#include "roccc_utils/warning_utils.h"
#include "roccc_utils/roccc2.0_utils.h"
#include "cleanup_store_statements.h"

CleanupStoreStatementsPass::CleanupStoreStatementsPass(SuifEnv* pEnv) :
  PipelinablePass(pEnv, "CleanupStoreStatementsPass")
{
  theEnv = pEnv ;
  procDef = NULL ;
}

void CleanupStoreStatementsPass::do_procedure_definition(ProcedureDefinition* 
							 proc_def)
{
  procDef = proc_def ;
  assert(procDef != NULL) ;

  OutputInformation("Cleanup store statements pass begins") ;

  list<StoreStatement*>* allStores = 
    collect_objects<StoreStatement>(procDef->get_body()) ;

  list<StoreStatement*>::iterator storeIter = allStores->begin() ;
  while (storeIter != allStores->end())
  {
    Expression* destAddress = (*storeIter)->get_destination_address() ;
    
    LoadVariableExpression* convertExp = 
      dynamic_cast<LoadVariableExpression*>(destAddress) ;
    if (convertExp != NULL)
    {
      // This store statement must be replaced with a store variable statement
      
      VariableSymbol* sym = convertExp->get_source() ;
      convertExp->set_source(NULL) ;

      Expression* value = (*storeIter)->get_value() ;
      (*storeIter)->set_value(NULL) ;
      StoreVariableStatement* replacement = 
	create_store_variable_statement(theEnv,
					sym,
					value) ;
      (*storeIter)->get_parent()->replace((*storeIter), replacement) ;
    }
    else if (dynamic_cast<SymbolAddressExpression*>(destAddress) != NULL)
    {
      SymbolAddressExpression* symAddr = 
	dynamic_cast<SymbolAddressExpression*>(destAddress) ;
      VariableSymbol* sym = 
	dynamic_cast<VariableSymbol*>(symAddr->get_addressed_symbol()) ;
      assert(sym != NULL) ;

      Expression* value = (*storeIter)->get_value() ;
      (*storeIter)->set_value(NULL) ;
      StoreVariableStatement* replacement = 
	create_store_variable_statement(theEnv,
					sym,
					value) ;
      (*storeIter)->get_parent()->replace((*storeIter), replacement) ;
    }

    ++storeIter ;
  }
  
  delete allStores ;

  OutputInformation("Cleanup store statements pass ends") ;
}
