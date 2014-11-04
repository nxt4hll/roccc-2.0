// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

#ifndef DIV_BY_CONST_ELIMINATION_DOT_H
#define DIV_BY_CONST_ELIMINATION_DOT_H

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>

class DivByConstEliminationPass2 : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;

  bool IsIntegerType(Type* t) ;

  int PowerOfTwo(IntConstant* i) ;

  void ProcessBinaryExpression(BinaryExpression* b) ;

 public:
  DivByConstEliminationPass2(SuifEnv* pEnv) ;
  Module* clone() const { return (Module*) this ; }
  void do_procedure_definition(ProcedureDefinition* p) ;
} ;

#endif 
