
#ifndef SCALAR_REPLACEMENT_TWO_DOT_H
#define SCALAR_REPLACEMENT_TWO_DOT_H

#include <map>

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>

class ScalarReplacementPass2 : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;

  // For verifying...
  std::map<VariableSymbol*, list<VariableSymbol*>* > arrayIndicies ;
  void ClearArrayIndicies() ;
  void VerifyArrayReferences() ;
  
  void CollectArrayReferences() ;  
  list<std::pair<Expression*, VariableSymbol*> > Identified ;
  list<std::pair<Expression*, VariableSymbol*> > IdentifiedLoads ;
  list<std::pair<Expression*, VariableSymbol*> > IdentifiedStores ;

  void ProcessLoads() ;
  void ProcessStores() ;

  void ProcessLoad(LoadExpression* e) ;
  void ProcessStore(StoreStatement* s) ;

  void PrependLoads() ;
  void AppendStores() ;
  
 public:
  ScalarReplacementPass2(SuifEnv* pEnv) ;
  ~ScalarReplacementPass2() ;
  Module* clone() const { return (Module*) this ; }
  void do_procedure_definition(ProcedureDefinition* p) ;  
} ;

#endif
