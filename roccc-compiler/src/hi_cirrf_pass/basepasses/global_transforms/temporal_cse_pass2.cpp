// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

#include <sstream>
#include <map>

#include <suifkernel/utilities.h>
#include <utils/symbol_utils.h>
#include <basicnodes/basic_factory.h>
#include <suifnodes/suif_factory.h>

#include "roccc_utils/warning_utils.h"
#include "roccc_utils/roccc2.0_utils.h"
#include "temporal_cse_pass2.h"

TemporalCSEPass::TemporalCSEPass(SuifEnv *pEnv) : 
  PipelinablePass(pEnv, "TemporalCSEPass") 
{
  theEnv = pEnv ;
  procDef = NULL ;
  tmpNum = 0 ;
  stride = 0 ;
  maxLevels = 0 ;
}

void TemporalCSEPass::do_procedure_definition(ProcedureDefinition* proc_def)
{
  procDef = proc_def ;
  assert(procDef != NULL) ;

  OutputInformation("Temporal common subexpression elimination begins") ;

  // Temporal common subexpression elimination must happen in loops, and
  //  we assume perfectly nested loops, so we need to collect and process 
  //  the innermost for loop.

  CForStatement* innermostLoop = InnermostLoop(procDef) ;

  if (innermostLoop == NULL)
  {
    OutputInformation("Temporal common subexpression elmination ends") ;
    return ;
  }
  
  // We currently only work on one-dimensional loops and arrays
  if (!IsOutermostLoop(innermostLoop))
  {
    SetStrideMultiDimensional(innermostLoop) ;
    ProcessLoopStatementsMulti(innermostLoop) ;
    OutputInformation("Temporal common subexpression for array reuse"
		      " currently not supported"
  		      " on multidimensional loops") ;
    return ;
  }
  
  // Determine how much we are increasing every loop iteration.  We will
  //  need to match this exactly in order to determine equivalent statements.
  SetStride(innermostLoop) ;

  // All of the expressions removed by this pass are identical across
  //  loop bounds.

  // The first type we are looking for is uses of an array that is stored 
  //  in the current iteration.  The use has to be before the definition:
  // .... = B[i-i]
  // B[i] = .... 
  HandleArrayReuse(innermostLoop) ;

  // There are two types of common subexpressions that we will eliminate.
  //  I will assume that all statements in the code are assignments 
  //  (even call statements).  The first type of common subexpressions we look
  //  for is the same computation on the right hand side of the = operator
  //  across loop boundaries.  The second type of common subexpression we look
  //  for is a inter-iteration dependency for a certain depth.
  //  We will look both forward and backward in time (loop iterations).
  ProcessLoopStatements(innermostLoop) ;

  OutputInformation("Temporal common subexpression elimination ends") ;
}

bool TemporalCSEPass::ProperFormat(CForStatement* c)
{
  // There are two things I need to check in this function.  First, 
  //  we are not writing to the same location multiple times.  Second,
  //  we are never reading and writing to the exact same location.
  list<ArrayReferenceExpression*> destinations ;

  list<StoreStatement*>* allStores = 
    collect_objects<StoreStatement>(c->get_body()) ;
  list<StoreStatement*>::iterator storeIter = allStores->begin() ;
  while (storeIter != allStores->end())
  {
    Expression* dest = (*storeIter)->get_destination_address() ;
    ArrayReferenceExpression* arrayDest = 
      dynamic_cast<ArrayReferenceExpression*>(dest) ;
    assert(dest != NULL) ;
    
    list<ArrayReferenceExpression*>::iterator destIter = destinations.begin() ;
    while (destIter != destinations.end())
    {
      if (EquivalentExpressions(arrayDest, (*destIter)))
      {
	delete allStores ;
	return false ;
      }
      ++destIter ;
    }
    destinations.push_back(arrayDest) ;
    ++storeIter ;
  }
  delete allStores ;

  list<LoadExpression*>* allLoads = 
    collect_objects<LoadExpression>(c->get_body()) ;
  list<LoadExpression*>::iterator loadIter = allLoads->begin() ;
  while (loadIter != allLoads->end())
  {
    ArrayReferenceExpression* currentRef = 
      dynamic_cast<ArrayReferenceExpression*>((*loadIter)->
					      get_source_address());
    assert(currentRef != NULL) ;
    list<ArrayReferenceExpression*>::iterator destIter = destinations.begin() ;
    while (destIter != destinations.end())
    {
      if (EquivalentExpressions(currentRef, (*destIter)))
      {
	delete allLoads ;
	return false ;
      }
      ++destIter ;
    }
    ++loadIter ;
  }
  delete allLoads ;

  return true ;
}

bool TemporalCSEPass::InList(list<ArrayReferenceExpression*>& x,
			     ArrayReferenceExpression* y)
{
  list<ArrayReferenceExpression*>::iterator findIter = x.begin() ;
  while (findIter != x.end())
  {
    if (EquivalentExpressions((*findIter), y))
    {
      return true ;
    }
    ++findIter ;
  }
  return false ;
}

// The purpose of this function is to forwardly propagate reuses of array
//  accesses.  This is for instances where arrays are both read and written
//  to in the code.  We normally don't allow this, but in this instance 
//  we are going to allow it.
void TemporalCSEPass::HandleArrayReuse(CForStatement* c)
{
  assert(c != NULL) ;

  StatementList* bodyList = dynamic_cast<StatementList*>(c->get_body()) ;
  assert(bodyList != NULL) ;
  
  if (!ProperFormat(c))
  {
    OutputWarning("Warning: You are attempting to perform TCSE on improper"
		  " code!") ;
    return ;
  }

  list<ArrayReferenceExpression*> candidates ;

  // Go statement by statement and determine if any array references
  //  fit the pattern and can be replaced with a scalar.

  int i = 0 ;
  while (i < bodyList->get_statement_count())
  {
    Statement* currentStatement = bodyList->get_statement(i) ;

    // Add all of the loads to the list that are not currently in the list
    //  to the list of candidates.
    AddLoads(candidates, currentStatement) ;

    // Replace Equivalences across loop boundaries
    i += ReplaceEquivalences(currentStatement, candidates, bodyList, i) ;

    // Remove exact equivalences if this has not already been done
    RemoveExactEquivalences(currentStatement, candidates) ;
  }
}

void TemporalCSEPass::RemoveExactEquivalences(Statement* currentStatement,
					      list<ArrayReferenceExpression*>& x)
{
  StoreStatement* currentStore = 
    dynamic_cast<StoreStatement*>(currentStatement) ;
  if (currentStore == NULL)
  {
    return ;
  }
  ArrayReferenceExpression* dest = 
    dynamic_cast<ArrayReferenceExpression*>(currentStore->get_destination_address()) ;
  if (dest == NULL)
  {
    return ;
  }
  list<ArrayReferenceExpression*>::iterator xIter = x.begin() ;
  while (xIter != x.end())
  {
    if (EquivalentExpressions((*xIter), dest))
    {
      xIter = x.erase(xIter) ;
    }
    else
    {
      ++xIter ;
    }
  }

}

void TemporalCSEPass::AddLoads(list<ArrayReferenceExpression*>& x,
			       Statement* s)
{
  list<LoadExpression*>* allLoads = 
    collect_objects<LoadExpression>(s) ;
  list<LoadExpression*>::iterator loadIter = allLoads->begin() ;
  while (loadIter != allLoads->end())
  {  
    bool found = false ;
    ArrayReferenceExpression* currentRef = 
      dynamic_cast<ArrayReferenceExpression*>((*loadIter)->get_source_address()) ;
    if (currentRef == NULL)
    {
      ++loadIter ;
      continue ;
    }
    list<ArrayReferenceExpression*>::iterator candidateIter = 
      x.begin() ;
    while (candidateIter != x.end())
    {
      if (EquivalentExpressions(*candidateIter, currentRef))
      {
	found = true ;
	break ;
      }
      ++candidateIter ;
    }
    if (!found)
    {
      x.push_back(currentRef) ;
    }
    ++loadIter ;
  }
  delete allLoads ;
}

// This function returns a number, which states how many statements to 
//  skip over in the main iteration loop.
int TemporalCSEPass::ReplaceEquivalences(Statement* currentStatement,
					 list<ArrayReferenceExpression*>& x,
					 StatementList* parentList,
					 int position)
{
  StoreStatement* currentStore = 
    dynamic_cast<StoreStatement*>(currentStatement) ;
  if (currentStore == NULL)
  {
    return 1 ;
  }

  ArrayReferenceExpression* dest = 
    dynamic_cast<ArrayReferenceExpression*>(currentStore->
					    get_destination_address()) ;
  assert(dest != NULL && "Generic pointers not yet supported!") ;

  Expression* base = dest->get_base_array_address() ;
  Expression* index = dest->get_index() ;

  // Go through the list of candidates and see if any of them are equivalent
  //  across loop iterations
  list<ArrayReferenceExpression*>::iterator candidateIter = x.begin() ;
  while (candidateIter != x.end())
  {
    if (EquivalentExpressions(base, (*candidateIter)->get_base_array_address())
	&& SameIndex2(index, (*candidateIter)->get_index(), stride))
    {     

      // Create two variable symbols.  One that handles feedback and one
      //  that handles scalar replacement
      VariableSymbol* temp =
	create_variable_symbol(theEnv, 
			       create_qualified_type(theEnv,
						     dest->get_result_type()),
			       TempName("tcseTemp")) ;
      LString yName = GetArrayVariable(*candidateIter)->get_name() ;
      int offset = GetOffset(*candidateIter) ;
      if (offset < 0 )
      {
	yName = yName + "_minus" ;
	offset = -offset ;
      }
      yName = yName + "_" ;
      yName = yName + LString(offset) ;
      yName = yName + "_feedback" ;
      VariableSymbol* y =
	create_variable_symbol(theEnv, 
			       create_qualified_type(theEnv,
						     dest->get_result_type()),
			       TempName(yName)) ;

      procDef->get_symbol_table()->append_symbol_table_object(temp) ;
      procDef->get_symbol_table()->append_symbol_table_object(y) ;
      temp->append_annote(create_brick_annote(theEnv, "NeedsFake")) ;
      y->append_annote(create_brick_annote(theEnv, "NeedsFake")) ;
      
      ReplaceAllUsesWith((*candidateIter), temp, parentList, position) ;

      // Create a new store variable statement and add it before the current
      //  statement
      LoadVariableExpression* loadY = 
	create_load_variable_expression(theEnv, 
					y->get_type()->get_base_type(),
					y) ;
      StoreVariableStatement* beforeStore = 
	create_store_variable_statement(theEnv, 
					temp,
					loadY) ;
      AddChildStatementBefore(parentList, beforeStore, currentStore) ;

      // Replace the destination of this statement
      LoadVariableExpression* loadY2 = 
	create_load_variable_expression(theEnv,
					y->get_type()->get_base_type(),
					y) ;
      currentStore->set_destination_address(loadY2) ;

      // Create a new store statement that goes afterwards
      LoadVariableExpression* loadY3 = 
	create_load_variable_expression(theEnv,
					y->get_type()->get_base_type(),
					y) ;
      StoreStatement* afterStore = 
	create_store_statement(theEnv,
			       loadY3,
			       dest) ;
      AddChildStatementAfter(parentList, afterStore, currentStore) ;
      return 3 ;
    }
    ++candidateIter ;
  }
  return 1 ;
}

void TemporalCSEPass::ProcessLoopStatementsMulti(CForStatement* c)
{
  assert(c != NULL) ;
  assert(c->get_body() != NULL) ;

  // We need to find two statements that compute the same value,
  //  but do so on different elements of an array that are equivalent
  //  across loop iterations.  The only types of statements that we 
  //  will find are call statements, store variable statements, and store
  //  statements.

  list< std::pair<Statement*, Statement*> > equivalentStatements ;

  // To begin with, I'm going to collect all of the call statements.

  list<CallStatement*>* allCalls = 
    collect_objects<CallStatement>(c->get_body()) ;
  assert(allCalls != NULL) ;

  list<CallStatement*>::iterator callIter = allCalls->begin() ;
  while (callIter != allCalls->end())
  {
    list<CallStatement*>::iterator innerIter = callIter ;
    ++innerIter ;
    while (innerIter != allCalls->end())
    {
      if (EquivalentStatements(*callIter, *innerIter))
      {
	std::pair<Statement*, Statement*> toAdd ;
	toAdd.first = (*callIter) ;
	toAdd.second = (*innerIter) ;
	// Because we can't go backwards in lists, I'm putting them in
	//  in backwards order...
	equivalentStatements.push_front(toAdd) ;
      }
      ++innerIter ;
    }    
    ++callIter ;
  }

  delete allCalls ;

  // Now collect all the store variable statements
  list<StoreVariableStatement*>* allStoreVars = 
    collect_objects<StoreVariableStatement>(c->get_body()) ;
  assert(allStoreVars != NULL) ;
  
  list<StoreVariableStatement*>::iterator storeVarIter = allStoreVars->begin();
  while (storeVarIter != allStoreVars->end())
  {
    list<StoreVariableStatement*>::iterator innerIter = storeVarIter ;
    ++innerIter ;
    while (innerIter != allStoreVars->end())
    {
      if (EquivalentStatements(*storeVarIter, *innerIter))
      {
	std::pair<Statement*, Statement*> toAdd ;
	toAdd.first = (*storeVarIter) ;
	toAdd.second = (*innerIter) ;
	// Because we can't go backwards in lists, I'm putting them in
	//  in backwards order...
	equivalentStatements.push_front(toAdd) ;
      }
      ++innerIter ;
    }
    ++storeVarIter ;
  }

  delete allStoreVars ;

  // And finally collect all of the store statements
  list<StoreStatement*>* allStores = 
    collect_objects<StoreStatement>(c->get_body()) ;
  assert(allStores != NULL) ;

  list<StoreStatement*>::iterator storeIter = allStores->begin() ;
  while (storeIter != allStores->end())
  {
    list<StoreStatement*>::iterator innerIter = storeIter ;
    ++innerIter ;
    while (innerIter != allStores->end())
    {
      if (EquivalentStatements(*storeIter, *innerIter))
      {
	std::pair<Statement*, Statement*> toAdd ;
	toAdd.first = (*storeIter) ;
	toAdd.second = (*innerIter) ;
	// Because we can't go backwards in lists, I'm putting them in
	//  in backwards order...
	equivalentStatements.push_front(toAdd) ;
      }
      ++innerIter ;
    }
    ++storeIter ;
  }

  delete allStores ;

  std::stringstream informationStream ;
  
  informationStream << "Found " << equivalentStatements.size() 
		    << " equivalent statements" << std::endl ;
  OutputInformation(informationStream.str().c_str()) ;

  // Now, I need to replace the statements with loadPrevious/storeNext
  //  pairs.  I need a pair for each output variable in the call statement
  //  (and the destination if one exists).  For each pair, I remove the latter
  //  of the two statements and replace it with a store variable statement
  //  for a new feeback variable.

  list< std::pair<Statement*, Statement*> >::iterator equivIter ;
  equivIter = equivalentStatements.begin() ;
  while (equivIter != equivalentStatements.end())
  {
    ProcessPair((*equivIter).first, (*equivIter).second) ;
    ++equivIter ;
  }
}

// This function will go statement by statement and check for equivalencies
//  among the right hand sides across loop iterations.
void TemporalCSEPass::ProcessLoopStatements(CForStatement* c)
{
  assert(c != NULL) ;
  assert(c->get_body() != NULL) ;
  
  // In order for temporal common sub expression elimination to occur we
  //  must find two statements that compute the same value, but do so
  //  on different elements of an array that are equivalent across 
  //  loop iterations.  The only types of statements that we will find
  //  are call statements, store variable statements, and store statements.

  list< std::pair<Statement*, Statement*> > equivalentStatements ;

  // To begin with, I'm going to collect all of the call statements.

  list<CallStatement*>* allCalls = 
    collect_objects<CallStatement>(c->get_body()) ;
  assert(allCalls != NULL) ;

  list<CallStatement*>::iterator callIter = allCalls->begin() ;
  while (callIter != allCalls->end())
  {
    list<CallStatement*>::iterator innerIter = callIter ;
    ++innerIter ;
    while (innerIter != allCalls->end())
    {
      if (EquivalentStatements(*callIter, *innerIter))
      {
	std::pair<Statement*, Statement*> toAdd ;
	toAdd.first = (*callIter) ;
	toAdd.second = (*innerIter) ;
	// Because we can't go backwards in lists, I'm putting them in
	//  in backwards order...
	equivalentStatements.push_front(toAdd) ;
      }
      ++innerIter ;
    }    
    ++callIter ;
  }

  delete allCalls ;

  // Now collect all the store variable statements
  list<StoreVariableStatement*>* allStoreVars = 
    collect_objects<StoreVariableStatement>(c->get_body()) ;
  assert(allStoreVars != NULL) ;
  
  list<StoreVariableStatement*>::iterator storeVarIter = allStoreVars->begin();
  while (storeVarIter != allStoreVars->end())
  {
    list<StoreVariableStatement*>::iterator innerIter = storeVarIter ;
    ++innerIter ;
    while (innerIter != allStoreVars->end())
    {
      if (EquivalentStatements(*storeVarIter, *innerIter))
      {
	std::pair<Statement*, Statement*> toAdd ;
	toAdd.first = (*storeVarIter) ;
	toAdd.second = (*innerIter) ;
	// Because we can't go backwards in lists, I'm putting them in
	//  in backwards order...
	equivalentStatements.push_front(toAdd) ;
      }
      ++innerIter ;
    }
    ++storeVarIter ;
  }

  delete allStoreVars ;

  // And finally collect all of the store statements
  list<StoreStatement*>* allStores = 
    collect_objects<StoreStatement>(c->get_body()) ;
  assert(allStores != NULL) ;

  list<StoreStatement*>::iterator storeIter = allStores->begin() ;
  while (storeIter != allStores->end())
  {
    list<StoreStatement*>::iterator innerIter = storeIter ;
    ++innerIter ;
    while (innerIter != allStores->end())
    {
      if (EquivalentStatements(*storeIter, *innerIter))
      {
	std::pair<Statement*, Statement*> toAdd ;
	toAdd.first = (*storeIter) ;
	toAdd.second = (*innerIter) ;
	// Because we can't go backwards in lists, I'm putting them in
	//  in backwards order...
	equivalentStatements.push_front(toAdd) ;
      }
      ++innerIter ;
    }
    ++storeIter ;
  }

  delete allStores ;

  std::stringstream informationStream ;
  
  informationStream << "Found " << equivalentStatements.size() 
		    << " equivalent statements" << std::endl ;
  OutputInformation(informationStream.str().c_str()) ;

  // Now, I need to replace the statements with loadPrevious/storeNext
  //  pairs.  I need a pair for each output variable in the call statement
  //  (and the destination if one exists).  For each pair, I remove the latter
  //  of the two statements and replace it with a store variable statement
  //  for a new feeback variable.

  list< std::pair<Statement*, Statement*> >::iterator equivIter ;
  equivIter = equivalentStatements.begin() ;
  while (equivIter != equivalentStatements.end())
  {
    ProcessPair((*equivIter).first, (*equivIter).second) ;
    ++equivIter ;
  }

}

bool TemporalCSEPass::equivalent(ArrayReferenceExpression* x, 
				 ArrayReferenceExpression* y,
				 int depth)
{
  Expression* xBase = x->get_base_array_address() ;
  Expression* yBase = y->get_base_array_address() ;

  Expression* xIndex = x->get_index() ;
  Expression* yIndex = y->get_index() ;

  bool baseEquivalent ;
  bool indexEquivalent = true ;

  if (dynamic_cast<ArrayReferenceExpression*>(xBase) != NULL &&
      dynamic_cast<ArrayReferenceExpression*>(yBase) != NULL)
  {
    ArrayReferenceExpression* xRef = 
      dynamic_cast<ArrayReferenceExpression*>(xBase) ;
    ArrayReferenceExpression* yRef = 
      dynamic_cast<ArrayReferenceExpression*>(yBase) ;
    baseEquivalent = equivalent(xRef, yRef, depth + 1) ;
  }
  else if (dynamic_cast<SymbolAddressExpression*>(xBase) != NULL &&
	   dynamic_cast<SymbolAddressExpression*>(yBase) != NULL)
  {
    SymbolAddressExpression* xSym = 
      dynamic_cast<SymbolAddressExpression*>(xBase) ;
    SymbolAddressExpression* ySym = 
      dynamic_cast<SymbolAddressExpression*>(yBase) ;
    baseEquivalent = (xSym->get_addressed_symbol() == 
		      ySym->get_addressed_symbol()) ;
  }
  else
  {
    baseEquivalent = false ;
  }

  // Index equivalence is based upon depth.  If the depth is the same as
  //  the outermost loop, then pass the stride along.  Otherwise, don't.

  if (depth == maxLevels)
  {
    indexEquivalent = SameIndex(xIndex, yIndex, stride) ;
  }
  else
  {
    indexEquivalent = SameIndex(xIndex, yIndex) ;
  }

  return baseEquivalent && indexEquivalent ;
}

bool TemporalCSEPass::SameIndex(Expression* x, Expression* y, int diff)
{
  VariableSymbol* xSym = NULL ;
  VariableSymbol* ySym = NULL ;
  int xOffset = 0 ;
  int yOffset = 0 ;

  if (dynamic_cast<LoadVariableExpression*>(x) != NULL)
  {
    xSym = dynamic_cast<LoadVariableExpression*>(x)->get_source() ;
  }
  if (dynamic_cast<LoadVariableExpression*>(y) != NULL)
  {
    ySym = dynamic_cast<LoadVariableExpression*>(y)->get_source() ;
  }
  
  if (dynamic_cast<BinaryExpression*>(x) != NULL)
  {
    Expression* left = dynamic_cast<BinaryExpression*>(x)->get_source1() ;
    Expression* right = dynamic_cast<BinaryExpression*>(x)->get_source2() ;

    // One of these has to be a constant, and the other has to be 
    //  a load variable expression
    if (dynamic_cast<LoadVariableExpression*>(left) != NULL &&
	dynamic_cast<IntConstant*>(right) != NULL)
    {
      xSym = 
	dynamic_cast<LoadVariableExpression*>(left)->get_source() ;
      xOffset = 
	dynamic_cast<IntConstant*>(right)->get_value().c_int() ;
    }
    else if (dynamic_cast<LoadVariableExpression*>(right) != NULL &&
	     dynamic_cast<IntConstant*>(left) != NULL)
    {
      xSym = 
	dynamic_cast<LoadVariableExpression*>(right)->get_source() ;
      xOffset = 
	dynamic_cast<IntConstant*>(left)->get_value().c_int() ;
    }
    else
    {
      assert(0) ;
      return false ;
    }
    if (dynamic_cast<BinaryExpression*>(x)->get_opcode() == LString("sub"))
    {
      xOffset = -xOffset ;
    }
  }
  
  if (dynamic_cast<BinaryExpression*>(y) != NULL)
  {
    Expression* left = dynamic_cast<BinaryExpression*>(y)->get_source1() ;
    Expression* right = dynamic_cast<BinaryExpression*>(y)->get_source2() ;

    // One of these has to be a constant, and the other has to be 
    //  a load variable expression
    if (dynamic_cast<LoadVariableExpression*>(left) != NULL &&
	dynamic_cast<IntConstant*>(right) != NULL)
    {
      ySym = 
	dynamic_cast<LoadVariableExpression*>(left)->get_source() ;
      yOffset = 
	dynamic_cast<IntConstant*>(right)->get_value().c_int() ;
    }
    else if (dynamic_cast<LoadVariableExpression*>(right) != NULL &&
	     dynamic_cast<IntConstant*>(left) != NULL)
    {
      ySym = 
	dynamic_cast<LoadVariableExpression*>(right)->get_source() ;
      yOffset = 
	dynamic_cast<IntConstant*>(left)->get_value().c_int() ;
    }
    else
    {
      assert(0) ;
      return false ;
    }
    if (dynamic_cast<BinaryExpression*>(y)->get_opcode() == LString("sub"))
    {
      yOffset = -yOffset ;
    }
  }

  return (xSym != NULL) && (ySym != NULL) && 
         (xSym == ySym) && 
         ((xOffset + diff == yOffset) || (xOffset == yOffset + diff)) ;
}

// This function is identical to the one above, but only returns true 
//  if the destination stores a value that will be used by the source
//  in a future iteration
bool TemporalCSEPass::SameIndex2(Expression* x, Expression* y, int diff)
{
  VariableSymbol* xSym = NULL ;
  VariableSymbol* ySym = NULL ;
  int xOffset = 0 ;
  int yOffset = 0 ;

  if (dynamic_cast<LoadVariableExpression*>(x) != NULL)
  {
    xSym = dynamic_cast<LoadVariableExpression*>(x)->get_source() ;
  }
  if (dynamic_cast<LoadVariableExpression*>(y) != NULL)
  {
    ySym = dynamic_cast<LoadVariableExpression*>(y)->get_source() ;
  }
  
  if (dynamic_cast<BinaryExpression*>(x) != NULL)
  {
    Expression* left = dynamic_cast<BinaryExpression*>(x)->get_source1() ;
    Expression* right = dynamic_cast<BinaryExpression*>(x)->get_source2() ;

    // One of these has to be a constant, and the other has to be 
    //  a load variable expression
    if (dynamic_cast<LoadVariableExpression*>(left) != NULL &&
	dynamic_cast<IntConstant*>(right) != NULL)
    {
      xSym = 
	dynamic_cast<LoadVariableExpression*>(left)->get_source() ;
      xOffset = 
	dynamic_cast<IntConstant*>(right)->get_value().c_int() ;
    }
    else if (dynamic_cast<LoadVariableExpression*>(right) != NULL &&
	     dynamic_cast<IntConstant*>(left) != NULL)
    {
      xSym = 
	dynamic_cast<LoadVariableExpression*>(right)->get_source() ;
      xOffset = 
	dynamic_cast<IntConstant*>(left)->get_value().c_int() ;
    }
    else
    {
      assert(0) ;
      return false ;
    }
    if (dynamic_cast<BinaryExpression*>(x)->get_opcode() == LString("sub"))
    {
      xOffset = -xOffset ;
    }
  }
  
  if (dynamic_cast<BinaryExpression*>(y) != NULL)
  {
    Expression* left = dynamic_cast<BinaryExpression*>(y)->get_source1() ;
    Expression* right = dynamic_cast<BinaryExpression*>(y)->get_source2() ;

    // One of these has to be a constant, and the other has to be 
    //  a load variable expression
    if (dynamic_cast<LoadVariableExpression*>(left) != NULL &&
	dynamic_cast<IntConstant*>(right) != NULL)
    {
      ySym = 
	dynamic_cast<LoadVariableExpression*>(left)->get_source() ;
      yOffset = 
	dynamic_cast<IntConstant*>(right)->get_value().c_int() ;
    }
    else if (dynamic_cast<LoadVariableExpression*>(right) != NULL &&
	     dynamic_cast<IntConstant*>(left) != NULL)
    {
      ySym = 
	dynamic_cast<LoadVariableExpression*>(right)->get_source() ;
      yOffset = 
	dynamic_cast<IntConstant*>(left)->get_value().c_int() ;
    }
    else
    {
      assert(0) ;
      return false ;
    }
    if (dynamic_cast<BinaryExpression*>(y)->get_opcode() == LString("sub"))
    {
      yOffset = -yOffset ;
    }
  }

  return (xSym != NULL) && (ySym != NULL) && 
         (xSym == ySym) && 
         ((xOffset + diff == yOffset)) ;
}

void TemporalCSEPass::SetStride(CForStatement* c)
{
  Statement* step = c->get_step() ;
  assert(step != NULL) ;
  // The step should be a store variable statement
  StoreVariableStatement* storeStep = 
    dynamic_cast<StoreVariableStatement*>(step) ;
  assert(storeStep != NULL) ;
  Expression* increment = storeStep->get_value() ;
  assert(increment != NULL) ;
  // This should be a binary expression
  BinaryExpression* binIncrement = 
    dynamic_cast<BinaryExpression*>(increment) ;
  assert(binIncrement != NULL) ;
  // One of these should be a constant
  Expression* left = binIncrement->get_source1() ;
  Expression* right = binIncrement->get_source2() ;

  assert(dynamic_cast<IntConstant*>(left) != NULL ||
	 dynamic_cast<IntConstant*>(right) != NULL) ;

  if (dynamic_cast<IntConstant*>(left) != NULL)
  {
    stride = dynamic_cast<IntConstant*>(left)->get_value().c_int() ;
  }
  else
  {
    stride = dynamic_cast<IntConstant*>(right)->get_value().c_int() ;
  }
  
  if (binIncrement->get_opcode() == LString("subtract"))
  {
    stride = -stride ;
  }
}

CForStatement* TemporalCSEPass::FindParentForLoop(CForStatement* c)
{
  SuifObject* currentParent = c->get_parent() ;
  CForStatement* toReturn = NULL ;
  
  while (currentParent != NULL)
  {
    if (dynamic_cast<CForStatement*>(currentParent) != NULL)
    {
      toReturn = dynamic_cast<CForStatement*>(currentParent) ;     
      ++maxLevels ;
    }
    currentParent = currentParent->get_parent() ;
  }
  return toReturn ;
}

void TemporalCSEPass::SetStrideMultiDimensional(CForStatement* c)
{
  CForStatement* parentLoop = FindParentForLoop(c) ;

  if (parentLoop == NULL)
  {
    parentLoop = c ;
  }

  Statement* step = parentLoop->get_step() ;
  assert(step != NULL) ;
  StoreVariableStatement* storeStep = 
    dynamic_cast<StoreVariableStatement*>(step) ;
  assert(storeStep != NULL) ;
  Expression* increment = storeStep->get_value() ;
  assert(increment != NULL) ;
  BinaryExpression* binIncrement = 
    dynamic_cast<BinaryExpression*>(increment) ;
  assert(binIncrement != NULL) ;
  Expression* left = binIncrement->get_source1() ;
  Expression* right = binIncrement->get_source2() ;
  
  assert(dynamic_cast<IntConstant*>(left) != NULL ||
	 dynamic_cast<IntConstant*>(right) != NULL) ;

  if (dynamic_cast<IntConstant*>(left) != NULL)
  {
    stride = dynamic_cast<IntConstant*>(left)->get_value().c_int() ;
  }
  else
  {
    stride = dynamic_cast<IntConstant*>(right)->get_value().c_int() ;
  }

  if (binIncrement->get_opcode() == LString("sub"))
  {
    stride = -stride ;
  }
}

bool TemporalCSEPass::EquivalentStatements(Statement* x, Statement* y)
{
  // The statements should both be the same type, either store variable
  //  statements, store statements, or call statements

  CallStatement* xCall = dynamic_cast<CallStatement*>(x) ;
  CallStatement* yCall = dynamic_cast<CallStatement*>(y) ;
  StoreVariableStatement* xStoreVars = 
    dynamic_cast<StoreVariableStatement*>(x) ;
  StoreVariableStatement* yStoreVars = 
    dynamic_cast<StoreVariableStatement*>(y) ;
  StoreStatement* xStore = dynamic_cast<StoreStatement*>(x) ;
  StoreStatement* yStore = dynamic_cast<StoreStatement*>(y) ;

  // Return false if they are of different types
  if (!(xCall != NULL && yCall != NULL) &&
      !(xStoreVars != NULL && yStoreVars != NULL) &&
      !(xStore != NULL && yStore != NULL))
  {
    return false ;
  }

  if (xCall != NULL && yCall != NULL)
  {
    return EquivalentCalls(xCall, yCall) ;
  }
  else if (xStoreVars != NULL && yStoreVars != NULL)
  {
    return EquivalentStoreVars(xStoreVars, yStoreVars) ;
  }
  else if (xStore != NULL && yStore != NULL)
  {
    return EquivalentStores(xStore, yStore) ;
  }
  else
  {
    assert(0) ;
  }

  return false ;
}

bool TemporalCSEPass::EquivalentCalls(CallStatement* x, CallStatement* y)
{
  // If the argument count isn't the same, they can't be equivalent
  if (x->get_argument_count() != y->get_argument_count())
  {
    return false ;
  }

  // First, check to see if the call statement address is the same
  Expression* xAddr = x->get_callee_address() ;
  Expression* yAddr = y->get_callee_address() ;

  SymbolAddressExpression* xSymAddr = 
    dynamic_cast<SymbolAddressExpression*>(xAddr) ;
  SymbolAddressExpression* ySymAddr = 
    dynamic_cast<SymbolAddressExpression*>(yAddr) ;

  assert(xSymAddr != NULL && ySymAddr != NULL) ;

  if (xSymAddr->get_addressed_symbol() != ySymAddr->get_addressed_symbol())
  {
    return false ;
  }

  // Now, check to see if the arguments (which are input) are equivalent
  //  across loop iterations.
  for (unsigned int i = 0 ; i < x->get_argument_count() ; ++i)
  {
    Expression* xArg = x->get_argument(i) ;
    Expression* yArg = y->get_argument(i) ;

    if (dynamic_cast<SymbolAddressExpression*>(xArg) != NULL &&
	dynamic_cast<SymbolAddressExpression*>(yArg) != NULL)
    {
      // This is an output variable
      continue ;
    }

    // This must be an input variable
    if (dynamic_cast<LoadExpression*>(xArg) == NULL ||
	dynamic_cast<LoadExpression*>(yArg) == NULL)
    {
      return false ;
    }
    
    assert(dynamic_cast<LoadExpression*>(xArg) != NULL &&
	   dynamic_cast<LoadExpression*>(yArg) != NULL) ;

    Expression* xSource = 
      dynamic_cast<LoadExpression*>(xArg)->get_source_address() ;
    Expression* ySource =
      dynamic_cast<LoadExpression*>(yArg)->get_source_address() ;

    assert(xSource != NULL && ySource != NULL) ;

    if (dynamic_cast<ArrayReferenceExpression*>(xSource) == NULL ||
	dynamic_cast<ArrayReferenceExpression*>(ySource) == NULL)
    {
      return false ;
    }

    ArrayReferenceExpression* xRef = 
      dynamic_cast<ArrayReferenceExpression*>(xSource) ;
    ArrayReferenceExpression* yRef = 
      dynamic_cast<ArrayReferenceExpression*>(ySource) ;

    assert(xRef != NULL && yRef != NULL) ;

    if (!equivalent(xRef, yRef, 0))
    {
      return false ;
    }

  }

  return true ;
}

bool TemporalCSEPass::EquivalentStoreVars(StoreVariableStatement* x,
					  StoreVariableStatement* y)
{
  assert(x != NULL) ;
  assert(y != NULL) ;

  // As a special case, store variables should not be considered equivalent
  //  if either of them is reading from a constant.  This prevents us from
  //  feeding constants back when they should don't need registers
  
  if (dynamic_cast<Constant*>(x->get_value()) != NULL ||
      dynamic_cast<Constant*>(y->get_value()) != NULL)
  {
    return false ;
  }

  return EquivalentExpressionTrees(x->get_value(), y->get_value()) ;
}

// Currently, this function is identical to the EquivalentStoreVars, so
//  it doesn't really work the way it should yet...
bool TemporalCSEPass::EquivalentStores(StoreStatement* x,
				       StoreStatement* y)
{
  assert(x != NULL) ;
  assert(y != NULL) ;

  return EquivalentExpressionTrees(x->get_value(), y->get_value()) ;
}

void TemporalCSEPass::ProcessPair(Statement* x, Statement* y)
{
  assert(x != NULL && y != NULL) ;

  // We've already determined that these two statements are equivalent,
  //  so we just need to deal with them individually.

  if (dynamic_cast<CallStatement*>(x) != NULL && 
      dynamic_cast<CallStatement*>(y) != NULL)
  {
    ProcessPairCalls(dynamic_cast<CallStatement*>(x), 
		     dynamic_cast<CallStatement*>(y)) ;
  }
  else if (dynamic_cast<StoreVariableStatement*>(x) != NULL &&
           dynamic_cast<StoreVariableStatement*>(y) != NULL) 
  {
    ProcessPairStoreVars(dynamic_cast<StoreVariableStatement*>(x),
			 dynamic_cast<StoreVariableStatement*>(y)) ;
  }
  else if (dynamic_cast<StoreStatement*>(x) != NULL &&
	   dynamic_cast<StoreStatement*>(y) != NULL)
  {
    ProcessPairStores(dynamic_cast<StoreStatement*>(x),
		      dynamic_cast<StoreStatement*>(y)) ;
  }
  else
  {
    assert(0) ;
  }
}

void TemporalCSEPass::ProcessPairCalls(CallStatement* x, CallStatement* y)
{
  assert(x != NULL && y != NULL) ;

  // Remember, we have verified that these are equivalent!

  // Remove y and replace with a store from the destination of x

  // For call statements, I have to check the destination as well as all of
  //  the arguments for output variables.

  VariableSymbol* dest = y->get_destination() ;
  
  if (dest != NULL)
  {
    // Not yet implemented 
    std::cerr << "Call statements with destinations are not yet supported"
	      << std::endl ;
    assert(0) ;
  }

  for(unsigned int i = 0 ; i < x->get_argument_count() ; ++i)
  {
    assert(i < y->get_argument_count()) ;
    Expression* nextXArg = x->get_argument(i) ; 
    Expression* nextYArg = y->get_argument(i) ;

    if (dynamic_cast<SymbolAddressExpression*>(nextXArg) != NULL)
    {
      // We've found an output variable 
      SymbolAddressExpression* xSymAddr = 
	dynamic_cast<SymbolAddressExpression*>(nextXArg) ;
      SymbolAddressExpression* ySymAddr = 
	dynamic_cast<SymbolAddressExpression*>(nextYArg) ;
      assert(xSymAddr != NULL && ySymAddr != NULL) ;

      VariableSymbol* xSym = 
	dynamic_cast<VariableSymbol*>(xSymAddr->get_addressed_symbol()) ;
      VariableSymbol* ySym = 
	dynamic_cast<VariableSymbol*>(ySymAddr->get_addressed_symbol()) ;

      assert(xSym != NULL && ySym != NULL) ;

      // Create a new variable
      VariableSymbol* replacement = new_unique_variable(theEnv,
							find_scope(procDef->get_body()),
							xSym->get_type(),
							TempName("temporalTmp")) ;

      // I KNOW this is a feedback variable, so go ahead and annote it
      //  so the correct loadPrevious and storeNext are generated.
      BrickAnnote* feedbackAnnote = 
	to<BrickAnnote>(create_brick_annote(theEnv, "FeedbackVariable")) ;

      feedbackAnnote->append_brick(create_suif_object_brick(theEnv, ySym)) ;
      feedbackAnnote->append_brick(create_suif_object_brick(theEnv, xSym)) ;

      replacement->append_annote(feedbackAnnote) ;

      // Also, put the fact that this is a storeNext, not a systolicNext
      BrickAnnote* nonsystolic = 
	to<BrickAnnote>(create_brick_annote(theEnv, "NonSystolic")) ;
      replacement->append_annote(nonsystolic) ;

      replacement->append_annote(create_brick_annote(theEnv, "TemporalFeedback")) ;
      xSym->append_annote(create_brick_annote(theEnv, "TemporalFeedback")) ;
      ySym->append_annote(create_brick_annote(theEnv, "TemporalFeedback")) ;
    }
      
  }
  
  // Finally, remove the original statement
  y->get_parent()->replace(y, NULL) ;
  delete y ;      

}

void TemporalCSEPass::ProcessPairStoreVars(StoreVariableStatement* x,
					   StoreVariableStatement* y)
{
  assert(x != NULL) ;
  assert(y != NULL) ;

  // Remember, we have verified these as equivalent!
  VariableSymbol* xSym = x->get_destination() ;
  VariableSymbol* ySym = y->get_destination() ;

  assert(xSym != NULL) ;
  assert(ySym != NULL) ;

  // Create two variables.  One for feedback and one for use.
  LString feedbackName = xSym->get_name() + LString("_temporalTmp") ;
  
  VariableSymbol* feedbackSymbol = 
    create_variable_symbol(theEnv, xSym->get_type(), ::TempName(feedbackName));

  procDef->get_symbol_table()->append_symbol_table_object(feedbackSymbol) ;

  BrickAnnote* feedbackAnnote = 
    to<BrickAnnote>(create_brick_annote(theEnv, "TemporalFeedbackVariable")) ;

  feedbackAnnote->append_brick(create_suif_object_brick(theEnv, ySym)) ;
  feedbackAnnote->append_brick(create_suif_object_brick(theEnv, xSym)) ;

  feedbackSymbol->append_annote(feedbackAnnote) ;
  feedbackSymbol->append_annote(create_brick_annote(theEnv, "NeedsFake")) ;

  // Also, put the fact that this is a storeNext, not a systolicNext
  BrickAnnote* nonsystolic = 
    to<BrickAnnote>(create_brick_annote(theEnv, "NonSystolic")) ;
  feedbackSymbol->append_annote(nonsystolic) ;

  // Now, create a load variable expression for this feedback variable and
  //  replace the value being fetched for y with this expression.

  LoadVariableExpression* copyLoad = 
    create_load_variable_expression(theEnv,
				    ySym->get_type()->get_base_type(),
				    ySym) ;

  y->replace(y->get_value(), copyLoad) ;

  feedbackSymbol->append_annote(create_brick_annote(theEnv, 
  						    "TemporalFeedback"));
  xSym->append_annote(create_brick_annote(theEnv, "TemporalFeedback")) ;
  ySym->append_annote(create_brick_annote(theEnv, "TemporalFeedback")) ;
}

void TemporalCSEPass::ProcessPairStores(StoreStatement* x, StoreStatement* y)
{
  assert(x != NULL) ;
  assert(y != NULL) ;

  // Remember, we have verified that these are equivalent!

  // For this case, we want to create three new variables.  One will
  //  be a feedback variable and will replace the value of y.  One will
  //  be used as a sort of reverse copy propagation variable.  And the third
  //  will be used to propagate values over loop iterations.
  
  // In order to create any variable, we need to get the type of the
  //  destination, which unfortunately is buried deep inside an
  //  array reference expression (hopefully, remember we don't support 
  //  pointers).

  Expression* destAddr = x->get_destination_address() ;
  assert(destAddr != NULL) ;
  ArrayReferenceExpression* destRef = 
    dynamic_cast<ArrayReferenceExpression*>(destAddr) ;
  assert(destRef != NULL) ;

  while (dynamic_cast<ArrayReferenceExpression*>(destRef->get_base_array_address()) != NULL)
  {
    destRef = 
      dynamic_cast<ArrayReferenceExpression*>(destRef->get_base_array_address());
  }

  Expression* finalExp = destRef->get_base_array_address() ;
  assert(finalExp != NULL) ;
  SymbolAddressExpression* finalSym = 
    dynamic_cast<SymbolAddressExpression*>(finalExp) ;
  assert(finalSym != NULL) ;

  Symbol* internalSym = finalSym->get_addressed_symbol() ;
  assert(internalSym != NULL) ;

  VariableSymbol* finalVarSym = 
    dynamic_cast<VariableSymbol*>(internalSym) ;
  assert(finalVarSym != NULL) ;

  // Now I have the variable symbol of the array, but I need the element
  //  type.

  DataType* arrayType = finalVarSym->get_type()->get_base_type() ;
  assert(arrayType != NULL) ;
  ArrayType* trueArrayType = 
    dynamic_cast<ArrayType*>(arrayType) ;
  assert(trueArrayType != NULL) ;
  
  QualifiedType* finalType = trueArrayType->get_element_type() ;

  // If this is a multidimensional array access, we must continue until
  //  we hit the concrete element
  if (dynamic_cast<ArrayType*>(finalType->get_base_type()) != NULL)
  {
    finalType = dynamic_cast<ArrayType*>(finalType->get_base_type())->get_element_type() ;
  }

  // Now, create the three different variables.
  VariableSymbol* previousIterationValue = 
    new_unique_variable(theEnv,
			find_scope(procDef->get_body()),
			finalType,
			TempName("temporalTmp")) ;

  VariableSymbol* feedback = 
    new_unique_variable(theEnv,
			find_scope(procDef->get_body()),
			finalType,
			TempName("temporalTmp")) ;

  VariableSymbol* splitVariable = 
    new_unique_variable(theEnv,
			find_scope(procDef->get_body()),
			finalType,
			TempName("temporalTmp")) ;

  // Now, split the statement x
  Expression* xValue = x->get_value() ;
  x->set_value(NULL) ;

  StoreVariableStatement* firstX = 
    create_store_variable_statement(theEnv,
				    splitVariable,
				    xValue) ;
  LoadVariableExpression* secondX = 
    create_load_variable_expression(theEnv,
				    splitVariable->get_type()->get_base_type(),
				    splitVariable) ;
  x->set_value(secondX) ;

  // Now, put the new statement in the correct position

  SuifObject* xParent = x->get_parent() ;
  StatementList* parentList = 
    dynamic_cast<StatementList*>(xParent) ;
  assert(parentList != NULL) ;

  int position ;
  for (position = 0 ; 
       position < parentList->get_statement_count() ; 
       ++position)

  {
    if (parentList->get_statement(position) == x)
    {
      break ;
    }
  }
  
  assert(position < parentList->get_statement_count()) ;

  parentList->insert_statement(position, firstX) ;

  // Now, deal with the second store statement.  If the second store
  //  statement has already been changed by this pass, though,
  //  we will be dealing with a store variable statement instead.

  // The second store statement might have changed, though.  We have to 
  //  check to see if the second store statement has been replaced or not.

  if (y->lookup_annote_by_name("temporally_replaced") != NULL)
  {
    // We do a little more eliminating in this case
    BrickAnnote* temporallyReplaced = 
      to<BrickAnnote>(y->lookup_annote_by_name("temporally_replaced")) ;
    assert(temporallyReplaced != NULL) ;

    assert(temporallyReplaced->get_brick_count() == 1) ;
    
    SuifBrick* onlyBrick = temporallyReplaced->get_brick(0) ;
    SuifObjectBrick* onlySOB = dynamic_cast<SuifObjectBrick*>(onlyBrick) ;
    assert(onlySOB != NULL) ;

    SuifObject* obj = onlySOB->get_object() ;

    StoreVariableStatement* replaced = 
      dynamic_cast<StoreVariableStatement*>(obj) ;

    assert(replaced != NULL) ;

    // Remove the redundant expression
    Expression* toDelete = replaced->get_value() ;
    assert(toDelete != NULL) ;
    LoadVariableExpression* yLoad = 
      create_load_variable_expression(theEnv,
				      previousIterationValue->get_type()->get_base_type(),
				      previousIterationValue) ;
    
    replaced->set_value(yLoad) ;

    delete toDelete ; 
  }
  else
  {
    // Set the y statement
    Expression* toDelete = y->get_value() ;

    LoadVariableExpression* yLoad = 
      create_load_variable_expression(theEnv,
				      previousIterationValue->get_type()->get_base_type(),
				      previousIterationValue) ;
    
    y->set_value(yLoad) ;

    delete toDelete ;
  }

  // Now, set the feedback attributes for the feedback variable  

  BrickAnnote* feedbackAnnote = 
    to<BrickAnnote>(create_brick_annote(theEnv, "FeedbackVariable")) ;

  feedbackAnnote->append_brick(create_suif_object_brick(theEnv, previousIterationValue)) ;
  feedbackAnnote->append_brick(create_suif_object_brick(theEnv, splitVariable)) ;

  feedback->append_annote(feedbackAnnote) ;

  // Also, put the fact that this is a storeNext, not a systolicNext
  BrickAnnote* nonsystolic = 
    to<BrickAnnote>(create_brick_annote(theEnv, "NonSystolic")) ;
  feedback->append_annote(nonsystolic) ;

  // Add the temporal feedback annotation all the created feedback variables
  previousIterationValue->append_annote(create_brick_annote(theEnv, 
						    "TemporalFeedback"));
  feedback->append_annote(create_brick_annote(theEnv, "TemporalFeedback")) ;
  splitVariable->append_annote(create_brick_annote(theEnv, "TemporalFeedback"));

  // Make sure that all values get a fake
  previousIterationValue->append_annote(create_brick_annote(theEnv, 
							    "NeedsFake")) ;
  feedback->append_annote(create_brick_annote(theEnv, "NeedsFake")) ;
  splitVariable->append_annote(create_brick_annote(theEnv, "NeedsFake")) ;

  // Finally, we want to add an annote to the first statement that
  //  points to the statement that we used to replace the main expression

  BrickAnnote* temporallyReplaced = 
    to<BrickAnnote>(create_brick_annote(theEnv,
					"temporally_replaced")) ;
  temporallyReplaced->append_brick(create_suif_object_brick(theEnv,
							    firstX)) ;

  x->append_annote(temporallyReplaced) ;

}

// Equvialency expressions
bool TemporalCSEPass::EquivalentExpressionTrees(Expression* x, Expression* y)
{

  if (dynamic_cast<BinaryExpression*>(x) != NULL &&
      dynamic_cast<BinaryExpression*>(y) != NULL)
  {
    BinaryExpression* xBinExpr = dynamic_cast<BinaryExpression*>(x) ;
    BinaryExpression* yBinExpr = dynamic_cast<BinaryExpression*>(y) ;
    
    bool sameOps = (xBinExpr->get_opcode() == yBinExpr->get_opcode()) ;
    
    if (sameOps == false)
    {
      return false ;
    }
    
    return (EquivalentExpressionTrees(xBinExpr->get_source1(), 
				      yBinExpr->get_source1()) &&
            EquivalentExpressionTrees(xBinExpr->get_source2(), 
				      yBinExpr->get_source2())) ;
  }

  if (dynamic_cast<UnaryExpression*>(x) != NULL &&
      dynamic_cast<UnaryExpression*>(y) != NULL)
  {
    UnaryExpression* xUnExpr = dynamic_cast<UnaryExpression*>(x) ;
    UnaryExpression* yUnExpr = dynamic_cast<UnaryExpression*>(y) ;
    bool sameOps = (xUnExpr->get_opcode() == yUnExpr->get_opcode()) ;
    if (sameOps == false)
    {
      return false ;
    }
    return EquivalentExpressionTrees(xUnExpr->get_source(), 
				     yUnExpr->get_source()) ;
  }

  // We shouldn't see any select expressions...
  if (dynamic_cast<SelectExpression*>(x) != NULL &&
      dynamic_cast<SelectExpression*>(y) != NULL)
  {
    std::cerr << "Select expressions not yet supported" << std::endl ;
    assert(0) ;
  }

  // We also shouldn't have any multi-dim array expressions
  if (dynamic_cast<MultiDimArrayExpression*>(x) != NULL &&
      dynamic_cast<MultiDimArrayExpression*>(y) != NULL)
  {
    std::cerr << "Multi-Dim array expressions not yet supported" << std::endl ;
    assert(0) ;
  }

  if (dynamic_cast<ArrayReferenceExpression*>(x) != NULL &&
      dynamic_cast<ArrayReferenceExpression*>(y) != NULL)
  {
    // The starting depth is 0
    return equivalent(dynamic_cast<ArrayReferenceExpression*>(x),
		      dynamic_cast<ArrayReferenceExpression*>(y),
		      0) ;
  }

  // Field Access expressions need to be handled specially due to 
  //  the use of modules.  I'll get back to this in a little while
  if (dynamic_cast<FieldAccessExpression*>(x) != NULL &&
      dynamic_cast<FieldAccessExpression*>(y) != NULL)
  {
    std::cerr << "Field access expressions not yet supported" << std::endl ;
    assert(0) ;
  }

  // Variable arguments are definitely not supported
  if (dynamic_cast<VaArgExpression*>(x) != NULL &&
      dynamic_cast<VaArgExpression*>(y) != NULL)
  {
    std::cerr << "Variable length arguments not yet supported" << std::endl ;
    assert(0) ;
  }
  
  if (dynamic_cast<LoadExpression*>(x) != NULL &&
      dynamic_cast<LoadExpression*>(y) != NULL)
  {
    LoadExpression* xLoad = dynamic_cast<LoadExpression*>(x) ;
    LoadExpression* yLoad = dynamic_cast<LoadExpression*>(y) ;

    return EquivalentExpressionTrees(xLoad->get_source_address(),
				     yLoad->get_source_address()) ;
  }

  if (dynamic_cast<LoadVariableExpression*>(x) != NULL &&
      dynamic_cast<LoadVariableExpression*>(y) != NULL)
  {
    LoadVariableExpression* xLoadVar = 
      dynamic_cast<LoadVariableExpression*>(x) ;
    LoadVariableExpression* yLoadVar = 
      dynamic_cast<LoadVariableExpression*>(y) ;

    return (xLoadVar->get_source() == yLoadVar->get_source()) ;
  }

  if (dynamic_cast<SymbolAddressExpression*>(x) != NULL &&
      dynamic_cast<SymbolAddressExpression*>(y) != NULL)
  {
    SymbolAddressExpression* xSymAddr = 
      dynamic_cast<SymbolAddressExpression*>(x) ;
    SymbolAddressExpression* ySymAddr =
      dynamic_cast<SymbolAddressExpression*>(y) ;
    
    return (xSymAddr->get_addressed_symbol() == 
	    ySymAddr->get_addressed_symbol()) ;
  }

  if (dynamic_cast<LoadValueBlockExpression*>(x) != NULL &&
      dynamic_cast<LoadValueBlockExpression*>(y) != NULL)
  {
    std::cerr << "What are you doing?" << std::endl ;
    assert(0) ;
  }

  if (dynamic_cast<IntConstant*>(x) != NULL &&
      dynamic_cast<IntConstant*>(y) != NULL)
  {
    IntConstant* xInt = dynamic_cast<IntConstant*>(x) ;
    IntConstant* yInt = dynamic_cast<IntConstant*>(y) ;

    return (xInt->get_value() == yInt->get_value()) ;
  }

  if (dynamic_cast<FloatConstant*>(x) != NULL &&
      dynamic_cast<FloatConstant*>(y) != NULL)
  {
    FloatConstant* xFloat = dynamic_cast<FloatConstant*>(x) ;
    FloatConstant* yFloat = dynamic_cast<FloatConstant*>(y) ;
    return (xFloat->get_value() == yFloat->get_value()) ;
  }

  // x and y aren't the same type of expression, so they can't be 
  //  equivalent

  return false ;
}

int TemporalCSEPass::ReplaceAllUsesWith(ArrayReferenceExpression* original,
					VariableSymbol* newSym,
					StatementList* containingList,
					int position)
{
  assert(original != NULL) ;
  assert(newSym != NULL) ;
  assert(containingList != NULL) ;
  
  int replacedUses = 0 ; 
  
  for (int i = position ; i < containingList->get_statement_count() ; ++i)
  {
    list<LoadExpression*>* allUses = 
      collect_objects<LoadExpression>(containingList->get_statement(i)) ;
    list<LoadExpression*>::iterator useIter = allUses->begin() ;
    while (useIter != allUses->end())
    {
      if (EquivalentExpressions((*useIter)->get_source_address(), original))
      {
	LoadVariableExpression* replacement = 
	  create_load_variable_expression(theEnv,
					  newSym->get_type()->get_base_type(),
					  newSym) ;
	(*useIter)->set_source_address(replacement) ;
	++replacedUses ;
      }
      ++useIter ;
    }
    delete allUses ;
  }
  return replacedUses ;
}

int TemporalCSEPass::GetOffset(ArrayReferenceExpression* x)
{
  assert(x != NULL) ;
  Expression* index = x->get_index() ;
  IntConstant* offset = 
    dynamic_cast<IntConstant*>(index) ;
  BinaryExpression* binOffset = 
    dynamic_cast<BinaryExpression*>(index) ;
  if (offset != NULL)
  {
    return offset->get_value().c_int() ;
  }
  if (binOffset != NULL)
  {
    IntConstant* leftInt = 
      dynamic_cast<IntConstant*>(binOffset->get_source1()) ;
    IntConstant* rightInt = 
      dynamic_cast<IntConstant*>(binOffset->get_source2()) ;
    if (leftInt != NULL)
    {
      return leftInt->get_value().c_int() ;
    }
    if (rightInt != NULL)
    {
      return rightInt->get_value().c_int() ;
    }
  }
  return 0 ;
}
