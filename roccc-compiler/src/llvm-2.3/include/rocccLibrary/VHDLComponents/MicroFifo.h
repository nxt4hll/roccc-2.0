#ifndef _MICRO_FIFO_H__
#define _MICRO_FIFO_H__

#include "rocccLibrary/VHDLInterface.h"
#include <string>
#include <vector>

class MicroFifo {
  int ADDRESS_WIDTH;
  int DATA_WIDTH;
  int ALMOST_FULL_COUNT;
  int ALMOST_EMPTY_COUNT;
  VHDLInterface::Variable* clk;
  VHDLInterface::Variable* rst;
  VHDLInterface::Variable* data_in;
  VHDLInterface::Variable* valid_in;
  VHDLInterface::Variable* full_out;
  VHDLInterface::Variable* data_out;
  VHDLInterface::Variable* read_enable_in;
  VHDLInterface::Variable* empty_out;
  std::string name;
public:
  MicroFifo(std::string n);
  std::string getName();
  void mapAddressWidth(int aw);
  void mapDataWidth(int dw);
  void mapAlmostFullCount(int afc);
  void mapAlmostEmptyCount(int aec);
  void mapClk(VHDLInterface::Variable* c);
  void mapRst(VHDLInterface::Variable* r);
  void mapDataIn(VHDLInterface::Variable* dat);
  void mapValidIn(VHDLInterface::Variable* v);
  void mapFullOut(VHDLInterface::Variable* f);
  void mapDataOut(VHDLInterface::Variable* dat);
  void mapReadEnableIn(VHDLInterface::Variable* r);
  void mapEmptyOut(VHDLInterface::Variable* e);
  int getNumElements();
  void generateCode(VHDLInterface::Entity* e);
  void virtex6Workaround(VHDLInterface::Entity* e) ;
  //in case we have several variables we want to map to the input and output,
  //  we can use this helper function to automatically set the data width,
  //  create a signal to hold the input and output, and slice the output into
  //  each of the output elements and combine each of the inputs into the input
  //  signal.
  void mapInputAndOutputVector(std::vector<VHDLInterface::Variable*> input, std::vector<VHDLInterface::Variable*> output, VHDLInterface::Entity* e);
  //instead of using mapInputAndOutputVector() directly, we can create vectors from other types and use those instead
  template<class ForwardIteratorIn, class ForwardIteratorOut>
  void mapInputAndOutputIterator(ForwardIteratorIn in_b, ForwardIteratorIn in_e, ForwardIteratorOut out_b, ForwardIteratorOut out_e, VHDLInterface::Entity* e)
  {
    std::vector<VHDLInterface::Variable*> in(in_b, in_e);
    std::vector<VHDLInterface::Variable*> out(out_b, out_e);
    mapInputAndOutputVector(in, out, e);
  }
};

#endif
