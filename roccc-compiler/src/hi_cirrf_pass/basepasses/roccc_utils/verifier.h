/*
  This file contains a class object that will verify that all of the pointers
    in a suif tree are valid and there are no hanging pointers.  Hopefully,
    this will fix the issues I've been having and allow me to 
    verify the suif tree at different locations.
 
*/
#ifndef VERIFIER_DOT_H
#define VERIFIER_DOT_H

#include <suifnodes/suif.h>
#include <cfenodes/cfe.h>

class SuifTreeVerifier
{
 public:
  void VerifyFSB(FileSetBlock* fsb) ;
  void VerifySymbolTable(SymbolTable* symTab) ;
  void VerifyFileBlock(FileBlock* fb) ;
  void VerifyInformationBlock(GlobalInformationBlock* gb) ;
  void VerifyAnnotations(AnnotableObject* a) ;
  void VerifyBricks(BrickAnnote* b) ;
  void VerifyDefinitionBlock(DefinitionBlock* db) ;
  void VerifyVariableDefinition(VariableDefinition* v) ;
  void VerifyProcedureDefinition(ProcedureDefinition* p) ;
  void VerifyExecutionObject(ExecutionObject* e) ;
  void VerifyStatement(Statement* s) ;
  void VerifyExpression(Expression* e) ;
  void VerifySymbol(Symbol* s) ;

  // Printing operations...
  void PrintFSB(FileSetBlock* fsb) ;
  void PrintSymbolTable(SymbolTable* symTab) ;
  void PrintFileBlock(FileBlock* fb) ;
  void PrintInformationBlock(GlobalInformationBlock* gb) ;
  void PrintAnnotations(AnnotableObject* a) ;
  void PrintBricks(BrickAnnote* b) ;
  void PrintDefinitionBlock(DefinitionBlock* db) ;
  void PrintVariableDefinition(VariableDefinition* v) ;
  void PrintProcedureDefinition(ProcedureDefinition* p) ;
  void PrintExecutionObject(ExecutionObject* e) ;
  void PrintStatement(Statement* s) ;
  void PrintExpression(Expression* e) ;
  void PrintSymbol(Symbol* s) ;

  void PrintType(Type* t) ;

 public:
  SuifTreeVerifier() ;
  ~SuifTreeVerifier() ;  
} ;

#endif
