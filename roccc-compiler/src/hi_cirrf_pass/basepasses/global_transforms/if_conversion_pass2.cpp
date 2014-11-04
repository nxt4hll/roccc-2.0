// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
/*

  This code represents the restructuring of the original if conversion pass
   to include both struct accesses and direct stores to memory.
   Also, it should be written in a clearer and more algorithmically efficent
   manner.

*/

#include <cassert>

#include <basicnodes/basic_factory.h>
#include <suifnodes/suif_factory.h>
#include <cfenodes/cfe_factory.h>
#include <suifkernel/utilities.h>
#include <typebuilder/type_builder.h>

#include "roccc_utils/warning_utils.h"
#include "roccc_utils/roccc2.0_utils.h"
#include "if_conversion_pass2.h"

IfConversionPass2::IfConversionPass2(SuifEnv *pEnv) : 
  PipelinablePass(pEnv, "IfConversionPass2") 
{
  theEnv = pEnv ;
  tb = get_type_builder(theEnv) ;
}

void IfConversionPass2::do_procedure_definition(ProcedureDefinition* proc_def)
{
  procDef = proc_def ;
  assert(procDef != NULL) ;

  OutputInformation("If conversion pass 2.0 begins") ;

  bool changed ;

  do
  {
    changed = false ;
    list<IfStatement*>* allIfs =
      collect_objects<IfStatement>(procDef->get_body()) ;
    list<IfStatement*>::iterator ifIter = allIfs->begin() ;
    while (ifIter != allIfs->end())
    {
      changed |= ConvertIf(*ifIter) ;
      ++ifIter ;
    }
    delete allIfs ;

  } while (changed == true) ;

  OutputInformation("If conversion pass 2.0 ends") ;
}

bool IfConversionPass2::ConvertIf(IfStatement* toConvert)
{
  // First, verify that this if statement is in the correct format or not

  if (!VerifyIf(toConvert))
  {
    return false ;
  }

  // This should be in the right format, but I need to decide what to do
  Statement* thenPart = toConvert->get_then_part() ;
  Statement* elsePart = toConvert->get_else_part() ;
  thenPart = Denormalize(thenPart) ;
  elsePart = Denormalize(elsePart) ;
  
  if (dynamic_cast<StoreVariableStatement*>(thenPart) != NULL && 
      dynamic_cast<StoreVariableStatement*>(elsePart) != NULL)
  {
    return ConvertStoreVariableIf(toConvert) ;
  }
  else if (dynamic_cast<StoreStatement*>(thenPart) != NULL)
  {
    return ConvertStoreIf(toConvert) ;
  } 
  else if (dynamic_cast<CallStatement*>(thenPart) != NULL &&
	   dynamic_cast<CallStatement*>(elsePart) != NULL)
  {
    return ConvertCallsIf(toConvert) ;
  }
  else if (dynamic_cast<CallStatement*>(thenPart) != NULL &&
	   dynamic_cast<StoreVariableStatement*>(elsePart) != NULL)
  {
    return ConvertCallStoreVarIf(toConvert) ;
  }
  else if (dynamic_cast<StoreVariableStatement*>(thenPart) != NULL &&
	   dynamic_cast<CallStatement*>(elsePart) != NULL)
  {
    return ConvertStoreVarCall(toConvert) ;
  }
  else
  {
    OutputError("Unknown if format caused a problem!") ;
    FormattedText tmpText ;
    toConvert->print(tmpText) ;
    std::cout << tmpText.get_value() << std::endl ;
    assert(0) ;
  }
  return false ;
}

bool IfConversionPass2::VerifyIf(IfStatement* toConvert)
{
  Statement* thenPart = toConvert->get_then_part() ;
  Statement* elsePart = toConvert->get_else_part() ;

  if (thenPart == NULL || elsePart == NULL)
  {
    return false ;
  }

  StatementList* thenList = dynamic_cast<StatementList*>(thenPart) ;
  StatementList* elseList = dynamic_cast<StatementList*>(elsePart) ;
  
  if (thenList != NULL && thenList->get_statement_count() != 1)
  {
    return false ;
  }
  
  if (elseList != NULL && elseList->get_statement_count() != 1)
  {
    return false ;
  }

  thenPart = Denormalize(thenPart) ;
  elsePart = Denormalize(elsePart) ;

  return CorrectTypes(thenPart, elsePart) ;
}

Statement* IfConversionPass2::Denormalize(Statement* x)
{
  if (dynamic_cast<StatementList*>(x) != NULL)
  {
    StatementList* theList = dynamic_cast<StatementList*>(x) ;
    assert(theList->get_statement_count() == 1 && "Unsupported if detected!") ;
    return theList->get_statement(0) ;
  }
  else
  {
    return x ;
  }
}

bool IfConversionPass2::CorrectTypes(Statement* x, Statement* y)
{
  if (dynamic_cast<StoreStatement*>(x) != NULL &&
      dynamic_cast<StoreStatement*>(y) != NULL)
  {
    return true ;
  }
  if (dynamic_cast<StoreVariableStatement*>(x) != NULL &&
      dynamic_cast<StoreVariableStatement*>(y) != NULL)
  {
    StoreVariableStatement* xStoreVar = 
      dynamic_cast<StoreVariableStatement*>(x) ;
    StoreVariableStatement* yStoreVar = 
      dynamic_cast<StoreVariableStatement*>(y) ;

    return (xStoreVar->get_destination() == yStoreVar->get_destination()) ;
  }
  if (dynamic_cast<CallStatement*>(x) != NULL &&
      dynamic_cast<CallStatement*>(y) != NULL)
  {
    CallStatement* xCall = dynamic_cast<CallStatement*>(x) ;
    CallStatement* yCall = dynamic_cast<CallStatement*>(y) ;
    return ((xCall->get_destination() == yCall->get_destination()) && 
	    (xCall->get_destination() != NULL)) ;
  }

  // Now, the mixtures
  if (dynamic_cast<CallStatement*>(x) != NULL &&
      dynamic_cast<StoreVariableStatement*>(y) != NULL)
  {
    CallStatement* xCall = dynamic_cast<CallStatement*>(x) ;
    StoreVariableStatement* yStoreVar = 
      dynamic_cast<StoreVariableStatement*>(y) ;
    return ((xCall->get_destination() == yStoreVar->get_destination()) &&
	    (xCall->get_destination() != NULL)) ;
  }

  if (dynamic_cast<StoreVariableStatement*>(x) != NULL &&
      dynamic_cast<CallStatement*>(y) != NULL)
  {
    StoreVariableStatement* xStoreVar = 
      dynamic_cast<StoreVariableStatement*>(x) ;
    CallStatement* yCall = dynamic_cast<CallStatement*>(y) ;
    return ((xStoreVar->get_destination() == yCall->get_destination()) &&
	    (xStoreVar->get_destination() != NULL)) ;
  }

  return false ;
}

bool IfConversionPass2::ConvertStoreVariableIf(IfStatement* toConvert)
{
  Statement* thenPart = Denormalize(toConvert->get_then_part()) ;
  Statement* elsePart = Denormalize(toConvert->get_else_part()) ;

  StoreVariableStatement* thenStoreVar = 
    dynamic_cast<StoreVariableStatement*>(thenPart) ;
  StoreVariableStatement* elseStoreVar =
    dynamic_cast<StoreVariableStatement*>(elsePart) ;

  assert(thenStoreVar != NULL) ;
  assert(elseStoreVar != NULL) ;

  // Make sure that the variables are the same.
  if (thenStoreVar->get_destination() != elseStoreVar->get_destination())
  {
    return false ;
  }

  CallStatement* boolSelectCall = 
    CreateBoolCall(thenStoreVar->get_destination()) ;

  // Append the arguments
  Expression* thenStoreVarValue = thenStoreVar->get_value() ;
  thenStoreVar->set_value(NULL) ;
  boolSelectCall->append_argument(thenStoreVarValue) ;

  Expression* elseStoreVarValue = elseStoreVar->get_value() ;
  elseStoreVar->set_value(NULL) ;
  boolSelectCall->append_argument(elseStoreVarValue) ;

  Expression* condition = toConvert->get_condition() ;
  toConvert->set_condition(NULL) ;
  boolSelectCall->append_argument(condition) ;

  toConvert->get_parent()->replace(toConvert, boolSelectCall) ;  
  return true ;
}

CallStatement* IfConversionPass2::CreateBoolCall(VariableSymbol* destVar) 
{
  assert(theEnv != NULL) ;
  assert(tb != NULL) ;
  ProcedureSymbol* procSym = CreateBoolSym(destVar->get_type()) ;
  SymbolAddressExpression* procSymAddr = 
    create_symbol_address_expression(theEnv,
				     tb->get_pointer_type(procSym->get_type()),
				     procSym) ;

  return create_call_statement(theEnv, destVar, procSymAddr) ;
}


bool IfConversionPass2::ConvertStoreIf(IfStatement* toConvert)
{
  Statement* thenPart = Denormalize(toConvert->get_then_part()) ;
  StoreStatement* thenStore = dynamic_cast<StoreStatement*>(thenPart) ;
  assert(thenStore != NULL) ;

  Expression* destination = thenStore->get_destination_address() ;
  
  if (dynamic_cast<FieldAccessExpression*>(destination) != NULL)
  {
    return ConvertStructStoreIf(toConvert) ;    
  }
  else if (dynamic_cast<ArrayReferenceExpression*>(destination) != NULL)
  {
    return ConvertArrayStoreIf(toConvert) ;
  }
  else
  {
    OutputError("Unsupported if detected!") ;
    assert(0) ;
    return false ; // To avoid a warning
  }  
}

bool IfConversionPass2::ConvertStructStoreIf(IfStatement* toConvert)
{
  Statement* thenPart = Denormalize(toConvert->get_then_part()) ;
  Statement* elsePart = Denormalize(toConvert->get_else_part()) ;
  StoreStatement* thenStore = dynamic_cast<StoreStatement*>(thenPart) ;
  StoreStatement* elseStore = dynamic_cast<StoreStatement*>(elsePart) ;

  assert(thenStore != NULL) ;
  assert(elseStore != NULL) ;

  FieldAccessExpression* thenField =
    dynamic_cast<FieldAccessExpression*>(thenStore->get_destination_address());
  FieldAccessExpression* elseField = 
    dynamic_cast<FieldAccessExpression*>(elseStore->get_destination_address());

  assert(thenField != NULL) ;
  assert(elseField != NULL) ;

  // Check to make sure that each field access is accessing the same field
  if (thenField->get_field() != elseField->get_field())
  {
    return false ;
  }

  CallStatement* replacement = CreateBoolCall(thenField->get_field()) ;

  // Append the arguments
  Expression* thenStoreValue = thenStore->get_value() ;
  thenStore->set_value(NULL) ;
  replacement->append_argument(thenStoreValue) ;

  Expression* elseStoreValue = elseStore->get_value() ;
  elseStore->set_value(NULL) ;
  replacement->append_argument(elseStoreValue) ;

  Expression* condition = toConvert->get_condition() ;
  toConvert->set_condition(NULL) ;
  replacement->append_argument(condition) ;

  toConvert->get_parent()->replace(toConvert, replacement) ;  
  return true ;
}

bool IfConversionPass2::ConvertArrayStoreIf(IfStatement* toConvert)
{
  Statement* thenPart = Denormalize(toConvert->get_then_part()) ;
  Statement* elsePart = Denormalize(toConvert->get_else_part()) ;
  StoreStatement* thenStore = dynamic_cast<StoreStatement*>(thenPart) ;
  StoreStatement* elseStore = dynamic_cast<StoreStatement*>(elsePart) ;
  
  assert(thenStore != NULL) ;
  assert(elseStore != NULL) ;

  ArrayReferenceExpression* thenRef = 
    dynamic_cast<ArrayReferenceExpression*>(thenStore->get_destination_address()) ;
  ArrayReferenceExpression* elseRef = 
    dynamic_cast<ArrayReferenceExpression*>(elseStore->get_destination_address()) ;

  assert(thenRef != NULL) ;
  assert(elseRef != NULL) ;

  // Verify that these array references are to the same location.
  if (!EquivalentExpressions(thenRef, elseRef))
  {
    return false ;
  }

  // This will hopefully be scalar replaced later, but for now we just need
  //  to do something...
  //  What I need is the qualified element type of the array.  
  QualifiedType* qualType = create_qualified_type(theEnv,
						  thenRef->get_result_type()) ;

  StoreStatement* replacement = CreateBoolCall(thenRef) ;

  // Append the arguments to the call expression
  CallExpression* callExpr = 
    dynamic_cast<CallExpression*>(replacement->get_value()) ;
  assert(callExpr != NULL) ;

  Expression* thenStoreValue = thenStore->get_value() ;
  thenStore->set_value(NULL) ;
  callExpr->append_argument(thenStoreValue) ;

  Expression* elseStoreValue = elseStore->get_value() ;
  elseStore->set_value(NULL) ;
  callExpr->append_argument(elseStoreValue) ;

  Expression* condition = toConvert->get_condition() ;
  toConvert->set_condition(NULL) ;
  callExpr->append_argument(condition) ;

  // Replace the if
  toConvert->get_parent()->replace(toConvert, replacement) ;
  return true ;
}

// This function is responsible for creating the procedure symbol and
//  adding it to the appropriate symbol table.
ProcedureSymbol* IfConversionPass2::CreateBoolSym(QualifiedType* qualType)
{
  assert(procDef != NULL) ;
  assert(theEnv != NULL) ;
  assert(tb != NULL) ;

  // First, create the procedure type
  CProcedureType* cProcType = 
    create_c_procedure_type(theEnv,
			    qualType->get_base_type(),
			    false, // has varargs
			    true, // arguments known
			    0, // bit alignment,
			    TempName("ROCCC_boolsel_type")) ;


  // Next, add all of the arguments.  These should be two arguments with
  //  the same type as the destination variable and one boolean argument.
  cProcType->append_argument(qualType) ;
  cProcType->append_argument(qualType) ;
  cProcType->append_argument(create_qualified_type(theEnv,
						   tb->get_boolean_type())) ;
  
  // Now create the procedure symbol with this type and add it to 
  //  the external symbol table.
  ProcedureSymbol* procSym = 
    create_procedure_symbol(theEnv,
			    cProcType,
			    TempName("ROCCC_boolsel")) ;

  procDef->get_symbol_table()->append_symbol_table_object(cProcType) ;
  procDef->get_symbol_table()->append_symbol_table_object(procSym) ;

  return procSym ;
}

StoreStatement* IfConversionPass2::CreateBoolCall(ArrayReferenceExpression* dest)
{
  assert(tb != NULL) ;
  ProcedureSymbol* procSym = 
    CreateBoolSym(create_qualified_type(theEnv, dest->get_result_type())) ;

  SymbolAddressExpression* procAddress =
    create_symbol_address_expression(theEnv,
				     tb->get_pointer_type(procSym->get_type()),
				     procSym) ;

  CallExpression* callExpr = create_call_expression(theEnv,
						    dest->get_result_type(),
						    procAddress) ;

  dest->set_parent(NULL) ;
  return create_store_statement(theEnv,
				callExpr,
				dest) ;
}

// Both the if and then portion are call statements
bool IfConversionPass2::ConvertCallsIf(IfStatement* toConvert)
{
  Statement* thenPart = Denormalize(toConvert->get_then_part()) ;
  Statement* elsePart = Denormalize(toConvert->get_else_part()) ;

  CallStatement* thenCall = dynamic_cast<CallStatement*>(thenPart) ;
  CallStatement* elseCall = dynamic_cast<CallStatement*>(elsePart) ;

  assert(thenCall != NULL) ;
  assert(elseCall != NULL) ;

  CallStatement* boolSelectCall = 
    CreateBoolCall(thenCall->get_destination()) ;

  // Create call expression for each of the call statements
  //  and append them to the boolean select we are creating.

  Expression* thenCallValue = thenCall->get_callee_address() ;
  thenCall->set_callee_address(NULL) ;

  CallExpression* thenCallExp = 
    create_call_expression(theEnv,
			   thenCall->get_destination()->get_type()->get_base_type(),
			   thenCallValue) ;

  // Append all of the call arguments to the expression
  for (int i = 0 ; i < thenCall->get_argument_count() ; ++i)
  {
    Expression* nextArg = thenCall->get_argument(i) ;
    nextArg->set_parent(NULL) ;
    thenCallExp->append_argument(nextArg) ;
  }

  boolSelectCall->append_argument(thenCallExp) ;

  Expression* elseCallValue = elseCall->get_callee_address() ;
  elseCall->set_callee_address(NULL) ;
  CallExpression* elseCallExp =
    create_call_expression(theEnv,
			   thenCall->get_destination()->get_type()->get_base_type(),
			   elseCallValue) ;

  // Append all of the call arguments to the expression
  for (int i = 0 ; i < elseCall->get_argument_count() ; ++i)
  {
    Expression* nextArg = elseCall->get_argument(i) ;
    nextArg->set_parent(NULL) ;
    elseCallExp->append_argument(nextArg) ;
  }

  boolSelectCall->append_argument(elseCallExp) ;

  Expression* condition = toConvert->get_condition() ;
  toConvert->set_condition(NULL) ;
  boolSelectCall->append_argument(condition) ;

  toConvert->get_parent()->replace(toConvert, boolSelectCall) ;  
  return true ;
}

// Then is a call statement, else is a store variable statement
bool IfConversionPass2::ConvertCallStoreVarIf(IfStatement* toConvert)
{
  Statement* thenPart = Denormalize(toConvert->get_then_part()) ;
  Statement* elsePart = Denormalize(toConvert->get_else_part()) ;

  CallStatement* thenCall = dynamic_cast<CallStatement*>(thenPart) ;
  StoreVariableStatement* elseStoreVar = 
    dynamic_cast<StoreVariableStatement*>(elsePart) ;

  assert(thenCall != NULL) ;
  assert(elseStoreVar != NULL) ;

  CallStatement* boolSelectCall = 
    CreateBoolCall(thenCall->get_destination()) ;

  // Append the arguments  
  Expression* thenCallValue = thenCall->get_callee_address() ;
  thenCall->set_callee_address(NULL) ;
  CallExpression* thenCallExp = 
    create_call_expression(theEnv,
			   thenCall->get_destination()->get_type()->get_base_type(),
			   thenCallValue) ;

  // Append all of the call arguments to the expression
  for (int i = 0 ; i < thenCall->get_argument_count() ; ++i)
  {
    Expression* nextArg = thenCall->get_argument(i) ;
    nextArg->set_parent(NULL) ;
    thenCallExp->append_argument(nextArg) ;
  }

  boolSelectCall->append_argument(thenCallExp) ;

  Expression* elseStoreVarValue = elseStoreVar->get_value() ;
  elseStoreVar->set_value(NULL) ;
  boolSelectCall->append_argument(elseStoreVarValue) ;

  Expression* condition = toConvert->get_condition() ;
  toConvert->set_condition(NULL) ;
  boolSelectCall->append_argument(condition) ;

  toConvert->get_parent()->replace(toConvert, boolSelectCall) ;  
  return true ;
}

bool IfConversionPass2::ConvertStoreVarCall(IfStatement* toConvert)
{
  Statement* thenPart = Denormalize(toConvert->get_then_part()) ;
  Statement* elsePart = Denormalize(toConvert->get_else_part()) ;

  StoreVariableStatement* thenStoreVar = 
    dynamic_cast<StoreVariableStatement*>(thenPart) ;
  CallStatement* elseCall = dynamic_cast<CallStatement*>(elsePart) ;

  assert(thenStoreVar != NULL) ;
  assert(elseCall != NULL) ;

  CallStatement* boolSelectCall = 
    CreateBoolCall(thenStoreVar->get_destination()) ;

  // Append the arguments
  Expression* thenStoreVarValue = thenStoreVar->get_value() ;
  thenStoreVar->set_value(NULL) ;
  boolSelectCall->append_argument(thenStoreVarValue) ;

  Expression* elseCallValue = elseCall->get_callee_address() ;
  elseCall->set_callee_address(NULL) ;
  CallExpression* elseCallExp = 
    create_call_expression(theEnv,
			   elseCall->get_destination()->get_type()->get_base_type(),
			   elseCallValue) ;

  // Append all of the call arguments to the expression
  for (int i = 0 ; i < elseCall->get_argument_count() ; ++i)
  {
    Expression* nextArg = elseCall->get_argument(i) ;
    nextArg->set_parent(NULL) ;
    elseCallExp->append_argument(nextArg) ;
  }

  boolSelectCall->append_argument(elseCallExp) ;

  Expression* condition = toConvert->get_condition() ;
  toConvert->set_condition(NULL) ;
  boolSelectCall->append_argument(condition) ;

  toConvert->get_parent()->replace(toConvert, boolSelectCall) ;  
  return true ;
}
