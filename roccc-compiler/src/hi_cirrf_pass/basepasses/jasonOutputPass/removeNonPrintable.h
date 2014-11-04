// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
/*
  This pass is responsible for finding each non printable instruction and
   removing it from the tree as well as cleaning up any memory associated 
   with the annotation.
*/

#ifndef __NONPRINTABLE_DOT_H__
#define __NONPRINTABLE_DOT_H__

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"
#include "cfenodes/cfe.h"

class RemoveNonPrintablePass : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;

 public:
  RemoveNonPrintablePass(SuifEnv* pEnv) ;
  Module* clone() const { return (Module*) this ; }
  void do_procedure_definition(ProcedureDefinition* p) ;
} ;

#endif 
