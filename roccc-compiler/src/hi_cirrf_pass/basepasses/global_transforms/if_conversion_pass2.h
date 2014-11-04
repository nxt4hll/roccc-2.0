// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
/*
  This pass is responsible for changing if statements into bool select
    function calls where applicable.
*/

#ifndef IF_CONVERSION_PASS_TWO_H
#define IF_CONVERSION_PASS_TWO_H

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"
#include "typebuilder/type_builder.h"

class IfConversionPass2 : public PipelinablePass 
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;
  TypeBuilder* tb ;

  bool ConvertIf(IfStatement* toConvert) ;

  bool VerifyIf(IfStatement* toConvert) ;
  bool CorrectTypes(Statement* x, Statement* y) ;
  
  CallStatement* CreateBoolCall(VariableSymbol* destVar) ;
  StoreStatement* CreateBoolCall(ArrayReferenceExpression* dest) ;

  ProcedureSymbol* CreateBoolSym(QualifiedType* qualType) ;


  Statement* Denormalize(Statement* x) ;

  // This whole set of functions is approximating what it would look like
  //  if I had a different inheritence structure.
  bool ConvertStoreVariableIf(IfStatement* toConvert) ;
  bool ConvertStoreIf(IfStatement* toConvert) ;
  bool ConvertStructStoreIf(IfStatement* toConvert) ;
  bool ConvertArrayStoreIf(IfStatement* toConvert) ;

  bool ConvertCallsIf(IfStatement* toConvert) ;
  bool ConvertCallStoreVarIf(IfStatement* toConvert) ;
  bool ConvertStoreVarCall(IfStatement* toConvert) ;

public:
  IfConversionPass2(SuifEnv *pEnv);
  Module* clone() const { return (Module*)this; }
  void do_procedure_definition(ProcedureDefinition *proc_def);
};

#endif
