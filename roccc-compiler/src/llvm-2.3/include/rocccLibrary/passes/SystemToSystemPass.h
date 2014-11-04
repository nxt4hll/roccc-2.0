// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details

/*

  This file connects systems.

 */

#ifndef _SYSTEM_TO_SYSTEM_PASS_H__
#define _SYSTEM_TO_SYSTEM_PASS_H__

#include "llvm/Pass.h"
#include "llvm/Function.h"
#include "rocccLibrary/VHDLInterface.h"
#include "rocccLibrary/StreamVariable.h"

namespace llvm 
{

class ROCCCSystemToSystemPass : public FunctionPass
{
  std::vector<StreamVariable> streams;
public:
  VHDLInterface::Entity* entity;
  std::vector<StreamVariable>::iterator streamBegin();
  std::vector<StreamVariable>::iterator streamEnd();
  // Used for registration in llvm's pass manager
  static char ID ;

  ROCCCSystemToSystemPass() ;
  ~ROCCCSystemToSystemPass() ;

  virtual bool runOnFunction(Function& M);
  void getAnalysisUsage(AnalysisUsage &AU) const;
};

}

#endif

