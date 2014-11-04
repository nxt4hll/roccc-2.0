
// This pass is for when things get unrolled and there is no more loop

#ifndef TRANSFORM_SYSTEMS_TO_MODULES_DOT_H
#define TRANSFORM_SYSTEMS_TO_MODULES_DOT_H

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>
#include <cfenodes/cfe.h>

class TransformSystemsToModules : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;

  bool HasLoop() ;
  bool PassedStruct() ;
  bool ComposedSystem() ;

  void HandleFeedback() ;
  void Transform() ;
  void ReplaceUses(VariableSymbol* original, VariableSymbol* replacement) ;
  void ReplaceDefinition(VariableSymbol* original, 
			 VariableSymbol* replacement) ;

 public:
  TransformSystemsToModules(SuifEnv* pEnv) ;
  Module* clone() const { return (Module*) this ; }
  void do_procedure_definition(ProcedureDefinition* proc_def) ;
} ;

#endif
