#ifndef _PIPELINE_BLOCKS_H__
#define _PIPELINE_BLOCKS_H__

#include "rocccLibrary/DFBasicBlock.h"
#include "llvm/Function.h"
#include <map>

namespace llvm {

std::map<DFBasicBlock*, bool> getPipelineBlocks(Function& f);

}

#endif
