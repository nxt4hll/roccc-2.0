// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#ifndef CLEAN_REPOSITORY_DOT_H
#define CLEAN_REPOSITORY_DOT_H

#include <suifpasses/suifpasses.h>
#include <suifnodes/suif.h>
#include <suifkernel/suif_env.h>

class CleanRepositoryPass : public PipelinablePass
{
 private:
  SuifEnv* theEnv ;
  FileSetBlock* repository ;

  String localDirectory ;
  
  void InitializeRepository() ;
  void CleanRepository() ;
  void DumpRepository();

 public:
  CleanRepositoryPass(SuifEnv* pEnv) ;
  ~CleanRepositoryPass() ;
  void initialize() ;
  void execute() ;

  Module* clone() const { return (Module*) this ; }
} ;

#endif 
