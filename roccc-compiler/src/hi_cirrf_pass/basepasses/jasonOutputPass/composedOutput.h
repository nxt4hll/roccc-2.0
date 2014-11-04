#ifndef COMPOSABLE_OUTPUT_DOT_H
#define COMPOSABLE_OUTPUT_DOT_H

#include "systemOutput.h"

// A composable system is a special case of a system generator
//  Currently, the only difference is that I need to look for inputs
//  and outputs in the parameter list as well.
class ComposedGenerator : public SystemGenerator
{
 private:
  
  virtual void Output() ;
  virtual void OutputFakes() ;
  virtual void OutputDotH() ;

 public:

  virtual void OutputDeclarations() ;
  virtual void OutputFunctionType() ;

  ComposedGenerator(SuifEnv* e, ProcedureDefinition* p, String s) ;
  ~ComposedGenerator() ;

} ;

#endif
