
#ifndef CONSTANT_ARRAY_PROPAGATION_PASS_DOT_H
#define CONSTANT_ARRAY_PROPAGATION_PASS_DOT_H

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>
#include <map>

class ConstantArrayPropagationPass : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;

  std::map<VariableSymbol*, ValueBlock*> initializations ;

  void CollectInitializations() ;
  bool ValidSymbol(VariableSymbol* varSym) ;

  void ReplaceLoads() ;
  void ReplaceLoad(LoadExpression* load, 
		   ArrayReferenceExpression* ref,
		   VariableSymbol* var,
		   ValueBlock* topBlock) ;

 public:
  ConstantArrayPropagationPass(SuifEnv* pEnv) ;
  Module* clone() const { return (Module*) this ; }
  void do_procedure_definition(ProcedureDefinition* proc_def) ;
} ;

#endif
