#include "rocccLibrary/PipelineBlocks.h"
#include "rocccLibrary/DFFunction.h"
#include "llvm/Support/CFG.h"
#include <algorithm>

namespace llvm {

/*
getPipelineBlocks returns a map of blocks that are reachable from
the sink of the function passed in.
Internally, keep a map of BasicBlocks->bool that says whether a BasicBlock
has been analyzed yet, and, going through the nodes depth first starting at
the sink, push each node onto the process queue only if it hasnt been 
analyzed yet.
*/
std::map<DFBasicBlock*,bool> getPipelineBlocks(Function& f)
{
  if( !f.getDFFunction() ) //its not a DFFunction, so just nothing
  {
    std::map<DFBasicBlock*,bool> allBlocks;
    return allBlocks;
  }
  DFBasicBlock* sink = f.getDFFunction()->getSink() ;
  assert(sink != NULL) ;
  
  std::map<DFBasicBlock*,bool> allBlocks;
  std::list<DFBasicBlock*> toProcess;
  assert(toProcess.empty()) ;
  toProcess.push_back(sink) ;
  while(!toProcess.empty())
  {
    DFBasicBlock* nextBlock = toProcess.front() ;
    toProcess.pop_front() ;
    
    allBlocks[nextBlock] = true;

    pred_iterator goThrough = pred_begin(nextBlock) ;
    while (goThrough != pred_end(nextBlock))
    {
      if( allBlocks[(*goThrough)->getDFBasicBlock()] == false and std::find(toProcess.begin(), toProcess.end(), *goThrough) == toProcess.end() )
        toProcess.push_back( (*goThrough)->getDFBasicBlock() );
      ++goThrough;
    }
  }
  return allBlocks;
}

}
