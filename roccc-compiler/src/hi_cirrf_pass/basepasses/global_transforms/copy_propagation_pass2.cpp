// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include <cassert>
#include <iostream>

#include <suifkernel/utilities.h>
#include <utils/expression_utils.h>
#include <cfenodes/cfe.h>
#include <basicnodes/basic_factory.h>

#include <roccc_utils/bitVector2.h>
#include <roccc_utils/warning_utils.h>

#include "roccc_utils/roccc2.0_utils.h"

#include "copy_propagation_pass2.h"

CopyPropagationPass2::CopyPropagationPass2(SuifEnv *pEnv) : 
  PipelinablePass(pEnv, "CopyPropagationPass2") 
{
  theEnv = pEnv ;
  procDef = NULL ;
}

void CopyPropagationPass2::do_procedure_definition(ProcedureDefinition* proc_def)
{
  procDef = proc_def ;
  assert(procDef != NULL) ;

  OutputInformation("Copy propagation pass 2.0 begins") ;
  totalDefinitions = 0 ;

  Initialize() ;

  if (Special())
  {
    ProcessSpecialIfs() ;
  }
  else
  { 
    // First, find all the possible copy instructions.  These
    //  can only be store variable statements

    list<StoreVariableStatement*>* allStoreVars =
      collect_objects<StoreVariableStatement>(procDef->get_body()) ;
    assert(allStoreVars != NULL) ;

    list<StoreVariableStatement*>::iterator storeVarIter = 
      allStoreVars->begin();
    while(storeVarIter != allStoreVars->end())
    {
      ProcessPossibleCopy(*storeVarIter) ;
      ++storeVarIter ;
    }
  
    delete allStoreVars ;
  }
  
  CleanUp() ;

  OutputInformation("Copy propagation pass 2.0 ends") ;
}

void CopyPropagationPass2::Initialize()
{
  // I don't want to delete any of the statements, but I must clear
  //  out the list I was using to contain the statements
  if (!toBeRemoved.empty())
  {
    while (toBeRemoved.begin() != toBeRemoved.end())
    {
      toBeRemoved.pop_front() ;
    }
  }

  assert(procDef != NULL) ;

  InitializeMap() ;

  // Also, collect all of the feedback variables for later
  
  SymbolTable* procSymTable = procDef->get_symbol_table() ;
  assert(procSymTable != NULL) ;

  for (int i = 0 ; i < procSymTable->get_symbol_table_object_count() ; ++i)
  {
    if(dynamic_cast<VariableSymbol*>(procSymTable->get_symbol_table_object(i)) != NULL)
    {
      VariableSymbol* nextSymbol = 
	dynamic_cast<VariableSymbol*>(procSymTable->get_symbol_table_object(i));
      if (nextSymbol->lookup_annote_by_name("FeedbackVariable") != NULL)
      {
	feedbackVariables.push_back(nextSymbol) ;
      }
    }
  }

}

void CopyPropagationPass2::ProcessPossibleCopy(StoreVariableStatement* c)
{
  assert(c != NULL) ;
  // If this isn't a straight copy, just return
  LoadVariableExpression* replacement = 
    dynamic_cast<LoadVariableExpression*>(c->get_value()) ;
  if (replacement == NULL)
  {
    return ;
  }

  // If the variables are different types, don't propagate this away 
  //  (it is a cast)
  DataType* destType = c->get_destination()->get_type()->get_base_type() ;
  DataType* sourceType = replacement->get_source()->get_type()->get_base_type();
  if (!EquivalentTypes(destType, sourceType))
  {
    return ;
  }
     
  // Find all the reached uses
  BrickAnnote* reachedUses = 
    to<BrickAnnote>(c->lookup_annote_by_name("reached_uses")) ;

  assert(reachedUses != NULL) ;

  // Just in case we have no reached uses, we don't want to do 
  //  dead code elimination in this pass as well...
  bool removable = false ;

  Iter<SuifBrick*> useIter = reachedUses->get_brick_iterator() ;

  // First verify that we are the only definition for all of our uses.
  //  If we aren't then we can't make the replacement
  while(useIter.is_valid())
  {
    SuifObjectBrick* sob = to<SuifObjectBrick>(useIter.current()) ;
    assert (sob != NULL) ;
    LoadVariableExpression* nextLoad = 
      dynamic_cast<LoadVariableExpression*>(sob->get_object()) ;
    assert(nextLoad != NULL) ;
    if (IsOnlyDefinition(c, nextLoad) == false)
    {
      return ;
    }
    useIter.next() ;
  }

  // We also need to make sure that for each reached use, the copy is
  //  not redefined.  We do this by checking to make sure all of the
  //  definitions associated with it in the kill map are identical.
  //  This is a little conservative, but will make sure that no incorrect
  //  code is created.


  // Get the bit vector associated with all the reaching definitions
  //  coming into this statement
  BrickAnnote* inStatements = 
    dynamic_cast<BrickAnnote*>(c->lookup_annote_by_name("in_stmts")) ;
  assert(inStatements != NULL) ;
  SuifBrick* inBrick = inStatements->get_brick(0) ;
  assert(inBrick != NULL) ;
  SuifObjectBrick* inSOB = dynamic_cast<SuifObjectBrick*>(inBrick) ;
  assert(inSOB != NULL) ;
  SuifObject* inObj = inSOB->get_object() ;
  assert(inObj != NULL) ;
  BitVector2* inBits = dynamic_cast<BitVector2*>(inObj) ;
  assert(inBits != NULL) ;

  VariableSymbol* replacementVariable = replacement->get_source() ;
  assert(replacementVariable != NULL) ;

  list<std::pair<Statement*, int> >* definitions =killMap[replacementVariable];
  assert(definitions != NULL) ;

  list<bool> activeDefinitions ;
  list<std::pair<Statement*, int> >::iterator defIter = definitions->begin() ;
  while (defIter != definitions->end())
  {
    activeDefinitions.push_back(inBits->isMarked((*defIter).second)) ;
    ++defIter ;
  }

  useIter = reachedUses->get_brick_iterator() ;
  while (useIter.is_valid())
  {
    SuifObjectBrick* sob = to<SuifObjectBrick>(useIter.current()) ;
    assert(sob != NULL) ;
    LoadVariableExpression* nextLoad = 
      dynamic_cast<LoadVariableExpression*>(sob->get_object()) ;
    assert(nextLoad != NULL) ;
    SuifObject* loadParent = nextLoad->get_parent() ;
    while (dynamic_cast<Statement*>(loadParent) == NULL && loadParent != NULL)
    {
      loadParent = loadParent->get_parent() ;
    }
    assert(loadParent != NULL) ;
    Statement* parentStatement = dynamic_cast<Statement*>(loadParent) ;
    assert(parentStatement != NULL) ;
    BrickAnnote* parentInAnnote = dynamic_cast<BrickAnnote*>
      (parentStatement->lookup_annote_by_name("in_stmts")) ;
    assert(parentInAnnote != NULL) ;
    SuifBrick* parentInBrick = parentInAnnote->get_brick(0) ;
    assert(parentInBrick != NULL) ;
    SuifObjectBrick* parentInSOB = 
      dynamic_cast<SuifObjectBrick*>(parentInBrick) ;
    assert(parentInSOB != NULL) ;
    SuifObject* parentInObj = parentInSOB->get_object() ;
    assert(parentInObj != NULL) ;
    BitVector2* parentInBits = dynamic_cast<BitVector2*>(parentInObj) ;
    assert(parentInBits != NULL) ;

    defIter = definitions->begin() ;
    list<bool>::iterator activeIter = activeDefinitions.begin() ;
    while (defIter != definitions->end())
    {
      if ((*activeIter) != parentInBits->isMarked((*defIter).second))
      {
	// They are different, so don't do the replacement.
	return ;
      }
      ++activeIter ;
      ++defIter ;
    }

    useIter.next() ; 
  }
  
  // Now go through each reached use and replace it with the copy.
  //  Each reached use should be a load variable expression.
  //  We also have to deal with any feedback variables
  useIter = reachedUses->get_brick_iterator() ;
  while (useIter.is_valid())
  {
    SuifObjectBrick* sob = to<SuifObjectBrick>(useIter.current()) ;
    assert(sob != NULL) ;
    LoadVariableExpression* nextLoad = 
      dynamic_cast<LoadVariableExpression*>(sob->get_object()) ;
    assert(nextLoad != NULL) ;

    // Keep track of if we need to handle feedback variables
    HandleFeedbackVariables(nextLoad, replacement->get_source()) ;
    
    nextLoad->set_source(replacement->get_source()) ;

    removable = true ;

    useIter.next() ;
  }

  if (removable)
  {
    toBeRemoved.push_back(c) ;
  }  

}

void CopyPropagationPass2::CleanUp()
{
  // The parent should be a statement list.  Let us make sure...
  list<Statement*>::iterator removeIter = toBeRemoved.begin() ;
  while (removeIter != toBeRemoved.end())
  {

    SuifObject* parent = (*removeIter)->get_parent() ;

    // The parent could be a statement list (the easy case)
    //  or it could be a conditional with a single statement (if, while, etc.)
    //  in which case we need to do a little more work.

    StatementList* parentList = dynamic_cast<StatementList*>(parent) ;
    IfStatement* parentIf = dynamic_cast<IfStatement*>(parent) ;
    CForStatement* parentFor = dynamic_cast<CForStatement*>(parent) ;

    if (parentList != NULL)
    {
      RemoveFromStatementList(parentList, (*removeIter)) ;
    }
    else if (parentIf != NULL)
    {
      assert(0 && "Not yet supported") ;
    }
    else if (parentFor != NULL)
    {
      assert(0 && "Not yet supported") ;
    }
    else
    {
      assert(0 && "Trying to remove a statement from an unknown parent") ;
    }

    ++removeIter ;
  }
}

bool CopyPropagationPass2::IsOnlyDefinition(StoreVariableStatement* def,
					    LoadVariableExpression* use)
{
  BrickAnnote* defAnnote =
    to<BrickAnnote>(use->lookup_annote_by_name("reaching_defs")) ;
  assert(defAnnote != NULL) ;

  if (defAnnote->get_brick_count() != 1)
  {
    return false ;
  }

  SuifBrick* defBrick = defAnnote->get_brick(0) ;
  SuifObjectBrick* sob = dynamic_cast<SuifObjectBrick*>(defBrick) ;
  assert(sob != NULL) ;

  if (def == sob->get_object())
  {
    return true ;
  }
  else
  {
    return false ;
  }
}

void CopyPropagationPass2::RemoveFromStatementList(StatementList* parent,
						   Statement* child)
{
  assert(parent != NULL) ;
  assert(child != NULL) ;

  // Find the position of statement and remove it
  int pos = 0 ; 
  Iter<Statement*> statementIter = parent->get_statement_iterator() ;
  while (statementIter.is_valid())
  {
    if (statementIter.current() == child)
    {
      break ;
    }
    statementIter.next() ;
    ++pos ;
  }

  assert(parent->get_statement(pos) == child) ;
  parent->remove_statement(pos) ;
}

void CopyPropagationPass2::ProcessSpecialIfs()
{
  list<IfStatement*>* allIfs = 
    collect_objects<IfStatement>(procDef->get_body()) ;

  assert(allIfs != NULL) ;

  list<IfStatement*>::iterator ifIter = allIfs->begin() ;
  while (ifIter != allIfs->end())
  {

    Statement* elsePart = (*ifIter)->get_else_part() ;
    assert(elsePart != NULL) ;

    StatementList* elseList = dynamic_cast<StatementList*>(elsePart) ;
    if (elseList == NULL)
    {
      ++ifIter ;
      continue ;
    }

    assert(elseList != NULL) ;

    if (elseList->get_statement_count() == 2)
    {
      // Process this if statement
      Statement* thenPart = (*ifIter)->get_then_part() ;
      assert(thenPart != NULL) ;
      /*
      StatementList* thenList = dynamic_cast<StatementList*>(thenPart) ;
      assert(thenList != NULL) ;

      assert(thenList->get_statement_count() == 1) ;
      Statement* thenStatement = thenList->get_statement(0) ;
      assert(thenStatement != NULL) ;
      */
      StoreVariableStatement* thenStoreVar = 
	dynamic_cast<StoreVariableStatement*>(thenPart) ;
      assert(thenStoreVar != NULL) ;
      
      Statement* firstElseStatement = elseList->get_statement(0) ;
      Statement* secondElseStatement = elseList->get_statement(1) ;
      assert(firstElseStatement != NULL && secondElseStatement != NULL) ;

      // We are definitely going to break the rules here
      //  We know that the destination has to be replaced with 
      //  the source

      StoreVariableStatement* secondElseStore = 
	dynamic_cast<StoreVariableStatement*>(secondElseStatement) ;

      assert(secondElseStore != NULL) ;

      Expression* source = secondElseStore->get_value() ;
      assert(source != NULL) ;
      
      LoadVariableExpression* sourceLoadExp = 
	dynamic_cast<LoadVariableExpression*>(source) ;
      assert(sourceLoadExp != NULL) ;

      VariableSymbol* sourceVariable = sourceLoadExp->get_source() ;
      assert(sourceVariable != NULL) ;

      // First, find the use of the then portion and replace that use
      //  with the source variable
      BrickAnnote* ba = 
	to<BrickAnnote>(thenStoreVar->lookup_annote_by_name("reached_uses")) ;
      assert(ba != NULL) ;

      assert(ba->get_brick_count() == 1) ;

      Iter<SuifBrick*> tmpIter = ba->get_brick_iterator() ;
      
      SuifObjectBrick* sob = 
	to<SuifObjectBrick>(tmpIter.current()) ;
      assert(sob != NULL) ;

      SuifObject* finalDest = sob->get_object() ;

      LoadVariableExpression* finalLoad =
	dynamic_cast<LoadVariableExpression*>(finalDest) ;
      assert(finalLoad != NULL) ;

      // Before we make the change, mark the variable we are replacing as
      //  removed.
      finalLoad->get_source()->append_annote(create_brick_annote(theEnv, "RemovedVariable")) ;

      finalLoad->set_source(sourceVariable) ;

      // Now, change the then portion
      thenStoreVar->set_destination(sourceVariable) ;

      // Now, remove the second else statement
      elseList->remove_statement(1) ;

      // We should be done.

    }

    ++ifIter ;
  }

  delete allIfs ;
}


// This determines if we are in the middle of a systolic array transformation.
//  In this case, we have certain unfinished buisness that we need to clean up
bool CopyPropagationPass2::Special()
{
  // Check to see if we are in the special position.  In order for this 
  //  to happen, we need to see if any if statement exists with an
  //  else portion that has two statements.

  bool toReturn = false ;
 
  list<IfStatement*>* allIfs = 
    collect_objects<IfStatement>(procDef->get_body()) ;
  assert(allIfs != NULL) ;

  list<IfStatement*>::iterator ifIter = allIfs->begin() ;

  while (ifIter != allIfs->end())
  {

    Statement* elsePart = (*ifIter)->get_else_part() ;
    if (dynamic_cast<StatementList*>(elsePart) != NULL &&
	dynamic_cast<StatementList*>(elsePart)->get_statement_count() == 2)
    {
      toReturn = true ;
    }

    ++ifIter ;
  }

  delete allIfs ;

  return toReturn ;
}

void CopyPropagationPass2::HandleFeedbackVariables(LoadVariableExpression* n,
						   VariableSymbol* replacement)
{
  assert(n != NULL) ;
  assert(replacement != NULL) ;

  VariableSymbol* toReplace = n->get_source() ;

  // Go through all the feedback variables
  list<VariableSymbol*>::iterator feedbackIter = feedbackVariables.begin() ;
  while (feedbackIter != feedbackVariables.end())
  {
    BrickAnnote* nextAnnote = 
      to<BrickAnnote>((*feedbackIter)->lookup_annote_by_name("FeedbackVariable")) ;
    assert(nextAnnote != NULL) ;

    SuifBrick* one = nextAnnote->get_brick(0) ;
    SuifBrick* two = nextAnnote->get_brick(1) ;

    SuifObjectBrick* firstObj = dynamic_cast<SuifObjectBrick*>(one) ;
    SuifObjectBrick* secondObj = dynamic_cast<SuifObjectBrick*>(two) ;
    assert(firstObj != NULL) ;
    assert(secondObj != NULL) ;

    VariableSymbol* dest = 
      dynamic_cast<VariableSymbol*>(firstObj->get_object()) ;
    VariableSymbol* source = 
      dynamic_cast<VariableSymbol*>(secondObj->get_object()) ;
    
    assert(dest != NULL) ;
    assert(source != NULL) ;

    if (dest == toReplace)
    {
      firstObj->set_object(replacement) ;
    }

    if (source == toReplace)
    {
      secondObj->set_object(replacement) ;
    }

    ++feedbackIter ;
  }

}


// These are copied from data flow solve 2 pass.  It turns out they are
//  necessary.
void CopyPropagationPass2::ClearMap()
{
  std::map<VariableSymbol*, list<std::pair<Statement*, int> >* >::iterator
    delIter ;
  delIter = killMap.begin() ;
  while (delIter != killMap.end())
  {
    delete (*delIter).second ;
    ++delIter ;
  }
  killMap.clear() ;
}

void CopyPropagationPass2::InitializeMap()
{
  CollectVariables() ;
  CollectDefinitions() ;
}

void CopyPropagationPass2::CollectVariables()
{
  assert(procDef != NULL) ;
  if (killMap.empty() == false)
  {
    ClearMap() ;
  }
  assert(killMap.empty() == true) ;
  assert(procDef != NULL) ;
  SymbolTable* symTab = procDef->get_symbol_table() ;
  Iter<SymbolTableObject*> symIter = 
    symTab->get_symbol_table_object_iterator() ;
  while (symIter.is_valid())
  {
    if (is_a<VariableSymbol>(symIter.current()))
    {
      // For each variable symbol, create a new list of statement/int pairs
      //  and add that to the kill map
      list<std::pair<Statement*, int> >* assocList = 
	new list<std::pair<Statement*, int> > ;
      killMap[to<VariableSymbol>(symIter.current())] = assocList ;
    }
    else if (is_a<ParameterSymbol>(symIter.current()))
    {
      // If we are compiling a module, we also have to find the argument
      //  (which should be a struct) and collect all of those variable
      //  symbols..
      ParameterSymbol* nextParm =
	dynamic_cast<ParameterSymbol*>(symIter.current()) ;
      assert(nextParm != NULL) ;

      // Now check that we have a struct type
      DataType* parmType = nextParm->get_type()->get_base_type() ;
      StructType* parmStructType =
        dynamic_cast<StructType*>(parmType) ;

      if (parmStructType == NULL)
      {
	list<std::pair<Statement*, int> >* assocList =
	  new list<std::pair<Statement*, int> > ;
	killMap[dynamic_cast<VariableSymbol*>(symIter.current())] = assocList ;
	symIter.next() ;
	continue ;
      }

      // Go through this symbol table, just like the parent table...
      SymbolTable* structSymTab = parmStructType->get_group_symbol_table() ;

      Iter<SymbolTableObject*> structSymIter =
        structSymTab->get_symbol_table_object_iterator() ;

      while (structSymIter.is_valid())
      {
	// These should all be variable symbols!
	if (is_a<FieldSymbol>(structSymIter.current()))
	{
	  // For each variable symbol, create a new list of statement/int pairs
	  //  and add that to the killMap.
	  list<std::pair<Statement*, int> >* assocList =
	    new list<std::pair<Statement*, int> > ;
	  
	  killMap[to<VariableSymbol>(structSymIter.current())] = assocList ;
	}
	else
	{
	  std::cerr << "Non variable symbol inside a struct!" << std::endl ;
	  assert(0) ;
	}
	
	structSymIter.next() ;
      }
    }
    symIter.next() ;
  }
}

void CopyPropagationPass2::CollectDefinitions()
{
  assert(procDef != NULL) ;

  list<StoreVariableStatement*>* allStoreVariables =
    collect_objects<StoreVariableStatement>(procDef->get_body()) ;
  assert(allStoreVariables != NULL) ;
  list<StoreVariableStatement*>::iterator storeVarIter =
    allStoreVariables->begin() ;
  while (storeVarIter != allStoreVariables->end())
  {
    VariableSymbol* storedVar = (*storeVarIter)->get_destination() ;
    
    std::pair<Statement*, int> toAdd ;
    toAdd.first = (*storeVarIter) ;
    toAdd.second = totalDefinitions ;
    
    // Get the list associated with this and push back a new pair
    (killMap[storedVar])->push_back(toAdd) ;
    ++totalDefinitions ;

    ++storeVarIter ;
  }
  delete allStoreVariables ;
  // Now do the same thing for call statements                                  
  list<CallStatement*>* allCalls =
    collect_objects<CallStatement>(procDef->get_body()) ;
  assert(allCalls != NULL) ;
  list<CallStatement*>::iterator callIter = allCalls->begin() ;
  while(callIter != allCalls->end())
  {
    // For call statements we must check both the destination and
    //  all of the arguments
    VariableSymbol* dest = (*callIter)->get_destination() ;
    if (dest != NULL)
    {
      std::pair<Statement*, int> toAdd ;
      toAdd.first = (*callIter) ;
      toAdd.second = totalDefinitions ;
      
      (killMap[dest])->push_back(toAdd) ;

      ++totalDefinitions ;
    }
    for(unsigned int i = 0 ; i < (*callIter)->get_argument_count() ; ++i)
    {
      Expression* nextArg = (*callIter)->get_argument(i) ;
      if (dynamic_cast<SymbolAddressExpression*>(nextArg) != NULL)
      {
	// This is an output variable, so get the associated variable
	Symbol* nextSym = to<SymbolAddressExpression>(nextArg)->get_addressed_symbol() ;
	VariableSymbol* nextVarSym = dynamic_cast<VariableSymbol*>(nextSym) ;
	assert(nextVarSym != NULL) ;
	
	std::pair<Statement*, int> toAdd ;
	toAdd.first = (*callIter) ;
	toAdd.second = totalDefinitions ;
	
	// Get the list associated with this and push back a new pair
	(killMap[nextVarSym])->push_back(toAdd) ;
	++totalDefinitions ;
      }
    }
    ++callIter ;
  }
  delete allCalls ;
}
