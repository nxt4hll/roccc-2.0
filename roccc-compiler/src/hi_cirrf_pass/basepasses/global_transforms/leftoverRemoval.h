// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
/*
  This pass goes through and removes the extraneous information generated
   during systolic array generation that mess up the lower levels in llvm.
  
*/

#ifndef __LEFT_OVER_REMOVAL_DOT_H__
#define __LEFT_OVER_REMOVAL_DOT_H__

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"
#include "common/suif_hash_map.h"

class LeftoverRemovalPass : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;
 public:
  LeftoverRemovalPass(SuifEnv* pEnv) ;
  Module* clone() const { return (Module*) this ; }
  void do_procedure_definition(ProcedureDefinition* proc_def) ;
} ;

#endif
