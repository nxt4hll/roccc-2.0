// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
/*
  This pass performs copy propagation and works on ROCCC 2.0.

  It requires the bit vector 2 that I have defined.

  Important: This pass requires that control flow, data flow, and UD/DU
  be built directly before.

  There is an issue with the copy propagation pass when used with
   systolic arrays.  I'm writing a special case here.
  
*/

#ifndef COPY_PROPAGATION_PASS_TWO_H
#define COPY_PROPAGATION_PASS_TWO_H

#include <map>

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"
#include "common/suif_hash_map.h"

class CopyPropagationPass2 : public PipelinablePass 
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;

  list<Statement*> toBeRemoved ;

  // We need to make sure we don't remove any variables that are used
  //  as feedback, so this list keeps track of that.
  list<VariableSymbol*> feedbackVariables ;

  void Initialize() ;

  bool Special() ;

  // Warning!  This function only works in a very specific instance!
  //  this might break everything if it is called outside of systolic
  //  array generation!
  void ProcessSpecialIfs() ;

  void ProcessPossibleCopy(StoreVariableStatement* c) ;

  // This function returns true if the definition is the only
  //  reaching definition of the use
  bool IsOnlyDefinition(StoreVariableStatement* def, 
			LoadVariableExpression* use) ;

  // This is responsible for removing all of the statements that
  //  need to be added.
  void CleanUp() ;

  void RemoveFromStatementList(StatementList* parent, Statement* child) ;

  void HandleFeedbackVariables(LoadVariableExpression* nextLoad,
			       VariableSymbol* replacement) ;

  // These functions are copied from the data flow solve pass
  std::map<VariableSymbol*, list< std::pair< Statement*, int> >* > killMap ;
  int totalDefinitions ;
  void ClearMap() ;
  void InitializeMap() ;
  void CollectVariables() ;
  void CollectDefinitions() ;

public:
  CopyPropagationPass2(SuifEnv *pEnv);
  Module* clone() const { return (Module*)this; }
  void do_procedure_definition(ProcedureDefinition *proc_def);
};

#endif
