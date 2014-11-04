#ifndef _FIFO_INTERFACE_BLOCK_DOT_HPP__
#define _FIFO_INTERFACE_BLOCK_DOT_HPP__

#include <map>
#include <vector>
#include "rocccLibrary/VHDLInterface.h"

template< class INPUT_INDEX_T, class OUTPUT_INDEX_T >
class FifoInterfaceBlock {
protected:
  //default control ports
  VHDLInterface::Variable* clk;
  VHDLInterface::Variable* rst;
  //input interface ports
  struct InputFifoInterface {
    VHDLInterface::Variable* read_enable_out;
    VHDLInterface::Variable* empty_in;
    std::vector<VHDLInterface::Variable*> data_in;
    InputFifoInterface() : read_enable_out(NULL), empty_in(NULL) {}
  };
  std::map<INPUT_INDEX_T, InputFifoInterface> inputs;
  //output interface ports
  struct OutputFifoInterface {
    VHDLInterface::Variable* read_enable_in;
    VHDLInterface::Variable* empty_out;
    std::vector<VHDLInterface::Variable*> data_out;
    OutputFifoInterface() : read_enable_in(NULL), empty_out(NULL) {}
  };
  std::map<OUTPUT_INDEX_T, OutputFifoInterface> outputs;
public:
  FifoInterfaceBlock() : clk(NULL), rst(NULL){}
  void initializeInputInterfacePorts(INPUT_INDEX_T index, VHDLInterface::Variable* read_out, VHDLInterface::Variable* empty_in, std::vector<VHDLInterface::Variable*> dat_in)
  {
    inputs[index].read_enable_out = read_out;
    inputs[index].empty_in = empty_in;
    inputs[index].data_in = dat_in;
  }
  void initializeOutputInterfacePorts(OUTPUT_INDEX_T index, VHDLInterface::Variable* read_in, VHDLInterface::Variable* empty_out, std::vector<VHDLInterface::Variable*> dat_out)
  {
    outputs[index].read_enable_in = read_in;
    outputs[index].empty_out = empty_out;
    outputs[index].data_out = dat_out;
  }
  VHDLInterface::Variable* getInputReadEnableOut(INPUT_INDEX_T index){return inputs[index].read_enable_out;}
  VHDLInterface::Variable* getInputEmptyIn(INPUT_INDEX_T index){return inputs[index].empty_in;}
  std::vector<VHDLInterface::Variable*> getInputDataIn(INPUT_INDEX_T index){return inputs[index].data_in;}
  VHDLInterface::Variable* getOutputReadEnableIn(OUTPUT_INDEX_T index){return outputs[index].read_enable_in;}
  VHDLInterface::Variable* getOutputEmptyOut(OUTPUT_INDEX_T index){return outputs[index].empty_out;}
  std::vector<VHDLInterface::Variable*> getOutputDataOut(OUTPUT_INDEX_T index){return outputs[index].data_out;}
  std::vector<INPUT_INDEX_T> getInputIndexes()
  {
    std::vector<INPUT_INDEX_T> ret;
    for(typename std::map<INPUT_INDEX_T, InputFifoInterface>::iterator III = inputs.begin(); III != inputs.end(); ++III)
      ret.push_back(III->first);
    return ret;
  }
  std::vector<OUTPUT_INDEX_T> getOutputIndexes()
  {
    std::vector<OUTPUT_INDEX_T> ret;
    for(typename std::map<OUTPUT_INDEX_T, OutputFifoInterface>::iterator III = outputs.begin(); III != outputs.end(); ++III)
      ret.push_back(III->first);
    return ret;
  }
  //generate synchronous statements in the given entity that connect
  //   the bottom level fifo of this block to the top level control logic
  //   of the rhs block
  template< class OTHER_INDEX_T >
  void flowsInto(FifoInterfaceBlock<OUTPUT_INDEX_T, OTHER_INDEX_T>& rhs, VHDLInterface::Entity* p)
  {
    for(typename std::map<OUTPUT_INDEX_T, OutputFifoInterface>::iterator OII = outputs.begin(); OII != outputs.end(); ++OII)
    {
      const OUTPUT_INDEX_T index = OII->first;
      assert(rhs.inputs.find(index) != rhs.inputs.end());
      assert(outputs[index].read_enable_in);
      assert(rhs.inputs[index].read_enable_out);
      p->createSynchronousStatement(outputs[index].read_enable_in, rhs.inputs[index].read_enable_out);
      assert(outputs[index].empty_out);
      assert(rhs.inputs[index].empty_in);
      p->createSynchronousStatement(rhs.inputs[index].empty_in, outputs[index].empty_out);
      assert(outputs[index].data_out.size() == rhs.inputs[index].data_in.size());
      std::vector<VHDLInterface::Variable*>::iterator DOI = outputs[index].data_out.begin();
      for(std::vector<VHDLInterface::Variable*>::iterator DII = rhs.inputs[index].data_in.begin(); DOI != outputs[index].data_out.end() and DII != rhs.inputs[index].data_in.end(); ++DOI, ++DII)
      {
        p->createSynchronousStatement(*DII, *DOI);
      }
    }
  }
};

#endif
