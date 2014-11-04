#ifndef __BRAM_FIFO_DOT_H__
#define __BRAM_FIFO_DOT_H__

#include "rocccLibrary/VHDLInterface.h"

namespace VHDLInterface {

class BRAMFifo : public VHDLInterface::ComponentDeclaration {
  class BRAMFifo_impl;
  BRAMFifo_impl* impl;
public:
  BRAMFifo(int data_width, int data_depth); //search database for bram with width and depth
  VHDLInterface::Port* getRst();
  VHDLInterface::Port* getRClk();
  VHDLInterface::Port* getEmptyOut();
  VHDLInterface::Port* getReadEnIn();
  VHDLInterface::Port* getDataOut();
  VHDLInterface::Port* getWClk();
  VHDLInterface::Port* getFullOut();
  VHDLInterface::Port* getWriteEnIn();
  VHDLInterface::Port* getDataIn();
  VHDLInterface::ComponentDefinition* getInstantiation(VHDLInterface::Entity* e);
};

}

#endif
