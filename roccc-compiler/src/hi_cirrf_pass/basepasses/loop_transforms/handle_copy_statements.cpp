
#include <cassert>

#include <basicnodes/basic.h>
#include <suifnodes/suif.h>
#include <basicnodes/basic_factory.h>
#include <suifnodes/suif_factory.h>

#include "handle_copy_statements.h"
#include "roccc_utils/warning_utils.h"
#include "roccc_utils/roccc2.0_utils.h"

HandleCopyStatements::HandleCopyStatements(SuifEnv* pEnv) : 
  PipelinablePass(pEnv, "HandleCopyStatements")
{
  theEnv = pEnv ;
  procDef = NULL ;
}

void HandleCopyStatements::do_procedure_definition(ProcedureDefinition* proc)
{
  procDef = proc ;
  assert(procDef != NULL) ;

  OutputInformation("Handle Copy Statements Begins") ;
  
  // There might not be an innermost loop, we could have a lot of 
  //  loops with statements in between...

  CForStatement* innermostLoop = InnermostLoop(procDef) ;
  StatementList* bodyList = NULL ;
  
  if (innermostLoop == NULL)
  {
    Statement* body = dynamic_cast<Statement*>(procDef->get_body()) ;
    bodyList = dynamic_cast<StatementList*>(body) ;
  }
  else
  {
    Statement* body = innermostLoop->get_body() ;
    bodyList = dynamic_cast<StatementList*>(body) ;
  }
  assert(bodyList != NULL) ;

  HandleCopies(bodyList) ;  

  OutputInformation("Handle Copy Statements Ends") ;
}

void HandleCopyStatements::HandleCopies(StatementList* bodyList)
{
  assert(theEnv != NULL) ;
  assert(procDef != NULL) ;
  assert(bodyList != NULL) ;

  int originalStatementLength ;
  Annote* unrolledAnnote = 
    procDef->lookup_annote_by_name("UnrolledWithCalls") ;
  if (unrolledAnnote != NULL)
  {
    BrickAnnote* unrolledBrick = dynamic_cast<BrickAnnote*>(unrolledAnnote) ;
    originalStatementLength = 
      dynamic_cast<IntegerBrick*>(unrolledBrick->get_brick(1))->get_value().c_int() ;
  }
  else
  {
    unrolledAnnote = procDef->lookup_annote_by_name("UnrolledNoCalls") ;
    if (unrolledAnnote == NULL)
    {
      OutputInformation("No loop unrolling occured") ;
      originalStatementLength = bodyList->get_statement_count() ; 
    }
    else
    {
      BrickAnnote* unrolledBrick = dynamic_cast<BrickAnnote*>(unrolledAnnote) ;
      originalStatementLength = 
	dynamic_cast<IntegerBrick*>(unrolledBrick->get_brick(0))->get_value().c_int() ;    
    }
  }

  // Go through each statement individually
  //  I have to assume all statement lists have been flattened and
  //  normalized.
  for (int i = 0 ; i < bodyList->get_statement_count() ; ++i)
  {
    if (dynamic_cast<IfStatement*>(bodyList->get_statement(i)) != NULL)
    {
      HandleIf(dynamic_cast<IfStatement*>(bodyList->get_statement(i)), 
	       bodyList, 
	       i) ;
      continue ;
    }

    // Collect each store variable statement
    list<StoreVariableStatement*>* allStores = 
      collect_objects<StoreVariableStatement>(bodyList->get_statement(i)) ;
    list<StoreVariableStatement*>::iterator storeIter = allStores->begin() ;
    while (storeIter != allStores->end())
    {
      VariableSymbol* outputVar = (*storeIter)->get_destination() ;
      if (outputVar != NULL)
      {
	VariableSymbol* dupe = CreateDuplicate(outputVar) ;
	dupe->append_annote(create_brick_annote(theEnv, "NeedsFake")) ;
	int replacedUses = 
	  ReplaceAllUsesWith(outputVar, dupe, bodyList, i + 1) ;
	if (replacedUses > 0 || 
	    (i < (bodyList->get_statement_count() - originalStatementLength)))
	{
	  (*storeIter)->set_destination(dupe) ;
	  if (outputVar->lookup_annote_by_name("NeedsFake") == NULL)
	  {
	    outputVar->append_annote(create_brick_annote(theEnv, "NeedsFake"));
	  }
	}
      }

      ++storeIter ;
    }
    delete allStores ;
  }

}

VariableSymbol* HandleCopyStatements::CreateDuplicate(VariableSymbol* original)
{
  assert(theEnv != NULL) ;
  assert(procDef != NULL) ;

  LString dupeTmpName = original->get_name() ;
  dupeTmpName = dupeTmpName + "_ssaTmp" ;

  VariableSymbol* dupe =
    create_variable_symbol(theEnv,
			   original->get_type(),
			   TempName(dupeTmpName)) ;
  procDef->get_symbol_table()->append_symbol_table_object(dupe) ;
  return dupe ;
}

int HandleCopyStatements::ReplaceAllUsesWith(VariableSymbol* original,
					     VariableSymbol* newSym,
					     StatementList* containingList,
					     int position)
{
  int replacedUses = 0 ;
  for (int i = position ; i < containingList->get_statement_count() ; ++i)
  {
    list<LoadVariableExpression*>* allUses = 
     collect_objects<LoadVariableExpression>(containingList->get_statement(i));

    list<LoadVariableExpression*>::iterator useIter = allUses->begin() ;
    while (useIter != allUses->end())
    {
      if ((*useIter)->get_source() == original)
      {
	(*useIter)->set_source(newSym) ;
	++replacedUses ;
      }
      ++useIter ;
    }
    delete allUses ;
    if (IsDefinition(containingList->get_statement(i), original))
    {
      break ;
    }
  }
  return replacedUses ;
}

 void HandleCopyStatements::HandleIf(IfStatement* s, 
				     StatementList* containingList, 
				     int position)
 {
   // What we have to do special here is make sure that we create one
   //  duplicate for both the then and the else portion
   
   Statement* thenPart = s->get_then_part() ;
   Statement* elsePart = s->get_else_part() ;
   
   // Denormalize if necessary
   if (dynamic_cast<StatementList*>(thenPart) != NULL)
   {
     thenPart = dynamic_cast<StatementList*>(thenPart)->get_statement(0) ;
   }
   if (dynamic_cast<StatementList*>(elsePart) != NULL)
   {
     elsePart = dynamic_cast<StatementList*>(elsePart)->get_statement(0) ;
   }

   // If they are not store variable statements, then just skip this.
   if (dynamic_cast<StoreVariableStatement*>(thenPart) == NULL ||
       dynamic_cast<StoreVariableStatement*>(elsePart) == NULL)
   {
     return ;
   }

   // These should both be store variable statements.

   StoreVariableStatement* thenStoreVar = 
     dynamic_cast<StoreVariableStatement*>(thenPart) ;
   StoreVariableStatement* elseStoreVar = 
     dynamic_cast<StoreVariableStatement*>(elsePart) ;

   assert(thenStoreVar != NULL) ;
   assert(elseStoreVar != NULL) ;

   VariableSymbol* original = thenStoreVar->get_destination() ;
   VariableSymbol* newSym = CreateDuplicate(original) ;

   thenStoreVar->set_destination(newSym) ;
   elseStoreVar->set_destination(newSym) ;

   ReplaceAllUsesWith(original, newSym, containingList, position + 1) ;
 }
