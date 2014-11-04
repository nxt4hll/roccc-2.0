// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*

  This pass is responsible for finding all evaluation statements and 
    converting them into something that works with all of the other
    passes.

*/

#ifndef EVAL_TRANSFORM_DOT_H
#define EVAL_TRANSFORM_DOT_H

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>

class EvalTransformPass : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;

  void ProcessEval(EvalStatement* e) ;

 public:
  EvalTransformPass(SuifEnv* pEnv) ;
  Module* clone() const { return (Module*) this ; }
  void do_procedure_definition(ProcedureDefinition* p) ;
} ;

#endif
