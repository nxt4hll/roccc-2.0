// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

#ifndef MINI_SCALAR_REPLACEMENT_DOT_H
#define MINI_SCALAR_REPLACEMENT_DOT_H

/*
  The purpose of this pass is to find all of the stores in if statements that
   go to array accesses and create scalars for them.  I should not do this
   for array accesses that access a constant value as these should
   be handled in another pass.
*/

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>

class MiniScalarReplacementPass : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;

  void ProcessIf(IfStatement* i) ;

 public:
  MiniScalarReplacementPass(SuifEnv* pEnv) ;
  ~MiniScalarReplacementPass() ;
  Module* clone() const { return (Module*) this ; }
  void do_procedure_definition(ProcedureDefinition* p) ;  
} ;

#endif
