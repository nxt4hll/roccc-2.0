#ifndef CAST_PASS_DOT_H
#define CAST_PASS_DOT_H

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>
#include <suifkernel/suif_env.h>
#include <cfenodes/cfe.h>

class CastPass : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;

  ProcedureSymbol* FloatToInt ;
  ProcedureSymbol* IntToFloat ;
  ProcedureSymbol* IntToInt ;
  ProcedureSymbol* FloatToFloat ;

  CProcedureType* FloatToIntType ;
  CProcedureType* IntToFloatType ;
  CProcedureType* IntToIntType ;
  CProcedureType* FloatToFloatType ;

  FloatingPointType* baseFloat ;
  IntegerType* baseInt ;

  bool FloatToIntUsed ;
  bool IntToFloatUsed ;
  bool IntToIntUsed ;
  bool FloatToFloatUsed ;

  void CreateProcedures() ;

  void HandleCallStatements() ;
  void HandleBinaryExpressions() ;
  void HandleStoreVariableStatements() ;
  void HandleStoreStatements() ;

  bool SingularVariable(Expression* e) ;

  void CleanupProcedures() ;

 public:
  CastPass(SuifEnv* pEnv) ;
  ~CastPass() ;
  Module* clone() const { return (Module*) this ; }
  void do_procedure_definition(ProcedureDefinition* proc_def) ;
} ;

#endif
