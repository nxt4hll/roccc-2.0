#ifndef _SHIFT_BUFFER_H__
#define _SHIFT_BUFFER_H__

#include "rocccLibrary/VHDLInterface.h"
#include <string>

class ShiftBuffer {
  int DATA_WIDTH;
  int SHIFT_DEPTH;
  VHDLInterface::Variable* rst;
  VHDLInterface::Variable* clk;
  VHDLInterface::Variable* data_in;
  VHDLInterface::Variable* data_out;
  std::string name;
public:
  ShiftBuffer(std::string n);
  void mapDataWidth(int dw);
  void mapShiftDepth(int sd);
  void mapRst(VHDLInterface::Variable* r);
  void mapClk(VHDLInterface::Variable* c);
  void mapDataIn(VHDLInterface::Variable* dat);
  void mapDataOut(VHDLInterface::Variable* dat);
  void generateCode(VHDLInterface::Entity* e);
};

#endif
