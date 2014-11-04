#ifndef MODULE_GENERATOR_DOT_H
#define MODULE_GENERATOR_DOT_H

#include "baseOutput.h"

class ModuleGenerator : public HiCirrfGenerator
{
 private:
  
 public:
  ModuleGenerator(SuifEnv* e, ProcedureDefinition* p) ;
  ~ModuleGenerator() ;
  
  virtual void Setup() ;
  virtual LString CleanInputName(LString n) ;
  virtual LString CleanOutputName(LString n) ;
  virtual void Output() ;
  virtual void OutputDotH() ;
  virtual void OutputDeclarations() ;
  virtual void OutputFunctionType() ;
  virtual void CollectVariables() ;
  virtual void PrintFieldAccessExpression(FieldAccessExpression* e) ;

  void OutputModuleOrder() ;
  void OutputModuleStructName() ;
  void OutputIntrinsicType() ;

} ;

#endif
