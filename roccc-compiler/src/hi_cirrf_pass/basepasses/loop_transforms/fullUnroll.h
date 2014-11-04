
#ifndef FULL_UNROLL_DOT_H
#define FULL_UNROLL_DOT_H

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>
#include <cfenodes/cfe.h>

class FullUnrollPass : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;

  // The information for the current loop being unrolled
  int stepValue ;
  int unrollAmount ;
  VariableSymbol* index ;

  void ProcessLoop(CForStatement* loop) ;
  void CollectInformation(CForStatement* loop) ;

  void CollectStep(CForStatement* loop) ;
  void CollectIndex(CForStatement* loop) ;
  void CollectUnrollAmount(CForStatement* loop) ;

  void AdjustOuterLoop(CForStatement* loop) ;
  void AnnotateIndex() ;

  void ResetValues() ;

  void ProcessLoadsParallel(Statement* n, int loopCopy) ;

  void ControlFlowAnalysis() ;
  void DataFlowAnalysis() ;
  void BuildChains() ;
  void ConstantPropagation() ;
  void CleanUpAnnotations() ;

 public:
  FullUnrollPass(SuifEnv* pEnv) ;
  ~FullUnrollPass() ;
  Module* clone() const { return (Module*) this ; }
  void do_procedure_definition(ProcedureDefinition* proc_def) ;
  
} ;

class AnnotationCleaner
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;
  
  void CleanObject(AnnotableObject* o) ;
  void RemoveAnnotations(AnnotableObject* o, const char* annoteName) ;
 public:
  AnnotationCleaner(SuifEnv* pEnv) ;
  void Clean(ProcedureDefinition* p) ;
} ;

#endif
