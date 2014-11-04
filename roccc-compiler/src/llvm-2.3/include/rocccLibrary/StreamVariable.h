#ifndef __STREAM_VARIABLE_DOT_H__
#define __STREAM_VARIABLE_DOT_H__

#include <string>
#include <vector>
#include <utility>
#include "rocccLibrary/VHDLInterface.h"
#include "llvm/Value.h"

namespace llvm {  

struct StreamVariable {
  std::string readableName;
  llvm::Value* value;
  VHDLInterface::Variable* cross_clk;
  VHDLInterface::Variable* stop_access;
  VHDLInterface::Variable* enable_access;
  std::vector<VHDLInterface::Variable*> data_channels;
  VHDLInterface::Variable* address_clk;
  VHDLInterface::Variable* address_stall;
  VHDLInterface::Variable* address_rdy;
  //address_channels is now the pair <base,count>
  std::vector<std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*> > address_channels;
  
  StreamVariable(std::string rn="", llvm::Value* v=NULL) : readableName(rn), value(v), cross_clk(NULL), stop_access(NULL), enable_access(NULL), address_clk(NULL), address_stall(NULL), address_rdy(NULL) {}
};

}

#endif
