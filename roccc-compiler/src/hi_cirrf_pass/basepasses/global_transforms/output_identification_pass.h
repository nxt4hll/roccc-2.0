// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
/*

  This should be one of the last passes performed, but you must have 
   performed control flow analysis, data flow analysis, and ud/du chain
   building before this pass.

*/

#ifndef OUTPUT_IDENTIFICATION_PASS_DOT_H
#define OUTPUT_IDENTIFICATION_PASS_DOT_H

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"
#include "cfenodes/cfe.h"

class OutputIdentificationPass : public PipelinablePass 
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;

  void IdentifyComposedSystem() ;

  void ProcessLoop(StatementList* innermost) ;
  
  void ProcessOutputs(StatementList* innermost) ;
  void ProcessInputs(StatementList* innermost) ;

  void CleanupInductionVariables(CForStatement* innermost) ;

  void RemoveUses(list<VariableSymbol*>& alive, Statement* s) ;
  void AddDefinitions(list<VariableSymbol*>& alive, Statement* s) ;
  
 public:
  OutputIdentificationPass(SuifEnv* pEnv) ;
  Module* clone() const { return (Module*)this ; } 
  void do_procedure_definition(ProcedureDefinition *proc_def);
};

#endif
