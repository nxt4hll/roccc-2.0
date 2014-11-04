// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

#ifndef REMOVE_MODULE_PASS_DOT_H
#define REMOVE_MODULE_PASS_DOT_H

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>
#include <suifkernel/suif_env.h>

class RemoveModulePass : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;

  String moduleName ;
  String localDirectory ;

  FileSetBlock* repository ;
  
  void InitializeRepository() ;
  void DumpRepository() ;
  ProcedureSymbol* FindProcedure(String moduleName) ;
  void RemoveProcedure(ProcedureSymbol* p) ;

  bool InList(list<Type*>& used, Type* x) ;

 public:
  void initialize() ;

  RemoveModulePass(SuifEnv* pEnv) ;
  ~RemoveModulePass() ;
  Module* clone() const { return (Module*) this ; }
  void execute() ;
} ;

#endif
