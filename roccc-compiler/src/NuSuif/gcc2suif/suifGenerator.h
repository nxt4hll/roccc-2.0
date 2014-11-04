/*

  This file contains a module that creates suif constructs.  One of these
   modules should be instantiated for the program (since the global 
   symbol table exists in the environment).

  This file evolved from the original work done by John Cortes.

  Included in this module are the different enums used by the
   SUIF Generator.

*/

#ifndef __SUIF_GENERATOR_DOT_H__
#define __SUIF_GENERATOR_DOT_H__

#include <common/MString.h>
#include <common/system_specific.h>
#include <suifkernel/suifkernel_forwarders.h>
#include <suifkernel/suif_env.h>
#include <basicnodes/basic.h>
#include <basicnodes/basic_factory.h>
#include <suifnodes/suif.h>
#include <suifnodes/suif_factory.h>
#include <cfenodes/cfe.h>
#include <cfenodes/cfe_factory.h>
#include <typebuilder/type_builder.h>

enum SuifBinaryOp 
{ 
  BOP_ADD=0, 
  BOP_SUB, 
  BOP_MUL, 
  BOP_DIV,
  BOP_BITWISE_AND, 
  BOP_BITWISE_OR,
  BOP_BITWISE_NAND,
  BOP_BITWISE_NOR,
  BOP_BITWISE_XOR,
  BOP_LEFT_SHIFT, 
  BOP_RIGHT_SHIFT,
  BOP_IS_EQUAL_TO,
  BOP_IS_NOT_EQUAL_TO,
  BOP_IS_LESS_THAN, 
  BOP_IS_LESS_THAN_OR_EQUAL_TO,
  BOP_IS_GREATER_THAN, 
  BOP_IS_GREATER_THAN_OR_EQUAL_TO,
  BOP_LOGICAL_AND, 
  BOP_LOGICAL_OR, 
  BOP_MOD,
  BOP_UNKNOWN
} ;

enum SuifUnaryOp 
{
  UOP_NEGATE,
  UOP_INVERT, 
  UOP_ABS, 
  UOP_BITWISE_NOT, 
  UOP_LOGICAL_NOT, 
  UOP_CONVERT, 
  UOP_TREAT_AS, 
  UOP_NULL, 
  UOP_UNKNOWN
} ;

class SuifGenerator 
{ 
 private:

  // A variable to hold the entire suif environment
  SuifEnv* env; 

  // A string to hold the name of the file we are outputting
  LString outputFileName ;

  // Pointers to the global suif structures (the ones that individual
  //  procedures are added to)
  FileSetBlock* globalFSB ;
  FileBlock* globalFileBlock ;
  BasicSymbolTable* fileSymbolTable ;
  DefinitionBlock* fileDefBlock ;

  // Pointers to the current function's suif structures
  StatementList* currentFunctionStmtList ;
  BasicSymbolTable* currentFunctionSymbolTable ;
  ProcedureDefinition* currentFunctionDefinition ;

  // Create types for anything that can have a constant associated with it

  // A stack that keeps track of the labels that break statements jump to.
  list<CodeLabelSymbol*> breakStack ;
  
 public:

  // Object factories ... I don't like making them public, but I'll 
  //  come back to these later
  BasicObjectFactory* globalBasicObjfactory;
  SuifObjectFactory* globalSuifObjfactory;
  CfeObjectFactory* globalCfeObjfactory;
  TypeBuilder* globalTb ;

  SuifGenerator(const LString& inFile, const LString &outFile);
  ~SuifGenerator();

  // Functions that handle the breakStack
  CodeLabelSymbol* currentBreakTarget() ;
  void popBreakTarget() ;
  void pushBreakTarget(CodeLabelSymbol* x) ;

  void StartNewFunction(const LString& functionName, 
			list<Symbol*>& args, 
			//			list<QualifiedType*>& args, 
			DataType* returnType) ;

  void FinalizeCurrentFunction() ;
  void WriteSuifFile() ;

  // Add a statement to the current function's main code
  void appendStatementToMainBody(Statement* stmt);

  // Function to create an integer constant (which is created every time and
  //  not stored in the symbol table)
  Expression* createIntegerConstantExpression(int constant, int size = 32);

  // Function to create a floating point constant (not stored in 
  //  the symbol table)

  Expression* createFloatingPointConstantExpression(double constant, 
						    int size = 32) ;

  // Functions to create unary and binary expressions from two
  //  other expressions
  Expression* createUnaryExpression(SuifUnaryOp op, 
				    Expression* exp);
  Expression* createBinaryExpression(SuifBinaryOp op, 
				     Expression* exp1, 
				     Expression* exp2); 

  // A function to create a load variable expression.  This is used
  //  whenever a variable is used in a statement (since variable symbols
  //  are not themselves expressions)
  Expression* createLoadVariableExpression(VariableSymbol *var_symbol);

  // A function that will store an expression into a variable.  This is
  //  unfortunately incompatible with array references and must be
  //  done seperately.
  Statement* createStoreVariableStatement(VariableSymbol* vsym, 
					  Expression* exp);  

  // Create a Statement from an expression
  Statement* createExpressionStatement(Expression* exp);

  // A function that will create an If statement given the
  //  condition, then portion, and else portion
  Statement* createIfStatement(Expression* cond, Statement* then_stmt, 
			       Statement* else_stmt);

  // A function that will create a do...while statement given the
  //  condition and the body
  Statement* createDoWhileStatement(Expression* cond, Statement* body);

  // A function that will create a while statement given the condition
  //  and the body
  Statement* createWhileStatement(Expression* cond, Statement* body);

  // A function that will create a for statement using cfenodes.
  //  Note: The before statement is currently empty because gcc moves
  //  the initialization code outside of the for loop.  This might 
  //  have to change in the future
  Statement* createCForStatement(Statement* before, Expression* cond, 
				 Statement* step, Statement* body) ;

  // A function that adds a variable symbol to the current symbol table
  //  as well as initialization code
  VariableSymbol* addVariableSymbol(const LString& name, 
				    QualifiedType* qType,
				    ValueBlock* initialization = NULL,
				    bool addrTaken = false);

  // A function that adds a variable symbol to the global symbol table
  VariableSymbol* addGlobalSymbol(const LString& name, QualifiedType* qType);

  // A function that creates a value block from an expression
  ExpressionValueBlock* createExpressionValueBlock(Expression* v) ;

  void addLabelSymbolToCurrentSymbolTable(Symbol* s) ;
  void addProcedureSymbolToCurrentSymbolTable(Symbol* s) ;
  void addStructTypeToCurrentSymbolTable(Type* t) ;

  // A function that creates a non lvalue expression
  Expression* createNonLvalueExpression(Expression* addressedExpr) ;

  Expression* createAddressExpression(VariableSymbol* v) ;

  // This function returns a unary operator that casts according to 
  //  the type passed in
  Expression* createCastExpression(Expression* toCast, 
				   DataType* typeToCastTo) ;

  // These functions exist from my attempt at creating scoped structures.
  //  That attempt failed, but I'm leaving these just in case I 
  //  come up with another idea in the future.

  inline BasicSymbolTable* getCurrentSymTab() 
    { return currentFunctionSymbolTable ; }

  inline void setCurrentSymTab(BasicSymbolTable* x) 
    { currentFunctionSymbolTable = x ;}

  inline BasicSymbolTable* getNewSymTab()
    { 
      return 
	globalBasicObjfactory->create_basic_symbol_table(fileSymbolTable) ;
    }

  inline DefinitionBlock* getFileDefBlock() { return fileDefBlock ; }

  inline void setOutputFile(LString& n) { outputFileName = n ; }

  inline SuifEnv* getEnv() { return env ; } 
  
};



#endif

