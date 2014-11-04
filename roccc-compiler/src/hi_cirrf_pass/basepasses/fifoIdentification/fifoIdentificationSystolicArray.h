// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
/* 

   This pass is responsible for identifying fifos in the generation of
    systolic arrays in ROCCC 2.0.

*/

#ifndef __FIFO_IDENTIFICATION_SYSTOLIC_DOT_H__
#define __FIFO_IDENTIFICATION_SYSTOLIC_DOT_H__

#include <string>

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"
#include "cfenodes/cfe.h"

class FifoIdentificationSystolicArrayPass : public PipelinablePass 
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;

  // These two symbols will be the replacement one-dimensional 
  //  arrays.  One is used for the input stream and one is used
  //  for the output stream.
  VariableSymbol* inputArray ;
  VariableSymbol* outputArray ;

  // There can be multiple store variable statements, but we
  //  might not want to add all of them.
  list<StoreVariableStatement*> allStoreVariablesToReplace ;
  list<Expression*> allInputIndicies ;

  // We need to locate where the output fifo is, and we might have to
  //  create one just in case.
  list<StoreStatement*> allStoresToReplace ;
  list<Expression*> allOutputIndicies ;

  // This array reference expression is used to grab the name and index of
  //  the two dimensional array for us to create the single dimensional
  //  arrays.
  ArrayReferenceExpression* refExpr ;

  // Just in case everything is a feedback, we are going to keep track
  //  of the last variable that is stored.
  VariableSymbol* lastDest ;

  // Functions
  CForStatement* FindInnermostLoop() ;
  void CollectInformation(CForStatement* c) ;
  void CreateDuplicates() ;
  void Replace(CForStatement* c) ;

  // This function is passed an index and returns if there are nothing
  //  but constants (in which case we should change the variable into
  //  an input scalar).
  bool AllConstants(Expression* i) ;

 public:
  FifoIdentificationSystolicArrayPass(SuifEnv* pEnv) ;
  Module* clone() const { return (Module*) this ; }
  void do_procedure_definition(ProcedureDefinition* p) ;
} ;

#endif
