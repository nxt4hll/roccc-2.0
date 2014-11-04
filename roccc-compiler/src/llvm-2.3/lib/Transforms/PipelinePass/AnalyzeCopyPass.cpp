// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*

  This file contains the definiton of the AnalyzeCopyPass class.  This 
    pass is responsible for creating temporaries and inserting copy
    instructions in each empty slot.

*/

#include "AnalyzeCopyPass.h"

#include "llvm/Support/CFG.h"
#include "llvm/Constants.h"

#include <algorithm>

#include "rocccLibrary/InternalWarning.h"
#include "rocccLibrary/ROCCCNames.h"
#include "rocccLibrary/DefinitionInst.h"

#include "AnalyzeCopyPass.h"
#include "rocccLibrary/PipelineBlocks.h"

namespace llvm {

char AnalyzeCopyPass::ID = 0 ;

static RegisterPass<AnalyzeCopyPass> X("analyzeCopy",
				      "Find copies necessary to correctly pipeline.") ;

AnalyzeCopyPass::AnalyzeCopyPass() : FunctionPass((intptr_t)&ID) 
{
}

AnalyzeCopyPass::~AnalyzeCopyPass()
{

}

/*
getPipelineCopies finds all copies that should be made at each pipeline level.
A copy should be made if a value A is used by some instruction B, but the pipeline
level of B is more than 1 less than the pipeline level of the instruction that
acts as a definition of A. For every pipeline level between the two instructions,
a pipeline copy needs to be made at that pipeline level.
getPipelineCopies returns a map from pipeline level -> connection information, 
where the connection information is a map from (A, A->getName()) -> (BasicBlock
that defines A, list of Blocks that need a copy at that pipeline level).
*/
std::map< int, std::map<AnalyzeCopyPass::instructionPair, AnalyzeCopyPass::connectionList> > getPipelineCopies( std::map<DFBasicBlock*, bool> pipelineBlocks )
{
  std::map< int, std::map<AnalyzeCopyPass::instructionPair, AnalyzeCopyPass::connectionList> > ret;
  //go through each block in the pipeline, checking to see if any of the values
  //it uses need copies made
  for(std::map<DFBasicBlock*, bool>::iterator PBI = pipelineBlocks.begin(); PBI != pipelineBlocks.end(); ++PBI)
  {
    DFBasicBlock* currBlock = PBI->first;
    Instruction* currInst = currBlock->getFirstNonPHI();
    //the pipeline level of where the current block actually starts is
    //its base pipeline level + its delay
    int currBlock_top_level = currBlock->getPipelineLevel() + currBlock->getDelay();
    //go through the list of values this block uses; check each one to see if
    //copies need to be made
    for(User::op_iterator op = currInst->op_begin(); op != currInst->op_end(); ++op)
    {
      Instruction* inst = dynamic_cast<Instruction*>(&**op);
      if ( !inst )
        continue;
      Instruction* possibleDef = getDefinitionInstruction(inst, currBlock);
      DFBasicBlock* nextBlock = possibleDef->getParent()->getDFBasicBlock();
      //we need to create a copy for every level between our top and the 
      //pipeline level of the block that defines that value
      for( int level = currBlock_top_level; level < nextBlock->getPipelineLevel(); ++level )
      {
        Instruction* i = dynamic_cast<Instruction*>(op->get());
        ret[level][AnalyzeCopyPass::instructionPair(i,i->getName())].first = nextBlock;
        if( level == currBlock_top_level )
        {
          ret[level][AnalyzeCopyPass::instructionPair(i,i->getName())].second.push_back( currBlock );
        }
      }            
    }
  }
  return ret;
}


std::map< int, std::map<AnalyzeCopyPass::instructionPair, AnalyzeCopyPass::connectionList> >& AnalyzeCopyPass::getPipelineCopies()
{
  return pipelineCopies;
}

bool AnalyzeCopyPass::runOnFunction(Function& f)
{
  CurrentFile::set(__FILE__);
  bool changed = false;
  if (f.isDeclaration() || f.getDFFunction() == NULL)
  {
    return changed;
  }
  pipelineCopies = llvm::getPipelineCopies(getPipelineBlocks(f));
  return changed;
}

}
