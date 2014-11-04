
#include <cassert>

#include "roccc_utils/warning_utils.h"

#include <basicnodes/basic_factory.h>

#include "identify_parameters_pass.h"

IdentifyParametersPass::IdentifyParametersPass(SuifEnv* pEnv) :
  PipelinablePass(pEnv, "IdentifyParametersPass")
{
  theEnv = pEnv ;
  procDef = NULL ;
}

void IdentifyParametersPass::do_procedure_definition(ProcedureDefinition* p)
{
  procDef = p ;
  assert(procDef != NULL) ;

  OutputInformation("Identify parameters pass begins") ;

  SymbolTable* symTab = procDef->get_symbol_table() ;
  for (int i = 0 ; i < symTab->get_symbol_table_object_count() ; ++i)
  {
    SymbolTableObject* currentObject = symTab->get_symbol_table_object(i) ;
    ParameterSymbol* currentParam = 
      dynamic_cast<ParameterSymbol*>(currentObject) ;
    if (currentParam != NULL)
    {
      MarkParameter(currentParam) ;
    }
  }
  OutputInformation("Identify paramters pass ends") ;
}

void IdentifyParametersPass::MarkParameter(ParameterSymbol* p)
{
  assert(p != NULL) ;
  DataType* varType = p->get_type()->get_base_type() ;
  PointerType* pType =
    dynamic_cast<PointerType*>(varType) ;
  ReferenceType* rType =
    dynamic_cast<ReferenceType*>(varType) ;

  if (pType != NULL)
  {
    return ;
  }

  if (rType != NULL)
  {
    if (dynamic_cast<PointerType*>(dynamic_cast<QualifiedType*>(rType->get_reference_type())->get_base_type()) != NULL)
    {
      p->append_annote(create_brick_annote(theEnv, "OutputFifo")) ;
    }
    else
    {
      p->append_annote(create_brick_annote(theEnv, "OutputScalar")) ;
    }
    return ;
  }

  p->append_annote(create_brick_annote(theEnv, "InputScalar")) ;

}
