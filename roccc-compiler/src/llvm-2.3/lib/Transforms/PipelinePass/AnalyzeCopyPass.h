// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*

  This file contains the declaration of the InsertCopyPass class.  This 
    pass is responsible for creating temporaries and inserting copy
    instructions in each empty slot.
 
*/

#ifndef __ANALYZE_COPY_PASS_DOT_H__
#define __ANALYZE_COPY_PASS_DOT_H__

#include "llvm/Pass.h"
#include "rocccLibrary/DFFunction.h"
#include "rocccLibrary/DFBasicBlock.h"

namespace llvm
{

  class AnalyzeCopyPass : public FunctionPass
  {
  public:
    static char ID ;
    AnalyzeCopyPass() ;
    ~AnalyzeCopyPass() ;
    virtual bool runOnFunction(Function& f) ;
  
    //connectionList contains a DFBasicBlock* that contains the definition of a value
    //   that is then used in a list of DFBasicBlocks
    typedef std::pair< DFBasicBlock*, std::list<DFBasicBlock*> > connectionList;
    //instructionPair contains the value we need to copy at a certain pipeline level,
    //   and the original value's name
    typedef std::pair< Instruction*, std::string > instructionPair;
    //getPipelineCopies gets all of the copies that need to be created.
    //it returns a map of ints, which are the pipeline levels, mapped to a map
    //of instructions that need to be copied, along with the blocks that define and 
    //use that instruction
    std::map< int, std::map<instructionPair, connectionList> >& getPipelineCopies();
  private:
    std::map< int, std::map<instructionPair, connectionList> > pipelineCopies;
  } ;
}

#endif
