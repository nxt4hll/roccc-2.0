// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*
  The purpose of this pass is to go through all of the variables in the
   symbol table and remove any that are not used in the procedure.  This
   happens because all of our transformations completely change the variables
   used.
*/

#ifndef REMOVE_UNUSED_VARIABLES_DOT_H
#define REMOVE_UNUSED_VARIABLES_DOT_H

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>

class RemoveUnusedVariables : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;

  // For checking if arrays are used or not
  list<ArrayReferenceExpression*>* allRefs ;

  // Only collect once
  list<LoadVariableExpression*>* allLoads ;
  list<StoreVariableStatement*>* allStores ;
  list<CallStatement*>* allCalls ;

  bool HasDefinition(VariableSymbol* v) ;
  bool HasUse(VariableSymbol* v) ;

 public:
  RemoveUnusedVariables(SuifEnv* pEnv) ;
  ~RemoveUnusedVariables() ;
  Module* clone() const { return (Module*) this ; }
  void do_procedure_definition(ProcedureDefinition* proc_def) ;
} ;

#endif
