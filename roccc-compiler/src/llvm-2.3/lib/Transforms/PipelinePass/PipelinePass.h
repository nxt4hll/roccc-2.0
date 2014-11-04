// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*  --*- C++ -*--

  This pass is responsible for numbering both the dataflow level of each node
    as well as the appropriate pipeline level.  The dataflow level is done
    first as a simple breadth first traversal and the pipeline level is
    interpolated off of that.  Each type of instruction has an 
    associated cost.  If the addition of the cost is less than 
    the threshold cost, the instructions are merged into the same pipeline
    state.  This is done before any copy instructions are created.

*/


#ifndef __VHDL_OUTPUT_PASS_DOT_H__
#define __VHDL_OUTPUT_PASS_DOT_H__

#include "llvm/Pass.h"
#include "rocccLibrary/DFFunction.h"

namespace llvm 
{

  class PipelinePass : public FunctionPass
  {
  public:
    // Used for registration in llvm's pass manager
    static char ID ;
    PipelinePass() ;
    ~PipelinePass() ;
  
    virtual bool runOnFunction(Function& F) ;
  } ;
}

#endif 
