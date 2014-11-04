// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

#ifndef LUT_DETECTION_DOT_H
#define LUT_DETECTION_DOT_H

// The purpose of this pass is to 

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>

class LUTDetectionPass : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;
 public:
  LUTDetectionPass(SuifEnv* pEnv) ;
  Module* clone() const { return (Module*) this ; }
  void do_procedure_definition(ProcedureDefinition* p) ;
} ;

#endif
