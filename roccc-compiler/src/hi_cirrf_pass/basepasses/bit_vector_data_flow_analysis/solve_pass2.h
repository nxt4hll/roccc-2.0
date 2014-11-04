// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*
  This file defines the data flow solve pass for ROCCC 2.0.
    Note: The control flow pass should be identical for both ROCCC 1.0 and
    ROCCC 2.0.  The only difference in the data flow is that call statements
    are no longer defining one variable, but may in fact define several 
    variables.  This has to be accounted for at every level.
*/

#ifndef DATAFLOW_SOLVE_PASS_TWO_H
#define DATAFLOW_SOLVE_PASS_TWO_H

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"
#include <suifkernel/group_walker.h>
#include "cfenodes/cfe.h"
#include <map>

// Forward declaration
class BitVector2 ;

class DataFlowSolvePass2 : public PipelinablePass 
{

 private:

  // This map associates each variable symbol with a list of all
  //  the definitions of that variable symbol.  Additionally, each
  //  definition is numbered.
  std::map<VariableSymbol*, list< std::pair< Statement*, int> >* > killMap ;

  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;

  int totalDefinitions ;

  void SolveStatement(Statement* s) ;
  void SolveIfStatement(IfStatement* s) ;
  void SolveWhileStatement(WhileStatement* s) ;
  void SolveCForStatement(CForStatement* s) ;
  void SolveScopeStatement(ScopeStatement* s) ;
  void SolveStoreVariableStatement(StoreVariableStatement* s) ;
  void SolveCallStatement(CallStatement* s) ;
  void SolveStatementList(StatementList* s) ;

  // This one is used when the statement does not generate any definitions
  //  or kill any previous definitions AND there are no children statements.
  void StandardSolve(Statement* s) ;

  // This function initializes the big map that keeps track of every definition
  //  associated with a variable.
  void InitializeMap() ;

  void CollectVariables() ;
  void CollectDefinitions() ;

  void SetupAnnotations(Statement* s) ;
  void CleanupOldAnnotations(Statement* s) ;
  void AddNewAnnotations(Statement* s) ;

  // This is a helper function that takes in the predecessor annotation
  //  and merges all of the out_stmt annotations into one BitVector2
  // Warning! "final" is modified by this function.
  void UnionPredecessors(BitVector2* final, BrickAnnote* predAnnote) ;

  // Test functionality to verify that data flow is correct.
  void DumpDataFlow() ;

  // If this pass is called mulitple times, we want to empty out
  //  the kill map and restart.
  void ClearMap() ;

public:
  DataFlowSolvePass2(SuifEnv *pEnv);
  ~DataFlowSolvePass2() ;
  Module* clone() const { return (Module*)this; }
  void do_procedure_definition(ProcedureDefinition *procDef);
};

#endif
