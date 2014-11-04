// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*

  This is the pass that solves feedback variables for ROCCC 2.0.
    It finds any variable that has a write after read inside of the innermost
    loop.  

  We should perform the SolveFeedbackCalls pass before this one, meaning we
    only need to look at Store Variable Statements and not calls.

  Note: Scalar Replacement must have been run before this pass.  Also, 
   UD/DU chain building must have been called before this pass.

*/

#ifndef __SOLVE_FEEDBACK_VARIABLES_THREE_DOT_H__
#define __SOLVE_FEEDBACK_VARIABLES_THREE_DOT_H__

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"
#include "cfenodes/cfe.h"

struct PossibleFeedbackPair
{
  VariableSymbol* varSym ;
  LoadVariableExpression* use ;
  Statement* definition ; 
  int definitionLocation ;
} ;

class SolveFeedbackVariablesPass3 : public PipelinablePass
{
 private:
  ProcedureDefinition* procDef ;
  SuifEnv* theEnv ;
  CForStatement* innermost ;

  list<PossibleFeedbackPair> actualFeedbacks ;

  int isSystolic ;

  void SetupAnnotations() ;

  void SetupAnnotations(VariableSymbol* toAnnote, 
			VariableSymbol* source,
			VariableSymbol* destination) ;

  void DetermineNewFeedbacks() ;

 public:
  SolveFeedbackVariablesPass3(SuifEnv* pEnv) ;
  Module* clone() const { return (Module*) this ; }
  void initialize() ;
  void do_procedure_definition(ProcedureDefinition* proc_def) ;
} ;

#endif
