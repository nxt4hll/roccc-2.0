// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

#ifndef INLINING_PASS_DOT_H
#define INLINING_PASS_DOT_H

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>
#include <suifkernel/suif_env.h>

#include <string>
#include <map>

class InliningPass : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;
  String functionName ;
  String mapFileName ;

  std::map<ParameterSymbol*, Expression*> parameterMapping ;
  std::map<VariableSymbol*, VariableSymbol*> localVarMapping ;

  void ProcessCall(CallStatement* c) ;

  std::string LookupFile(LString functionName) ;

  Statement* CopyStatement(Statement* s) ;

  void ClearMappings() ;
  void CreateMappings(ProcedureDefinition* p, CallStatement* c) ;
  void CopyValueBlocks(ProcedureDefinition* p) ;

  void ReplaceLoadVariables(ProcedureDefinition* p) ;
  void ReplaceStoreVariables(ProcedureDefinition* p) ;
  void ReplaceSymbolAddresses(ProcedureDefinition* p) ;
  StatementList* ConstructReplacement(ProcedureDefinition* p) ;
  
  void VerifyProcedure(ProcedureDefinition* p) ;

 public:
  InliningPass(SuifEnv* pEnv) ;
  ~InliningPass() ;
  void initialize() ;

  void do_procedure_definition(ProcedureDefinition* p) ;
  Module* clone() const { return (Module*) this ; }
} ;


#endif
