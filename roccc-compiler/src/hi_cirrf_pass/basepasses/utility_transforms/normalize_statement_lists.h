// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

#ifndef NORMALIZE_STATEMENT_LISTS_DOT_H
#define NORMALIZE_STATEMENT_LISTS_DOT_H

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>

class NormalizeStatementListsPass : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;

  void NormalizeForLoops() ;
  void NormalizeIfStatements() ;
  void NormalizeProcDef() ;

 public:
  NormalizeStatementListsPass(SuifEnv* pEnv) ;
  Module* clone() const { return (Module*) this ; }
  void do_procedure_definition(ProcedureDefinition* proc_def) ;
} ;

#endif
