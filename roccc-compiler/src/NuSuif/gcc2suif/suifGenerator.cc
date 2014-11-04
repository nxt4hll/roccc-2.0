/*

  This file contains the function definitions of the SuifGenerator class
   that is responsible for creating the suif structure.

*/

#include <cassert>

#include "suifGenerator.h"
#include "generic.h"

SuifGenerator::SuifGenerator(const LString& inFile, const LString& outFile) 
{

  outputFileName = outFile ;

  // Initialize the global scope
  env = create_suif_env();
  env->init();
  init_basicnodes(env);
  init_suifnodes(env);
  init_typebuilder(env);
  init_cfenodes(env) ;

  // Get the pointers to the object factories
  
  globalBasicObjfactory = (BasicObjectFactory *)  
    env->get_object_factory(BasicObjectFactory::get_class_name());
  globalSuifObjfactory = (SuifObjectFactory *)
    env->get_object_factory(SuifObjectFactory::get_class_name());
  globalCfeObjfactory = (CfeObjectFactory *)
    env->get_object_factory(CfeObjectFactory::get_class_name());
  globalTb = (TypeBuilder *)
    env->get_object_factory(TypeBuilder::get_class_name());

  // Set up the global symbol tables

  BasicSymbolTable* externalSymbolTable = 
    globalBasicObjfactory->create_basic_symbol_table(NULL) ;
  fileSymbolTable =
    globalBasicObjfactory->create_basic_symbol_table(NULL) ;

  // Set up the file set block

  globalFSB = 
    globalBasicObjfactory->create_file_set_block(externalSymbolTable) ;
  env->set_file_set_block(globalFSB);

  fileDefBlock = 
    globalBasicObjfactory->create_definition_block();

  globalFileBlock = 
    globalBasicObjfactory->create_file_block(inFile, fileSymbolTable, 
					    fileDefBlock);

  globalFSB->append_file_block(globalFileBlock);

  env->set_file_set_block(globalFSB) ;

  // Create a target information block (for some reason)

  globalFSB->append_information_block(create_target_information_block(env, "", "", "", "", 
							"", "", "", 0, 8, 
    dynamic_cast<IntegerType*>(globalTb->get_integer_type(32, 32, false)), 
    dynamic_cast<BooleanType*>(globalTb->get_boolean_type()), 
    dynamic_cast<VoidType*>(globalTb->get_void_type()), true, 32, true, 32, 
    true, 8, true, 8, true, 32, false)) ;

  // Initialize the other values to NULL

  currentFunctionStmtList = NULL ;
  currentFunctionSymbolTable = NULL ;
  currentFunctionDefinition = NULL ;

} 

SuifGenerator::~SuifGenerator()
{
  delete env ; // Does this get rid of everything?
}

// This function needs to also take in a list of argument symbols to
//  add.  I could probably pass those in and then get the argument
//  types from them individually

void SuifGenerator::StartNewFunction(const LString& functionName,
				     list<Symbol*>& args, 
				     DataType* returnType)
{

  // Grab the QualifiedTypes from all of the arguments that
  //  were passed in.

  list<QualifiedType*> argTypes ;

  // I don't deal with const or volatile qualifiers at this point, 
  //  but I should come back here and fix this.
  list<Symbol*>::iterator argIter = args.begin() ;
  while(argIter != args.end())
  {
    Type* currentType = (*argIter)->get_type() ;
    argTypes.push_back(globalTb->get_qualified_type(currentType)) ;
    ++argIter ;
  }

  // First create the type of this function

  CProcedureType* nextFunctionType = 
    globalTb->get_c_procedure_type(returnType, argTypes, false, true) ;

  // Now create a symbol for this function in the file symbol table
  ProcedureSymbol* nextFunctionSymbol = 
    globalBasicObjfactory->create_procedure_symbol(nextFunctionType,
						   functionName,
						   true) ;

  globalFSB->get_external_symbol_table()->add_symbol(nextFunctionSymbol) ;

  // Create a new statement list
  currentFunctionStmtList = globalBasicObjfactory->create_statement_list() ;

  // Create a new symbol table for this function
  currentFunctionSymbolTable = 
    globalBasicObjfactory->create_basic_symbol_table(fileSymbolTable) ;

  // Create the procedure definition for this current function
  DefinitionBlock* currentDefBlock = 
    globalBasicObjfactory->create_definition_block() ;
  currentFunctionDefinition = 
    globalBasicObjfactory->create_procedure_definition(nextFunctionSymbol,
						       currentFunctionStmtList,
						    currentFunctionSymbolTable,
						       currentDefBlock) ;
  // Add all parameters

  argIter = args.begin() ;
  while(argIter != args.end())
  {
    ParameterSymbol* nextParam = 
      dynamic_cast<ParameterSymbol*>(*argIter) ;
    assert(nextParam != NULL) ;
    currentFunctionDefinition->get_symbol_table()->add_symbol(nextParam) ;
    currentFunctionDefinition->append_formal_parameter(nextParam) ;
    ++argIter ;
  }

}

void SuifGenerator::FinalizeCurrentFunction()
{
  // Add the current function statement list to the file set block
  globalFileBlock->get_definition_block()->
    append_procedure_definition(currentFunctionDefinition);
}

void SuifGenerator::WriteSuifFile() 
{
  env->write(outputFileName);
}

void SuifGenerator::appendStatementToMainBody(Statement *stmt) 
{
  currentFunctionStmtList->append_statement(stmt);
}

Expression* SuifGenerator::createIntegerConstantExpression(int constantVal,
							   int size) 
{

  IntegerType* sintType ;

  sintType = globalTb->get_integer_type(size, size, true) ;

  return (Expression *)
    globalBasicObjfactory->create_int_constant(sintType, constantVal);
}

Expression* SuifGenerator::createFloatingPointConstantExpression(double value,
								 int size)
{

  String stringValue = value ;

  FloatingPointType* fType ;
  
  fType = globalTb->get_floating_point_type() ;

  return (Expression*)
    globalBasicObjfactory->create_float_constant(fType, stringValue) ;
}

Expression* SuifGenerator::createUnaryExpression(SuifUnaryOp op, 
						  Expression *exp) 
{
  
  LString op_str = "";
  switch(op) 
  {
    case UOP_NEGATE: 

      op_str = "negate";    
      break;

    case UOP_INVERT: 

      op_str = "invert";    
      break;
      
    case UOP_ABS: 
    
      op_str = "absolute_value";    
      break;
    
    case UOP_BITWISE_NOT: 
	
      op_str = "bitwise_not";       
      break;
        
    case UOP_LOGICAL_NOT: 

      op_str = "logical_not";       
      break;

    case UOP_CONVERT: 

      op_str = "convert";           
      break;
        
    case UOP_TREAT_AS: 
	
      op_str = "treat_as";
      break;
      
    case UOP_UNKNOWN:    
    default:
     
      op_str = "UOP_UNKNOWN";
    }

  // Used to be globalTb->get_boolean_type()

  return (Expression *) 
    globalSuifObjfactory->create_unary_expression(exp->get_result_type(),
						  op_str, 
						  exp);
}

Expression* SuifGenerator::createCastExpression(Expression* toCast, 
						DataType* typeToCastTo)
{
  return globalSuifObjfactory->create_unary_expression(typeToCastTo,
						       "convert",
						       toCast) ;
}


Expression* SuifGenerator::createBinaryExpression(SuifBinaryOp op, 
						  Expression* exp1, 
						  Expression* exp2) 
{

  // A special case for field_access_expressions
  //  Ugly, yes, but necessary for right now...

  Expression* trueExp1 ;
  Expression* trueExp2 ;

  if (dynamic_cast<FieldAccessExpression*>(exp1) != NULL)
  {
    trueExp1 = globalSuifObjfactory->
      create_load_expression(dynamic_cast<FieldAccessExpression*>(exp1)->get_field()->get_type()->get_base_type(),
			     exp1) ;
  }
  else
  {
    trueExp1 = exp1 ;
  }

  if (dynamic_cast<FieldAccessExpression*>(exp2) != NULL)
  {
    trueExp2 = globalSuifObjfactory->
      create_load_expression(dynamic_cast<FieldAccessExpression*>(exp2)->get_field()->get_type()->get_base_type(),
				      exp2) ;
				      //				      dynamic_cast<FieldAccessExpression*>(exp2)->get_field()) ;
  }
  else
  {
    trueExp2 = exp2 ;
  }

  DataType* resultType ; 

  //= globalTb->get_integer_type(sizeof(int)*8, 
  //						       sizeof(int)*8,
  //					       true) ;

  // Figure out what type the result of this binary operation should be.
  //  If both expressions are integers, choose the largest integer as the
  //  result type (unless it is a multiplication, in that case double the
  //  size up to 64 bits).  If one is a float/double, make sure you promote
  //  the expression to the appropriate type.  If this is a compoarison
  //  we should return a boolean.

  DataType* exp1ResultType = trueExp1->get_result_type() ;
  DataType* exp2ResultType = trueExp2->get_result_type() ;

  // First, remove pointer indirection
  while (dynamic_cast<PointerType*>(exp1ResultType) != NULL)
  {
    PointerType* pType = dynamic_cast<PointerType*>(exp1ResultType) ;
    QualifiedType* qType = 
      dynamic_cast<QualifiedType*>(pType->get_reference_type()) ;
    assert(qType != NULL) ;
    exp1ResultType = qType->get_base_type() ; 
  }

  while (dynamic_cast<PointerType*>(exp2ResultType) != NULL)
  {
    PointerType* pType = dynamic_cast<PointerType*>(exp2ResultType) ;
    QualifiedType* qType = 
      dynamic_cast<QualifiedType*>(pType->get_reference_type()) ;
    assert(qType != NULL) ;
    exp2ResultType = qType->get_base_type() ; 
  }

  // Check if we need to upgrade to the float type
  if (dynamic_cast<FloatingPointType*>(exp1ResultType) != NULL &&
      dynamic_cast<IntegerType*>(exp2ResultType) != NULL)
  {
    resultType = exp1ResultType ;
  }
  else if (dynamic_cast<IntegerType*>(exp1ResultType) != NULL &&
	   dynamic_cast<FloatingPointType*>(exp2ResultType) != NULL)
  {
    resultType = exp2ResultType ;
  }
  else if (exp1ResultType->get_bit_size() > exp2ResultType->get_bit_size())
  {
    resultType = exp1ResultType ;
  }
  else
  {
    resultType = exp2ResultType ;
  }
  
  LString op_str = "";
  switch(op) 
  {
    case BOP_ADD: 
      op_str = "add";         
      break;
    case BOP_SUB: 
      op_str = "subtract";    
      break;
    case BOP_MUL: 
      op_str = "multiply";    
      break;
    case BOP_DIV: 
      op_str = "divide";      
      break;
    case BOP_BITWISE_AND: 
      op_str = "bitwise_and";   
      break;
    case BOP_BITWISE_OR: 
      op_str = "bitwise_or";
      break;
    case BOP_BITWISE_NAND: 
      op_str = "bitwise_nand";
      break; 
    case BOP_BITWISE_NOR: 
      op_str = "bitwise_nor";   
      break;
    case BOP_BITWISE_XOR: 
      op_str = "bitwise_xor";   
      break;
    case BOP_LEFT_SHIFT: 
      op_str = "left_shift";    
      break;
    case BOP_RIGHT_SHIFT: 
      op_str = "right_shift";   
      break;
    case BOP_IS_EQUAL_TO: 
      op_str = "is_equal_to";   
      resultType = globalSuifObjfactory->create_boolean_type(IInteger(1), 0) ; 
      break;
    case BOP_IS_NOT_EQUAL_TO: 
      op_str = "is_not_equal_to";   
      resultType = globalSuifObjfactory->create_boolean_type(IInteger(1), 0) ; 
      break;
    case BOP_IS_LESS_THAN: 
      op_str = "is_less_than";
      resultType = globalSuifObjfactory->create_boolean_type(IInteger(1), 0) ; 
      break;
    case BOP_IS_LESS_THAN_OR_EQUAL_TO: 
      op_str = "is_less_than_or_equal_to";  
      resultType = globalSuifObjfactory->create_boolean_type(IInteger(1), 0) ; 
      break;
    case BOP_IS_GREATER_THAN: 
      op_str = "is_greater_than";
      resultType = globalSuifObjfactory->create_boolean_type(IInteger(1), 0) ; 
      break;
    case BOP_IS_GREATER_THAN_OR_EQUAL_TO: 
      op_str = "is_greater_than_or_equal_to";   
      resultType = globalSuifObjfactory->create_boolean_type(IInteger(1), 0) ; 
      break;
    case BOP_LOGICAL_AND: 
      op_str = "logical_and";
      resultType = globalSuifObjfactory->create_boolean_type(IInteger(1), 0) ; 
      break;
    case BOP_LOGICAL_OR: 
      op_str = "logical_or";
      resultType = globalSuifObjfactory->create_boolean_type(IInteger(1), 0) ; 
      break;
    case BOP_MOD:
      op_str = "remainder" ;
      break ;
    case BOP_UNKNOWN:
      // Fall through
    default: 
      op_str = "BOP_UNKNOWN";
    }
    
    return (Expression *) 
      globalSuifObjfactory->create_binary_expression(resultType, 
						     op_str, 
						     trueExp1, 
						     trueExp2);
}

Expression* SuifGenerator::createLoadVariableExpression(VariableSymbol* varSym)
{

  DataType* base_type = varSym->get_type()->get_base_type();
  return (Expression *) 
    globalSuifObjfactory->create_load_variable_expression(base_type, varSym);
}

Statement* SuifGenerator::createStoreVariableStatement(VariableSymbol* vsym, 
						       Expression* exp) 
{
  return (Statement *) 
    globalSuifObjfactory->create_store_variable_statement(vsym, exp);
}


Statement* SuifGenerator::createExpressionStatement(Expression* exp) 
{
  EvalStatement *evalStmt = 
    globalSuifObjfactory->create_eval_statement();
  evalStmt->append_expression(exp);
    
  return (Statement *)evalStmt;
}


Statement* SuifGenerator::createIfStatement(Expression* cond, 
					    Statement* then_stmt, 
					    Statement* else_stmt) 
{
  return (Statement *) 
    globalSuifObjfactory->create_if_statement(cond, then_stmt, else_stmt);
}

Statement* SuifGenerator::createDoWhileStatement(Expression* cond, 
						 Statement* body) 
{
  // Also create two blank labels for continue and break
  
  CodeLabelSymbol* breakLabel = globalBasicObjfactory->create_code_label_symbol(globalTb->get_label_type(), "") ;

  CodeLabelSymbol* continueLabel = globalBasicObjfactory->create_code_label_symbol(globalTb->get_label_type(), "");

  addLabelSymbolToCurrentSymbolTable(breakLabel) ;
  addLabelSymbolToCurrentSymbolTable(continueLabel) ;

  return (Statement *) 
    globalSuifObjfactory->create_do_while_statement(cond, body, breakLabel, 
						    continueLabel);
}


Statement* SuifGenerator::createWhileStatement(Expression* cond, 
					       Statement* body) 
{
  // Also create two blank labels for continue and break
  
  CodeLabelSymbol* breakLabel = globalBasicObjfactory->create_code_label_symbol(globalTb->get_label_type(), "") ;

  CodeLabelSymbol* continueLabel = globalBasicObjfactory->create_code_label_symbol(globalTb->get_label_type(), "");

  addLabelSymbolToCurrentSymbolTable(breakLabel) ;
  addLabelSymbolToCurrentSymbolTable(continueLabel) ;

  return (Statement *) 
    globalSuifObjfactory->create_while_statement(cond, body, breakLabel, 
						 continueLabel);
}

Statement* SuifGenerator::createCForStatement(Statement* before, 
					      Expression* cond, 
					      Statement* step, 
					      Statement* body) 
{

  // Also create two blank labels for continue and break
  
  CodeLabelSymbol* breakLabel = globalBasicObjfactory->create_code_label_symbol(globalTb->get_label_type(), "") ;

  CodeLabelSymbol* continueLabel = globalBasicObjfactory->create_code_label_symbol(globalTb->get_label_type(), "");

  addLabelSymbolToCurrentSymbolTable(breakLabel) ;
  addLabelSymbolToCurrentSymbolTable(continueLabel) ;
  
  // Also, because of other passes that ASSUME (incorrectly) that all 
  //  for statements have a statement list as the body instead of 
  //  just a statement, I am going to create a statement list
  //  and add the body to that.

  StatementList* stupidStep ;

  if (is_a<StatementList*>(step))
  {
    stupidStep = dynamic_cast<StatementList*>(step) ;
  }
  else
  {
    stupidStep = globalBasicObjfactory->create_statement_list() ;
    stupidStep->append_statement(step) ;
  }

  StatementList* stupidBody ;
 
  if (is_a<StatementList*>(body))
  {
    stupidBody = dynamic_cast<StatementList*>(body) ;
  }
  else
  {
    stupidBody = globalBasicObjfactory->create_statement_list() ;
    stupidBody->append_statement(body) ;
  }

  return (Statement *) 
    globalCfeObjfactory->create_c_for_statement(before, cond, stupidStep, stupidBody, 
						NULL, breakLabel, continueLabel);
}

VariableSymbol* SuifGenerator::addVariableSymbol(const LString& name,
						 QualifiedType* qType,
						 ValueBlock* initialization,
						 bool addrTaken) 
{

  VariableSymbol* varSym = 
    globalBasicObjfactory->create_variable_symbol(qType, name, addrTaken);

  if (initialization != NULL)
  {
    VariableDefinition* varDef = 
      globalBasicObjfactory->create_variable_definition(varSym,
							32, //Alignment
							initialization,
							false) ; // is_static
    currentFunctionDefinition->get_definition_block()->append_variable_definition(varDef) ;
  }

  //currentFunctionSymbolTable->append_symbol_table_object(varSym) ;

  currentFunctionDefinition->get_symbol_table()->
      append_symbol_table_object(varSym);

  return varSym;
}

VariableSymbol* SuifGenerator::addGlobalSymbol(const LString& name, 
					       QualifiedType* qType)
{
  VariableSymbol* varSym = 
    globalBasicObjfactory->create_variable_symbol(qType, name, false);

  fileSymbolTable->append_symbol_table_object(varSym);
  return varSym;
}

ExpressionValueBlock* SuifGenerator::createExpressionValueBlock(Expression* v)
{
  return globalSuifObjfactory->create_expression_value_block(v) ;
}

void SuifGenerator::addLabelSymbolToCurrentSymbolTable(Symbol* s)
{
  currentFunctionDefinition->get_symbol_table()->append_symbol_table_object(s) ;
}

void SuifGenerator::addStructTypeToCurrentSymbolTable(Type* t)
{
  fileSymbolTable->append_symbol_table_object(t) ;
}

// Duplicate, I know, but let me see if this works, then I'll combine these
//  two functions into one.
void SuifGenerator::addProcedureSymbolToCurrentSymbolTable(Symbol* s)
{
  globalFSB->get_external_symbol_table()->add_symbol(dynamic_cast<ProcedureSymbol*>(s)) ;
  //  currentFunctionDefinition->get_symbol_table()->append_symbol_table_object(s) ;
}

Expression* SuifGenerator::createNonLvalueExpression(Expression* addressedExpr)
{
  return globalSuifObjfactory->create_non_lvalue_expression(addressedExpr->get_result_type(),
							    addressedExpr) ;
}

// This function has to create a SYMADDR expression and return it.

Expression* SuifGenerator::createAddressExpression(VariableSymbol* v)
{
  return globalSuifObjfactory->create_symbol_address_expression(v->get_type()->get_base_type(), v) ;

}

CodeLabelSymbol* SuifGenerator::currentBreakTarget()
{
  assert(!(breakStack.empty())) ;
  return *(breakStack.begin()) ;
}

void SuifGenerator::popBreakTarget()
{
  assert(!(breakStack.empty())) ;
  breakStack.pop_front() ;
}

void SuifGenerator::pushBreakTarget(CodeLabelSymbol* x)
{
  breakStack.push_front(x) ;
}
