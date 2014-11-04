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
#include "llvm/Support/CFG.h"

#include "rocccLibrary/InternalWarning.h"
#include "rocccLibrary/ROCCCNames.h"
#include "rocccLibrary/DFFunction.h"
#include "rocccLibrary/CopyValue.h"
#include "rocccLibrary/DefinitionInst.h"
#include "rocccLibrary/GetValueName.h"
#include "rocccLibrary/DatabaseInterface.h"

#include <fstream>
#include <sstream>

using namespace llvm ;

bool tryCombineBlocks(Function& f)
{
  bool changed = false;
  for(Function::iterator BB = f.begin(); BB != f.end(); ++BB)
  {
    if( !BB->getDFBasicBlock() )
    {
      continue;
    }
    if( &*BB == f.getDFFunction()->getSink() )
    {
      continue;
    }
    std::map<DFBasicBlock*,bool> canBeCombined;
    for(pred_iterator PBI = pred_begin(BB); PBI != pred_end(BB); ++PBI)
    {
      if( (*PBI)->getDFBasicBlock() )
      {
        canBeCombined[(*PBI)->getDFBasicBlock()] = ((*PBI)->getDFBasicBlock()->getPipelineLevel() == BB->getDFBasicBlock()->getPipelineLevel());
        if( *PBI == f.getDFFunction()->getSource() )
          canBeCombined[(*PBI)->getDFBasicBlock()] = false;
      }
    }
    for(std::map<DFBasicBlock*,bool>::iterator PBI = canBeCombined.begin(); PBI != canBeCombined.end(); ++PBI)
    {
      if( PBI->second )
      {
        DFBasicBlock* dfPBI = PBI->first;
        if( !dfPBI )
          continue;
        //move the instructions in that block to here
        std::vector<Instruction*> instList;
        for(BasicBlock::iterator II = dfPBI->begin(); II != dfPBI->end(); ++II)
        {
          if( !II->isTerminator() )
            instList.push_back( II );
        }
        for(std::vector<Instruction*>::iterator II = instList.begin(); II != instList.end(); ++II)
        {
          (*II)->removeFromParent();
          BB->getDFBasicBlock()->addInstruction(*II);
          changed = true;
        }
        dfPBI->getTerminator()->eraseFromParent();
        while( dfPBI->getNumUses() != 0 )
        {
          Instruction* useInst = dynamic_cast<Instruction*>(*dfPBI->use_begin());
          assert(useInst);
          if( useInst->getParent() and useInst->getParent()->getDFBasicBlock() )
          {
            useInst->getParent()->getDFBasicBlock()->RemoveUse(dfPBI);
          }
          else
          {
            assert(0);
          }
        }
        assert( dfPBI->getNumUses() == 0 );
        dfPBI->eraseFromParent();
      }
    }
    if( changed )
      return changed;
  }
  return changed;
}

struct PipelineStageCombinePass : public FunctionPass 
{
  static char ID;
  PipelineStageCombinePass() : FunctionPass((intptr_t)&ID) {}
  virtual bool runOnFunction(Function& f)
  {
    CurrentFile::set(__FILE__);
    bool changed = false;
    // Make sure this is a function that we can use
    if (f.isDeclaration() || !f.isDFFunction() )
    {
      return changed ;
    }
    while(tryCombineBlocks(f))
    {
      changed = true;
    }
    
    return changed;
  }
};
char PipelineStageCombinePass::ID = 0;
RegisterPass<PipelineStageCombinePass> X_PipelineStageCombinePass("combinePipelineStages", "Combine pipeline stages.");

