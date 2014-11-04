/*

  This pass is responsible for cleaning up all of the unrolled calls that
   we might have.

  What that means, is if we have a module call that was replicated, 
   output variables need to be replicated.

*/

#ifndef CLEANUP_UNROLLED_CALLS_DOT_H
#define CLEANUP_UNROLLED_CALLS_DOT_H

#include <string>

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>

class CleanupUnrolledCalls : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;

  int nameCounter ;

  void ProcessCall(CallStatement* c, StatementList* parentList, int position) ;
  void ProcessStatement(Statement* p) ;

 public:
  CleanupUnrolledCalls(SuifEnv* pEnv) ;
  Module* clone() const { return (Module*)this ; }
  void do_procedure_definition(ProcedureDefinition* proc_def) ;
} ;

#endif
