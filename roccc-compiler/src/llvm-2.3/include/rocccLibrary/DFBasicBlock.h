//===-- llvm/DFBasicBlock.h - Class to represent a data flow node for ROCCC
// --*- C++ -*-===

#ifndef __DFBASIC_BLOCK_DOT_H__
#define __DFBASIC_BLOCK_DOT_H__

#include "llvm/BasicBlock.h"

namespace llvm
{
  // ROCCC Code  
  
  class DFBasicBlock : public BasicBlock
    {
    private:
      int dataflowLevel ;
      int pipelineLevel ;
      
      bool marked ;
      
      bool isSynch ;
            
    public:
      
      static DFBasicBlock* Create(const std::string &Name = "", 
                                  Function *Parent = 0,
                                  BasicBlock *InsertBefore = 0) 
      {
        return new DFBasicBlock(Name, Parent, InsertBefore);
      }
      
      DFBasicBlock(const std::string& Name = "", 
                   Function* Parent = 0,
                   BasicBlock* InsertBefore = 0) ;
      ~DFBasicBlock() ;
      
      static inline bool classof(const DFBasicBlock*) { return true ; }
      
      inline void mark() { marked = true ; }
      inline void unmark() {marked = false ; }
      inline bool isMarked() { return marked ; }
      
      inline void setSynchronous() { isSynch = true ; } 
      inline bool isSynchronous() { return isSynch ; }
      
      inline int getDataflowLevel() { return dataflowLevel ; }
      inline int getPipelineLevel() { return pipelineLevel ; }
      
      void setDataflowLevel(int d) { dataflowLevel = d ; }
      void setPipelineLevel(int p) { pipelineLevel = p ; } 
      
      void AddUse(DFBasicBlock* u) ; // This block is the definition
      
      void RemoveUse(DFBasicBlock* u) ; // This block used to be the definition
      
      //this function adds an instruction to the basicblock, making any connections between
      //   basicblocks that are necessary
      Instruction* addInstruction(Instruction* II);
      
      
      // Return the number of pipeline levels necessary for this block
      //  This is usually 1, but for components it is larger.
      int getDelay() ;

      bool isDFBasicBlock() { return true ; }
      DFBasicBlock* getDFBasicBlock() { return this ; }
      
    } ;
  
}

#endif
