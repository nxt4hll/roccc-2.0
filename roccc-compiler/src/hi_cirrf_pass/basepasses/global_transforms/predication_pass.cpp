
#include <cassert>
#include <iostream>
#include <sstream>

#include <basicnodes/basic_factory.h>
#include <suifnodes/suif_factory.h>
#include <suifkernel/utilities.h>

#include "predication_pass.h"
#include "roccc_utils/warning_utils.h"
#include "roccc_utils/roccc2.0_utils.h"

PredicationPass::PredicationPass(SuifEnv* pEnv) :
  PipelinablePass(pEnv, "PredicationPass")
{
  theEnv = pEnv ;
  procDef = NULL ;
}

void PredicationPass::do_procedure_definition(ProcedureDefinition* p)
{
  procDef = p ;
  assert(procDef != NULL) ;

  OutputInformation("Predication pass begins") ;

  // Set up all if statements to have statement lists as thier "then" and
  //  "else" parts
  //  PreprocessIfs() ;

  // Convert all ifs to single predications
  ProcessIfs() ;

  // Make sure there is only one level of ifs at the topmost level
  FlattenIfs() ;

  // Move predicate calculation as high as they can get.
  PullPredicatesUp() ;

  // Change the ifs into call statements
  ConvertCalls() ;

  // Determine which variables need to be initialized.
  DetermineUndefinedVariables() ;

  // Add fall throughs to each if statement as the else portion
  PostprocessIfs() ;

  OutputInformation("Predication pass ends") ;
}

void PredicationPass::PreprocessIfs()
{
  assert(procDef != NULL) ;
  list<IfStatement*>* allIfs = 
    collect_objects<IfStatement>(procDef->get_body()) ;
  list<IfStatement*>::iterator ifIter = allIfs->begin() ;
  while (ifIter != allIfs->end())
  {
    PreprocessIf(*ifIter) ;
    ++ifIter ;
  }
  delete allIfs ;
}

// In order for this pass to work, I must find the innermost ifs and process
//  them first.  Since I don't know what order these things are, I'll
//  just do an N squared algorithm at first to see if my logic is sound.

void PredicationPass::ProcessIfs()
{
  assert(procDef != NULL) ;

  bool changed = false ;

  list<IfStatement*>* allIfs = 
    collect_objects<IfStatement>(procDef->get_body()) ;

  while (!(allIfs->empty()))
  {
    // Choose an if statement that is innermost
    list<IfStatement*>::iterator ifIter = allIfs->begin() ;
    while (ifIter != allIfs->end())
    {
      if (IsInnermostIf(*ifIter))
      {
	ProcessIf(*ifIter) ;
	allIfs->erase(ifIter) ;
	break ;
      }
      ++ifIter ;
    }    
  }
  delete allIfs ;
}

// Returns if change occurred
bool PredicationPass::ProcessIf(IfStatement* i)
{  
  assert(theEnv != NULL) ;

  if (i->lookup_annote_by_name("Predicated") != NULL)
  {
    return false ;
  }

  Expression* condition = i->get_condition() ;
  Statement* thenPart = i->get_then_part() ;
  Statement* elsePart = i->get_else_part() ;

  // For every if statement, we need to pull out the conditional expression and
  //  create a new comparison for both the positive and negative  

  assert(condition != NULL) ;
  condition->set_parent(NULL) ;

  QualifiedType* predicateType = 
    create_qualified_type(theEnv, condition->get_result_type()) ;

  VariableSymbol* predicateVariable = 
    create_variable_symbol(theEnv, 
			   predicateType,
			   TempName("predicate")) ;

  predicateVariable->append_annote(create_brick_annote(theEnv, 
						       "PredicateVariable")) ;
  
  procDef->get_symbol_table()->append_symbol_table_object(predicateType) ;
  procDef->get_symbol_table()->append_symbol_table_object(predicateVariable) ;

  StoreVariableStatement* predicate = 
    create_store_variable_statement(theEnv, predicateVariable, condition) ;

  predicate->append_annote(create_brick_annote(theEnv, "PredicateCalculation"));

  LoadVariableExpression* positiveLoad = 
    create_load_variable_expression(theEnv, 
			    predicateVariable->get_type()->get_base_type(),
				    predicateVariable) ;
  LoadVariableExpression* negativeLoad = 
    create_load_variable_expression(theEnv,
			    predicateVariable->get_type()->get_base_type(),
				    predicateVariable) ;
  //  IntConstant* zero = 
  //    create_int_constant(theEnv, 
  //			predicateVariable->get_type()->get_base_type(),
  //			IInteger(0)) ;

  LoadVariableExpression* positiveExp = positiveLoad ;

  /*BinaryExpression* positiveExp = 
    create_binary_expression(theEnv,
			     condition->get_result_type(),
			     LString("is_not_equal_to"),
			     positiveLoad,
			     zero) ;
  */
  UnaryExpression* negativeExp = 
    create_unary_expression(theEnv,
			    condition->get_result_type(),
			    LString("logical_not"),
			    negativeLoad) ;
  
  /*
  BinaryExpression* negativeExp = 
    create_binary_expression(theEnv,
			     condition->get_result_type(),
			     LString("is_equal_to"),
			     negativeLoad,
			     dynamic_cast<Expression*>(zero->deep_clone())) ;
  */
  // Create a statement list for the replacements
  StatementList* replacement = create_statement_list(theEnv) ;

  replacement->append_statement(predicate) ;

  list<IfStatement*> positivePredicates = 
    CreatePredicates(thenPart, positiveExp) ;
  list<IfStatement*> negativePredicates = 
    CreatePredicates(elsePart, negativeExp) ;

  list<IfStatement*>::iterator predIter = positivePredicates.begin() ;
  while (predIter != positivePredicates.end())
  {
    replacement->append_statement(*predIter) ;
    (*predIter)->append_annote(create_brick_annote(theEnv, "Predicated")) ;
    ++predIter ;
  }
  
  predIter = negativePredicates.begin() ;
  while (predIter != negativePredicates.end())
  {
    replacement->append_statement(*predIter) ;
    (*predIter)->append_annote(create_brick_annote(theEnv, "Predicated")) ;
    ++predIter ;
  }

  // Make the replacement
  StatementList* parent = dynamic_cast<StatementList*>(i->get_parent()) ;
  assert(parent != NULL) ;
  assert(parent->get_statement_count() != 0) ;

  // Find the location in the original statement list
  int j ;
  for (j = 0 ; j < parent->get_statement_count() ; ++j)
  {
    if (i == parent->get_statement(j))
    {
      break ;
    }
  }
  assert(j != parent->get_statement_count()) ;

  int k ;
  for (k = 0 ; k < replacement->get_statement_count() ; ++k)
  {
    parent->insert_statement(j + k, 
			     dynamic_cast<Statement*>(replacement->get_statement(k)->deep_clone())) ;
  }
  delete parent->remove_statement(j+k) ;
 
  //  i->get_parent()->replace(i, replacement) ;
  return true ;
}

// The purpose of this function is to make sure that every if statement 
//  contains statement lists for the then and else portions.  If there
//  is no else portion, we don't change that fact.
void PredicationPass::PreprocessIf(IfStatement* i)
{
  Statement* thenPart = i->get_then_part() ;
  Statement* elsePart = i->get_else_part() ;

  if (dynamic_cast<StatementList*>(thenPart) == NULL)
  {
    // Create a new statement list and put it in place of the then
    //  part of the if statement.  Make sure to add the original statement
    //  to the new list.

    StatementList* newThenPart = 
      create_statement_list(theEnv) ;

    // Disconnect ...
    thenPart->set_parent(NULL) ;
    i->set_then_part(NULL) ;

    // And then reconnect
    newThenPart->append_statement(thenPart) ;
    i->set_then_part(newThenPart) ;
  }

  if (dynamic_cast<StatementList*>(elsePart) == NULL &&
      elsePart != NULL)
  {
    StatementList* newElsePart = 
      create_statement_list(theEnv) ;

    // Disconnect...
    elsePart->set_parent(NULL) ;
    i->set_else_part(NULL) ;

    // And reconnect
    newElsePart->append_statement(elsePart) ;
    i->set_else_part(newElsePart) ;
  }
}

list<IfStatement*> PredicationPass::CreatePredicates(Statement* toPredicate,
						     Expression* condition)
{
  assert(theEnv != NULL) ;

  list<IfStatement*> constructed ;

  if (toPredicate == NULL)
  {
    return constructed ;
  }
  
  StatementList* predList = dynamic_cast<StatementList*>(toPredicate) ;
  if (predList != NULL)
  {
    predList = FlattenedList(predList) ;
    assert(predList != NULL) ;
    
    for (int i = 0 ; i < predList->get_statement_count() ; ++i)
    {
      Statement* nextStatement = predList->get_statement(i) ;
      
      IfStatement* toAdd = 
	create_if_statement(theEnv,
			    dynamic_cast<Expression*>(condition->deep_clone()),
			    dynamic_cast<Statement*>(nextStatement->deep_clone())) ;
      constructed.push_back(toAdd) ;
    }
  }
  else // predList == NULL 
  {
    IfStatement* toAdd = 
      create_if_statement(theEnv,
			  dynamic_cast<Expression*>(condition->deep_clone()),
			  dynamic_cast<Statement*>(toPredicate->deep_clone()));
    constructed.push_back(toAdd) ;
  }

  return constructed ;
}

// The purpose of this function is to make all if statements one level.  By the
//  time I call this pass, all of the if statements should have a singular
//  statement in them.
void PredicationPass::FlattenIfs()
{
  assert(procDef != NULL) ;

  bool changed = false ;

  list<IfStatement*>* allIfs = 
    collect_objects<IfStatement>(procDef->get_body()) ;
  list<IfStatement*>::iterator ifIter = allIfs->begin() ;
  while (ifIter != allIfs->end())
  {

    changed = FlattenIf(*ifIter) ;
    
    if (changed == true)
    {
      // We need to restart
      delete allIfs ;
      allIfs = collect_objects<IfStatement>(procDef->get_body()) ;
      ifIter = allIfs->begin() ;
    }
    else
    {
      ++ifIter ;
    }
  }
  delete allIfs ;
}

// No if at this point should have else portions.  Everything should be
//  predicated.
bool PredicationPass::FlattenIf(IfStatement* i)
{
  assert(i != NULL) ;
  assert(i->get_else_part() == NULL) ;

  bool changed = false ;

  Statement* thenPortion = i->get_then_part() ;
  assert(thenPortion != NULL) ;

  IfStatement* thenIf = dynamic_cast<IfStatement*>(thenPortion) ;
  while (thenIf != NULL)
  {
    changed = true ;
    
    assert(thenIf->get_else_part() == NULL) ;

    // Combine the operations
    Expression* topCondition = i->get_condition() ;
    Expression* innerCondition = thenIf->get_condition() ;

    thenIf->set_condition(NULL) ;

    topCondition->set_parent(NULL) ;
    innerCondition->set_parent(NULL) ;

    BinaryExpression* newCondition = 
      create_binary_expression(theEnv,
			       topCondition->get_result_type(),
			       LString("bitwise_and"),
			       dynamic_cast<Expression*>(topCondition->deep_clone()),
			       dynamic_cast<Expression*>(innerCondition->deep_clone())) ;

    i->set_condition(newCondition) ;

    // Propagate the then portion
    Statement* internalThen = thenIf->get_then_part() ;
    internalThen->set_parent(NULL) ;
    i->set_then_part(internalThen) ;
    thenIf->set_then_part(NULL) ;
    thenIf = dynamic_cast<IfStatement*>(internalThen) ;
  }

  return changed ;
}

void PredicationPass::PullPredicatesUp()
{
  assert(procDef != NULL) ;
  list<IfStatement*>* allIfs = 
    collect_objects<IfStatement>(procDef->get_body()) ;
  list<IfStatement*>::iterator ifIter = allIfs->begin() ;
  while (ifIter != allIfs->end()) 
  {
    Statement* thenPortion = (*ifIter)->get_then_part() ;
    assert(thenPortion != NULL) ;
    if (thenPortion->lookup_annote_by_name("PredicateCalculation") != NULL)
    {
      (*ifIter)->set_then_part(NULL) ;
      thenPortion->set_parent(NULL) ;
      (*ifIter)->get_parent()->replace((*ifIter), thenPortion) ;
    }
    ++ifIter ;
  }
  delete allIfs ;
}

// For all of these I'm going to have to check that the variable isn't already
//  located in the list that we are constructing.
list<VariableSymbol*> PredicationPass::FindDefinedVariables(Statement* def)
{
  assert(def != NULL) ;

  list<VariableSymbol*> constructed ;

  // Base cases
  StoreStatement* storeDef = 
    dynamic_cast<StoreStatement*>(def) ;
  StoreVariableStatement* storeVarDef = 
    dynamic_cast<StoreVariableStatement*>(def) ;
  CallStatement* callDef =
    dynamic_cast<CallStatement*>(def) ;

  if (storeDef != NULL)
  {
    // Either an array access or a field symbol
    Expression* dest = storeDef->get_destination_address() ;
    FieldAccessExpression* fieldDest = 
      dynamic_cast<FieldAccessExpression*>(dest) ;
    ArrayReferenceExpression* refDest =
      dynamic_cast<ArrayReferenceExpression*>(dest) ;
    if (fieldDest != NULL)
    {
      constructed.push_back(fieldDest->get_field()) ;
    }
    else if (refDest != NULL)
    {
      // Don't do anything
    }
    else
    {
      assert(0 && "Generic pointers not supported!") ;
    }

  }
  
  if (storeVarDef != NULL)
  {
    constructed.push_back(storeVarDef->get_destination()) ;
  }

  if (callDef != NULL)
  {
    // Add all of the variables defined

    if (callDef->get_destination() != NULL)
    {
      constructed.push_back(callDef->get_destination()) ;
    }

  }

  // Recursive cases
  StatementList* defList = dynamic_cast<StatementList*>(def) ;
  IfStatement* defIf = dynamic_cast<IfStatement*>(def) ;
  
  if (defList != NULL)
  {
    for (int i = 0 ; i < defList->get_statement_count() ; ++i)
    {
      list<VariableSymbol*> tmpList = 
	FindDefinedVariables(defList->get_statement(i)) ;
      list<VariableSymbol*>::iterator tmpIter = tmpList.begin() ;
      while (tmpIter != tmpList.end())
      {
	if (!InList(constructed, *tmpIter))
	{
	  constructed.push_back(*tmpIter) ;
	}
	++tmpIter ;
      }
    }
  }

  if (defIf != NULL)
  {
    list<VariableSymbol*> thenDefined ;
    list<VariableSymbol*> elseDefined ;
    
    if (defIf->get_then_part() != NULL)
    {
      thenDefined = FindDefinedVariables(defIf->get_then_part()) ;
    }
    if (defIf->get_else_part() != NULL)
    {
      elseDefined = FindDefinedVariables(defIf->get_else_part()) ;
    }
    
    list<VariableSymbol*>::iterator defIter = thenDefined.begin() ;
    while (defIter != thenDefined.end())
    {
      if (!InList(constructed, *defIter))
      {
	constructed.push_back(*defIter) ;
      }
      ++defIter ;
    }
    
    defIter = elseDefined.begin() ;
    while (defIter != elseDefined.end())
    {
      if (!InList(constructed, *defIter))
      {
	constructed.push_back(*defIter) ;
      }
      ++defIter ;
    }
  }

  return constructed ;
}

void PredicationPass::PostprocessIfs()
{
  assert(procDef != NULL) ;
  list<IfStatement*>* allIfs = 
    collect_objects<IfStatement>(procDef->get_body()) ;
  list<IfStatement*>::iterator ifIter = allIfs->begin() ;
  while (ifIter != allIfs->end())
  {
    PostprocessIf(*ifIter) ;
    ++ifIter ;
  }
  delete allIfs ;
}

void PredicationPass::PostprocessIf(IfStatement* i)
{
  assert(theEnv != NULL) ;
  assert(i != NULL) ;
  assert(i->get_else_part() == NULL) ;
  assert(i->get_then_part() != NULL) ;
  
  list<VariableSymbol*> defined = FindDefinedVariables(i->get_then_part()) ;
  if (defined.size() == 0)
  {
    // In the case of a single store statement, we will have no
    //  defined variables, but we will still to create a fall through
    if (dynamic_cast<StoreStatement*>(i->get_then_part()) != NULL)
    {
      StoreStatement* originalStatement = 
	dynamic_cast<StoreStatement*>(i->get_then_part()) ;
      StatementList* elsePortion = create_statement_list(theEnv) ;

      // The load value will either be 0 for an array access, or 
      //  a clone of the destination address if it is a field access
      Expression* fallLoad = NULL ;
      if (dynamic_cast<FieldAccessExpression*>(originalStatement->get_destination_address()) != NULL)
      {
	fallLoad = dynamic_cast<Expression*>(originalStatement->get_destination_address()->deep_clone()) ;
      }
      else if (dynamic_cast<ArrayReferenceExpression*>(originalStatement->get_destination_address()) != NULL)
      {
	fallLoad = create_int_constant(theEnv,
				       originalStatement->get_value()->get_result_type(),
				       IInteger(0)) ;
      }
      assert(fallLoad != NULL) ;

      StoreStatement* fallthrough =
	create_store_statement(theEnv,
			       fallLoad,
			       dynamic_cast<Expression*>(originalStatement->get_destination_address()->deep_clone())) ;
      elsePortion->append_statement(fallthrough) ;
      i->set_else_part(elsePortion) ;
    }
    return ;
  }
  
  StatementList* elsePortion = create_statement_list(theEnv) ;

  list<VariableSymbol*>::iterator defIter = defined.begin() ;
  while (defIter != defined.end())
  {    
    Expression* fallLoad ;
    if ((*defIter)->lookup_annote_by_name("PredicateVariable") != NULL)
    {
      delete ((*defIter)->remove_annote_by_name("PredicateVariable")) ;
      fallLoad = create_int_constant(theEnv, 
				     (*defIter)->get_type()->get_base_type(),
				     IInteger(0)) ;
    }
    else if ((*defIter)->lookup_annote_by_name("MiniReplaced") != NULL)
    {
      delete ((*defIter)->remove_annote_by_name("MiniReplaced")) ;
      fallLoad = create_int_constant(theEnv, 
				     (*defIter)->get_type()->get_base_type(),
				     IInteger(0)) ;
    }
    else if ((*defIter)->lookup_annote_by_name("UndefinedVariable") != NULL)
    {
      delete (*defIter)->remove_annote_by_name("UndefinedVariable") ;
      // If I'm in a module, I have to assign 0 as the fall through.
      // If I'm in a system, I have to assign the variable as the fall through

      if ((isModule(procDef) || isLegacyModule(procDef)) && 
	  !hasInfiniteLoop(procDef))
      {	
	fallLoad = create_int_constant(theEnv,
				       (*defIter)->get_type()->get_base_type(),
				       IInteger(0)) ;
      }
      else
      {
	fallLoad = create_load_variable_expression(theEnv, 
				  (*defIter)->get_type()->get_base_type(),
				  (*defIter)) ;
	fallLoad->append_annote(create_brick_annote(theEnv, "UndefinedPath")) ;
      }
    }
    else
    {
      fallLoad = 
	create_load_variable_expression(theEnv,
				       (*defIter)->get_type()->get_base_type(),
					(*defIter)) ;
    }

    Statement* fallthrough ;

    FieldSymbol* storeVar = dynamic_cast<FieldSymbol*>(*defIter) ;
    if (storeVar == NULL)
    {
      fallthrough  = 
	create_store_variable_statement(theEnv,
					(*defIter),
					fallLoad) ;
    }
    else
    {
      // This is an incorrectly formed field access expression, but it will
      //  not be used.  It will instead be eliminated in the if conversion
      //  pass, so it does not need to be correctly formed.
      FieldAccessExpression* fieldLoad =
	create_field_access_expression(theEnv,
				       storeVar->get_type()->get_base_type(),
				       NULL,
				       storeVar) ;

      fallthrough = create_store_statement(theEnv,
					   fallLoad,
					   fieldLoad) ;
    }   
    assert(fallthrough != NULL) ;

    elsePortion->append_statement(fallthrough) ;
    
    ++defIter ;
  }

  i->set_else_part(elsePortion) ;
}

bool PredicationPass::InList(list<VariableSymbol*>& toCheck, 
			     VariableSymbol* var)
{
  list<VariableSymbol*>::iterator varIter = toCheck.begin() ;
  while (varIter != toCheck.end())
  {
    if (*varIter == var)
    {
      return true ;
    }
    ++varIter ;
  }
  return false ;
}

bool PredicationPass::IsInnermostIf(IfStatement* i)
{
  assert(i != NULL) ;
  assert(i->get_then_part() != NULL) ;
  Iter<IfStatement> ifIter = object_iterator<IfStatement>(i->get_then_part()) ;

  while (ifIter.is_valid())
  {
    if (ifIter.current().lookup_annote_by_name("Predicated") == NULL)
    {
      return false ;
    }
    ifIter.next() ;
  }

  if (i->get_else_part() != NULL) 
  {
    ifIter = object_iterator<IfStatement>(i->get_else_part()) ;
    while (ifIter.is_valid())
    {
      if (ifIter.current().lookup_annote_by_name("Predicated") == NULL)
      {
	return false ;
      }
      ifIter.next() ;
    }
  }
  return true ;
}

void PredicationPass::FlattenStatementLists(IfStatement* i)
{
  Statement* thenPart = i->get_then_part() ;
  StatementList* thenList = dynamic_cast<StatementList*>(thenPart) ;
  assert(thenList != NULL) ;
  
  StatementList* constructedThenList = FlattenedList(thenList) ;
  i->set_then_part(constructedThenList) ;

  Statement* elsePart = i->get_else_part() ;
  StatementList* elseList = dynamic_cast<StatementList*>(elsePart) ;  
  if (elseList != NULL)
  {
    StatementList* constructedElseList = FlattenedList(elseList) ;
    i->set_else_part(constructedElseList) ;
  }
}

// In order to flatten a list, I must do the following:
//  1) Create a new statement list
//  2) Append each statement in the original statement list to the new list
//  3) Expand each internal statement list into a flattened list

StatementList* PredicationPass::FlattenedList(StatementList* s)
{
  StatementList* constructedList = create_statement_list(theEnv) ;
  while (s->get_statement_count() > 0) 
  {
    Statement* nextStatement = s->remove_statement(0) ;
    assert(nextStatement != NULL) ;
    StatementList* nextList = dynamic_cast<StatementList*>(nextStatement) ;
    if (nextList == NULL)
    {
      constructedList->append_statement(nextStatement) ;
    }
    else
    {
      nextList = FlattenedList(nextList) ;
      while (nextList->get_statement_count() > 0 )
      {
	Statement* internalStatement = nextList->remove_statement(0) ;
	constructedList->append_statement(internalStatement) ;
      }
    }    
  }

  return constructedList ;
}

void PredicationPass::DetermineUndefinedVariables()
{
  assert(procDef != NULL) ;
  StatementList* innermost = InnermostList(procDef) ;
  assert(innermost != NULL) ;

  list<VariableSymbol*> allDefined ;
  list<VariableSymbol*> usedVariables ;

  for (int i = 0 ; i < innermost->get_statement_count() ; ++i)
  {
    Statement* currentStatement = innermost->get_statement(i) ;
    list<LoadVariableExpression*>* allLoads = 
      collect_objects<LoadVariableExpression>(currentStatement) ;
    list<LoadVariableExpression*>::iterator loadIter = allLoads->begin() ;
    while (loadIter != allLoads->end())
    {
      if (!InList(usedVariables, (*loadIter)->get_source()))
      {
	usedVariables.push_back((*loadIter)->get_source()) ;
      }
      ++loadIter ;
    }
    delete allLoads ;

    if (dynamic_cast<IfStatement*>(currentStatement) == NULL)
    {
      // A normal statement.  Collect all of the defined variables
      list<VariableSymbol*> innerDefined = 
	AllDefinedVariables(currentStatement) ;
      list<VariableSymbol*>::iterator innerIter = innerDefined.begin() ;
      while (innerIter != innerDefined.end())
      {
	if (!InList(allDefined, *innerIter))
	{
	  allDefined.push_back(*innerIter) ;
	}
	++innerIter ;
      }
    }
    else
    {
      // At this point in time, each if statement should only have a then
      //  portion, and it should only be a single statement.
      IfStatement* currentIf = dynamic_cast<IfStatement*>(currentStatement) ;
      Statement* thenPart = currentIf->get_then_part() ;
      assert(thenPart != NULL) ;
      assert(currentIf->get_else_part() == NULL) ;
      assert(dynamic_cast<StatementList*>(thenPart) == NULL) ;
      
      // Find all variables that are defined
      list<VariableSymbol*> thenDefined = AllDefinedVariables(thenPart) ;

      // Remove from the list any variable that are used
      //  previously or in the same statement (these could be feedback).

      list<VariableSymbol*>::iterator usedIter = usedVariables.begin() ;
      while (usedIter != usedVariables.end())
      {
	if (InList(thenDefined, (*usedIter)))
	{
	  for (int k = 0 ; k < thenDefined.size() ; ++k)
	  {
	    if (thenDefined[k] == (*usedIter))
	    {
	      thenDefined.erase(k) ;
	      break ;
	    }
	  }
	}
	++usedIter ;
      }
      
      // For all variables that are defined in the then portion, but not
      //  defined in the total, mark them and then add them
      list<VariableSymbol*>::iterator varIter = thenDefined.begin() ;
      while (varIter != thenDefined.end())
      {
	if (!InList(allDefined, (*varIter)))
	{
	  std::stringstream warning ;
	  warning << "Warning!  A control flow path exists where the variable "
		  << (*varIter)->get_name() << " is undefined!" ;
	  OutputWarning(warning.str().c_str()) ;

	  (*varIter)->append_annote(create_brick_annote(theEnv,
							"UndefinedVariable"));
	  allDefined.push_back(*varIter) ;
	}
	++varIter ;
      }
    }
  }
}

void PredicationPass::ConvertCalls()
{
  assert(procDef != NULL) ;
  
  // All of these ifs are the predicated format.
  list<IfStatement*>* allIfs = 
    collect_objects<IfStatement>(procDef->get_body()) ;

  list<IfStatement*>::iterator ifIter = allIfs->begin() ;
  while (ifIter != allIfs->end())
  {
    // At this point in time, each if statement should only have a then
    //  portion, and it should only be a single statement.
    Statement* thenPart = (*ifIter)->get_then_part() ;
    assert(thenPart != NULL) ;
    assert((*ifIter)->get_else_part() == NULL) ;
    assert(dynamic_cast<StatementList*>(thenPart) == NULL) ;
  
    if (AppropriateCall(thenPart))
    {
      CallStatement* originalCall = dynamic_cast<CallStatement*>(thenPart) ;
      Expression* originalCondition = (*ifIter)->get_condition() ;

      // Create copies for each variable that is defined in the call statement
      list<VariableSymbol*> definedVariables = 
	AllDefinedVariables(originalCall) ;
      list<VariableSymbol*> copies ;
      list<VariableSymbol*>::iterator varIter = definedVariables.begin() ;
      while (varIter != definedVariables.end())
      {
	LString copyName = (*varIter)->get_name() ;
	copyName = copyName + LString("ifCopy") ;
	VariableSymbol* nextCopy = 
	  create_variable_symbol(theEnv,
				 (*varIter)->get_type(),
				 TempName(copyName)) ;
	copies.push_back(nextCopy) ;
	procDef->get_symbol_table()->append_symbol_table_object(nextCopy) ;
	++varIter ;
      }

      // Replace all of the variables with the copies in the call statement
      varIter = definedVariables.begin() ;
      list<VariableSymbol*>::iterator copyIter = copies.begin() ;
      while (varIter != definedVariables.end())
      {
	assert(copyIter != copies.end()) ;
	ReplaceOutputVariable(originalCall, (*varIter), (*copyIter)) ;

	++varIter ;
	++copyIter ;
      }
      
      // Detach the call statement
      originalCall->set_parent(NULL) ;

      // Replace the if statement with the call statement
      SuifObject* parent = (*ifIter)->get_parent() ;
      StatementList* parentList = 
	dynamic_cast<StatementList*>(parent) ;
      assert(parentList != NULL && "Unsupported if structure") ;

      // Find the location in the parent statement list that we need
      //  to put statements behind.
      int ifLocation = -1 ;
      for (int j = 0 ; j < parentList->get_statement_count() ; ++j)
      {
	if (parentList->get_statement(j) == (*ifIter))
	{
	  ifLocation = j ;
	  break ;
	}
      }
      assert(ifLocation != -1) ;

      (*ifIter)->get_parent()->replace((*ifIter), originalCall) ;

      // Create a new if statement to define each variable
      varIter = definedVariables.begin() ;
      copyIter = copies.begin() ;
      while (varIter != definedVariables.end())
      {
	assert(copyIter != copies.end()) ;

	// Create a store variable statement to be the then portion of 
	//  the new if

	LoadVariableExpression* loadCopy = 
	  create_load_variable_expression(theEnv,
				  (*copyIter)->get_type()->get_base_type(),
					  (*copyIter)) ;

	StoreVariableStatement* nextThen = 
	  create_store_variable_statement(theEnv,
					  (*varIter),
					  loadCopy) ;

	IfStatement* addedIf = 
	  create_if_statement(theEnv,
	      dynamic_cast<Expression*>(originalCondition->deep_clone()),
			      nextThen) ;

	// Add all of these if statements to the parent
	parentList->insert_statement(ifLocation + 1, addedIf) ;

	++varIter ;
	++copyIter ;
      }

    }
    ++ifIter ;
  }

  delete allIfs ;
}

bool PredicationPass::AppropriateCall(Statement* s)
{
  CallStatement* call = dynamic_cast<CallStatement*>(s) ;
  if (call == NULL)
  {
    return false ;
  }

  // See if this is a call to bool select or a module.  Only return true if
  //  this is a module.

  Expression* calleeAddress = call->get_callee_address() ;
  SymbolAddressExpression* symAddr = 
    dynamic_cast<SymbolAddressExpression*>(calleeAddress) ;
  assert(symAddr != NULL) ;
  
  LString symbolName = symAddr->get_addressed_symbol()->get_name() ;
  if (strstr(symbolName.c_str(), "ROCCC_boolsel") == NULL)
  {
    return true ;
  }
  else
  {
    return false ;
  }

}
