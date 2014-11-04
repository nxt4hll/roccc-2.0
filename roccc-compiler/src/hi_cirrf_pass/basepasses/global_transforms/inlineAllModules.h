// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

#ifndef INLINE_ALL_MODULES_DOT_H
#define INLINE_ALL_MODULES_DOT_H

#include <string>
#include <map>

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>

// A lot of the code in these files is copied from the normal inlining pass
//  and should be refactored.
class InlineAllModulesPass : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  ProcedureDefinition* procDef ;
  
  String depth ;
  String mapFileName ;
  
  bool InlineAll() ;
  bool ProcessCall(CallStatement* c) ;

  // These are the same as in InliningPass
  std::map<ParameterSymbol*, Expression*> parameterMapping ;
  std::map<VariableSymbol*, VariableSymbol*> localVarMapping ;
  
  std::string LookupFile(LString functionName) ;

  void VerifyProcedure(ProcedureDefinition* p) ;
  void ClearMappings() ;
  void CreateMappings(ProcedureDefinition* p, CallStatement* c) ;
  void CopyValueBlocks(ProcedureDefinition* p) ;
  void ReplaceSymbolAddresses(ProcedureDefinition* p) ;
  void ReplaceLoadVariables(ProcedureDefinition* p) ;
  void ReplaceStoreVariables(ProcedureDefinition* p) ;
  StatementList* ConstructReplacement(ProcedureDefinition* p) ;

  // This is functionally equivalent to the code in the eval transform 
  //  pass.
  void ProcessEval(EvalStatement* e) ;

 public:
  InlineAllModulesPass(SuifEnv* pEnv) ;
  ~InlineAllModulesPass() ;

  Module* clone() const { return (Module*) this ; }

  void initialize() ;
  void do_procedure_definition(ProcedureDefinition* p) ;
} ;

#endif
