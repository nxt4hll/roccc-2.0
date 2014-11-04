/*
  This file defines the pass that numbers each instructions dataflow level
   and pipeline level.  These are two different numbers, but are dependant
   on one another.

*/


#include "PipelinePass.h"
#include "llvm/Support/CFG.h"
#include "llvm/Instructions.h"

#include <algorithm>
#include <sstream>
#include <fstream>

#include "rocccLibrary/DFFunction.h"
#include "rocccLibrary/DFBasicBlock.h"
#include "rocccLibrary/InternalWarning.h"
#include "rocccLibrary/ROCCCNames.h"

using namespace std;
using namespace llvm ;

// The one ID used in all Pipeline Pass instances
char PipelinePass::ID = 0 ;

// The call to the constructor that registers this pass
static RegisterPass<PipelinePass> X("pipeline", "Pipeline Pass") ;

PipelinePass::PipelinePass() : FunctionPass((intptr_t)&ID)
{
}

PipelinePass::~PipelinePass() 
{
  ; // Nothing to clean up yet
}

void NumberDFStages(DFBasicBlock* node, int currentLevel, bool isSink=true)
{  
  if (node->getDataflowLevel() >= currentLevel)
  {
    // We've already marked this level with a further pipeline stage
    return ;
  }
  node->setDataflowLevel(currentLevel);
  node->setPipelineLevel(currentLevel);
  
  // The delay could be larger if we are loading a module
  int incLevel = node->getDelay() ;
  
  // Go to all of the predecessors and traverse the graph.
  pred_iterator goThrough = pred_begin(node) ;
  while(goThrough != pred_end(node))
  {
    DFBasicBlock* nextPred = (*goThrough)->getDFBasicBlock() ;
    assert(nextPred != NULL and "Error in internal graph!") ;
    // The sink actually shouldn't have a dataflow level, nor should
    //  any of the functions for identifying inputs or outputs.
    if (isSink)
    {
      NumberDFStages(nextPred, currentLevel, false) ;
    }
    else
    {
      NumberDFStages(nextPred, currentLevel + incLevel, false) ;
    }
    ++goThrough ;
  }
}

// This is the entry point to our pass and where all of our work gets done
bool PipelinePass::runOnFunction(Function& f)
{
  CurrentFile::set(__FILE__);

  if (f.isDeclaration() || f.getDFFunction() == NULL)
  {
    return false ; // No modification and nothing to do.
  }

  DFFunction* dataFlowFunc = f.getDFFunction() ;
  assert(dataFlowFunc != NULL) ;

  DFBasicBlock* sink = dataFlowFunc->getSink() ;
  assert(sink != NULL) ;

  // Recursively start numbering nodes from the bottom up.
  //  Start at -1 because the first node is empty and not in the pipe.

  NumberDFStages(sink, 0) ;

  // Set the delay of the whole function based upon the pipeline stage
  //  of the source node.
  DFBasicBlock* source = dataFlowFunc->getSource() ;
  assert(source != NULL) ;
  // One less than the pipeline level of the source because the
  //  source is actually an empty node
  dataFlowFunc->setDelay(source->getPipelineLevel() - 1) ;
  
  //go through and make sure that all inputs are set to be one level below the source,
  //except for the block that contains the index variables (phi nodes)
  for(succ_iterator BB = succ_begin(source); BB != succ_end(source); ++BB)
  {
    DFBasicBlock* dfbb = BB->getDFBasicBlock();
    assert(dfbb);
    if(!dynamic_cast<PHINode*>(&*dfbb->begin()))
    {
      dfbb->setPipelineLevel( dataFlowFunc->getDelay() );
    }
  }

  // Technically, I'm changing the representation, but only
  //  in ways that ROCCC will recognize.  I'll return true to be 
  //  consistant with the way LLVM expects it. 
  return true ; 
}

//@TODO: Just in case we ever need this, save it
//this function takes two DFBasicBlocks and merges them,
//and returns the resulting combined node
DFBasicBlock* mergeNodes( DFBasicBlock* n1, DFBasicBlock* n2 )
{
  assert( n1 );
  assert( n2 );
  assert( n1 != n2 );
  assert( n1->getParent() == n2->getParent() );
  DFFunction* parent = dynamic_cast<DFFunction*>( n1->getParent() );
  assert( parent );
  std::string combName = n1->getName() + "+" + n2->getName();
  DFBasicBlock* combinedNode = DFBasicBlock::Create(combName, parent) ;
  assert( combinedNode );
  combinedNode->setSynchronous() ;
  //set possible sink and source info
  if ( parent->getSource() == n1 or parent->getSource() == n2 )
    parent->setSource( combinedNode );
  if ( parent->getSink() == n1 or parent->getSink() == n2 )
    parent->setSink( combinedNode );

  //replace n1 and n2 with combinedNode
  n1->replaceAllUsesWith( combinedNode );
  n2->replaceAllUsesWith( combinedNode );
  //set all the predecessors of n1 as preds of combinedNode
  for (pred_iterator pred = pred_begin(n1); pred != pred_end(n1); ++pred)
  {  
    DFBasicBlock* h = dynamic_cast<DFBasicBlock*> (*pred);
    if ( h && h != n1 && h != n2 && h != combinedNode )
      h->AddUse( combinedNode );
  }
  //set all the predecessors of n2 as preds of combinedNode
  for (pred_iterator pred = pred_begin(n2); pred != pred_end(n2); ++pred)
  {  
    DFBasicBlock* h = dynamic_cast<DFBasicBlock*> (*pred);
    if ( h && h != n1 && h != n2 && h != combinedNode )
      h->AddUse( combinedNode );
  }
  //set all the successors of n1 as successors of combinedNode
  for (succ_iterator succ = succ_begin(n1); succ != succ_end(n1); ++succ)
  {  
    DFBasicBlock* h = dynamic_cast<DFBasicBlock*> (*succ);
    if ( h && h != n1 && h != n2 && h != combinedNode )
      combinedNode->AddUse( h );
  }
  //set all the successors of n2 as successors of combinedNode
  for (succ_iterator succ = succ_begin(n2); succ != succ_end(n2); ++succ)
  {  
    DFBasicBlock* h = dynamic_cast<DFBasicBlock*> (*succ);
    if ( h && h != n1 && h != n2 && h != combinedNode )
      combinedNode->AddUse( h );
  }
  //move all of the instructions in n2 over to the new node
  for ( BasicBlock::iterator II = n2->begin(); II != n2->end(); )
  {
    if ( !II->isTerminator() )
    {
      Instruction* hold = &*II;
      ++II;
      assert( combinedNode->getTerminator() );
      hold->moveBefore( combinedNode->getTerminator() );
    }
    else
      ++II;
  }
  //move all of the instructions in n1 over to the new node
  for ( BasicBlock::iterator II = n1->begin(); II != n1->end(); )
  {
    if ( !II->isTerminator() )
    {
      Instruction* hold = &*II;
      ++II;
      assert( combinedNode->getTerminator() );
      hold->moveBefore( combinedNode->getTerminator() );
    }
    else
      ++II;
  }
  //if there are any PHIs that were successors of both n1 and n2, we need to remove any duplicate definitions
  for ( succ_iterator succ = succ_begin( combinedNode ); succ != succ_end( combinedNode ); ++succ )
  {
    for (BasicBlock::iterator II = succ->begin(); II != succ->end() && &*II != succ->getFirstNonPHI(); ++II)
    {
      PHINode* PN = dynamic_cast<PHINode*>( &*II );
      assert( PN );
      bool found = false;
      for ( unsigned int inc = 0; inc < PN->getNumIncomingValues(); ++inc )
      {
        if ( PN->getIncomingBlock(inc) == combinedNode  )
        {
          if ( found )
          {
            assert( inc >= 3 && "Removing critical value (true, false, or condition value)!" );
            PN->removeIncomingValue(inc);
          }
          found = true;
        }
      }
    }
  }
  int minLevel = (n1->getDataflowLevel() < n2->getDataflowLevel()) ? n1->getDataflowLevel() : n2->getDataflowLevel();
  int maxLevel = (n1->getDataflowLevel() > n2->getDataflowLevel()) ? n1->getDataflowLevel() : n2->getDataflowLevel();
  //assert( minLevel + 1 == maxLevel );
  //delete n1 and n2
  n1->eraseFromParent();
  n2->eraseFromParent();

  combinedNode->setDataflowLevel( maxLevel  );
  //set the level of the combinedNode as the minimum of the predecessors of combinedNode, minus 1
  for ( pred_iterator pred = pred_begin( combinedNode ); pred != pred_end( combinedNode ); ++pred )
  {
    DFBasicBlock* predCast = dynamic_cast<DFBasicBlock*>( *pred );
    assert( predCast );
    int impLevel = predCast->getDataflowLevel() - 1;
    assert( impLevel >= minLevel );
    if ( impLevel < combinedNode->getDataflowLevel() )
      combinedNode->setDataflowLevel( impLevel );
  }
  //return the created node
  return combinedNode;
}

