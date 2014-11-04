// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

#include <cassert>

#include <suifkernel/utilities.h>
#include <basicnodes/basic.h>
#include <basicnodes/basic_factory.h>
#include <suifnodes/suif.h>
#include <suifnodes/suif_factory.h>

#include "roccc_utils/warning_utils.h"
#include "roccc_utils/roccc2.0_utils.h"

#include "reference_cleanup_pass.h"

ReferenceCleanupPass::ReferenceCleanupPass(SuifEnv* pEnv) : 
  PipelinablePass(pEnv, "ReferenceCleanupPass")
{
  theEnv = pEnv ;
  procDef = NULL ;
}

ReferenceCleanupPass::~ReferenceCleanupPass()
{
  ; // Nothing to delete yet
}

void ReferenceCleanupPass::do_procedure_definition(ProcedureDefinition* p)
{
  procDef = p ;
  assert(procDef != NULL) ;
  
  OutputInformation("Reference Cleanup pass begins") ;

  CleanupCalls() ;
  //  CleanupArrayStores() ;
  
  OutputInformation("Reference Cleanup pass ends") ;
}

void ReferenceCleanupPass::CleanupCalls()
{
  assert(procDef != NULL) ;
  list<CallStatement*>* allCalls = 
    collect_objects<CallStatement>(procDef->get_body()) ;
  list<CallStatement*>::iterator callIter = allCalls->begin() ;
  while (callIter != allCalls->end())
  {
    CleanupCall(*callIter) ;
    ++callIter ;
  }
  delete allCalls ;
}

void ReferenceCleanupPass::CleanupCall(CallStatement* c)
{
  assert(procDef != NULL) ;
  assert(c != NULL) ;

  // We only need to clean up module calls.  If they are built in
  //  functions, like boolsel, we don't want to do this.
  if (IsBuiltIn(c))
  {
    return ;
  }
  
  // Go through the arguments and see if any of them are load variable
  //  expressions to a reference typed variable, and replace those with
  //  symbol address expressions
  for (unsigned int i = 0 ; i < c->get_argument_count() ; ++i)
  {
    Expression* currentArg = c->get_argument(i) ;
    LoadVariableExpression* currentLoadVar = 
      dynamic_cast<LoadVariableExpression*>(currentArg) ;
    if (currentLoadVar != NULL)
    {
      VariableSymbol* currentVar = currentLoadVar->get_source() ;
      DataType* varType = currentVar->get_type()->get_base_type() ;
      ReferenceType* refType = dynamic_cast<ReferenceType*>(varType) ;
      if (refType != NULL)
      {
	QualifiedType* internalType = 
	  dynamic_cast<QualifiedType*>(refType->get_reference_type()) ;
	assert(internalType != NULL) ;
	//	currentVar->set_type(internalType) ;
	SymbolAddressExpression* symAddrExp = 
	  create_symbol_address_expression(theEnv, 
					   internalType->get_base_type(),
					   currentVar) ;
	if (currentLoadVar->lookup_annote_by_name("UndefinedPath") != NULL)
	{
	  symAddrExp->append_annote(create_brick_annote(theEnv, "UndefinedPath")) ;
	}
	currentLoadVar->get_parent()->replace(currentLoadVar, symAddrExp) ;
      }
    }
  }
}

void ReferenceCleanupPass::CleanupArrayStores()
{
  assert(procDef != NULL) ;

  list<StoreStatement*>* allStores =
    collect_objects<StoreStatement>(procDef->get_body()) ;
  list<StoreStatement*>::iterator storeIter = allStores->begin() ;
  while (storeIter != allStores->end())
  {
    CleanupArrayStore(*storeIter) ;
    ++storeIter ;
  }
  delete allStores ;
}

// Turn an reference pointer into an array reference expression
void ReferenceCleanupPass::CleanupArrayStore(StoreStatement* s)
{
  assert(s != NULL) ;
  
  // Check to see if the destination is a reference variable
  Expression* destination = s->get_destination_address() ;

  VariableSymbol* storedVariable = FindVariable(destination) ;

  if (storedVariable == NULL)
  {
    return ;
  }

  if (dynamic_cast<ReferenceType*>(storedVariable->get_type()->get_base_type()))
  {
    // Can I just change the type?  Pointer conversion should take care of it
    //  then, but I'll have to annotate it
    ReferenceType* refType = 
      dynamic_cast<ReferenceType*>(storedVariable->get_type()->get_base_type()) ;
    QualifiedType* internalType = 
      dynamic_cast<QualifiedType*>(refType->get_reference_type()) ;
    assert(internalType != NULL) ;

    DataType* internalType2 = internalType->get_base_type() ;
    QualifiedType* qualType = storedVariable->get_type() ;
    qualType->set_base_type(NULL) ;
    refType->set_parent(NULL) ;
    internalType->set_parent(NULL) ;
    refType->set_reference_type(NULL) ;
    qualType->set_base_type(internalType2) ;
  }
}

VariableSymbol* ReferenceCleanupPass::FindVariable(Expression* e)
{
  LoadVariableExpression* lve = dynamic_cast<LoadVariableExpression*>(e) ;
  BinaryExpression* be = dynamic_cast<BinaryExpression*>(e) ;
    
  if (lve != NULL)
  {
    return lve->get_source() ;
  }

  if (be != NULL)
  {
    return FindVariable(be->get_source1()) ;
  }

  FormattedText tmpText ;
  e->print(tmpText) ;
  std::cout << tmpText.get_value() << std::endl ;

  return NULL ;
}
