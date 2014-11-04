// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

#include "InsertCopyPass.h"

#include <algorithm>

#include "rocccLibrary/InternalWarning.h"
#include "rocccLibrary/MessageLogger.h"
#include "rocccLibrary/ROCCCNames.h"
#include "rocccLibrary/CopyValue.h"
#include "rocccLibrary/SizeInBits.h"

#include "AnalyzeCopyPass.h"

using namespace llvm ;

char InsertCopyPass::ID = 0 ;

static RegisterPass<InsertCopyPass> X("insertCopy",
				      "Insert copies to fill the pipeline") ;

InsertCopyPass::InsertCopyPass() : FunctionPass((intptr_t)&ID) 
{
}

InsertCopyPass::~InsertCopyPass()
{

}

bool InsertCopyPass::runOnFunction(Function& f)
{
  CurrentFile::set(__FILE__);
  bool changed = false;
  if (f.isDeclaration() || f.getDFFunction() == NULL)
  {
    return changed;
  }
  int count = 0;
  int bit_count = 0;
  std::map< int, std::map<AnalyzeCopyPass::instructionPair, AnalyzeCopyPass::connectionList> > pipelineCopies = getAnalysis<AnalyzeCopyPass>().getPipelineCopies();
  //we make a copy, so that we have where the original values were going to
  std::map< int, std::map<AnalyzeCopyPass::instructionPair, AnalyzeCopyPass::connectionList> > pipelineCopiesOg = pipelineCopies;
  
  //we want to go through the pipeline levels in order from greatest to smallest,
  //since we will start making copies at the top, and replace any uses with the
  //news copy, including in lower pipeline copies
  //so make a copy of all of the pipeline levels and sort them
  std::vector<int> pipelineCopyLevels;
  for(std::map< int, std::map<AnalyzeCopyPass::instructionPair, AnalyzeCopyPass::connectionList> >::iterator CI = pipelineCopies.begin(); CI != pipelineCopies.end(); ++CI)
  {
    pipelineCopyLevels.push_back( CI->first );
  }
  std::sort( pipelineCopyLevels.begin(), pipelineCopyLevels.end() );
  //we need a map of Instructions to the current Instruction that represent them as copies
  std::map<Instruction*,Instruction*> currentCopyMap;
  //then go through the sorted vector in reverse order, ie greatest to smallest
  for(std::vector<int>::reverse_iterator level = pipelineCopyLevels.rbegin(); level != pipelineCopyLevels.rend(); ++level)
  {
    //for each pipeline level, we are creating one block that contains
    //all of the copies for that level
    DFBasicBlock* copyBlock = DFBasicBlock::Create("compactedCopy", &f);
    copyBlock->setPipelineLevel(*level);
    copyBlock->setDataflowLevel(*level);
    //now, create a copy for each value in the map for that level
    for(std::map<AnalyzeCopyPass::instructionPair, AnalyzeCopyPass::connectionList>::iterator II = pipelineCopies[*level].begin(); II != pipelineCopies[*level].end(); ++II)
    {
      //the first block in the connectionList is the definition of that value
      //connect the definition block to the newly created copy block that uses
      //that definition block
      II->second.first->AddUse( copyBlock );
      //create a copy
      Instruction* copyVal = currentCopyMap[II->first.first];
      if( !copyVal )
        copyVal = II->first.first;
      Instruction* nextCopy = dynamic_cast<Instruction*>(getCopyOfValue(copyVal));
      if( !nextCopy )
        continue;
      nextCopy->setName( II->first.second );
      //add the newly created copy to the copy block
      copyBlock->getInstList().push_front(nextCopy) ;
      ++count;
      bit_count += getSizeInBits(nextCopy);
      //and update the map of instructions to current copies of those instructions
      currentCopyMap[II->first.first] = nextCopy;
      //for each successor block, we need to replace the uses of the old
      //value with the newly created value
      for(std::list<DFBasicBlock*>::iterator succ = II->second.second.begin(); succ != II->second.second.end(); ++succ)
      {
        DFBasicBlock* df_succ = *succ;
        //only replace the uses that are one immediate pipeline level below us
        //if( df_succ->getPipelineLevel() + df_succ->getDelay() == *level )
        //{
          copyBlock->AddUse(df_succ);
          for(BasicBlock::iterator inst = df_succ->begin(); inst != df_succ->end(); ++inst)
            inst->replaceUsesOfWith(II->first.first, nextCopy);
          II->second.first->RemoveUse( df_succ );
          //remove what the original connection was going to
          pipelineCopiesOg[*level][II->first].first->RemoveUse( df_succ );
        //}
      }
      //replace the links to the basicblock that houses the values
      std::vector<int>::reverse_iterator level2 = level+1;
      if(level2 != pipelineCopyLevels.rend() and
         pipelineCopies[*level2].find(II->first) != pipelineCopies[*level2].end())
      {
        pipelineCopies[*level2][II->first].first = copyBlock;
      }
    }
  }
  LOG_MESSAGE2("Pipelining", "Copy Insertion", 
               "In order to place operations in the correct pipeline stage, copies have been inserted to copy values from one pipeline stage to the next. "
               << "Created " << count << " total copies, with " << bit_count << " total bits copied.\n");
  return changed;
}

void InsertCopyPass::getAnalysisUsage(AnalysisUsage &AU) const
{
  AU.addRequired<AnalyzeCopyPass>();
  AU.addPreserved<AnalyzeCopyPass>();
}
