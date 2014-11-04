#include "rocccLibrary/VHDLComponents/ShiftBuffer.h"

#include "rocccLibrary/VHDLComponents/VHDLComponents.h"
#include <assert.h>
#include <sstream>
#include <fstream>

ShiftBuffer::ShiftBuffer(std::string n) : DATA_WIDTH(0), SHIFT_DEPTH(0), rst(NULL), clk(NULL), data_in(NULL), data_out(NULL), name(n)
{
}
void ShiftBuffer::mapDataWidth(int dw)
{
  DATA_WIDTH = dw;
}
void ShiftBuffer::mapShiftDepth(int sd)
{
  SHIFT_DEPTH = sd;
 }
void ShiftBuffer::mapRst(VHDLInterface::Variable* r)
{
  rst = r;
}
void ShiftBuffer::mapClk(VHDLInterface::Variable* c)
{
   clk = c;
}
void ShiftBuffer::mapDataIn(VHDLInterface::Variable* dat)
{
  data_in = dat;
   assert( data_in );
  assert( data_in->getSize() == DATA_WIDTH );
 }
void ShiftBuffer::mapDataOut(VHDLInterface::Variable* dat)
{
   data_out = dat;
  assert( data_out );
   assert( data_out->getSize() == DATA_WIDTH );
}
class ShiftBufferProcess : public VHDLInterface::MultiStatementProcess {
protected:
  virtual std::string generateCode()
  {
    std::stringstream ss;
    assert( getClockDomain() and "Clock domain has not been set!" );
    ss << "process(clk, rst)\n";
    for(std::vector<VHDLInterface::Value*>::iterator VI = vals.begin(); VI != vals.end(); ++VI)
    {
      VHDLInterface::ProcessVariable* v = dynamic_cast<VHDLInterface::ProcessVariable*>(*VI);
      if( v )
      {
        ss << v->generateDeclarationCode();
      }
    }
    ss << "begin\n";
    ss << "  if (" << getParent()->getDeclaration()->getPort("rst").at(0)->getName() << " = \'1\') then\n";
    ss << generateResetCode(2);
    ss << "  elsif( " << getClockDomain()->generateDefaultCode() << "\'event and " << getClockDomain()->generateDefaultCode() << " = \'1\' ) then\n";
    ss << generateSteadyState(2);
    ss << "  end if;\n";
    ss << "end process;\n";
    return ss.str();
  }
public:
  ShiftBufferProcess(VHDLInterface::Entity* p, VHDLInterface::Variable* clk=NULL) : VHDLInterface::MultiStatementProcess(p, clk) {}
};
void ShiftBuffer::generateCode(VHDLInterface::Entity* e)
 {
  assert( rst );
  assert( clk );
  assert( data_in );
  assert( data_out );
  assert( data_in->getSize() == data_out->getSize() );
  assert( data_in->getSize() == DATA_WIDTH );
  assert( e );
#ifndef INLINE_VHDL
  //instead of using the values directly, lets create a ShiftBuffer component and output it
  std::stringstream ss;
  ss << "ShiftBuffer" << DATA_WIDTH << "_" << SHIFT_DEPTH;
  VHDLInterface::Entity* ent = new VHDLInterface::Entity(ss.str());
  VHDLInterface::ComponentDeclaration* dec = ent->getDeclaration();
  dec->getPorts().clear();
  VHDLInterface::ComponentDefinition* def = e->createComponent(name, dec);
  VHDLInterface::Port* tmp;
  tmp = dec->addPort("clk", 1, VHDLInterface::Port::INPUT, NULL, false);
  def->map(clk, tmp); clk = tmp;
  tmp = dec->addPort("rst", 1, VHDLInterface::Port::INPUT, NULL, false);
  def->map(rst, tmp); rst = tmp;
  tmp = dec->addPort("data_in", data_in->getSize(), VHDLInterface::Port::INPUT, data_in->getLLVMValue(), false);
  def->map(data_in, tmp); data_in = tmp;
  tmp = dec->addPort("data_out", data_out->getSize(), VHDLInterface::Port::OUTPUT, data_out->getLLVMValue(), false);
  def->map(data_out, tmp); data_out = tmp;
  e = ent;
#endif
  std::vector<VHDLInterface::Variable*> buffer;
  for(int c = 0; c <= SHIFT_DEPTH; ++c)
  {
     std::stringstream buf_name;
    buf_name << name << "_shiftbuff_" << c;
     buffer.push_back(e->createSignal<VHDLInterface::Signal>(buf_name.str(), DATA_WIDTH));
  }
  e->createSynchronousStatement(buffer[SHIFT_DEPTH], data_in);
  VHDLInterface::MultiStatementProcess* p = e->createProcess<ShiftBufferProcess>(clk);
  //TODO - add code to change rst mapping
  for(int c = 0; c < SHIFT_DEPTH; ++c)
  {
    p->addStatement(new VHDLInterface::AssignmentStatement(buffer[c], buffer[c+1], p));
  }
  e->createSynchronousStatement(data_out, buffer[0]);
#ifndef INLINE_VHDL
  std::ofstream fout((e->getDeclaration()->getName()+".vhdl").c_str());
  fout << e->generateCode();
#endif
}
