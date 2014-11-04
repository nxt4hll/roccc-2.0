// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details
/*
  The purpose of this pass is to convert all pointer arithmetic into 
   array references...even if they aren't arrays!  We might be able
   to deal with them...and this will definitely help if I want to put
   streams as parameters to systems and then call them as a function.
*/

#ifndef POINTER_CONVERSION_DOT_H
#define POINTER_CONVERSION_DOT_H

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"

#include <map>

class PointerConversionPass : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;

  bool ReplaceLoad(LoadExpression* top) ;
  bool ReplaceStore(StoreStatement* top) ;


  int Form(LoadExpression* top) ;
  int FormInternal(BinaryExpression* topAdd) ;
  
  // One dimensional arrays
  bool FormOne(BinaryExpression* topAdd) ;
  bool FormTwo(BinaryExpression* topAdd) ;

  // Two dimensional arrays
  bool FormThree(BinaryExpression* topAdd) ;
  bool FormFour(BinaryExpression* topAdd) ;

  // Constant offset
  bool FormFive(BinaryExpression* topAdd) ;

  // This will take care of all constant accesses to arrays with offset 0
  void HandleFormSix() ;

  int StoreFormInternal(BinaryExpression* topAdd) ;

  bool StoreFormOne(BinaryExpression* topAdd) ;
  bool StoreFormTwo(BinaryExpression* topAdd) ;
  bool StoreFormThree(BinaryExpression* topAdd) ;
  bool StoreFormFour(BinaryExpression* topAdd) ;
  bool StoreFormFive(BinaryExpression* topAdd) ;

  Expression* NonArrayValue(BinaryExpression* topAdd) ;
  Expression* ArrayValue(BinaryExpression* topAdd) ;

  IntConstant* GetOffset(BinaryExpression* top, int form) ;
  
  IntConstant* GetOffsetTwo(BinaryExpression* top) ;
  IntConstant* GetOffsetFour(BinaryExpression* top) ;

  IntConstant* GetStoreOffset(BinaryExpression* top, int form) ;
  DataType* GetElementType(BinaryExpression* top, int form) ;
  DataType* GetStoreElementType(BinaryExpression* top, int form) ;

  Expression* GetArrayAddress(BinaryExpression* top, int form) ;
  Expression* GetArrayAddress(NonLvalueExpression* top) ;

  Expression* CombinedOffset(IntConstant* offset, 
			     Expression* index,
			     LString opcode) ;

  Expression* GetIndex(BinaryExpression* top, int form) ;
  Expression* GetStoreIndex(BinaryExpression* top, int form) ;
  
  Expression* GetIndexOne(BinaryExpression* top) ;
  Expression* GetIndexTwo(BinaryExpression* top) ;
  Expression* GetIndexThree(BinaryExpression* top) ;
  Expression* GetIndexFour(BinaryExpression* top) ;
  
  void Cleanup(LoadExpression* top) ;

  VariableSymbol* FindReplacement(VariableSymbol* original) ;

  ArrayType* ConvertType(PointerType* original) ;

  std::map<VariableSymbol*, VariableSymbol*> replacementMap ;

 public:
  PointerConversionPass(SuifEnv* pEnv) ;
  Module* clone() const { return (Module*) this ; }
  void do_procedure_definition(ProcedureDefinition* proc_def) ;
} ;

#endif
