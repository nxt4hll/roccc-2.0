// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details

/*

  This file declares the class that will convert all floating point 
    operations into hardware calls as ROCCC expects them.

 */

#ifndef __INTRINSIC_PASS_DOT_H__
#define __INTRINSIC_PASS_DOT_H__

#include "llvm/Pass.h"

namespace llvm 
{

class ROCCCIntrinsicPass : public ModulePass
{
public:
  // Used for registration in llvm's pass manager
  static char ID ;

  ROCCCIntrinsicPass() ;
  ~ROCCCIntrinsicPass() ;

  virtual bool runOnModule(Module& M);
};

}

#endif

