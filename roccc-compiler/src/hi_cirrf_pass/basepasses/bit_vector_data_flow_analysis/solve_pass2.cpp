// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*

 This file defines the operations that take place to completely annotate
  the suif tree with data flow information.  This includes the changes
  due to ROCCC 2.0's model.

 In order to have this pass flow smoothly between original ROCCC code 
  and ROCCC 2.0 code we keep track of "killed_stmts" only for definitions.

 REQUIREMENTS: Control flow solve must be run directly before this pass.

*/

#include <cassert>
#include <basicnodes/basic_factory.h>
#include <suifkernel/utilities.h>

#include "solve_pass2.h"
#include "roccc_utils/bitVector2.h"
#include "roccc_utils/warning_utils.h"

DataFlowSolvePass2::DataFlowSolvePass2(SuifEnv *pEnv) : 
  PipelinablePass(pEnv, "DataFlowSolvePass2") 
{
  theEnv = pEnv ;
  totalDefinitions = 0 ;
}

DataFlowSolvePass2::~DataFlowSolvePass2()
{
  std::map<VariableSymbol*, list<std::pair<Statement*, int> >* >::iterator 
    delIter ;
  delIter = killMap.begin() ;
  while(delIter != killMap.end())
  {
    delete (*delIter).second ;
    ++delIter ;
  }
  killMap.clear() ;
}

void DataFlowSolvePass2::ClearMap()
{
 std::map<VariableSymbol*, list<std::pair<Statement*, int> >* >::iterator delIter ;
  delIter = killMap.begin() ;
  while(delIter != killMap.end())
  {
    delete (*delIter).second ;
    ++delIter ;
  }
  killMap.clear() ;
}

void DataFlowSolvePass2::do_procedure_definition(ProcedureDefinition* p)
{
  procDef = p ;

  // Reset totalDefinitions
  if (totalDefinitions != 0)
  {
    totalDefinitions = 0 ;
  }

  assert(theEnv != NULL) ;
  assert(procDef != NULL) ;
  assert(procDef->get_body() != NULL) ;

  OutputInformation("Data flow analysis 2.0 begins") ;

  // Initialize the map that associates each variable with all of 
  //  the possible definitions of that variable.
  InitializeMap() ;
  
  // At this point we should know the total number of definitions.
  //  We will use this information to create bit vectors for each statement.
  
  // Let us go through all of the statements and set up the necessary 
  //  annotations, cleaning up anything that might have existed before
  list<Statement*>* allStatements = 
    collect_objects<Statement>(procDef->get_body()) ;
  assert(allStatements != NULL) ;

  list<Statement*>::iterator annoteIter = allStatements->begin() ;
  while (annoteIter != allStatements->end()) 
  {
    SetupAnnotations(*annoteIter) ;    
    ++annoteIter ;
  }
  delete allStatements ;

  // Now perform the actual solving
  Statement* body = to<Statement>(procDef->get_body()) ;
  assert(body != NULL) ;
  SolveStatement(body) ;

  OutputInformation("Data flow analysis 2.0 ends") ;
}

void DataFlowSolvePass2::InitializeMap()
{
  CollectVariables() ;
  CollectDefinitions() ;
}

// If we are compiling a module we need to remember that there might not be
//  any variables declared other than the passed in struct.  All the variables
//  declared in the struct must have been used or they wouldn't have been
//  created.
void DataFlowSolvePass2::CollectVariables()
{
  // If the map isn't empty (this isn't the first time we've called this
  //  pass) then we need to clear the map

  if (killMap.empty() == false)
  {
    ClearMap() ;
  }

  // The map should be empty, so let's make sure
  assert(killMap.empty() == true) ;
  assert(procDef != NULL) ;

  // To set up the map, we must go through the symbol table and 
  //  identify all of the variable symbols.  For each one we need to
  //  associate a blank list that will be filled in later.

  SymbolTable* symTab = procDef->get_symbol_table() ;
  Iter<SymbolTableObject*> symIter = 
    symTab->get_symbol_table_object_iterator() ;

  while (symIter.is_valid())
  {
    if (is_a<VariableSymbol>(symIter.current()))
    {
      // For each variable symbol, create a new list of statement/int pairs
      //  and add that to the killMap.
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

      // If we have a struct type, we need to process this specially,
      //  otherwise we can do exactly what we did above
      
      DataType* parmType = nextParm->get_type()->get_base_type() ;
      StructType* parmStructType = 
	dynamic_cast<StructType*>(parmType) ;
      
      if (parmStructType == NULL)
      {
	list<std::pair<Statement*, int> >* assocList = 
	  new list<std::pair<Statement*, int> > ;
	assert(assocList != NULL) ;
	assert(dynamic_cast<VariableSymbol*>(symIter.current()) != NULL) ;
      
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

void DataFlowSolvePass2::CollectDefinitions()
{
  // If the kill map is empty, then this must be a completely empty function
  //  so this function won't do anything.  We used to assert here, but
  //  that is unnecessary.  
  //  assert(killMap.empty() == false) ;

  assert(procDef != NULL) ;

  // Now, we collect every possible definition and add them to the
  //  list associated with the variable that is being defined.  Each
  //  definition will also have an integer associated with it to 
  //  tag it.  
  
  // Definitions are only StoreVariableStatements and CallStatements
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

    assert(killMap[storedVar] != NULL) ;

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

void DataFlowSolvePass2::SetupAnnotations(Statement* s)
{
  CleanupOldAnnotations(s) ;
  AddNewAnnotations(s) ;
}

// The three annotations that we must remove are the old values
//  of "in_stmts", "out_stmts", and "killed_stmts".  Each one of these
//  will posses a single brick
//
// in_stmts:     keeps track of all definitions that reach this statement
// out_stmts:    keeps track of all definitions that are either generated or
//               not killed by this statement
// killed_stmts: keeps track of which definitions are no longer valid
//               after this statement

void DataFlowSolvePass2::CleanupOldAnnotations(Statement* s)
{
  if (s->lookup_annote_by_name("in_stmts"))
  {
    BrickAnnote* toRemove = 
      to<BrickAnnote>(s->remove_annote_by_name("in_stmts")) ;
    if (toRemove->get_brick_count() > 0)
    {
      delete dynamic_cast<BitVector2*>((to<SuifObjectBrick>(toRemove->get_brick(0)))->get_object()) ;
    }
    delete toRemove ;
  }
  if (s->lookup_annote_by_name("out_stmts"))
  {
    BrickAnnote* toRemove = 
      to<BrickAnnote>(s->remove_annote_by_name("out_stmts")) ;
    if (toRemove->get_brick_count() > 0)
    {
      delete dynamic_cast<BitVector2*>((to<SuifObjectBrick>(toRemove->get_brick(0)))->get_object()) ;
    }
    delete toRemove ;
  }
  if (s->lookup_annote_by_name("killed_stmts"))
  {
    BrickAnnote* toRemove = 
      to<BrickAnnote>(s->remove_annote_by_name("killed_stmts")) ;
    if (toRemove->get_brick_count() > 0)
    {
      delete dynamic_cast<BitVector2*>(to<SuifObjectBrick>(toRemove->get_brick(0))->get_object()) ;
    }
    delete toRemove ;
  }  
}

void DataFlowSolvePass2::AddNewAnnotations(Statement* currentStmt)
{
  assert(currentStmt != NULL) ;

  SuifEnv* theEnv = currentStmt->get_suif_env() ;
  
  // Now go through and make sure this statement has the appropriate
  //  annotations.

  BrickAnnote* inStmtAnnote   = create_brick_annote(theEnv, "in_stmts") ;
  BrickAnnote* outStmtAnnote  = create_brick_annote(theEnv, "out_stmts") ;

  assert(inStmtAnnote != NULL) ;
  assert(outStmtAnnote != NULL) ;
 
  currentStmt->append_annote(inStmtAnnote) ;
  currentStmt->append_annote(outStmtAnnote) ;

  // Each of these annotations is going to require a brick.  That brick is
  //  going to contain a pointer to a bit vector.  That bit vector will 
  //  initially be empty.  The bit vector refers to the number of definitions
  //  that reach that location.

  // Note: the memory allocated here needs to remain over passes, so 
  //       it will be reclaimed at the strip annotes pass.
  BitVector2* inVector = new BitVector2(totalDefinitions) ;
  BitVector2* outVector = new BitVector2(totalDefinitions) ;


  inStmtAnnote->append_brick(create_suif_object_brick(theEnv, inVector)) ;
  outStmtAnnote->append_brick(create_suif_object_brick(theEnv, outVector)) ;

  // To match up with Betul's code, we create a killed statement annote
  //  only for definitions
  if (dynamic_cast<CallStatement*>(currentStmt) != NULL ||
      dynamic_cast<StoreVariableStatement*>(currentStmt) != NULL)
  {
    BrickAnnote* killStmtAnnote = create_brick_annote(theEnv, "killed_stmts");
    assert(killStmtAnnote != NULL) ;
    currentStmt->append_annote(killStmtAnnote) ;
    BitVector2* killVector = new BitVector2(totalDefinitions) ;
    killStmtAnnote->append_brick(create_suif_object_brick(theEnv, killVector)) ;
  }
}

void DataFlowSolvePass2::UnionPredecessors(BitVector2* final, 
					   BrickAnnote* predecessors) 
{
  assert(final != NULL) ;
  assert(predecessors != NULL) ;

  // Go through all precessor blocks.
  Iter<SuifBrick*> brickIter = predecessors->get_brick_iterator() ;
  while (brickIter.is_valid())
  {
    // Get the Statement that corresponds to the precedecessor
    SuifObjectBrick* currentSob = 
      dynamic_cast<SuifObjectBrick*>(brickIter.current()) ;
    assert(currentSob != NULL) ;
    SuifObject* nextObj = currentSob->get_object() ;
    assert(nextObj != NULL) ;
    Statement* nextPred = dynamic_cast<Statement*>(nextObj) ;
    assert(nextPred != NULL) ;
    
    // Get the out_stmt that corresponds to that predecessor
    Annote* predOutStmtAnnote = nextPred->lookup_annote_by_name("out_stmts") ;
    assert(predOutStmtAnnote != NULL) ;
    BrickAnnote* predOutStmts = dynamic_cast<BrickAnnote*>(predOutStmtAnnote) ;
    assert(predOutStmts != NULL) ;
    // Get the BitVector that corresponds to that brick
    BitVector2* predVector = dynamic_cast<BitVector2*>
    (dynamic_cast<SuifObjectBrick*>(predOutStmts->get_brick(0))->get_object());
    assert(predVector != NULL) ;

    // Union the in statements with this predecessor
    final->merge(predVector) ;

    // Move on
    brickIter.next() ;
  }
  
}

void DataFlowSolvePass2::SolveStatement(Statement* s)
{
  // We might have inserted NULL statements in a previous pass, so 
  //  just skip them if we run into any.
  if (s == NULL)
  {
    return ;
  }
  if (dynamic_cast<EvalStatement*>(s) != NULL)
  {
    StandardSolve(s) ;
  }
  else if (dynamic_cast<IfStatement*>(s) != NULL)
  {
    SolveIfStatement(dynamic_cast<IfStatement*>(s)) ;
  }
  else if (dynamic_cast<WhileStatement*>(s) != NULL)
  {
    SolveWhileStatement(dynamic_cast<WhileStatement*>(s)) ;
  }
  else if (dynamic_cast<CForStatement*>(s) != NULL)
  {
    SolveCForStatement(dynamic_cast<CForStatement*>(s)) ;
  }
  else if (dynamic_cast<ScopeStatement*>(s) != NULL)
  {
    SolveScopeStatement(dynamic_cast<ScopeStatement*>(s)) ;
  }
  else if (dynamic_cast<StoreStatement*>(s) != NULL)
  {
    // For store statements we don't consider anything killed or generated.
    //  We are only dealing with variables and not generic memory.  This
    //  statement probably refers to an array access, which is handled
    //  separately in our passes.
    StandardSolve(s) ;
  }
  else if (dynamic_cast<StoreVariableStatement*>(s) != NULL)
  {
    SolveStoreVariableStatement(dynamic_cast<StoreVariableStatement*>(s)) ;
  }
  else if (dynamic_cast<CallStatement*>(s) != NULL)
  {
    SolveCallStatement(dynamic_cast<CallStatement*>(s)) ;
  }
  else if (dynamic_cast<JumpStatement*>(s) != NULL)
  {
    StandardSolve(s) ;
  }
  else if (dynamic_cast<BranchStatement*>(s) != NULL)
  {
    StandardSolve(s) ;
  }
  else if (dynamic_cast<StatementList*>(s) != NULL)
  {
    SolveStatementList(dynamic_cast<StatementList*>(s)) ;
  }
  else if (dynamic_cast<MarkStatement*>(s) != NULL)
  {
    StandardSolve(s) ;
  }
  else if (dynamic_cast<ReturnStatement*>(s) != NULL)
  {
    StandardSolve(s) ;
  }
  else if (dynamic_cast<LabelLocationStatement*>(s) != NULL)
  {
    StandardSolve(s) ;
  }
  else
  {
    assert(0) ;
  }
}

void DataFlowSolvePass2::StandardSolve(Statement* s)
{  
  // The output (gen set) should just be the union of all of my 
  //  predecessors out sets and any in_statements that we previously had

  BrickAnnote* inStmts = to<BrickAnnote>(s->lookup_annote_by_name("in_stmts"));
  BrickAnnote* predecessors = 
    to<BrickAnnote>(s->lookup_annote_by_name("predecessors")) ;
  BrickAnnote* outStmts = 
    to<BrickAnnote>(s->lookup_annote_by_name("out_stmts")) ;

  assert(inStmts != NULL) ;
  assert(predecessors != NULL) ;
  assert(outStmts != NULL) ;

  // Get the corresponding bit vectors for each of these annotes

  BitVector2* inVector = dynamic_cast<BitVector2*>
    (dynamic_cast<SuifObjectBrick*>(inStmts->get_brick(0))->get_object()) ;

  BitVector2* outVector = dynamic_cast<BitVector2*>
    (dynamic_cast<SuifObjectBrick*>(outStmts->get_brick(0))->get_object()) ;
  
  assert(inVector != NULL) ;
  assert(outVector != NULL) ;

  // outStmts = predecessors U inStmts
  UnionPredecessors(inVector, predecessors) ;

  outVector->copy(inVector) ;
}

// For if statements we must deal with both the "then" and the "else" case.
void DataFlowSolvePass2::SolveIfStatement(IfStatement* s)
{
  // As normal, collect all of the predecessors out statements into the in
  //  statements

  BrickAnnote* inStmts = to<BrickAnnote>(s->lookup_annote_by_name("in_stmts"));
  BrickAnnote* predecessors = 
    to<BrickAnnote>(s->lookup_annote_by_name("predecessors")) ;
  BrickAnnote* outStmts = 
    to<BrickAnnote>(s->lookup_annote_by_name("out_stmts")) ;

  assert(inStmts != NULL) ;
  assert(predecessors != NULL) ;
  assert(outStmts != NULL) ;

  BitVector2* inVector = dynamic_cast<BitVector2*>
    (dynamic_cast<SuifObjectBrick*>(inStmts->get_brick(0))->get_object()) ;

  BitVector2* outVector = dynamic_cast<BitVector2*>
    (dynamic_cast<SuifObjectBrick*>(outStmts->get_brick(0))->get_object()) ;

  assert(inVector != NULL) ;
  assert (outVector != NULL) ;

  UnionPredecessors(inVector, predecessors) ;

  // Now deal with the "then" portion
  Statement* thenStmt = s->get_then_part() ;
  SolveStatement(thenStmt) ;
  BrickAnnote* thenOut = to<BrickAnnote>(thenStmt->lookup_annote_by_name("out_stmts")) ;
  assert(thenOut != NULL) ;
  SuifBrick* thenBrick = thenOut->get_brick(0) ;
  SuifObjectBrick* thenSob = dynamic_cast<SuifObjectBrick*>(thenBrick) ;
  SuifObject* thenObj = thenSob->get_object() ;
  BitVector2* thenOutVector = dynamic_cast<BitVector2*>(thenObj) ;

  outVector->copy(thenOutVector) ;
 
  // If the "else" portion exists, solve it
  Statement* elseStmt = s->get_else_part() ;
  if (elseStmt != NULL)
  {
    SolveStatement(elseStmt) ;
    BrickAnnote* elseOut = to<BrickAnnote>(elseStmt->lookup_annote_by_name("out_stmts")) ;

    assert(elseOut != NULL) ;
    SuifBrick* elseBrick = elseOut->get_brick(0) ;
    SuifObjectBrick* elseSob = dynamic_cast<SuifObjectBrick*>(elseBrick) ;
    SuifObject* elseObj = elseSob->get_object() ;
    BitVector2* elseOutVector = dynamic_cast<BitVector2*>(elseObj) ;

    outVector->merge(elseOutVector) ;
  }
  else
  {
    outVector->merge(inVector) ;
  }

}

void DataFlowSolvePass2::SolveWhileStatement(WhileStatement* s)
{
  // In loops, we are our own predecessor, so we have to 
  //  propagate the information we generate to ourselves at least once
  //  before we can be sure the vectors have stabilized.  This means
  //  we must call SolveStatement(body) TWICE.  We need to 
  //  do this before we union the out statements of our predecessors because
  //  once again, we are our own predecessor.

  Statement* whileBody = s->get_body() ;
  SolveStatement(whileBody) ;
  SolveStatement(whileBody) ;

  // Now we can perform the standard solve
  StandardSolve(s) ;
}

void DataFlowSolvePass2::SolveCForStatement(CForStatement* s)
{
  // Similar to a while loop, we must solve the body and step
  //  multiple times to propagate the information.  We process the before
  //  only once because it is outside of the loop.
  SolveStatement(s->get_before()) ;
  SolveStatement(s->get_body()) ;
  SolveStatement(s->get_step()) ;
  SolveStatement(s->get_body()) ;
  SolveStatement(s->get_step()) ;

  // Now do the standard solve on the actual statement
  StandardSolve(s) ;
}

void DataFlowSolvePass2::SolveScopeStatement(ScopeStatement* s)
{
  // For a scope statement we must union the predecessors and then 
  //  solve the body.  The output statements of the body will be
  //  the output statements of the whole scope.

  BrickAnnote* inStmts = to<BrickAnnote>(s->lookup_annote_by_name("in_stmts"));
  BrickAnnote* predecessors = 
    to<BrickAnnote>(s->lookup_annote_by_name("predecessors")) ;
  BrickAnnote* outStmts = 
    to<BrickAnnote>(s->lookup_annote_by_name("out_stmts")) ;

  assert(inStmts != NULL) ;
  assert(predecessors != NULL) ;
  assert(outStmts != NULL) ;

  // Get the corresponding bit vectors for each of these annotes

  BitVector2* inVector = dynamic_cast<BitVector2*>
    (dynamic_cast<SuifObjectBrick*>(inStmts->get_brick(0))->get_object()) ;

  BitVector2* outVector = dynamic_cast<BitVector2*>
    (dynamic_cast<SuifObjectBrick*>(outStmts->get_brick(0))->get_object()) ;
  
  assert(inVector != NULL) ;
  assert (outVector != NULL) ;

  UnionPredecessors(inVector, predecessors) ;  

  // Now solve the body

  SolveStatement(s->get_body()) ;

  // Now get the out_stmts from the body and copy that to our outVector
  BrickAnnote* bodyBrickAnnote = 
    to<BrickAnnote>(s->get_body()->lookup_annote_by_name("out_stmts")) ;
  SuifBrick* bodyBrick = bodyBrickAnnote->get_brick(0) ;
  SuifObjectBrick* bodySob = dynamic_cast<SuifObjectBrick*>(bodyBrick) ;
  SuifObject* bodyObj = bodySob->get_object() ;
  BitVector2* bodyOutVector = dynamic_cast<BitVector2*>(bodyObj) ;

  outVector->copy(bodyOutVector) ;
  
  
}

// Now we are talking.  This statement is a definition and is responsible
//  for killing previous definitions and generating one definition.
// There is one variable associated with this statement and we must
//  retrieve all the definitions based off of the map we created earlier.
void DataFlowSolvePass2::SolveStoreVariableStatement(StoreVariableStatement* s)
{
  // Just as always, union the predecessors
  BrickAnnote* inStmts = to<BrickAnnote>(s->lookup_annote_by_name("in_stmts"));
  BrickAnnote* predecessors = 
    to<BrickAnnote>(s->lookup_annote_by_name("predecessors")) ;
  BrickAnnote* outStmts = 
    to<BrickAnnote>(s->lookup_annote_by_name("out_stmts")) ;

  assert(inStmts != NULL) ;
  assert(predecessors != NULL) ;
  assert(outStmts != NULL) ;

  // Get the corresponding bit vectors for each of these annotes

  BitVector2* inVector = dynamic_cast<BitVector2*>
    (dynamic_cast<SuifObjectBrick*>(inStmts->get_brick(0))->get_object()) ;

  BitVector2* outVector = dynamic_cast<BitVector2*>
    (dynamic_cast<SuifObjectBrick*>(outStmts->get_brick(0))->get_object()) ;
  
  assert(inVector != NULL) ;
  assert (outVector != NULL) ;
  UnionPredecessors(inVector, predecessors) ;

  // Copy the in vector 
  outVector->copy(inVector) ;

  // Now go through and remove all of the annotations that this kills
  //  and add the definition that we generate
  VariableSymbol* storedSymbol = s->get_destination() ;
  
  list< std::pair<Statement*, int> >* assocDefs = killMap[storedSymbol] ;
  int myNumber = -1 ;
  assert(assocDefs != NULL) ;
  list< std::pair<Statement*, int> >::iterator defIter = assocDefs->begin() ;
  while (defIter != assocDefs->end())
  {
    if( (*defIter).first == s)
    {
      myNumber = (*defIter).second ;
      outVector->mark(myNumber) ;
    }
    else
    {
      outVector->unmark((*defIter).second) ;
    }    
    ++defIter ;
  }
  assert(myNumber != -1) ;
}

// And here it is.  This is the whole reason for the rewrite.  Call statements
//  have a variable number of outputs.  I'm going to go through all the 
//  destinations and arguments and adjust the gen/kill set appropriately
void DataFlowSolvePass2::SolveCallStatement(CallStatement* s)
{
  // Just like everything else, we first must union the predecessors
  BrickAnnote* inStmts = to<BrickAnnote>(s->lookup_annote_by_name("in_stmts"));
  BrickAnnote* predecessors = 
    to<BrickAnnote>(s->lookup_annote_by_name("predecessors")) ;
  BrickAnnote* outStmts = 
    to<BrickAnnote>(s->lookup_annote_by_name("out_stmts")) ;

  assert(inStmts != NULL) ;
  assert(predecessors != NULL) ;
  assert(outStmts != NULL) ;

  // Get the corresponding bit vectors for each of these annotes

  BitVector2* inVector = dynamic_cast<BitVector2*>
    (dynamic_cast<SuifObjectBrick*>(inStmts->get_brick(0))->get_object()) ;

  BitVector2* outVector = dynamic_cast<BitVector2*>
    (dynamic_cast<SuifObjectBrick*>(outStmts->get_brick(0))->get_object()) ;
  
  assert(inVector != NULL) ;
  assert (outVector != NULL) ;

  UnionPredecessors(inVector, predecessors) ;

  outVector->copy(inVector) ;

  // Now deal with the destination just like the store variable statement
  //  Of course, the destination may or may not exist.
  VariableSymbol* destSymbol = s->get_destination() ;
  if (destSymbol != NULL)
  {
    list< std::pair<Statement*, int> >* assocDefs = killMap[destSymbol] ;
    int myNumber = -1 ;
    assert(assocDefs != NULL) ;
    list< std::pair<Statement*, int> >::iterator defIter = assocDefs->begin() ;
    while (defIter != assocDefs->end())
    {
      if( (*defIter).first == s)
      {
	myNumber = (*defIter).second ;
	outVector->mark(myNumber) ;
      }
      else
      {
	outVector->unmark((*defIter).second) ;
      }    
      ++defIter ;
    }
  }
  
  // Now, go through all the arguments.  If any of them are 
  //  SymbolAddressExpressions we must gen/kill them the same way as before.
  Iter<Expression*> argIter = s->get_argument_iterator() ;
  while (argIter.is_valid())
  {
    Expression* expArg = argIter.current() ;
    if (dynamic_cast<SymbolAddressExpression*>(expArg) != NULL)
    {
      Symbol* outputSymbol = 
       dynamic_cast<SymbolAddressExpression*>(expArg)->get_addressed_symbol() ;
      VariableSymbol* outputVariable = 
	dynamic_cast<VariableSymbol*>(outputSymbol) ;
      assert(outputVariable != NULL) ;

      list< std::pair<Statement*, int> >* assocDefs = killMap[outputVariable] ;
      int myNumber = -1 ;
      assert(assocDefs != NULL) ;
      list< std::pair<Statement*, int> >::iterator defIter=assocDefs->begin() ;
      while (defIter != assocDefs->end())
      {
	if( (*defIter).first == s)
	{
	  myNumber = (*defIter).second ;
	  outVector->mark(myNumber) ;
	}
	else
	{
	  outVector->unmark((*defIter).second) ;
	}    
	++defIter ;
      }
    }
    argIter.next() ;
  }

}

void DataFlowSolvePass2::SolveStatementList(StatementList* s)
{    
  // The output (gen set) should just be the union of all of my 
  //  predecessors out sets and any in_statements that we previously had

  BrickAnnote* inStmts = to<BrickAnnote>(s->lookup_annote_by_name("in_stmts"));
  BrickAnnote* predecessors = 
    to<BrickAnnote>(s->lookup_annote_by_name("predecessors")) ;
  BrickAnnote* outStmts = 
    to<BrickAnnote>(s->lookup_annote_by_name("out_stmts")) ;

  assert(inStmts != NULL) ;
  assert(predecessors != NULL) ;
  assert(outStmts != NULL) ;

  // Get the corresponding bit vectors for each of these annotes

  BitVector2* inVector = dynamic_cast<BitVector2*>
    (dynamic_cast<SuifObjectBrick*>(inStmts->get_brick(0))->get_object()) ;

  BitVector2* outVector = dynamic_cast<BitVector2*>
    (dynamic_cast<SuifObjectBrick*>(outStmts->get_brick(0))->get_object()) ;

  assert(inVector != NULL) ;
  assert(outVector != NULL) ;

  UnionPredecessors(inVector, predecessors) ;

  // Now, make sure to solve all of the statements that are
  //  internal to the statement list
  for (Iter<Statement*> childIter = s->get_child_statement_iterator() ;
       childIter.is_valid() ; 
       childIter.next())
  {
    SolveStatement(childIter.current()) ;
  }

  // The out vector of a statement list is either the out vector of the last
  //  statement in the list or (if the list is empty) the in stmts.
  if (s->get_statement_count() > 0)
  {
    Statement* lastStatement = s->get_statement(s->get_statement_count() - 1) ;
    Annote* lastAnnote = lastStatement->lookup_annote_by_name("out_stmts") ;
    BrickAnnote* lastBrickAnnote = dynamic_cast<BrickAnnote*>(lastAnnote) ;
    SuifBrick* lastBrick = lastBrickAnnote->get_brick(0) ;
    SuifObjectBrick* lastSob = dynamic_cast<SuifObjectBrick*>(lastBrick) ;
    SuifObject* lastObject = lastSob->get_object() ;
    BitVector2* lastVector = dynamic_cast<BitVector2*>(lastObject) ;
    assert(lastVector != NULL) ;

    outVector->copy(lastVector) ;
  }
  else
  {
    outVector->copy(inVector) ;
  }
}

void DataFlowSolvePass2::DumpDataFlow()
{
  assert(procDef != NULL) ;
  assert(theEnv != NULL) ;

  // I want to go through and output the bit vector of type of statement.
  //  First, the for loop
  // For loop looks o.k, now let me see about all of the calls
  
  list<CallStatement*>* allFors = 
    collect_objects<CallStatement>(procDef->get_body()) ;
  
  assert(allFors != NULL) ;

  list<CallStatement*>::iterator forIter ;
  forIter = allFors->begin() ;
  while (forIter != allFors->end())
  {
    Annote* forAnnote = (*forIter)->lookup_annote_by_name("out_stmts") ;
    assert(forAnnote != NULL) ;
    BrickAnnote* forBrick = dynamic_cast<BrickAnnote*>(forAnnote) ;
    assert(forBrick != NULL) ;
    Iter<SuifBrick*> brickIter = forBrick->get_brick_iterator() ;
    while (brickIter.is_valid())
    {
      SuifBrick* currentBrick = brickIter.current() ;
      SuifObjectBrick* sob = dynamic_cast<SuifObjectBrick*>(currentBrick) ;
      assert(sob != NULL) ;
      SuifObject* currentObj = sob->get_object() ;
      assert(currentObj != NULL) ;
      BitVector2* inVector = dynamic_cast<BitVector2*>(currentObj) ;
      assert(inVector != NULL) ;
      std::cerr << "In_stmts:" << inVector->getString() << std::endl ;
      brickIter.next() ;
    }
    
    ++forIter ;
  }

  delete allFors ;

}
