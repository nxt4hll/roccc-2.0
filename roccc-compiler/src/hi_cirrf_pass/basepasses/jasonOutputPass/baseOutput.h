
#ifndef BASE_OUTPUT_DOT_H
#define BASE_OUTPUT_DOT_H

#include <iostream>
#include <fstream>
#include <list>

#include <suifnodes/suif.h>
#include <basicnodes/basic.h>
#include <cfenodes/cfe.h>

class HiCirrfGenerator
{
 protected:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;

  // The procedure's symbol table
  SymbolTable* symTab ;

  std::ofstream fout ; // For the code
  std::ofstream rocccOut ; // For the header

  std::list<VariableSymbol*> inputs ; // Scalars only
  std::list<VariableSymbol*> outputs ;

  // Used to differentiate each invoke hardware call
  int InvokeHardwareCounter ;

  virtual void CleanupNames() ;
  virtual LString CleanInputName(LString n) ;
  virtual LString CleanOutputName(LString n) ;

  virtual void OutputDotH() ;
  virtual void OutputHeader() ;
  virtual void OutputDeclarations() ;
  virtual void OutputFunctionType() ;
  virtual void OutputFakes() ;
  virtual void OutputInputScalars() ;
  virtual void OutputOutputScalars() ;
  virtual void OutputDatapath(ExecutionObject* body) ;
  virtual void OutputFooter() ;

  virtual void CollectVariables() ;
  virtual void OutputLookupTables() ;
  virtual void OutputLookupTableOrder() ;

  // A printing function that only the base class will use
  void PrintConstantValues(ValueBlock* v) ;

  // Printing functions that all derived classes will use.
  void PrintDeclaration(SymbolTableObject* s, bool toHeader = false) ;
  void PrintQualifications(QualifiedType* qType, bool toHeader = false) ;
  void PrintArrayDeclaration(ArrayType* aType, Symbol* s, 
			     bool toHeader = false) ; 
  void PrintFunctionType() ;

  // The main functions that actually print out the datapath code
  void PrintStatement(Statement* s) ;
  void PrintExpression(Expression* e, bool toHeader = false) ;

  void PrintStatementList(StatementList* s) ;
  void PrintIfStatement(IfStatement* s) ;
  void PrintCallStatement(CallStatement* s) ;
  void PrintStoreStatement(StoreStatement* s) ;
  void PrintStoreVariableStatement(StoreVariableStatement* s) ;
  virtual void PrintForStatement(CForStatement* s) ;

  void PrintBinaryExpression(BinaryExpression* e) ;
  void PrintUnaryExpression(UnaryExpression* e) ;
  void PrintConvertExpression(UnaryExpression* e) ;
  void PrintSymbolAddressExpression(SymbolAddressExpression* e) ;
  void PrintAddressExpression(AddressExpression* e) ;
  virtual void PrintFieldAccessExpression(FieldAccessExpression* e) ;
  void PrintArrayReferenceExpression(ArrayReferenceExpression* e) ;
  void PrintCallExpression(CallExpression* e) ;

  void PrintOpcode(String op) ;

  void OutputFake(VariableSymbol* v, int dimensionality = 0) ;
  void OutputPointerFake(VariableSymbol* v) ;

  void OutputSizes() ;
  void OutputSizesWorkhorse(SymbolTable* currentSymTab) ;

  // Functions that return strings for use in larger outputting statements
  String StringType(Type* t, bool convertArraysToPointers = false) ;
  String StringQualifications(QualifiedType* t) ;

  // Functions that pass information to the lo-cirrf side
  void OutputMaximizePrecision() ;
  void OutputDebugRegisters() ;
  void OutputDebugRegistersWorkhorse(SymbolTable* currentSymTab) ;
  void OutputCompilerVersion() ;

  // Helper functions 
  DataType* GetBaseType(Type* top) ;
  bool InInputList(VariableSymbol* value) ;
  bool FloatFormat(String s) ;
  
 public:

  // The main interfaces to print out our information
  virtual void Setup() ;
  virtual void Output() ;  

  HiCirrfGenerator(SuifEnv* e, ProcedureDefinition* p) ;
  virtual ~HiCirrfGenerator() ;
  

} ;


#endif
