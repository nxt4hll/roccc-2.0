// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

#include <cassert>

#include <suifkernel/utilities.h>
#include <basicnodes/basic_factory.h>
#include <suifnodes/suif_factory.h>

#include "roccc_utils/warning_utils.h"
#include "roccc_utils/roccc2.0_utils.h"

#include "summation_pass.h"

SummationPass::SummationPass(SuifEnv* pEnv) : 
  PipelinablePass(pEnv, "SummationPass")
{
  theEnv = pEnv ;
  procDef = NULL ;
}

SummationPass::~SummationPass()
{
  ; // Nothing to delete yet
}

void SummationPass::do_procedure_definition(ProcedureDefinition* p)
{
  procDef = p ;
  assert(procDef != NULL) ;
  
  OutputInformation("ROCCC Summation Pass begins") ;

  StatementList* innermostList = InnermostList(procDef) ;
  assert(innermostList != NULL) ;

  for (int i = 0 ; i < innermostList->get_statement_count() ; ++i)
  {
    Statement* currentStatement = innermostList->get_statement(i) ;
    StoreStatement* currentStore = 
      dynamic_cast<StoreStatement*>(currentStatement) ;
    StoreVariableStatement* currentStoreVar = 
      dynamic_cast<StoreVariableStatement*>(currentStatement) ;
    if (currentStore != NULL)
    {
      ProcessStoreStatement(currentStore, innermostList, i-1) ;
    }
    if (currentStoreVar != NULL)
    {
      ProcessStoreVariableStatement(currentStoreVar, innermostList, i-1) ;
    }
  }

  OutputInformation("ROCCC Summation Pass ends") ;
}

void SummationPass::ProcessStoreStatement(StoreStatement* s, 
					  StatementList* p,
					  int pos)
{
  assert(s != NULL) ;
  assert(p != NULL) ;

  // Store statements either go to field accesses, which should not
  //  be read and written, or to arrays, which again should not be 
  //  read and written.
  return ;
}

void SummationPass::ProcessStoreVariableStatement(StoreVariableStatement* s,
						  StatementList* p,
						  int pos)
{
  assert(s != NULL) ;
  VariableSymbol* dest = s->get_destination() ;
  assert(dest != NULL) ;

  if (dynamic_cast<IntegerType*>(dest->get_type()->get_base_type()) == NULL)
  {
    return ;
  }

  if (IsSummation(s->get_value(), dest) && !HasPreviousUses(p, pos, dest)
      && !HasDefs(p, pos+2, dest) && !HasPreviousDefs(p, pos, dest))
  {
    Expression* remains = RemoveValue(s->get_value(), dest) ;
    assert(remains != NULL) ;
    s->set_value(NULL) ;
    remains->set_parent(NULL) ;
    
    CProcedureType* summationType = 
      create_c_procedure_type(theEnv,
			      dest->get_type()->get_base_type(), // return type
			      false, // has varargs
			      false, // arguments known
			      0, // bit alignment
			      TempName(LString("ROCCCSummationType"))) ;

    ProcedureSymbol* summationSymbol =
      create_procedure_symbol(theEnv,
			      summationType,
			      TempName(LString("ROCCCSummation"))) ;

    procDef->get_symbol_table()->append_symbol_table_object(summationType) ;
    procDef->get_symbol_table()->append_symbol_table_object(summationSymbol) ;

    SymbolAddressExpression* functionAddress = 
      create_symbol_address_expression(theEnv,
				       summationType->get_result_type(),
				       summationSymbol) ;

    // Create the summation call
    CallStatement* summationCall = 
      create_call_statement(theEnv, 
			    NULL,
			    functionAddress) ;
    
    SymbolAddressExpression* symDest = 
      create_symbol_address_expression(theEnv,
				       dest->get_type()->get_base_type(),
				       dest) ;

    // Create a new variable that will be used for initialization.
    summationCall->append_argument(symDest) ;
    ReferenceType* destRef = 
      create_reference_type(theEnv, IInteger(32), 0, dest->get_type()) ;
    procDef->get_symbol_table()->append_symbol_table_object(destRef) ;
    QualifiedType* destQual = 
      create_qualified_type(theEnv, destRef) ;
    summationType->append_argument(destQual) ;
    dest->append_annote(create_brick_annote(theEnv, "NonInputScalar")) ;
    dest->append_annote(create_brick_annote(theEnv, "NeedsFake")) ;

    summationCall->append_argument(remains) ;
    summationType->append_argument(create_qualified_type(theEnv, remains->get_result_type())) ;
    // Perform the replacement
    s->get_parent()->replace(s, summationCall) ;
    delete s ;
    //dest->append_annote(create_brick_annote(theEnv, "SummationVariable")) ;
    
  }

}

Expression* SummationPass::RemoveValue(Expression* original, VariableSymbol* v)
{

  // Find the binary expression that has the variable symbol in it.
  BinaryExpression* binExp = dynamic_cast<BinaryExpression*>(original) ;
  if (binExp == NULL)
  {
    return NULL ;
  }

  Expression* leftSide = binExp->get_source1() ;
  Expression* rightSide = binExp->get_source2() ;

  // Check to see if we are the correct format
  
  LoadVariableExpression* leftVar = 
    dynamic_cast<LoadVariableExpression*>(leftSide) ;
  LoadVariableExpression* rightVar = 
    dynamic_cast<LoadVariableExpression*>(rightSide) ;

  if (leftVar != NULL && leftVar->get_source() == v)
  {
    binExp->set_source2(NULL) ;
    //    delete binExp ;
    return rightSide ;
  }

  if (rightVar != NULL && rightVar->get_source() == v)
  {
    binExp->set_source1(NULL) ;
    //    delete binExp ;
    return leftSide ;
  }

  // Recursively go through and find the value.

  Expression* leftSideReplacement = RemoveValue(leftSide, v) ;
  if (leftSideReplacement != NULL)
  {
    binExp->set_source1(leftSideReplacement) ;
    return binExp ;
  }
  Expression* rightSideReplacement = RemoveValue(rightSide, v) ;
  if (rightSideReplacement != NULL)
  {
    binExp->set_source2(rightSideReplacement) ;
    return binExp ;
  }

  return NULL ;
}

bool SummationPass::IsSummation(Expression* e, VariableSymbol* v)
{

  BinaryExpression* binExp = dynamic_cast<BinaryExpression*>(e) ;
  
  if (binExp == NULL)
  {
    return false ;
  }
  
  LString opcode = binExp->get_opcode() ;
  if (opcode != LString("add"))
  {
    return false ;
  }

  Expression* leftSide = binExp->get_source1() ;
  Expression* rightSide = binExp->get_source2() ;

  LoadVariableExpression* leftLoadVar = 
    dynamic_cast<LoadVariableExpression*>(leftSide) ;
  LoadVariableExpression* rightLoadVar =
    dynamic_cast<LoadVariableExpression*>(rightSide) ;
  
  if (leftLoadVar != NULL && leftLoadVar->get_source() == v)
  {
    return true ;
  }

  if (rightLoadVar != NULL && rightLoadVar->get_source() == v)
  {
    return true ;
  }

  return IsSummation(leftSide, v) || IsSummation(rightSide, v) ;
}
