//===-- llvm/DFFunction.h - Class to represent a ROCCC function in Data flow format --*- C++ -*-===

// This file contains the declaration of the DFFunction class, which
//  represents a function for hardware represented in the pure data
//  flow format.  This class is derived from the base Function class so
//  should be compatable with all of the infrastructure that currently 
//  exists.

#ifndef __DFFUNCTION_DOT_H__
#define __DFFUNCTION_DOT_H__

#include "llvm/Function.h"
#include "DFBasicBlock.h"
#include "ROCCCLoopInformation.h"

namespace llvm
{
  class DFFunction : public Function {
    private:
      // Used to output loop controllers in the VHDL
      ROCCCLoopInformation rocccLoopInfo ;
      // This is used to determine if the function is a module or 
      //  system code.  I'm not sure yet in which pass this should be done, 
      //  but the earlier the better.
      int functionType ;
      // This is the first node that all forward passes start at.
      DFBasicBlock* source ;
      // This is the last node in the data flow graph and the start
      //  of most backwards passes.
      DFBasicBlock* sink ;
      // Determined during the pipeline phase, this is the total number of
      //  clock cycles required to get data through the component.  This
      //  number can be exported to the VHDL library and other system code.
      int delay ;
      // Hmmm..  It appears that there is no default Function constructor, 
      //  so I won't be creating a default DFFunction constructor.
      DFFunction(const FunctionType* Ty, LinkageTypes Linkage,
                 const std::string& N = "", Module* M = 0) ;
    public:
      ~DFFunction();
      //these two functions are needed because dynamic_casts were not
      //working across module boundaries
      bool isDFFunction();
      DFFunction* getDFFunction();
      static DFFunction* Create(const FunctionType* Ty,
                                LinkageTypes Linkage,
                                const std::string& N,
                                Module* M);
      
      DFBasicBlock* getSink() ;
      DFBasicBlock* getSource() ;
      void setSource(DFBasicBlock* s);
      void setSink(DFBasicBlock* s);
      
      int getDelay();
      void setDelay(int d);
      
      int getFunctionType();
      void setFunctionType(int t);
      
      const ROCCCLoopInformation getROCCCLoopInfo();
      void setROCCCLoopInfo(ROCCCLoopInformation _r);
      
      // The two virtual functions in the base class don't need 
      //  any new definitions at this level.
    } ;
}

#endif
