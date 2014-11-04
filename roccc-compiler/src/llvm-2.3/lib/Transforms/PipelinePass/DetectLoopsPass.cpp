// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*

This pass is responsible for detecting loops in the ROCCC constructed
DFG.  

*/

#include "llvm/Pass.h"
#include "rocccLibrary/DFFunction.h"
#include "llvm/Support/CFG.h"

#include <map>
#include <algorithm>

#include "rocccLibrary/InternalWarning.h"
#include "rocccLibrary/MessageLogger.h"
#include "rocccLibrary/PipelineBlocks.h"

namespace llvm
{
  class DetectLoopsPass : public FunctionPass
  {
  private:
  public:
    static char ID ;
    DetectLoopsPass() ;
    ~DetectLoopsPass() ;
    virtual bool runOnFunction(Function& b) ;
  } ;
}

using namespace llvm ;

char DetectLoopsPass::ID = 0 ;

static RegisterPass<DetectLoopsPass> X ("detectLoops", 
					"Sanity check to make sure there are no loops in the dataflow graph");

DetectLoopsPass::DetectLoopsPass() : FunctionPass((intptr_t)&ID) 
{
  ; // Nothing in here
}

DetectLoopsPass::~DetectLoopsPass()
{
  ; // Nothing to delete either
}

/*
To detect loops, given a sink node:
-Calculate a map (for efficient lookups) holding all nodes reachable by the sink
-Number each node with the number of successors of that node reachable by the sink
-Going through breadth first, starting at the sink:
  -decrement the successor counter for each predecessor
  -any predecessors that got decremented to 0 means we are the last node to
     access that node
  -because we are accessing depth first, the last node to access a node will
     be the highest up in terms of depth
  -number the node with our depth + 1, and push it onto the back of the queue to be
     analyzed
-Go through each node, checking that its depth is strictly less than the depth
  of any predecessors
*/
void detectLoops(DFBasicBlock* sink)
{
  std::map<DFBasicBlock*, bool> allBlocks = getPipelineBlocks(*sink->getParent());
  std::list<DFBasicBlock*> toProcess;
  assert(toProcess.empty()) ;
  std::map<BasicBlock*, int> depth, succCount;
  for(std::map<DFBasicBlock*, bool>::iterator BB = allBlocks.begin(); BB != allBlocks.end(); ++BB)
  {
    succCount[BB->first] = 0;
    for(succ_iterator succ = succ_begin(BB->first); succ != succ_end(BB->first); ++succ)
    {
      if( allBlocks[(*succ)->getDFBasicBlock()] )
        ++succCount[BB->first];
    }
  }
  toProcess.clear();
  toProcess.push_back(sink);
  depth[sink] = 1;
  while(!toProcess.empty())
  {
    BasicBlock* nextBlock = toProcess.front() ;
    toProcess.pop_front() ;
    for(pred_iterator goThrough = pred_begin(nextBlock); goThrough != pred_end(nextBlock); ++goThrough)
    {
      --succCount[*goThrough];
      if( succCount[*goThrough] == 0 )
      {
        toProcess.push_back( (*goThrough)->getDFBasicBlock() );
        depth[*goThrough] = depth[nextBlock] + 1;
      }
    }
  }
  for(std::map<DFBasicBlock*, bool>::iterator BB = allBlocks.begin(); BB != allBlocks.end(); ++BB)
  {
    if( !BB->second )
      continue;
    for(pred_iterator pred = pred_begin(BB->first); pred != pred_end(BB->first); ++pred)
    {
      if( depth[*pred] <= depth[BB->first] )
      {
        INTERNAL_WARNING("Depth of " << (*pred)->getName() << "(" << depth[*pred] << ") <= depth of " << BB->first->getName() << "(" << depth[BB->first] << ")!\n");
        INTERNAL_MESSAGE(*(*pred) << "\n" << *BB->first << "\n");
      }
      assert( depth[*pred] > depth[BB->first] and "Loop detected!" );
    }
  }
  int numBlocks = 0;
  for(std::map<DFBasicBlock*, bool>::iterator MPI = allBlocks.begin(); MPI != allBlocks.end(); ++MPI)
  {
    ++numBlocks;
  }
  LOG_MESSAGE2("Pipelining", "Detect Loops", "No loops found. Total number of operations in datapath: " << numBlocks << ".\n");
}


bool DetectLoopsPass::runOnFunction(Function& f)
{
  CurrentFile::set(__FILE__);
  bool changed = false ;
  if (f.isDeclaration() || f.getDFFunction() == NULL)
  {
    return changed ;
  }
  //LOG_MESSAGE2("Pipelining", "Detect Loops", "Starting detect loops pass.\n");
  DFBasicBlock* sink = f.getDFFunction()->getSink() ;
  assert(sink != NULL) ;
  
  detectLoops(sink);

  return changed ;
}

