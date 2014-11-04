
#include <cassert>

#include <suifnodes/suif.h>
#include <suifnodes/suif_factory.h>
#include <suifkernel/utilities.h>

#include "roccc_utils/warning_utils.h"

#include "InitializationPass.h"

InitializationPass::InitializationPass(SuifEnv* pEnv) : 
  PipelinablePass(pEnv, "InitializationPass")
{
  theEnv = pEnv ;
  procDef = NULL ;
}

InitializationPass::~InitializationPass()
{
  ; // Nothing to delete yet
}

void InitializationPass::do_procedure_definition(ProcedureDefinition* p)
{
  procDef = p ;
  assert(procDef != NULL) ;

  OutputInformation("Initialization pass begins") ;
  
  SymbolTable* symTab = procDef->get_symbol_table() ;
  for (int i = 0 ; i < symTab->get_symbol_table_object_count() ; ++i)
  {
    SymbolTableObject* currentObject = symTab->get_symbol_table_object(i) ;
    if (currentObject->lookup_annote_by_name("InitializationPoint") != NULL)
    {
      AddInitialization(dynamic_cast<VariableSymbol*>(currentObject)) ;
    }
  }

  OutputInformation("Initialization pass ends") ;
}

void InitializationPass::AddInitialization(VariableSymbol* v)
{
  assert(v != NULL) ;

  // Only initialize scalars.  Do not initialize arrays or pointers.
  DataType* varType = v->get_type()->get_base_type() ;
  if (dynamic_cast<ArrayType*>(varType) != NULL ||
      dynamic_cast<PointerType*>(varType) != NULL)
  {
    return ;
  }

  // Also, do not adjust anything that has "const" as a qualifier
  QualifiedType* qualType = v->get_type() ;
  if (qualType->has_qualification_member("const"))
  {
    return ;
  }

  BrickAnnote* pointAnnote = 
  dynamic_cast<BrickAnnote*>(v->lookup_annote_by_name("InitializationPoint")) ;
  assert(pointAnnote != NULL) ;
  assert(pointAnnote->get_brick_count() == 1) ;
  SuifBrick* pointBrick = pointAnnote->get_brick(0) ;
  SuifObjectBrick* sobBrick = dynamic_cast<SuifObjectBrick*>(pointBrick) ;
  assert(sobBrick != NULL) ;
  StatementList* initializationPoint = 
    dynamic_cast<StatementList*>(sobBrick->get_object()) ;
  assert(initializationPoint != NULL) ;

  // Create an expression from the initialization block.  I need to get the
  //  definition from the definition block.
  ValueBlock* varBlock = NULL ;
  
  DefinitionBlock* procDefBlock = procDef->get_definition_block() ;
  assert(procDefBlock != NULL) ;
  Iter<VariableDefinition*> varDefIter = 
    procDefBlock->get_variable_definition_iterator() ;
  while (varDefIter.is_valid())
  {
    VariableDefinition* varDef = varDefIter.current() ;
    VariableSymbol* varSym = varDef->get_variable_symbol() ;
    if (varSym == v)
    {
      varBlock = varDef->get_initialization() ;
    }
    
    varDefIter.next() ;
  }

  if (varBlock == NULL)
  {
    return ;
  }

  ExpressionValueBlock* exprVarBlock = 
    dynamic_cast<ExpressionValueBlock*>(varBlock) ;
  assert(exprVarBlock != NULL) ;

  Expression* initExpr = exprVarBlock->get_expression() ;
  assert(initExpr != NULL) ;
  
  // Create a store variable statement
  StoreVariableStatement* initStore = 
    create_store_variable_statement(theEnv, v, 
			  dynamic_cast<Expression*>(initExpr->deep_clone())) ;

  initializationPoint->insert_statement(0, initStore) ;
}
