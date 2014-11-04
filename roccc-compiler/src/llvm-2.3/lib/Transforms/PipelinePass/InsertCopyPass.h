// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*

  This file contains the declaration of the InsertCopyPass class.  This 
    pass is responsible for creating temporaries and inserting copy
    instructions in each empty slot.
 
*/

#ifndef __INSERT_COPY_PASS_DOT_H__
#define __INSERT_COPY_PASS_DOT_H__

#include "llvm/Pass.h"
#include "rocccLibrary/DFFunction.h"
#include "rocccLibrary/DFBasicBlock.h"

namespace llvm
{
  class InsertCopyPass : public FunctionPass
  {
  public:
    static char ID ;
    InsertCopyPass() ;
    ~InsertCopyPass() ;
    virtual bool runOnFunction(Function& f) ;
    virtual void getAnalysisUsage(AnalysisUsage &AU) const;
  } ;
}

#endif
