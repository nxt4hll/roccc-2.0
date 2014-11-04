// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*

  This is the pass that solves feedback variables for ROCCC 2.0.
    It encompasses and includes the functionality of both the original
    solve feedback variables pass and the solve feedback variables 2 pass.
    This means it must handle both Store Variable Statements and 
    Call Statements.

  The purpose of this pass is to identify all variables which are used
    as feedback variables and annotate them with both the variables used 
    for storeNext and loadPrevious.  I will put the loadPrevious variable
    as the first brick and the storeNext variable as the second brick.

  With Call Statements, the output variables should not be replaced
    because they have already been handled.

  Note: Scalar Replacement must have been run before this pass.  Also, 
   UD/DU chain building must have been called before this pass.

*/

#ifndef __SOLVE_FEEDBACK_VARIABLES_TWO_DOT_H__
#define __SOLVE_FEEDBACK_VARIABLES_TWO_DOT_H__

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"

#include <map>

class SolveFeedbackVariablesPass2 : public PipelinablePass
{
 private:
  ProcedureDefinition* procDef ;
  SuifEnv* theEnv ;

  list<VariableSymbol*> storeVars ;

  void SetupAnnotations(VariableSymbol* toAnnote, 
			VariableSymbol* source,
			VariableSymbol* destination) ;

  // Annotate all store instructions that load from a feedback variable
  void ProcessStores() ;

  // Check to see if a variable symbol is inside the store var list
  bool isStoreVar(VariableSymbol* v) ;  

 public:
  SolveFeedbackVariablesPass2(SuifEnv* pEnv) ;
  Module* clone() const { return (Module*) this ; }
  void do_procedure_definition(ProcedureDefinition* proc_def) ;
} ;

#endif
