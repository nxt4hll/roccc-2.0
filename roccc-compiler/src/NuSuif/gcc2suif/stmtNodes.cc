/*

  This file contains the definitions of all of the functions associated with
   the statement nodes in the abstract syntax tree.

*/

#include "suifGenerator.h"

#include <cassert>
#include <iostream>

#include "stmtNodes.h"
#include "declNodes.h"
#include "exprNodes.h"
#include "function.h"
#include "option.h"
#include "program.h"

using namespace std ;

/*********************  ExprStmt **************************/

ExprStmt::ExprStmt(Option* t, Option* l, Option* e, Option* n) :
  Node(), typeop(t), lineop(l), exprop(e), nextop(n)
{
  type = NULL ;
  line = 0 ;
  expr = NULL ;
  next = NULL ;
}

ExprStmt::~ExprStmt()
{
  if (typeop != NULL)
    delete typeop ;
  if (lineop != NULL)
    delete lineop;
  if (exprop != NULL)
    delete exprop;
  if (nextop != NULL)
    delete nextop;
}

void ExprStmt::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;

  if (typeop != NULL)
    typeop->connect(t) ;
  if (lineop != NULL)
    lineop->connect(t);
  if (exprop != NULL)
    exprop->connect(t);
  if (nextop != NULL)
    nextop->connect(t);
}

void ExprStmt::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
  {
    type = NULL ;
  }
  if (lineop != NULL)
  {
    line = lineop->theNumber() ;
    delete lineop ;
    lineop = NULL ;
  }
  else
    line = -1 ;

  if (exprop != NULL)
  {
    expr = exprop->theNodePointer();
    delete exprop ;
    exprop = NULL ;
  }
  else
    expr = NULL ;

  if (nextop != NULL)
  {
    next = nextop->theNodePointer() ;
    delete nextop ;
    nextop = NULL ;
  }
  else
    next = NULL ;
}

// Treat the expression as a statement.  In SUIF, this is done with
//  an eval statement.  
ExecutionObject* ExprStmt::generateSuif(Program* p)
{
  assert(expr != NULL) ;
  ExecutionObject* internal = expr->generateSuif(p) ;
  if (dynamic_cast<Expression*>(internal) != NULL)
  {
    EvalStatement* toReturn = 
      p->converter->globalSuifObjfactory->create_eval_statement() ;
    toReturn->append_expression(dynamic_cast<Expression*>(internal)) ;
    return toReturn ;
  }
  else if (dynamic_cast<Statement*>(internal) != NULL)
  {
    return internal ;
  }
  else
  {
    assert(0 && "Blank expression") ;
  }
    
  assert(0) ;
  return NULL ;
}

/*********************  ForStmt **************************/

ForStmt::ForStmt(Option* t, Option* l, Option* i, Option* c, Option* e, 
		 Option* b, Option* n) :
  Node(), typeop(t), lineop(l), initop(i), condop(c), exprop(e), 
  bodyop(b), nextop(n)
{
  type = NULL ;
  line = -1 ;
  init = NULL ;
  cond = NULL ;
  expr = NULL ;
  body = NULL ;
  next = NULL ;
}

ForStmt::~ForStmt()
{
  if (typeop != NULL)
    delete typeop ;
  if (lineop != NULL)
    delete lineop;
  if (initop != NULL)
    delete initop;
  if (condop != NULL)
    delete condop;
  if (exprop != NULL)
    delete exprop;
  if (bodyop != NULL)
    delete bodyop;
  if (nextop != NULL)
    delete nextop;
}

void ForStmt::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;
  if (typeop != NULL)
    typeop->connect(t) ;
  if (lineop != NULL)
    lineop->connect(t);
  if (initop != NULL)
    initop->connect(t);
  if (condop != NULL)
    condop->connect(t);
  if (exprop != NULL)
    exprop->connect(t);
  if (bodyop != NULL)
    bodyop->connect(t);
  if (nextop != NULL)
    nextop->connect(t);
}

void ForStmt::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
  {
    type = NULL ;
  }
  if (lineop != NULL)
  {
    line = lineop->theNumber() ;
    delete lineop ;
    lineop = NULL ;
  }
  else
    line = -1 ;

  if (initop != NULL)
  {
    init = initop->theNodePointer() ;
    delete initop ;
    initop = NULL ;
  }
  else
    init = NULL ;

  if (condop != NULL)
  {
    cond = condop->theNodePointer() ;
    delete condop ;
    condop = NULL ;
  }
  else
    cond = NULL ;

  if (exprop != NULL)
  {
    expr = exprop->theNodePointer() ;
    delete exprop ;
    exprop = NULL ;
  }
  else
      expr = NULL ;

  if (bodyop != NULL)
  {
    body = bodyop->theNodePointer() ;
    delete bodyop ;
    bodyop = NULL ;
  }
  else
    body = NULL ;

  if (nextop != NULL)
  {
    next = nextop->theNodePointer() ;
    delete nextop ;
    nextop = NULL ;
  }
  else
    next = NULL ;
}

// When creating this loop, I have to go through and make sure that
//  if there is a label associated with this loop, I create an annotation
//  and add it to my SuifObject that I create.  I need to do this with 
//  all loops, but for now I will only concentrate on for loops.
//  Actually, looking a little deeper, I think I can do this at the statement 
//  list level and don't have to worry about anything else.

ExecutionObject* ForStmt::generateSuif(Program* p)
{
  assert(cond != NULL) ;
  assert(expr != NULL) ;
  assert(body != NULL) ;

  Statement* step = dynamic_cast<Statement*>(expr->generateSuif(p)) ;
  Expression* condition = dynamic_cast<Expression*>(cond->generateSuif(p)) ;

  ExecutionObject* tmp = (body->generateSuif(p)) ;
  Statement* bodyStatements ;
  if (dynamic_cast<Statement*>(tmp) != NULL)
  {
    bodyStatements = dynamic_cast<Statement*>(tmp) ;
  }
  else
  {
    assert(dynamic_cast<Expression*>(tmp) != NULL) ;
    // Create a statement that contains the expression
    //  I currently am under the impression that the only thing this
    //  could be is a call expression
    assert(dynamic_cast<CallExpression*>(tmp) != NULL) ;
    // Create a void call statement out of this call expression
    CallExpression* fixme = dynamic_cast<CallExpression*>(tmp) ;
    Expression* calleeAddress = fixme->get_callee_address() ;
    // Disconnect the address from the expression
    calleeAddress->set_parent(NULL) ;
    Statement* converted = p->converter->globalSuifObjfactory->create_call_statement(NULL, calleeAddress) ;
    // Individually add all of the parameters to the call statement
    //  from the call expression
    Iter<Expression*> goThrough = fixme->get_argument_iterator() ;
    for(unsigned int i = 0 ; i < fixme->get_argument_count() ; ++i)
    {
      goThrough.current()->set_parent(NULL) ;
      dynamic_cast<CallStatement*>(converted)->insert_argument(i, goThrough.current()) ;
      goThrough.next() ;
    }

    bodyStatements = converted ;

  }
    
  // before should be an empty list

  Statement* before = p->converter->globalBasicObjfactory->create_statement_list() ;


  return p->converter->createCForStatement(before,
					  condition,
					  step,
					  bodyStatements) ;
  
}

/*********************  IfStmt **************************/

IfStmt::IfStmt(Option* ty, Option* l, Option* c, Option* t, Option* e, 
	       Option* n) :
  Node(), typeop(ty), lineop(l), condop(c), thenop(t), elseop(e), nextop(n)
{
  ;
}

IfStmt::~IfStmt()
{
  if (typeop != NULL)
    delete typeop ;
  if (lineop != NULL)
    delete lineop;
  if (condop != NULL)
    delete condop;
  if (thenop != NULL)
    delete thenop;
  if (elseop != NULL)
    delete elseop;
  if (nextop != NULL)
    delete nextop;
}

void IfStmt::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;

  if(typeop != NULL)
    typeop->connect(t) ;
  if (lineop != NULL)
    lineop->connect(t);
  if (condop != NULL)
    condop->connect(t);
  if (thenop != NULL)
    thenop->connect(t);
  if (elseop != NULL)
    elseop->connect(t);
  if (nextop != NULL)
    nextop->connect(t);
}

void IfStmt::flatten()
{

  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
  {
    type = NULL ;
  }

  if (lineop != NULL)
  {
    line = lineop->theNumber() ;
    delete lineop ;
    lineop = NULL ;
  }
  else
    line = -1 ;

  if (condop != NULL)
  {
    cond = condop->theNodePointer() ;
    delete condop ;
    condop = NULL ;
  }
  else
    cond = NULL ;

  if (thenop != NULL)
  {
    then = thenop->theNodePointer() ;
    delete thenop ;
    thenop = NULL ;
  }
  else
    then = NULL ;

  if (elseop != NULL)
  {
    els = elseop->theNodePointer() ;
    delete elseop ;
    elseop = NULL ;
  }
  else
    els = NULL ;

  if (nextop != NULL)
  {
    next = nextop->theNodePointer() ;
    delete nextop ;
    nextop = NULL ;
  }
  else
    next = NULL ;
}

// Does this work when there is no else portion?
ExecutionObject* IfStmt::generateSuif(Program* p)
{
  assert(cond != NULL) ;
  assert(then != NULL) ;

  Expression* condition = dynamic_cast<Expression*>(cond->generateSuif(p)) ;
  Statement* thenPortion = dynamic_cast<Statement*>(then->generateSuif(p)) ;

  Statement* elsePortion ;

  if (els != NULL)
  {
    elsePortion = dynamic_cast<Statement*>(els->generateSuif(p)) ;
  }
  else
  {
    elsePortion = NULL ;
  }

  return p->converter->createIfStatement(condition, thenPortion, elsePortion) ;
}

/*********************  CleanupStmt **************************/

CleanupStmt::CleanupStmt(Option* l, Option* d, Option* e, Option* n) :
  Node(), lineop(l), declop(d), exprop(e), nextop(n) 
{
  ;
}

CleanupStmt::~CleanupStmt()
{
  if (lineop != NULL)
    delete lineop;
  if (declop != NULL)
    delete declop;
  if (exprop != NULL)
    delete exprop;
  if (nextop != NULL)
    delete nextop;
}

void CleanupStmt::connect(Function* t)
{
  if (!connected)
    connected = true ;
  else
    return ;

  if (lineop != NULL)
    lineop->connect(t);
  if (declop != NULL)
    declop->connect(t);
  if (exprop != NULL)
    exprop->connect(t);
  if (nextop != NULL)
    nextop->connect(t);
}

void CleanupStmt::flatten()
{
  if (lineop != NULL)
  {
    line = lineop->theNodePointer() ;
    delete lineop ;
    lineop = NULL ;
  }
  else
    line = NULL ;

  if (declop != NULL)
  {
    decl = declop->theNodePointer() ;
    delete declop ;
    declop = NULL ;
  }
  else
    decl = NULL ;

  if (exprop != NULL)
  {
    expr = exprop->theNodePointer() ;
    delete exprop ;
    exprop = NULL ;
  }
  else
    expr = NULL ;

  if (nextop != NULL)
  {
    next = nextop->theNodePointer() ;
    delete nextop ;
    nextop = NULL ;
  }
  else
    next = NULL ;
}

/*********************  WhileStmt **************************/

WhileStmt::WhileStmt(Option* t, Option* l, Option* c, Option* b) : Node(),
								   typeop(t),
								   lineop(l),
								   condop(c),
								   bodyop(b)
{
  type = NULL ;
  line = 0 ;
  cond = NULL ;
  body = NULL ;
}

WhileStmt::~WhileStmt()
{
  if (typeop != NULL)
    delete typeop ;
  if (lineop != NULL)
    delete lineop ;
  if (condop != NULL)
    delete condop ;
  if (bodyop != NULL)
    delete bodyop ;
}

void WhileStmt::connect(Function* t) 
{
  if (!connected)
  {
    connected = true ;
  }
  else
  {
    return ;
  }
  if (typeop != NULL)
    typeop->connect(t) ;
  if (condop != NULL)
    condop->connect(t) ;
  if (bodyop != NULL)
    bodyop->connect(t) ;
}

void WhileStmt::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
  {
    type = NULL ;
  }
  line = lineop->theNumber() ;
  if (condop != NULL)
  {
    cond = condop->theNodePointer() ;
    delete condop ;
    condop = NULL ;
  }
  else
  {
    cond = NULL ;
  }
  if (bodyop != NULL)
  {
    body = bodyop->theNodePointer() ;
    delete bodyop ;
    bodyop = NULL ;
  }
  else
  {
    body = NULL ;
  }
}

ExecutionObject* WhileStmt::generateSuif(Program* p)
{
  assert(cond != NULL) ;
  assert(body != NULL) ;
  Expression* conditionCode = 
    dynamic_cast<Expression*>(cond->generateSuif(p)) ;
  Statement* bodyCode = dynamic_cast<Statement*>(body->generateSuif(p)) ;

  return p->converter->createWhileStatement(conditionCode, bodyCode) ;
}

/*********************  DoStmt **************************/

DoStmt::DoStmt(Option* t, Option* l, Option* b, Option* c) : Node(),
							     typeop(t),
							     lineop(l),
							     bodyop(b),
							     condop(c)

{
  type = NULL ;
  line = 0 ;
  body = NULL ;
  cond = NULL ;
}

DoStmt::~DoStmt()
{
  if (typeop != NULL)
    delete typeop ;
  if (lineop != NULL)
    delete lineop ;
  if (bodyop != NULL)
    delete bodyop ;
  if (condop != NULL)
    delete condop ;
}

void DoStmt::connect(Function* t) 
{
  if (!connected)
  {
    connected = true ;
  }
  else
  {
    return ;
  }
  if (typeop != NULL)
    typeop->connect(t) ;
  if (bodyop != NULL)
    bodyop->connect(t) ;
  if (condop != NULL)
    condop->connect(t) ;
}

void DoStmt::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
  {
    type = NULL ;
  }
  line = lineop->theNumber() ;
  if (bodyop != NULL)
  {
    body = bodyop->theNodePointer() ;
    delete bodyop ;
    bodyop = NULL ;
  }
  else
  {
    body = NULL ;
  }
  if (condop != NULL)
  {
    cond = condop->theNodePointer() ;
    delete condop ;
    condop = NULL ;
  }
  else
  {
    cond = NULL ;
  }
}

ExecutionObject* DoStmt::generateSuif(Program* p)
{
  assert(body != NULL) ;
  assert(cond != NULL) ;
  
  Statement* bodyStatements  = dynamic_cast<Statement*>(body->generateSuif(p));
  Expression* conditionExpression = 
    dynamic_cast<Expression*>(cond->generateSuif(p)) ;

  return p->converter->createDoWhileStatement(conditionExpression,
					      bodyStatements) ;
  
}

/********************* SwitchStmt **************************/

SwitchStmt::SwitchStmt(Option* t, Option* l, Option* c, Option* b) : Node(),
								     typeop(t),
								     lineop(l),
								     condop(c),
								     bodyop(b)
{
  type = NULL ;
  line = -1 ;
  cond = NULL ;
  body = NULL ;
}

SwitchStmt::~SwitchStmt()
{
  if (typeop != NULL)
    delete typeop ;
  if (lineop != NULL)
    delete lineop ;
  if (condop != NULL)
    delete condop ;
  if (bodyop != NULL)
    delete bodyop ;
}

void SwitchStmt::connect(Function* t)
{
  if (!connected)
  {
    connected = true ;
  }
  else
  {
    return ;
  }
  if (typeop != NULL)
    typeop->connect(t) ;
  if (condop != NULL)
    condop->connect(t) ;
  if (bodyop != NULL)
    bodyop->connect(t) ;
}

void SwitchStmt::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
  {
    type = NULL ;
  }

  line = lineop->theNumber() ;

  if (condop != NULL)
  {
    cond = condop->theNodePointer() ;
    delete condop ;
    condop = NULL ;
  }
  else
  {
    cond = NULL ;
  }


  if (bodyop != NULL)
  {
    body = bodyop->theNodePointer() ;
    delete bodyop ;
    bodyop = NULL ;
  }
  else
  {
    body = NULL ;
  }
}

/*
 Make a statement list with the switch being first, and then 
  the body being second.  Convert "break" statements into goto 
  statements and then place a code label symbol for the end of
  the switch at the end of this statement list.

  Also, place the labels we generate in the symbol table.

*/

ExecutionObject* SwitchStmt::generateSuif(Program* p)
{

  assert(cond != NULL) ;

  // Generate all of the code associated with this switch statment, 
  //  and then create a Multi-way branch statement and return it.

  // First, create the condition

  Expression* condition = dynamic_cast<Expression*>(cond->generateSuif(p)) ;
  assert(condition != NULL) ;

  // There might not be a default target
  CodeLabelSymbol* defaultTarget = NULL ;

  CodeLabelSymbol* afterSwitch = p->converter->globalBasicObjfactory->create_code_label_symbol(p->converter->globalTb->get_label_type(), "") ;

  // Add this to the global symbol table

  p->converter->addLabelSymbolToCurrentSymbolTable(afterSwitch) ;

  p->converter->pushBreakTarget(afterSwitch) ;

  // Create the set of statements associated with the body of the 
  //  case statement (these go after the multiway branch statement)

  StatementList* suifBody = NULL ;
  if (body != NULL)
  {
    suifBody = dynamic_cast<StatementList*>(body->generateSuif(p)) ; 
  }
  else
  {
    suifBody = p->converter->globalBasicObjfactory->create_statement_list() ;
  }

  p->converter->popBreakTarget() ;

  // Attach the end label to this statement list...

  assert(suifBody != NULL) ;

  LabelLocationStatement* afterSwitchLocation = p->converter->globalSuifObjfactory->create_label_location_statement(afterSwitch) ;

  suifBody->append_statement(afterSwitchLocation) ;
  
  // Find the default label in the statement list
  defaultTarget = findDefault(suifBody, p) ;
  
  // If we didn't find it, we have to set the default label to the
  //  end label that we attached to the statement list.

  if (defaultTarget == NULL)
    defaultTarget = afterSwitch ;

  MultiWayBranchStatement* theSwitch = p->converter->globalSuifObjfactory->create_multi_way_branch_statement(condition, defaultTarget) ;

  SuifObjectBrick* labelAnnote = p->converter->globalBasicObjfactory->
    create_suif_object_brick(afterSwitch) ;

  BrickAnnote* theActualAnnote = p->converter->globalBasicObjfactory->
    create_brick_annote("end_label") ;

  theActualAnnote->append_brick(labelAnnote) ;

  // Add an annotation to the switch that says what the 
  //  after switch label is
  theSwitch->append_annote(theActualAnnote);

  // Add all of the labels individually to theSwitch

  list<CaseLabelExpr*> allCases ;

  allCases = body->collectCaseLabels() ;

  list<CaseLabelExpr*>::iterator addCasesIter = allCases.begin();

  while(addCasesIter != allCases.end())
  {

    theSwitch->add_case((*addCasesIter)->theSize(), 
			(*addCasesIter)->theSuifSymbol()) ;

    ++addCasesIter ;
  }

  // Prepend the switch to the body, and then return the body

  suifBody->insert_statement(0, theSwitch) ;

  return suifBody ;

}

CodeLabelSymbol* SwitchStmt::findDefault(StatementList* caseCode, Program* p)
{

  if (caseCode == NULL)
    return NULL ;

  // Go through all the statements in the list and check
  Iter<Statement*> findIter = caseCode->get_statement_iterator() ;

  while(findIter.is_valid())
  {
    if(is_a<LabelLocationStatement*>(findIter.current()))
    {
      CodeLabelSymbol* currentLabel = 
	dynamic_cast<LabelLocationStatement*>(findIter.current())->get_defined_label() ;
      if(currentLabel->get_name() != LString(""))
      {
	return currentLabel ;
      }
    }
    findIter.next();
  }

  // If I haven't found it, return NULL
  return NULL ;
}

/********************* BreakStmt **************************/

BreakStmt::BreakStmt(Option* t, Option* l) : typeop(t), lineop(l)
{
  type = NULL ;
  line = -1 ;
}

BreakStmt::~BreakStmt()
{
  if (typeop != NULL)
    delete typeop ;
}

void BreakStmt::connect(Function* t) 
{
  if (!connected)
  {
    connected = true ;
  }
  else
  {
    return ;
  }
  if (typeop != NULL)
    typeop->connect(t) ;
}

void BreakStmt::flatten()
{
  if (typeop != NULL)
  {
    type = typeop->theNodePointer() ;
    delete typeop ;
    typeop = NULL ;
  }
  else
  {
    type = NULL ;
  }
  line = lineop->theNumber() ;
}

ExecutionObject* BreakStmt::generateSuif(Program* p)
{

  // Break statements need to know if we are inside a switch statement
  //  or a loop nest.  In other words, there should be some global 
  //  label stack that contains the label that the break should go to.
  //  Loop nests and switch statements should update this stack
  //  and break statements should only look at that stack.  Because global
  //  variables are going to cause me grief, I'll put the label stack in 
  //  the suifGenerator!

  return p->converter->globalSuifObjfactory->create_jump_statement(p->converter->currentBreakTarget()) ;

}
