// The ROCCC Compiler Infrastructure 
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*

This pass generates the component that handles the timing of all inputs.
In particular, this pass manages generating addresses for all input streams,
windows the streams so as to reuse elements as possible, generates code
for the loadPrevious, and times everything so that data is only pushed onto
the datapath when all incoming elements are ready.  
   
*/

#ifndef __OUTPUT_CONTROLLER_DOT_H__
#define __OUTPUT_CONTROLLER_DOT_H__

#include "llvm/Pass.h"

#include <fstream>

#include "rocccLibrary/DFFunction.h"
#include "rocccLibrary/VHDLInterface.h"
#include "rocccLibrary/StreamVariable.h"

namespace llvm
{

class OutputControllerPass : public FunctionPass
{
public:
  //InputStream represents an incoming stream in the vhdl.
  //It is a struct that contains all the ports that are related to that stream.
  struct OutputStream {
    //external use
    StreamVariable* stream;
    OutputStream() : stream(NULL){}
  };
  VHDLInterface::Port* stall_internal;
  VHDLInterface::Entity* outputEntity;
  std::vector<OutputStream> output_streams;
public:
  static char ID ;
  OutputControllerPass() ;
  ~OutputControllerPass() ;
  virtual bool runOnFunction(Function& F) ;
  virtual void getAnalysisUsage(AnalysisUsage& Info) const ;
};

}

#endif 


