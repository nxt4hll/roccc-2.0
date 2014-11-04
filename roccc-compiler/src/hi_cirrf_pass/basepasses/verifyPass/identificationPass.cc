#include <cassert>

#include <basicnodes/basic_factory.h>

#include "roccc_utils/warning_utils.h"

#include "identificationPass.h"

const int MODULE = 1 ;
const int SYSTEM = 2 ;
const int COMPOSED_SYSTEM = 3 ;
const int UNDETERMINABLE = 99 ;

IdentificationPass::IdentificationPass(SuifEnv* pEnv) :
  PipelinablePass(pEnv, "IdentificationPass")
{
  theEnv = pEnv ;
  procDef = NULL ;
}

IdentificationPass::~IdentificationPass()
{
}

void IdentificationPass::do_procedure_definition(ProcedureDefinition* p)
{
  procDef = p ;
  assert(procDef != NULL) ;

  OutputInformation("Identification pass begins") ;
  int functionType = DetermineType() ;

  assert(functionType != UNDETERMINABLE && 
	 "Function is not a recognized ROCCC format!") ;

  // Clean up any that might already exist
  if (procDef->lookup_annote_by_name("FunctionType") != NULL)
  {
    delete procDef->remove_annote_by_name("FunctionType") ;
  }

  BrickAnnote* functionBrickAnnote = 
    create_brick_annote(theEnv, "FunctionType") ;
  functionBrickAnnote->append_brick(create_integer_brick(theEnv, functionType));  
  procDef->append_annote(functionBrickAnnote) ;
  
  OutputInformation("Identification pass ends") ;
}

// Currently, there are three types of code ROCCC will support:
//  Type 1: Modules (blocks)
//  Type 2: Systems (modules)
//  Type 3: Composed Systems
int IdentificationPass::DetermineType()
{
  if (IsModule())
  {
    return MODULE ;
  }
  
  if (IsComposedSystem())
  {
    return COMPOSED_SYSTEM ;
  }

  // Everything else is a system
  return SYSTEM ;
}

bool IdentificationPass::IsModule()
{
  assert(procDef != NULL) ;

  ProcedureSymbol* procSym = procDef->get_procedure_symbol() ;
  ProcedureType* procType = procSym->get_type() ;
  CProcedureType* cProcType = dynamic_cast<CProcedureType*>(procType) ;
  assert(cProcType != NULL) ;
  
  // There should only be one argument, and it should be a struct
  if (cProcType->get_argument_count() != 1)
  {
    return false ;
  }

  if (dynamic_cast<StructType*>(cProcType->get_argument(0)->get_base_type()) 
      == NULL)
  {
    return false ;
  }

  return true ;

}

// All composed systems should be marked by this time
bool IdentificationPass::IsComposedSystem()
{
  assert(procDef != NULL) ;
  return (procDef->lookup_annote_by_name("ComposedSystem") != NULL) ;
}
