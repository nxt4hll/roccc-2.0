// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*

DESC

*/

#include "llvm/Pass.h"
#include "llvm/Function.h"
#include "llvm/Instructions.h"
#include "llvm/Constants.h"

#include "rocccLibrary/InternalWarning.h"
#include "rocccLibrary/ROCCCNames.h"

namespace llvm
{
  class RemoveExtendsPass : public FunctionPass
  {
  private:
  public:
    static char ID ;
    RemoveExtendsPass() ;
    ~RemoveExtendsPass() ;
    virtual bool runOnFunction(Function& b) ;
  } ;
}

using namespace llvm ;

char RemoveExtendsPass::ID = 0 ;

static RegisterPass<RemoveExtendsPass> X ("removeExtends", 
					"Removes extend operations.");

RemoveExtendsPass::RemoveExtendsPass() : FunctionPass((intptr_t)&ID) 
{
  ; // Nothing in here
}

RemoveExtendsPass::~RemoveExtendsPass()
{
  ; // Nothing to delete either
}


bool RemoveExtendsPass::runOnFunction(Function& f)
{
  CurrentFile::set(__FILE__);
  bool changed = false ;
  
  //see if there are any ROCCCNames or ROCCCSizes that caused the extend
  for(Function::iterator BB = f.begin(); BB != f.end(); ++BB)
  {
    begin:
    for(BasicBlock::iterator II = BB->begin(); II != BB->end(); ++II)
    {
      if( dynamic_cast<FPExtInst*>(&*II) or
          dynamic_cast<ZExtInst*>(&*II) or 
          dynamic_cast<SExtInst*>(&*II) or
          dynamic_cast<BitCastInst*>(&*II) )
      {
        INTERNAL_MESSAGE("Attempting to remove uses of " << II->getName() << "\n");
        for(Value::use_iterator UI = II->use_begin(); UI != II->use_end(); ++UI)
        {
          dynamic_cast<Instruction*>(*UI)->replaceUsesOfWith(II, II->getOperand(0));
          goto begin;
        }
        if( II->use_begin() == II->use_end() )
        {
          II->eraseFromParent();
          II = BB->begin();
        }
        else
        {
          INTERNAL_ERROR("Extend " << *II << " is still used in " << **II->use_begin() << "!");
          assert(0 and "Extend operation still exists!");
        }
      }
    }
  }

  return changed ;
}

//if the switch has no more cases, it is deleted from its parent.
//This function returns whether or not it gets deleted.
bool removeSwitchCase(SwitchInst* SI, int case_num)
{
  if( case_num == 0 and SI->getNumCases() == 1 )
  {
    SI->replaceAllUsesWith(UndefValue::get(SI->getType()));
    SI->eraseFromParent();
    return true;
  }
  else if( case_num == 0 )
  {
    SI->setSuccessor(0, SI->getSuccessor(SI->getNumCases()-1));
    SI->removeCase(SI->getNumCases()-1);
  }
  else
  {
    SI->removeCase(case_num);
  }
  return false;
}

struct DeleteEverythingPass : public FunctionPass 
{
  static char ID;
  DeleteEverythingPass() : FunctionPass((intptr_t)&ID) {}
  virtual bool runOnFunction(Function& f)
  {
    CurrentFile::set(__FILE__);
    bool changed = false;
    // Make sure this is a function that we can use
    if (f.isDeclaration() /*|| !f.isDFFunction()*/ )
    {
      return changed ;
    }
    for(Function::iterator BB = f.begin(); BB != f.end(); ++BB)
    {
      begin:
      for(BasicBlock::iterator II = BB->begin(); II != BB->end(); ++II)
      {
        if( !dynamic_cast<TerminatorInst*>(&*II) )
        {
          II->replaceAllUsesWith(UndefValue::get(II->getType()));
          II->eraseFromParent();
          goto begin;
        }
      }
    }
    changed = true;
    return changed;
  }
};
char DeleteEverythingPass::ID = 0;
RegisterPass<DeleteEverythingPass> X_DeleteEverythingPass("deleteAll", "Cleans up.");  

