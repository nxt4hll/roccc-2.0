// The ROCCC Compiler Infrastructure
//  this file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

#include <cassert>

#include <basicnodes/basic_factory.h>

#include "roccc_utils/warning_utils.h"
#include "roccc_utils/roccc2.0_utils.h"

#include "lutDetectionPass.h"

LUTDetectionPass::LUTDetectionPass(SuifEnv* pEnv) : 
  PipelinablePass(pEnv, "LookupTableIdentification") 
{
  theEnv = pEnv ;
  procDef = NULL ;
}

void LUTDetectionPass::do_procedure_definition(ProcedureDefinition* p)
{
  procDef = p ;
  assert(procDef != NULL) ;
  OutputInformation("LUT Detection Pass begins") ;

  // LUTs can only exist in New Style Systems or Modules
  if (isLegacy(procDef))
  {    
    OutputInformation("Legacy code - No LUTs supported") ;
    return ;
  }    
  
  // LUTs are defined to be arrays that are not parameter symbols
  SymbolTable* symTab = procDef->get_symbol_table() ;
  for (int i = 0 ; i < symTab->get_symbol_table_object_count() ; ++i)
  {
    SymbolTableObject* currentObject = symTab->get_symbol_table_object(i) ;
    VariableSymbol* currentVar = 
      dynamic_cast<VariableSymbol*>(currentObject) ;
    ParameterSymbol* currentParam =
      dynamic_cast<ParameterSymbol*>(currentObject) ;
    if (currentVar != NULL &&
	dynamic_cast<ArrayType*>(currentVar->get_type()->get_base_type()) != NULL &&
	currentParam == NULL &&
	currentVar->lookup_annote_by_name("ConstPropArray") == NULL)
    {
      // Found one!  Let's mark it!
      currentObject->append_annote(create_brick_annote(theEnv, "LUT")) ;
    }
  }

  OutputInformation("LUT Detection Pass ends") ;
}


