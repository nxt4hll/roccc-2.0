// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

#ifndef INTERCHANGE_PASS_DOT_H
#define INTERCHANGE_PASS_DOT_H

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>
#include <cfenodes/cfe.h>

class InterchangePass : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;

  void Interchange(CForStatement* one, CForStatement* two) ;

  String loopOne ;
  String loopTwo ;

 public:
  InterchangePass(SuifEnv* pEnv) ;
  ~InterchangePass() ;
  void initialize() ;
  void do_procedure_definition(ProcedureDefinition* p) ;
  Module* clone() const { return (Module*) this ; }

} ;

#endif
