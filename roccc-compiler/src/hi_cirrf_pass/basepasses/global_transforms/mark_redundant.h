// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

#ifndef MARK_REDUNDANT_PASS_DOT_H
#define MARK_REDUNDANT_PASS_DOT_H

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>

class MarkRedundantPass : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;

  String redundantLabel ;
  int doubleOrTriple ; // 0 == double, 1 == triple

 public:
  MarkRedundantPass(SuifEnv* pEnv) ;
  Module* clone() const { return (Module*) this ; }
  void initialize() ;
  void do_procedure_definition(ProcedureDefinition* proc_def) ;
} ;

#endif
