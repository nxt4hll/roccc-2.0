// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*  --*- C++ -*--

  File: VHDLOutputPass.h

  This contains the declarations of the VHDLOutputPass class as well as 
   the helper classes of LibraryEntry and Port for definitions of 
   all the things located in the library.

*/


#ifndef __VHDL_OUTPUT_PASS_DOT_H__
#define __VHDL_OUTPUT_PASS_DOT_H__

#include "llvm/Pass.h"
#include "rocccLibrary/DFFunction.h"
#include "rocccLibrary/VHDLInterface.h"
#include "rocccLibrary/StreamVariable.h"

namespace llvm 
{
  // Our pass should work on a function by function basis.  The function
  //  will consist of many data flow nodes, but they all should fit
  //  within the same common framework as vanilla llvm.
  //
  // Also, since ROCCC hi-cirrf only supports one function definition per
  //  translation unit we don't have to worry about the random ordering
  //  a function pass provides.
  class VHDLOutputPass : public FunctionPass
  {
    std::vector<StreamVariable> streams;
  public:
    VHDLInterface::Entity* entity;
    std::vector<StreamVariable>::iterator streamBegin();
    std::vector<StreamVariable>::iterator streamEnd();
  private:

    //the vhdl interface
    void setupVHDLInterface(DFFunction* f);
    void finalizeVHDLInterface(DFFunction* f);
    VHDLInterface::ComponentDefinition* loopComponent;
    VHDLInterface::ComponentDefinition* inputComponent;
    VHDLInterface::ComponentDefinition* outputComponent;

    // These are the main entry points for outputting all VHDL
    void OutputBlock(DFFunction* f) ;
    void OutputModule(DFFunction* f) ;

  public:
    // Used for registration in llvm's pass manager
    static char ID ;
    VHDLOutputPass() ;
    ~VHDLOutputPass() ;
  
    virtual bool runOnFunction(Function& F) ;
    virtual void getAnalysisUsage(AnalysisUsage &AU) const;
  };
}
#endif 
