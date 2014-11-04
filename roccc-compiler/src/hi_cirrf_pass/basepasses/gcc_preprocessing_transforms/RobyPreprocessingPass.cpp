
#include <cassert>

#include <suifkernel/utilities.h>

#include "roccc_utils/warning_utils.h"

#include "RobyPreprocessingPass.h"

RobyPreprocessingPass::RobyPreprocessingPass(SuifEnv* pEnv) :
  PipelinablePass(pEnv, "RobyPreprocessingPass")
{
  theEnv = pEnv ;
  procDef = NULL ;
}

RobyPreprocessingPass::~RobyPreprocessingPass()
{
  ; // Nothing to delete yet
}

void RobyPreprocessingPass::do_procedure_definition(ProcedureDefinition* p)
{
  procDef = p ;
  assert(procDef != NULL) ;

  OutputInformation("Roby Preprocessing Pass begins") ;

  std::cout << "COMPONENT_NAME " << p->get_procedure_symbol()->get_name() 
	    << std::endl ;

  list<LabelLocationStatement*>* allLabels = 
    collect_objects<LabelLocationStatement>(procDef->get_body()) ;
  list<LabelLocationStatement*>::iterator labelIter = allLabels->begin() ;
  while (labelIter != allLabels->end())
  {
    std::cout << "LABEL " << (*labelIter)->get_defined_label()->get_name()
	      << std::endl ;
    ++labelIter ;
  }
  delete allLabels ;

  list<CallStatement*>* allCalls = 
    collect_objects<CallStatement>(procDef->get_body()) ;
  list<CallStatement*>::iterator callIter = allCalls->begin() ;
  while (callIter != allCalls->end())
  {
    SymbolAddressExpression* symAddr = 
     dynamic_cast<SymbolAddressExpression*>((*callIter)->get_callee_address());
    if (symAddr != NULL)
    {
      std::cout << "FUNCTION "
		<< symAddr->get_addressed_symbol()->get_name() 
		<< std::endl ;
    }
    ++callIter ;
  }
  delete allCalls ;

  SymbolTable* symTab = procDef->get_symbol_table() ;
  for (int i = 0 ; i < symTab->get_symbol_table_object_count() ; ++i)
  {
    SymbolTableObject* nextObject = symTab->get_symbol_table_object(i) ;
    VariableSymbol* nextVar = dynamic_cast<VariableSymbol*>(nextObject) ;
    if (nextVar != NULL)
    {
      DataType* baseType = nextVar->get_type()->get_base_type() ;
      if (dynamic_cast<ArrayType*>(baseType) != NULL ||
	  dynamic_cast<PointerType*>(baseType) != NULL)
      {
	std::cout << "STREAM " << nextVar->get_name() << std::endl ;	  
      }
    }
  }

  OutputInformation("Roby Preprocessing Pass ends") ;
}
