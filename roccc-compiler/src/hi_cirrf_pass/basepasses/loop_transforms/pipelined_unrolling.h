// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
/*

  This pass unrolls a loop that might contain a module instantiation and
   creates ways to pass data to and from the module instantiation.

  I am assuming that this pass works the same way the other loop 
   unrolling passes do.  Namely, we will pass two parameters to the pass,
   the first will be the label of the loop I am unrolling and the 
   second will be the maximum size we will unroll by.
*/

#ifndef _PIPELINED_UNROLLING_DOT_H_
#define _PIPELINED_UNROLLING_DOT_H_

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"

class PipelinedUnrollingPass : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;

  String loopToUnroll ;
  int unrollLimit ;

 public:
  PipelinedUnrollingPass(SuifEnv* pEnv) ;
  Module* clone() const { return (Module*) this ; }
  void do_procedure_definition(ProcedureDefinition* proc_def) ;

  // In order to use the command line options, I need to call the
  //  initialize function.  It must be named initialize!
  void initialize() ;

} ;

#endif
