// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*

  The purpose of this pass is to eliminate all available expressions in the
   innermost loop (or statement list).  It is not true available 
   code elimination because we are still assuming the ROCCC structure
   and limitations.

  This pass must be performed before PseduoSSA.

 */

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>

class AvailableCodeEliminationPass : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;

  bool ProcessStatementList(StatementList* s) ;

 public:
  AvailableCodeEliminationPass(SuifEnv* pEnv) ;
  Module* clone() const { return (Module*) this ; }
  void do_procedure_definition(ProcedureDefinition* p) ;
} ;
