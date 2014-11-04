// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

#ifndef ADD_MODULE_PASS_DOT_H
#define ADD_MODULE_PASS_DOT_H

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>
#include <suifkernel/suif_env.h>

// Forward declarations
class Port ;
class ModuleEntry ;

class AddModulePass : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;

  FileSetBlock* repository ;

  String moduleFile ;
  String localDirectory ;

  void InitializeRepository() ;
  void DumpRepository() ;

  ModuleEntry ProcessFile(String filename) ;
  void AddProcedure(ModuleEntry m) ;

 public:
  void initialize() ;
  void execute() ;

  AddModulePass(SuifEnv* pEnv) ;
  ~AddModulePass() ;
  Module* clone() const { return (Module*) this ; }
  
} ;

#endif
