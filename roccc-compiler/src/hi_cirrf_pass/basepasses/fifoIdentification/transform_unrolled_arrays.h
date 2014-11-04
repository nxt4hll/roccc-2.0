// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*
  The purpose of this pass is to locate all arrays that have been 
   fully unrolled and change them into something else (scalars or arrays
   of lesser dimensions).

   This isn't the most straightforward thing as the transformation may 
    completely change the way data is passed.

*/

#ifndef TRANSFORM_UNROLLED_ARRAYS_DOT_H
#define TRANSFORM_UNROLLED_ARRAYS_DOT_H

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>
#include <cfenodes/cfe.h>

// For each instance of an equivalent reference I need to have one
//  and only one replacement array
class EquivalentReferences
{
 public:
  ArrayReferenceExpression* original ;
  list<ArrayReferenceExpression*> allEquivalent ;
  
 public:
  EquivalentReferences() ;
  ~EquivalentReferences() ;
} ;

class TransformUnrolledArraysPass : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;

  StatementList* innermost ;

  list<EquivalentReferences*> currentReferences ;

  // Functions that deal with equivalent references
  void CollectArrays(int dimensionality) ;
  void ClearReferences() ;
  void Uniquify() ;  

  // Functions that turn all N-dimensional arrays with a constant
  //  index into N-1 Dimensional arrays with a new name.
  void TransformNDIntoNMinusOneD(int N) ;
  bool ReplaceNDReference(EquivalentReferences* a) ;

  // A function that just finds the maximium dimension of all array 
  //  accesses in the function.
  int MaxDimension() ;
  
  // Helper functions
  bool InList(list<VariableSymbol*>& theList, VariableSymbol* v) ;
  LString GetReplacementName(LString baseName, int offset) ;
  void ReplaceSymbol(ArrayReferenceExpression* x,
		     SymbolAddressExpression* addr) ;

  QualifiedType* OneLessDimension(QualifiedType* original, int dimensionality);

 public:
  TransformUnrolledArraysPass(SuifEnv* pEnv) ;
  ~TransformUnrolledArraysPass() ;
  Module* clone() const { return (Module*) this ; }
  void do_procedure_definition(ProcedureDefinition* proc_def) ;
} ;

#endif
