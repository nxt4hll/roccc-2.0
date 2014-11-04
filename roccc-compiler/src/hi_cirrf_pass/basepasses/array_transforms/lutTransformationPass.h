
#ifndef LUT_TRANSFORMATION_PASS_DOT_H
#define LUT_TRANSFORMATION_PASS_DOT_H

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>

class LUTTransformationPass : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;

  int LUTCounter ;

  void ReplaceLookup(LoadExpression* container, ArrayReferenceExpression* x) ;
  void ReplaceStore(StoreStatement* container, ArrayReferenceExpression* x) ;

  void ReplaceLoads() ;
  void ReplaceStores() ;

  Expression* CreateIndex(ArrayReferenceExpression* x) ;

 public:
  LUTTransformationPass(SuifEnv* pEnv) ;
  ~LUTTransformationPass() ;
  Module* clone() const { return (Module*) this ; } 
  void do_procedure_definition(ProcedureDefinition* p) ;
} ;

#endif
