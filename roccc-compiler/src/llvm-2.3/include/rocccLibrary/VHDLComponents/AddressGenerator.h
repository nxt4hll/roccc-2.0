#ifndef ADDRESS_GENERATOR_H__
#define ADDRESS_GENERATOR_H__

#include "rocccLibrary/VHDLInterface.h"
#include "llvm/Value.h"
#include <map>
#include <vector>
#include "LoopInductionVariableHandler.h"
#include "rocccLibrary/Window/BufferSpaceAccesser.h"

class AddressGenerator {
protected:
  //external VHDL interface
  VHDLInterface::Variable* address_rdy_out;
  VHDLInterface::Variable* address_stall_in;
  //pair of base-count ports that are the address out ports
  std::vector<std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*> > address_out;
  //implementation of the address calculation datapath
  class AddressCalculationDatapath;
  AddressCalculationDatapath* addressCalcImpl;
  //llvm Value indicating what stream we are
  llvm::Value* stream_value;
  //creates the state machine for the address calculator with a given accesser, and returns the first state created for that accesser
  VHDLInterface::State* createBufferVHDL(LoopInductionVariableHandler* livHandler, BufferSpaceAccesser accesser, std::string prefix, VHDLInterface::CaseStatement* cs);
  //create the initialization vhdl, including creating the process, the state, and the state machine, and return the case statement that is implementing the state machine
  VHDLInterface::CaseStatement* createInitializationVHDL(VHDLInterface::Entity* parent, LoopInductionVariableHandler* livHandler, BufferSpaceAccesser window_accesser, BufferSpaceAccesser step_accesser, VHDLInterface::Variable* clk);
  //save these so we arent passing them all over the place
  VHDLInterface::StateVar* state;
  VHDLInterface::MultiStatementProcess* p;
public:
  AddressGenerator(llvm::Value* v);
  void initializeVHDLInterface(VHDLInterface::Variable* rdy, VHDLInterface::Variable* stall, std::vector<std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*> > base_count_pair);
  virtual void createVHDL(VHDLInterface::Entity* parent, LoopInductionVariableHandler* livHandler, BufferSpaceAccesser window_accesser, BufferSpaceAccesser step_accesser, VHDLInterface::Variable* clk)=0;
};

class InputAddressGenerator : public AddressGenerator {
public:
  InputAddressGenerator(llvm::Value* v);
  virtual void createVHDL(VHDLInterface::Entity* parent, LoopInductionVariableHandler* livHandler, BufferSpaceAccesser window_accesser, BufferSpaceAccesser step_accesser, VHDLInterface::Variable* clk);
};

class OutputAddressGenerator : public AddressGenerator {
public:
  OutputAddressGenerator(llvm::Value* v);
  virtual void createVHDL(VHDLInterface::Entity* parent, LoopInductionVariableHandler* livHandler, BufferSpaceAccesser window_accesser, BufferSpaceAccesser step_accesser, VHDLInterface::Variable* clk);
};

#endif
