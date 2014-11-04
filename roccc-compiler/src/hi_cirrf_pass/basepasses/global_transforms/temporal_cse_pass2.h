// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
/*

  This file contains the temporal common subexpression elimination pass
   declaration for ROCCC 2.0.
   
   Note: This pass must be run before scalar replacement.

*/

#ifndef TEMPORAL_CSE_PASS_H
#define TEMPORAL_CSE_PASS_H

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"
#include "cfenodes/cfe.h"

class TemporalCSEPass : public PipelinablePass 
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;

  int tmpNum ;

  int stride ;
  
  int maxLevels ;

  bool ProperFormat(CForStatement* c) ;

  void SetStride(CForStatement* c) ;
  void SetStrideMultiDimensional(CForStatement* c) ;

  // This function will forwardly propagate reuses of array accesses.
  void HandleArrayReuse(CForStatement* c) ;

  void ProcessLoopStatements(CForStatement* c) ;
  void ProcessLoopStatementsMulti(CForStatement* c) ;

  void ProcessPair(Statement* x, Statement* y) ;
  void ProcessPairCalls(CallStatement* x, CallStatement* y) ;
  void ProcessPairStoreVars(StoreVariableStatement* x,
			    StoreVariableStatement* y) ;
  void ProcessPairStores(StoreStatement* x, 
			 StoreStatement* y) ;

  bool EquivalentStatements(Statement* x, Statement* y) ;
  bool EquivalentCalls(CallStatement* x, CallStatement* y) ;
  bool EquivalentStores(StoreStatement* x, StoreStatement* y) ;
  bool EquivalentStoreVars(StoreVariableStatement* x, 
			   StoreVariableStatement* y) ;
  
  bool equivalent(ArrayReferenceExpression* x, ArrayReferenceExpression* y,
		  int depth) ;

  bool SameIndex(Expression* x, Expression* y, int diff = 0) ;
  bool SameIndex2(Expression* x, Expression* y, int diff = 0) ;

  bool InList(list<ArrayReferenceExpression*>& x, 
	      ArrayReferenceExpression* y) ;
  
  void AddLoads(list<ArrayReferenceExpression*>& x, Statement* s) ;

  int ReplaceEquivalences(Statement* currentStatement, 
			  list<ArrayReferenceExpression*>& x,
			  StatementList* parentList,
			  int position) ;

  void RemoveExactEquivalences(Statement* currentStatement, 
			       list<ArrayReferenceExpression*>& x) ;

  // This function is used to verify that basic expressions are identical
  bool EquivalentExpressionTrees(Expression* x, Expression* y) ;

  int ReplaceAllUsesWith(ArrayReferenceExpression* original,
			 VariableSymbol* newSym,
			 StatementList* containingList,
			 int position) ;

  int GetOffset(ArrayReferenceExpression* x) ;

  CForStatement* FindParentForLoop(CForStatement* c) ;

 public:
  TemporalCSEPass(SuifEnv *pEnv);
  Module* clone() const { return (Module*)this; }
  void do_procedure_definition(ProcedureDefinition *proc_def);
};

#endif
