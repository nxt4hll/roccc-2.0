#ifndef _INPUTSMARTBUFFER_H__
#define _INPUTSMARTBUFFER_H__

#include "rocccLibrary/VHDLComponents/FifoInterfaceBlock.hpp"
#include "llvm/Value.h"
#include <map>
#include <vector>
#include "rocccLibrary/ROCCCLoopInformation.h"
#include "rocccLibrary/Window/BufferSpaceAccesser.h"

//many in, single out fifo interface
class InputSmartBuffer : public FifoInterfaceBlock<llvm::Value*,int> {
  struct AddressVariables {
    VHDLInterface::Variable* address_rdy_out;
    VHDLInterface::Variable* address_stall_in;
    //base-count pair
    std::vector<std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*> > address_out;
    VHDLInterface::Variable* clk;
    AddressVariables();
  };
  std::map<llvm::Value*, AddressVariables> addressVariables;
public:
  InputSmartBuffer();
  void setAddressVariables(llvm::Value* stream, VHDLInterface::Variable* rdy, VHDLInterface::Variable* stall, std::vector<std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*> > base_count_pair, VHDLInterface::Variable* clk);
  void generateVHDL(VHDLInterface::Entity* e, llvm::ROCCCLoopInformation &loopInfo);
};

#endif
