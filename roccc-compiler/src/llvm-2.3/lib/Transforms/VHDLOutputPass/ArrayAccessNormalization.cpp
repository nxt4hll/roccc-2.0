// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

#include "llvm/Pass.h"

#include "rocccLibrary/DFFunction.h"
#include "rocccLibrary/InternalWarning.h"

using namespace llvm ;

class ArrayAccessNormalizationPass : public FunctionPass
{
private:
public:
  static char ID ;
  ArrayAccessNormalizationPass() ;
  ~ArrayAccessNormalizationPass() ;
  virtual bool runOnFunction(Function& b) ;
  void getAnalysisUsage(AnalysisUsage& AU) const;
} ;

char ArrayAccessNormalizationPass::ID = 0 ;

static RegisterPass<ArrayAccessNormalizationPass> X ("arrayNorm", 
					"Normalizes array accesses.");

ArrayAccessNormalizationPass::ArrayAccessNormalizationPass() : FunctionPass((intptr_t)&ID) 
{
  ; // Nothing in here
}

ArrayAccessNormalizationPass::~ArrayAccessNormalizationPass()
{
  ; // Nothing to delete either
}


bool ArrayAccessNormalizationPass::runOnFunction(Function& f)
{
  CurrentFile::set(__FILE__);
  bool changed = false ;
  // Make sure this is a function that we can use
  if (f.isDeclaration() || !f.isDFFunction() )
  {
    return changed ;
  }
  DFFunction* df = f.getDFFunction() ;

  //load the information pertaining to loops in this function
  ROCCCLoopInformation allInfo = df->getROCCCLoopInfo();

  //go through the array accesses; if we find a negative array access,
  //  find all the other uses of that index variable in arrays and increment
  //  by the smallest negative value we can find.
  for( std::vector<Value*>::iterator inputBuffer = allInfo.inputBuffers.begin(); inputBuffer != allInfo.inputBuffers.end(); ++inputBuffer )
  {
    std::map<Value*,int> smallestNegative;
    //maps a inputBuffer to its indexes;
    //  each index is a pair (destination, numeric index into the array)
    for( std::vector<std::pair<Value*,std::vector<int> > >::iterator inputBufferIndexes = allInfo.inputBufferIndexes[*inputBuffer].begin(); inputBufferIndexes != allInfo.inputBufferIndexes[*inputBuffer].end(); ++inputBufferIndexes )
    {
      //go through the numeric indexes and check if any are negative
      std::vector<Value*>::iterator inputBufferLoopIndex = allInfo.inputBufferLoopIndexes[*inputBuffer].begin();
      for(std::vector<int>::iterator n = inputBufferIndexes->second.begin(); n != inputBufferIndexes->second.end() and inputBufferLoopIndex != allInfo.inputBufferLoopIndexes[*inputBuffer].end(); ++n, ++inputBufferLoopIndex)
      {
        if( *n < 0 and *n < smallestNegative[*inputBufferLoopIndex])
        {
          smallestNegative[*inputBufferLoopIndex] = *n;
        }
      }
    }
    for( std::vector<std::pair<Value*,std::vector<int> > >::iterator inputBufferIndexes = allInfo.inputBufferIndexes[*inputBuffer].begin(); inputBufferIndexes != allInfo.inputBufferIndexes[*inputBuffer].end(); ++inputBufferIndexes )
    {
      std::vector<Value*>::iterator inputBufferLoopIndex = allInfo.inputBufferLoopIndexes[*inputBuffer].begin();
      for(std::vector<int>::iterator n = inputBufferIndexes->second.begin(); n != inputBufferIndexes->second.end() and inputBufferLoopIndex != allInfo.inputBufferLoopIndexes[*inputBuffer].end(); ++n, ++inputBufferLoopIndex)
      {
        *n += -smallestNegative[*inputBufferLoopIndex];
      }
    }
  }
  //do the same thing for the outputs
  for( std::vector<Value*>::iterator outputBuffer = allInfo.outputBuffers.begin(); outputBuffer != allInfo.outputBuffers.end(); ++outputBuffer )
  {
    std::map<Value*,int> smallestNegative;
    //maps a outputBuffer to its indexes;
    //  each index is a pair (destination, numeric index into the array)
    for( std::vector<std::pair<Value*,std::vector<int> > >::iterator outputBufferIndexes = allInfo.outputBufferIndexes[*outputBuffer].begin(); outputBufferIndexes != allInfo.outputBufferIndexes[*outputBuffer].end(); ++outputBufferIndexes )
    {
      //go through the numeric indexes and check if any are negative
      std::vector<Value*>::iterator outputBufferLoopIndex = allInfo.outputBufferLoopIndexes[*outputBuffer].begin();
      for(std::vector<int>::iterator n = outputBufferIndexes->second.begin(); n != outputBufferIndexes->second.end() and outputBufferLoopIndex != allInfo.outputBufferLoopIndexes[*outputBuffer].end(); ++n, ++outputBufferLoopIndex)
      {
        if( *n < 0 and *n < smallestNegative[*outputBufferLoopIndex])
        {
          smallestNegative[*outputBufferLoopIndex] = *n;
        }
      }
    }
    for( std::vector<std::pair<Value*,std::vector<int> > >::iterator outputBufferIndexes = allInfo.outputBufferIndexes[*outputBuffer].begin(); outputBufferIndexes != allInfo.outputBufferIndexes[*outputBuffer].end(); ++outputBufferIndexes )
    {
      std::vector<Value*>::iterator outputBufferLoopIndex = allInfo.outputBufferLoopIndexes[*outputBuffer].begin();
      for(std::vector<int>::iterator n = outputBufferIndexes->second.begin(); n != outputBufferIndexes->second.end() and outputBufferLoopIndex != allInfo.outputBufferLoopIndexes[*outputBuffer].end(); ++n, ++outputBufferLoopIndex)
      {
        *n += -smallestNegative[*outputBufferLoopIndex];
      }
    }
  }
  //we have made changes to the loopInformation, so save it
  df->setROCCCLoopInfo( allInfo );

  return changed ;
}

void ArrayAccessNormalizationPass::getAnalysisUsage(AnalysisUsage& AU) const
{
}
