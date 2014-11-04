// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*

  DESC

 */

#include "llvm/Pass.h"
#include "llvm/Function.h"
#include "llvm/Constants.h"

#include "rocccLibrary/InternalWarning.h"

namespace llvm
{
  class UndefDetectionPass : public FunctionPass
  {
  private:
  public:
    static char ID ;
    UndefDetectionPass() ;
    ~UndefDetectionPass() ;
    virtual bool runOnFunction(Function& b) ;
  } ;
}

using namespace llvm ;

char UndefDetectionPass::ID = 0 ;

static RegisterPass<UndefDetectionPass> X ("undefDetect", 
					"Detect uses of undefined variables.");

UndefDetectionPass::UndefDetectionPass() : FunctionPass((intptr_t)&ID) 
{
  ; // Nothing in here
}

UndefDetectionPass::~UndefDetectionPass()
{
  ; // Nothing to delete either
}


bool UndefDetectionPass::runOnFunction(Function& f)
{
  CurrentFile::set(__FILE__);
  bool changed = false ;
  
  for(Function::iterator BB = f.begin(); BB != f.end(); ++BB)
  {
    for(BasicBlock::iterator II = BB->begin(); II != BB->end(); ++II)
    {
      for(User::op_iterator OI = II->op_begin(); OI != II->op_end(); ++OI)
      {
        if( dynamic_cast<UndefValue*>(OI->get()) )
        {
          INTERNAL_ERROR("Detected undef in " << *II << "\n");
          assert( 0 and "Undef found!" );
        }
      }
    }
  }

  return changed ;
}

