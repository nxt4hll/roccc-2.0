
// Included John's code
#include "suifGenerator.h"

#include <assert.h>
#include <stdio.h>

#include "nodes.h"
#include "typeNodes.h"
#include "declNodes.h"
#include "exprNodes.h"
#include "cstNodes.h"
#include "option.h"
#include "generic.h"
#include "function.h"

using namespace std;

/********************* Statement List Functions ************************/

StatementListMine::~StatementListMine()
{
  if (ops != NULL)
    delete ops ;
}

void StatementListMine::connect(Function* t)
{
  if (!connected)
  {
    connected = true ;
  }
  else
  {
    return ;
  }
  if (ops == NULL) 
    return ;
  list<opOptions>::iterator connectIter = ops->begin() ;
  while(connectIter != ops->end())
  {
    (*connectIter).nodePointer=(*(t->allNodes))[(*connectIter).nodeNumber - 1];
    ++connectIter ;
  }
}

void StatementListMine::flatten()
{
  // Nothing to do in here
  ; 
}

// In here, I have to add annotations to for loops if they happen to have a 
//  label attached.  This generate suif routine has a little more 
//  responsibility than it should.

ExecutionObject* StatementListMine::generateSuif(Program* p)
{
  // Make a statement list and then append individual statments to it
  StatementList* allStatements = p->converter->globalBasicObjfactory->create_statement_list() ;

  list<opOptions>::iterator suifIter = ops->begin() ;
  ExecutionObject* twoAgo = NULL ;
  ExecutionObject* oneAgo = NULL ;
  

  while (suifIter != ops->end())
  {

    ExecutionObject* nextToAdd = (*suifIter).nodePointer->generateSuif(p) ;

    if (is_a<Statement*>(nextToAdd))
    {
      allStatements->append_statement(dynamic_cast<Statement*>(nextToAdd)) ;

      // If it just so happens to be a loop, look for a preceding label 
      //  and annotate the loop with that information.  The annotation
      //  should have the name "c_for_label" and have the value of the label

      if(is_a<CForStatement*>(nextToAdd) || is_a<WhileStatement*>(nextToAdd))
      {
	// Check the one ago and two ago
	if (is_a<LabelLocationStatement*>(oneAgo))
	{
	  BrickAnnote* message = p->converter->globalBasicObjfactory->
	    create_brick_annote("c_for_label") ;
	  StringBrick* labelName = p->converter->globalBasicObjfactory->
	    create_string_brick(dynamic_cast<LabelLocationStatement*>(oneAgo)->get_defined_label()->get_name()) ;

	  message->append_brick(labelName) ;

	  nextToAdd->append_annote(message) ;
	}
	else if (is_a<LabelLocationStatement*>(twoAgo))
	{
	  BrickAnnote* message = p->converter->globalBasicObjfactory->
	    create_brick_annote("c_for_label") ;
	  StringBrick* labelName = p->converter->globalBasicObjfactory->
	    create_string_brick(dynamic_cast<LabelLocationStatement*>(twoAgo)->get_defined_label()->get_name()) ;

	  message->append_brick(labelName) ;

	  nextToAdd->append_annote(message) ;

	}
      }
    }
    else if (is_a<Expression*>(nextToAdd))
    {
      // Make a statement out of this expression
      if (is_a<CallExpression*>(nextToAdd))
      {
       	// This is a void function call.

	Expression* calleeAddress = dynamic_cast<CallExpression*>(nextToAdd)->get_callee_address() ;

	// Disconnect the address from the expression
       
	calleeAddress->set_parent(NULL) ;

	Statement* converted = p->converter->globalSuifObjfactory->create_call_statement(NULL, calleeAddress) ;

	// I also must go through and add all of the parameters and add
	//  them to the call statement (stupid SUIF)

	CallExpression* exprPointer = dynamic_cast<CallExpression*>(nextToAdd);

	Iter<Expression*> goThrough = exprPointer->get_argument_iterator() ;

	for(unsigned int i = 0 ; i < exprPointer->get_argument_count(); ++i)
	{

	  goThrough.current()->set_parent(NULL) ;

	  dynamic_cast<CallStatement*>(converted)->insert_argument(i, goThrough.current()) ;
	  goThrough.next();
	}

	allStatements->append_statement(dynamic_cast<Statement*>(converted)) ;
      }
      // What other type of expression could this be?
      else
      {
	cerr << "Error: Unsupported code."
	     << endl 
	     << "Initializations and code with no effect are not supported"
	     << endl ;
	assert(0) ;
      }
    }
    else
    {
      // Don't do anything yet
    }

    twoAgo = oneAgo ;
    oneAgo = nextToAdd ;
    ++suifIter ;
  }
  
  return allStatements ;  
}

list<CaseLabelExpr*> StatementListMine::collectCaseLabels()
{
  list<CaseLabelExpr*> toFill ;

  list<opOptions>::iterator caseIter = ops->begin() ;  
  while(caseIter != ops->end())
  {

    Node* nextToCheck = (*caseIter).nodePointer ;
    CaseLabelExpr* nextCasted = dynamic_cast<CaseLabelExpr*>(nextToCheck) ;
    // I'm taking advantage of short circuit evaluation to make sure no
    //  seg faults happen
    if (nextCasted != NULL && !nextCasted->isDefaultCase())
    {
      toFill.push_back(dynamic_cast<CaseLabelExpr*>(nextToCheck)) ;
    }
    
    ++caseIter ;
  }

  return toFill ;
}

/********************* ArrayRef Functions ************************/

ArrayRef::~ArrayRef()
{
  if (typeop != NULL)
    delete typeop ;
  if (ops != NULL)
    delete ops;
}

void ArrayRef::connect(Function* t)
{
  if (!connected)
  {
    connected = true;
  }
  else
  {
    return;
  }
  if (typeop != NULL)
    typeop->connect(t) ;
  if (ops == NULL)
    return ;
  list<opOptions>::iterator connectIter = ops->begin();
  while (connectIter != ops->end())
  {
    (*connectIter).nodePointer=(*(t->allNodes))[(*connectIter).nodeNumber - 1];
    ++connectIter;
  }  
}

void ArrayRef::flatten()
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
}

// Array ref should return a load expression at the base, but 
//  when dealing with the lower levels of a multidimensional array 
//  should return a symbol address expression.
//  Unfortunately, an multidimensional array ref could have the first 
//  operand be the recursive array reference call, meaning that the 
//  final thing returned is the symbol address expression rather than 
//  the load expression (and the load expression ends up in the middle
//  of the array reference in the suif code which causes problems in 
//  suif passes.

ExecutionObject* ArrayRef::generateSuif(Program* p)
{
  assert(ops != NULL) ;
  assert(ops->size() == 2) ;

  list<opOptions>::iterator arrayIter = ops->begin() ;

  Node* base = (*arrayIter).nodePointer ;
  ++arrayIter ;
  Node* index = (*arrayIter).nodePointer ;

  // The base could be either a var decl, parm decl, array ref, or
  //  component ref, and we are going to have to do different things for
  //  these cases.

  if (is_a<VarDecl*>(base) || is_a<ParmDecl*>(base) || 
      is_a<ComponentRef*>(base))
  {
    // The overall goal here is to return a load expression
    //  The load expression needs the base type of the variable
    //  and an array reference expression.

    // Here is the code to get the base type

    DataType* baseType ;
    Type* bType = base->suifType(p) ;
    assert(bType != NULL) ;

    if(is_a<QualifiedType*>(bType))
    {
      baseType = dynamic_cast<QualifiedType*>(bType)->get_base_type() ;
    }
    else if (is_a<DataType*>(bType))
    {
      baseType = dynamic_cast<DataType*>(bType) ;
    }
    else
    {
      assert(0) ;
    }

    // Find the base type
    while (is_a<ArrayType*>(baseType))
    {
      baseType = dynamic_cast<ArrayType*>(baseType)->get_element_type()->
	get_base_type() ;
    }

    // Here is the code to build the array reference expression
    //  The array reference expression requires the base type, 
    //  an expression that references the address, and the index
    Expression* indexExp = dynamic_cast<Expression*>(index->generateSuif(p)) ;

    Symbol* baseVar = base->getVariable(p);

    Expression* symAddrExp = p->converter->globalSuifObjfactory->
      create_symbol_address_expression(baseType, baseVar) ;

    ArrayReferenceExpression* arrayRefExp = p->converter->
      globalSuifObjfactory->
      create_array_reference_expression(baseType, symAddrExp, indexExp) ;

    // We're done!
    return p->converter->globalSuifObjfactory->
      create_load_expression(baseType, arrayRefExp);    
  }
  else if (is_a<ArrayRef*>(base))
  {
    // We are in the middle of a multidimensional array access

    VariableSymbol* baseVar = dynamic_cast<VariableSymbol*>(base->
							    getVariable(p));
    DataType* baseType = baseVar->get_type()->get_base_type() ;
    Expression* symAddrExp = dynamic_cast<Expression*>(base->generateSuif(p)) ;
    Expression* indexExp = dynamic_cast<Expression*>(index->generateSuif(p)) ;

    if (is_a<LoadExpression*>(symAddrExp))
    {
      // Get the symbol address of this load expression and create a new
      //  load expression

      Expression* arrayRefAddress = dynamic_cast<LoadExpression*>(symAddrExp)
	->get_source_address() ;
      dynamic_cast<LoadExpression*>(symAddrExp)->set_source_address(NULL) ;

      while (is_a<ArrayType*>(baseType))
      {
	baseType = dynamic_cast<ArrayType*>(baseType)->get_element_type()->
	  get_base_type() ;
      }

      ArrayReferenceExpression* stupidTemp = p->converter->
	globalSuifObjfactory->
	create_array_reference_expression(baseType, arrayRefAddress, indexExp);

      return p->converter->globalSuifObjfactory->
	create_load_expression(baseType, stupidTemp);
    }
    else
    {
      assert(0) ;
      return NULL ;
    }
  }
  else
  {
    // What is this?
    assert(0) ;
    return NULL ;
  }

}

Symbol* ArrayRef::getVariable(Program* p)
{
  assert(ops != NULL) ;
  assert(ops->size() == 2) ;

  list<opOptions>::iterator arrayIter = ops->begin() ;

  Node* base = (*arrayIter).nodePointer ;

  // Keep recursing until you hit a VarDecl node
  assert(base != NULL) ;
  return base->getVariable(p) ;
}

/********************* Constructor Functions ************************/

Constructor::~Constructor()
{
  if (typeop != NULL)
    delete typeop;
  if (eltsop != NULL)
    delete eltsop;
}

void Constructor::connect(Function* t)
{
  if (!connected)
  {
    connected = true;
  }
  else
  {
    return;
  }
  if (typeop != NULL)
    typeop->connect(t);
  if (eltsop != NULL)
    eltsop->connect(t);
}

void Constructor::flatten()
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
  if (eltsop != NULL)
  {
    elts = eltsop->theNodePointer() ;
    delete eltsop ;
    eltsop = NULL ;
  }
  else
  {
    elts = NULL ;
  }
}

ValueBlock* Constructor::generateSuifInitialization(Program* p,
						    ValueBlock* topLevel,
						    int currentLevel,
						    Type* elementType)
{
  assert(elts != NULL) ;
  assert(type != NULL) ;

  // I am assuming that constructors are for initializations of arrays
  //  (either single dimensional or multi-dimensional) and this code
  //  will break horribly if constructors are ever used for anything else.


  // In order to support user defined types, I must determine what the
  //  element type of this array currently is.  This could be either 
  //  a base type (like integer) or an array type in itself.

  // Jason here, for some reason this was declared incorrectly and never used.
  //  Type* elemntType = NULL ;

  Type* tempType = type->suifType(p) ;

  if (is_a<ArrayType*>(tempType))
  {
    elementType = dynamic_cast<ArrayType*>(tempType)->get_element_type()->get_base_type() ;
  }  

  return elts->generateSuifInitialization(p, topLevel, currentLevel, 
					  elementType) ;
  
}

ExecutionObject* Constructor::generateSuif(Program* p)
{
  assert(elts != NULL) ;
  return elts->generateSuif(p) ;
}

Type* Constructor::suifType(Program* p)
{
  assert(type != NULL) ;
  return type->suifType(p) ;
}

/********************* TreeList Functions ************************/

TreeList::~TreeList()
{
  if (purpop != NULL)
    delete purpop;
  if (valuop != NULL)
    delete valuop;
  if (chanop != NULL)
    delete chanop;
}

void TreeList::connect(Function* t)
{
  if (!connected)
  {
    connected = true;
  }
  else
  {
    return;
  }

  if (purpop != NULL)
    purpop->connect(t);
  if (valuop != NULL)
    valuop->connect(t);
  if (chanop != NULL)
    chanop->connect(t);
}

void TreeList::flatten()
{
  if (purpop != NULL)
  {
    purp = purpop->theNodePointer() ;
    delete purpop;
    purpop = NULL;
  }
  else
  {
    purp = NULL ;
  }
  if (valuop != NULL)
  {
    valu = valuop->theNodePointer() ;
    delete valuop;
    valuop = NULL ;
  }
  else
  {
    valu = NULL ;
  }
  if (chanop != NULL)
  {
    chan = chanop->theNodePointer() ;
    delete chanop;
    chanop = NULL ;
  }
  else
  {
    chan = NULL;
  }
 
}

LString TreeList::resultRegister()
{
  if (valu != NULL)
    return valu->resultRegister();
  return "" ;
}

ValueBlock* TreeList::generateSuifInitialization(Program* p,
						 ValueBlock* topLevel,
						 int currentLevel,
						 Type* elementType)
{
  assert(valu != NULL) ;
  assert(elementType != NULL) ;
  assert(is_a<DataType*>(elementType)) ;

  // The element type must have been passed in from the Constructor function,
  //  so I can use that where appropriate.

  MultiValueBlock* theChain = NULL ;
  
  if (topLevel == NULL)
  {
    theChain = p->converter->globalSuifObjfactory->
      create_multi_value_block(dynamic_cast<DataType*>(elementType)) ;
  }
  else
  {
    theChain = dynamic_cast<MultiValueBlock*>(topLevel) ;
  }
  
  if (is_a<Constructor*>(valu))
  {
    // This is a multidimensional array, so start that call fresh
    ValueBlock* myBlock = valu->generateSuifInitialization(p,
							   NULL,
							   0,
							   NULL) ;
    theChain->add_sub_block(IInteger(currentLevel), myBlock) ;
  }
  else if (is_a<IntegerCst*>(valu))
  {
    // Just attach our value
    ValueBlock* myBlock = valu->generateSuifInitialization(p,
							   topLevel,
							   currentLevel,
							   elementType) ;
    theChain->add_sub_block(IInteger(currentLevel), myBlock) ;
  }
  else
  {
    // Base case, just attach our value
    Expression* myValue = dynamic_cast<Expression*>(valu->generateSuif(p)) ;
    ValueBlock* myBlock = p->converter->createExpressionValueBlock(myValue) ;
    theChain->add_sub_block(IInteger(currentLevel), myBlock) ;
  }

  // Continue down the chain if necessary
  if (chan != NULL)
  {

    //    int elementSize = ((dynamic_cast<DataType*>(elementType))->get_bit_size()).c_long() ;

    chan->generateSuifInitialization(p, theChain, 
				     currentLevel + 1, // +  elementSize, 
				     elementType) ;
  }

  return theChain ;


  /*  
  */
}

ExecutionObject* TreeList::generateSuif(Program* p)
{
  if (valu != NULL)
    return valu->generateSuif(p) ;
  else
    return NULL ;
}

// This tree-list just happens to be a listing of parameters
list<QualifiedType*> TreeList::findParameters(Program* p)
{
  list<QualifiedType*> buildUp ;
  if (chan != NULL)
  {
    buildUp = chan->findParameters(p) ;
  }
  QualifiedType* nextType = p->converter->globalTb->get_qualified_type(valu->suifType(p)) ;
  buildUp.push_front(nextType) ;
  return buildUp ;
}

// This tree-list just happens to be a listing of arguments
list<Expression*> TreeList::findArguments(Program* p)
{
  list<Expression*> buildUp ;
  if (chan != NULL)
  {
    buildUp = chan->findArguments(p) ;
  }
  Expression* currentArg = dynamic_cast<Expression*>(valu->generateSuif(p)) ;

  buildUp.push_front(currentArg) ;

  return buildUp ;  
}

/********************* IndirectRef Functions ************************/

IndirectRef::~IndirectRef()
{
  if (typeop != NULL)
    delete typeop ;
  if (ops != NULL)
    delete ops;
}

void IndirectRef::connect(Function* t)
{
  if (!connected)
  {
    connected = true;
  }
  else
  {
    return;
  }
  if (typeop != NULL)
    typeop->connect(t) ;
  if (ops == NULL)
    return ;
  list<opOptions>::iterator connectIter = ops->begin();
  while (connectIter != ops->end())
  {
    (*connectIter).nodePointer=(*(t->allNodes))[(*connectIter).nodeNumber - 1];
    ++connectIter;
  }  
}

void IndirectRef::flatten()
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
}

// Used for loads (when the indirect ref is on the right side of the
//  equal sign).

ExecutionObject* IndirectRef::generateSuif(Program* p)
{
  assert(type != NULL) ;
  assert(ops != NULL) ;
  assert(ops->size() == 1) ;

  //  DataType* loadType = (p->converter->globalTb->get_qualified_type(type->suifType(p)))->get_base_type() ;

  list<opOptions>::iterator loadIter = ops->begin() ;
  
  Expression* loadExpression = 
    dynamic_cast<Expression*>((*loadIter).nodePointer->generateSuif(p)) ;

  DataType* loadType = 
    loadExpression->get_result_type() ;

  return p->converter->globalSuifObjfactory->create_load_expression(loadType, loadExpression) ;
}

// Used only because Suif treats indirect references on the left hand
//  side of the equal sign as completely different from when indirect
//  references are on the right hand side of an equal sign.
//  All this function basically does is remove one level of indirection
//  (which is going to be added by the store statement anyway).

ExecutionObject* IndirectRef::generateStore(Program* p)
{
  assert(type != NULL) ;
  assert(ops != NULL) ;
  assert(ops->size() == 1) ;

  list<opOptions>::iterator loadIter = ops->begin() ;
  
  Expression* loadExpression = 
    dynamic_cast<Expression*>((*loadIter).nodePointer->generateSuif(p)) ;

  return loadExpression ;

}

Symbol* IndirectRef::getVariable(Program* p) 
{
  return NULL ;
  // What I'd like to do is return a dereference to my actual variable (and 
  //  then return that).  So for an IndirectRef I'd like to somehow 
  //  create a load expression and return the symbol for that.
  /*
  return p->converter->globalSuifObjfactory->create_load_expression(loadType, loadExpression) ;

  VariableSymbol
  */
}

/********************* ComponentRef Functions ************************/

ComponentRef::~ComponentRef()
{
  if (typeop != NULL)
    delete typeop ;
  if (ops != NULL)
    delete ops;
}

void ComponentRef::connect(Function* t)
{
  if (!connected)
  {
    connected = true;
  }
  else
  {
    return;
  }
  if (typeop != NULL)
    typeop->connect(t) ;
  if (ops == NULL)
    return ;
  list<opOptions>::iterator connectIter = ops->begin();
  while (connectIter != ops->end())
  {
    (*connectIter).nodePointer=(*(t->allNodes))[(*connectIter).nodeNumber - 1];
    ++connectIter;
  }  
}

void ComponentRef::flatten()
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
}

ExecutionObject* ComponentRef::generateSuif(Program* p)
{

  assert(ops->size() == 2) ;

  list<opOptions>::iterator opIter = ops->begin() ;

  // The overall goal of this function is to return a field access
  //  expression.  I need the result type (the type of the field symbol),
  //  the field symbol, and an expression that refers to the base
  //  address of the struct variable.

  Node* baseVar = (*opIter).nodePointer ;
  ++opIter ;
  Node* fieldVar = (*opIter).nodePointer ;

  // The base struct should be either a parameter to the function
  //  or a variable declared in the function.

  assert(dynamic_cast<ParmDecl*>(baseVar) != NULL ||
	 dynamic_cast<VarDecl*>(baseVar) != NULL) ;

  Node* structType ;

  if (dynamic_cast<ParmDecl*>(baseVar))
  {
    structType = (dynamic_cast<ParmDecl*>(baseVar))->type ;
  }
  else
  {
    structType = (dynamic_cast<VarDecl*>(baseVar))->type ;
  }    

  Type* suifStructType = structType->suifType(p) ;

  assert(dynamic_cast<GroupType*>(suifStructType) != NULL) ;

  GroupSymbolTable* structFieldsTable = dynamic_cast<GroupType*>(suifStructType)->get_group_symbol_table() ;

  assert(structFieldsTable != NULL) ;

  FieldSymbol* field = dynamic_cast<FieldSymbol*>(fieldVar->getVariable(p)) ;

  assert(field != NULL) ;

  // We finally have the SUIF symbol for the field, now we must get the 
  //  SUIF type of this field

  DataType* resultType ;

  if (!(structFieldsTable->has_symbol_table_object_member(field)))
  {
    structFieldsTable->append_symbol_table_object(field) ;
  }

  resultType = dynamic_cast<DataType*>(field->get_type()->get_base_type()) ;

  assert(resultType != NULL) ;

  // The last thing we need is a pointer to the base variable.
  //  When we generate suif on the base var, we get back
  //  a LoadVariableExpression, and we must get the variable out of this
  //  and create a pointer to it.

  Expression* loadBaseVar=dynamic_cast<Expression*>(baseVar->generateSuif(p));

  assert(loadBaseVar != NULL) ;
  assert(dynamic_cast<LoadVariableExpression*>(loadBaseVar) != NULL) ;

  LoadVariableExpression* tempInstruction = dynamic_cast<LoadVariableExpression*>(loadBaseVar) ;

  DataType* pointerToStruct = p->converter->globalSuifObjfactory->
    create_pointer_type(32, 
			32, 
			tempInstruction->get_source()->
			                 get_type()->get_base_type()) ;

  SymbolAddressExpression* addressOfStruct = p->converter->globalSuifObjfactory
    ->create_symbol_address_expression(pointerToStruct,
				       tempInstruction->get_source()) ;

  return p->converter->globalSuifObjfactory->
    create_field_access_expression(resultType,
				   addressOfStruct,
				   field
				   ) ;

}

Type* ComponentRef::suifType(Program* p)
{
  assert(type != NULL) ;
  return type->suifType(p) ;
}

Symbol* ComponentRef::getVariable(Program* p)
{
  assert(ops != NULL) ;
  assert(ops->size() == 2) ;

  list<opOptions>::iterator findIt = ops->begin() ;
  ++findIt ;
  assert(dynamic_cast<FieldDecl*>((*findIt).nodePointer) != NULL) ;
  return (*findIt).nodePointer->getVariable(p) ;
}

/********************* Binfo Functions ************************/

Binfo::~Binfo()
{
  if (typeop != NULL)
    delete typeop ;
  if (basesop != NULL)
    delete basesop ;
}

void Binfo::connect(Function* t)
{
  if (typeop != NULL)
    typeop->connect(t) ;
  if (basesop != NULL) 
    basesop->connect(t) ;
}

void Binfo::flatten()
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
  if (basesop != NULL)
  {
    bases = basesop->theNumber() ;
    delete basesop ;
    basesop = NULL ;
  }
  else
  {
    bases = 0 ;
  }
}

/*
  Node Destructors
  
  All of the functions have the same form of checking if options exist, and
   if they do, deleting them.  These options should only exist if the tree
   has not been flattened by calling the flatten functions.
*/

IdentifierNode::~IdentifierNode()
{
  if (strgop != NULL)
    delete strgop;
  if (lngtop != NULL)
    delete lngtop;
  if (attributes != NULL)
    delete attributes;
}



TreeVec::~TreeVec()
{
  if (lngtop != NULL)
    delete lngtop;
  if (ops != NULL)
    delete ops;
}











/* 

  Node Connection functions 

  These functions will make sure that any pointers are connected to the
   appropriate Node after the file has been parsed.  Each function checks
   to see if it has already been connected first before doing anything to 
   prevent infinite recursion.

*/



void TreeVec::connect(Function* t)
{
  if (!connected)
  {
    connected = true;
  }
  else
  {
    return;
  }
  if (ops == NULL)
    return ;
  list<opOptions>::iterator connectIter = ops->begin();
  while (connectIter != ops->end())
  {
    (*connectIter).nodePointer=(*(t->allNodes))[(*connectIter).nodeNumber - 1];
    ++connectIter;
  }  
}











/* Flatten functions */

// These functions will remove all of the options in each node and set 
//  pointers and values directly in the node so there is no extra level
//  of indirection.

void IdentifierNode::flatten()
{
  if (strgop != NULL)
  {
    name = (dynamic_cast<StrgOption*>(strgop))->theName();
    delete strgop;
    strgop = NULL ;
  }
  else
  {
    name = "" ;
  }
  // We probably do not need this, but I will connect it anyway
  if (lngtop != NULL)
  {
    length = lngtop->theNumber() ;
    delete lngtop;
    lngtop = NULL ;
  }
  else
  {
    length = -1 ;
  }
}



void TreeVec::flatten()
{
  if (lngtop != NULL)
  {
    length = lngtop->theNumber();
    delete lngtop;
    lngtop = NULL ;
  }
  else
  {
    length = -1 ;
  }
}





/* resultRegister functions */





/* Misc. Functions */


