// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#ifndef OUTPUT_PASS_DOT_H
#define OUTPUT_PASS_DOT_H

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"
#include "suifkernel/suif_env.h"

#include "baseOutput.h"

class OutputPass : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;
  HiCirrfGenerator* theGenerator ;

  String streamFileName ;

 public:
  OutputPass(SuifEnv* pEnv) ;
  ~OutputPass() ;
  Module* clone() const { return (Module*) this ; }
  void do_procedure_definition(ProcedureDefinition *proc_def);
  void initialize() ;
} ;

#endif
