// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*

  STUBBY
   
*/

#ifndef __TIMING_REQUIREMENTS_H__
#define __TIMING_REQUIREMENTS_H__

#include "llvm/BasicBlock.h"
#include "llvm/Function.h"
#include <map>

namespace Pipelining {
class TimingRequirements {
  static std::map<llvm::Function*,TimingRequirements*> instances;
  TimingRequirements(llvm::Function* f);
  std::map<llvm::BasicBlock*,int> timingValues;
  int desiredDelay;
public:
  static TimingRequirements* getCurrentRequirements(llvm::Function* f);
  int getDesiredDelay();
  int getBasicBlockDelay(llvm::BasicBlock* BB);
  void setBasicBlockDelay(llvm::BasicBlock* BB, int delay);
};
}

#endif

