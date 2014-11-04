#ifndef _ROCCC_LOOP_INFORMATION__CPP_
#define _ROCCC_LOOP_INFORMATION__CPP_

#include "llvm/Value.h"

#include <vector>
#include <map>

namespace llvm {

  // A helper POD class.  Used to collect loop information so the
  //  controller can be generated correctly in VHDL.
  class ROCCCLoopInformation
    {
    public:
      // This value is the loop induction variables 
      std::vector<Value*> indexes ;
      //maps a loop induction variable to a vector of loop induction variables
      // of child loops,
      //used to create the index generator
      std::map<Value*,std::vector<Value*> > loopChildren;
      // This value indicates the end of the loop.  Loops are always normalized
      //  from 0 to endValue
      //maps loop induction variables to the endValue
      std::map<Value*,Value*> endValues ;
      //maps inputBuffers to the vector of loop induction variables they access
      std::map<Value*,std::vector<Value*> > inputBufferLoopIndexes;
      std::map<Value*,std::vector<Value*> > outputBufferLoopIndexes;
      
      std::vector<Value*> inputBuffers ;
      std::vector<Value*> outputBuffers ;
      //maps a inputBuffer to its indexes;
      //  each index is a pair (destination, numeric index into the array)
      std::map<Value*,std::vector<std::pair<Value*,std::vector<int> > > > inputBufferIndexes;
      std::map<Value*,std::vector<std::pair<Value*,std::vector<int> > > > outputBufferIndexes;
      //maps an inputBuffer to the number of buffers
      //  invalidated when the buffer dumps its data;
      //  for example, smart buffers, providing a window, 
      //  usually have 1 as the invalidated value
      std::map<Value*,int> inputBufferNumInvalidated;
      std::map<Value*,int> outputBufferNumInvalidated;
    public:
      ROCCCLoopInformation() ; //defined in DFFunction.cpp
      ~ROCCCLoopInformation() ; //defined in DFFunction.cpp
      
    } ;
    
}

#endif
