#include "rocccLibrary/VHDLComponents/MicroFifo.h"

#include "rocccLibrary/VHDLComponents/VHDLComponents.h"
#include <sstream>
#include <cmath>
#include "rocccLibrary/InternalWarning.h"
#include <fstream>

class Array : public VHDLInterface::Signal {
  int array_size;
  int element_size;
  class ArrayElement : public VHDLInterface::Variable {
    Array* parent;
    VHDLInterface::Value* index;
  protected:
    virtual void setOwner(VHDLInterface::ValueOwner* v) //from Value
    {
    }
    virtual VHDLInterface::ValueOwner* getOwner() //from Value
    {
      return NULL;
    }
  public:
    ArrayElement(Array* p, VHDLInterface::Value* i) : VHDLInterface::Variable(p->element_size), parent(p), index(i) {}
    virtual std::string getInternalName(){return parent->getName();} //from Value
    virtual int getSize(){return parent->element_size;} //from Value
    virtual std::string generateCode(int size) //from Value
    {
      assert( size == getSize() );
      std::stringstream ss;
      int width = std::ceil(std::log(parent->array_size)/std::log(2));
      ss << parent->getName() << "(conv_integer(" << index->generateCode(width) << "))";
      return ss.str();
    }
    virtual std::string generateDeclarationCode() //from Value
    {
      return "";
    }
    virtual void setReadFrom() //from Value
    {
      parent->setReadFrom();
      index->setReadFrom();
     }
    virtual void setWrittenTo() //from Value
    {
      parent->setWrittenTo();
      index->setReadFrom();
    }
  };
public:
  Array(std::string n, int s, llvm::Value* v=NULL) : VHDLInterface::Signal(n, 1, v), array_size(0), element_size(s) {}
  void setNumElements(int n){array_size=n;}
  virtual std::string generateCode(int size){assert(0 and "Cannot generate array code!");} //from Value
  virtual Value* generateResetValue(){assert(0 and "Cannot reset array code!");} //from Variable
  virtual std::string generateDeclarationCode() //from Signal
  {
    if( !isReadFrom() )
    {
      if( !isWrittenTo() )
      {
        return "";
      }
      else
      {
        INTERNAL_WARNING("Generating declaration for array " << getName() << ", which is never read!\n");
      }
    }
    Variable::declare();
    std::stringstream ss;
    ss << "type " << getName() << "_type is array (0 to " << array_size-1 << ") of ";
    if(element_size == 1)
      ss << " STD_LOGIC ;";
    else
      ss << " STD_LOGIC_VECTOR(" << element_size-1 << " downto 0) ;";
    ss << "\nsignal " << getName() << " : " << getName() << "_type := (others=>(others=>'0'));";
  #if ROCCC_DEBUG >= SOURCE_WARNINGS
     if( getLLVMValue() != NULL )
    {
      ss << " --" << getLLVMValue()->getName();
    }
  #endif
    ss << "\n"; 
    return ss.str();
  }
  Variable* getElement(VHDLInterface::Value* i)
  {
    return new ArrayElement(this, i);
  }
};

MicroFifo::MicroFifo(std::string n) : ADDRESS_WIDTH(0), DATA_WIDTH(0), ALMOST_FULL_COUNT(0), ALMOST_EMPTY_COUNT(0), clk(NULL), rst(NULL), data_in(NULL), valid_in(NULL), full_out(NULL), data_out(NULL), read_enable_in(NULL), empty_out(NULL), name(n)
{
}
std::string MicroFifo::getName(){return name;}
void MicroFifo::mapAddressWidth(int aw){ADDRESS_WIDTH=aw;}
void MicroFifo::mapDataWidth(int dw){DATA_WIDTH=dw;}
void MicroFifo::mapAlmostFullCount(int afc){ALMOST_FULL_COUNT=afc;}
void MicroFifo::mapAlmostEmptyCount(int aec){ALMOST_EMPTY_COUNT=aec;}
void MicroFifo::mapClk(VHDLInterface::Variable* c){clk=c;}
void MicroFifo::mapRst(VHDLInterface::Variable* r){rst=r;}
void MicroFifo::mapDataIn(VHDLInterface::Variable* dat){data_in=dat;}
void MicroFifo::mapValidIn(VHDLInterface::Variable* v){valid_in=v;}
void MicroFifo::mapFullOut(VHDLInterface::Variable* f){full_out=f;}
void MicroFifo::mapDataOut(VHDLInterface::Variable* dat){data_out=dat;}
void MicroFifo::mapReadEnableIn(VHDLInterface::Variable* r){read_enable_in=r;}
void MicroFifo::mapEmptyOut(VHDLInterface::Variable* e){empty_out=e;}
int MicroFifo::getNumElements()
{
  int numElements = 1;
  for(int c = 0; c < ADDRESS_WIDTH; ++c)
    numElements *= 2;
  return numElements;
}
class MicroFifoProcess : public VHDLInterface::MultiStatementProcess {
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
  MicroFifoProcess(VHDLInterface::Entity* p, VHDLInterface::Variable* clk=NULL) : VHDLInterface::MultiStatementProcess(p, clk) {}
};

// This function generates ugly code that synthesizes for the Virtex 6
//  tool chain whenthe size of the storage is too big.  It should be 
//  functionally equivalent to the normal code.  Added 3-4-2013
void MicroFifo::virtex6Workaround(VHDLInterface::Entity* e)
{
  assert(e != NULL) ;

  // The Virtex 6 workaround is to create the component as if it
  //  were not inlined, and then generate verilog instead of VHDL

  // In order to create this, I'm going to create the component 
  //  instantiation (which will be done in VHDL) and a component
  //  definition (which will be done with verilog).
  
  // The VHDL component instantiation
  std::stringstream ss;
  ss << "MicroFifo" << DATA_WIDTH;
  VHDLInterface::Entity* ent = new VHDLInterface::Entity(ss.str());
  VHDLInterface::ComponentDeclaration* dec = ent->getDeclaration();
  dec->getPorts().clear();
  VHDLInterface::ComponentDefinition* def = e->createComponent(name, dec);

  // Generics
  VHDLInterface::Generic* dw = 
    new VHDLInterface::IntegerGeneric(dec, "DATA_WIDTH", NULL) ;
  VHDLInterface::Generic* aw = 
    new VHDLInterface::IntegerGeneric(dec, "ADDRESS_WIDTH", NULL) ;
  VHDLInterface::Generic* af = 
    new VHDLInterface::IntegerGeneric(dec, "ALMOST_FULL_COUNT", NULL) ;
  VHDLInterface::Generic* ae = 
    new VHDLInterface::IntegerGeneric(dec, "ALMOST_EMPTY_COUNT", NULL) ;
  dec->addGeneric(dw) ;
  dec->addGeneric(aw) ;
  dec->addGeneric(af) ;
  dec->addGeneric(ae) ;
  def->mapGeneric(VHDLInterface::IntegerGeneric::getTypeMatchedInteger(DATA_WIDTH), dw) ;
  def->mapGeneric(VHDLInterface::IntegerGeneric::getTypeMatchedInteger(ADDRESS_WIDTH), aw) ;
  def->mapGeneric(VHDLInterface::IntegerGeneric::getTypeMatchedInteger(ALMOST_FULL_COUNT), af) ;
  def->mapGeneric(VHDLInterface::IntegerGeneric::getTypeMatchedInteger(ALMOST_EMPTY_COUNT), ae) ;
  
  // Ports
  VHDLInterface::Port* tmp ;
  tmp = dec->addPort("clk", 1, VHDLInterface::Port::INPUT, NULL, false);
  def->map(clk, tmp); clk = tmp;
  tmp = dec->addPort("rst", 1, VHDLInterface::Port::INPUT, NULL, false);
  def->map(rst, tmp); rst = tmp;
  tmp = dec->addPort("data_in", data_in->getSize(), VHDLInterface::Port::INPUT, data_in->getLLVMValue(), false);
  def->map(data_in, tmp); data_in = tmp;
  tmp = dec->addPort("valid_in", 1, VHDLInterface::Port::INPUT, NULL, false);
  def->map(valid_in, tmp); valid_in = tmp;
  tmp = dec->addPort("full_out", 1, VHDLInterface::Port::OUTPUT, NULL, false);
  def->map(full_out, tmp); full_out = tmp;
  tmp = dec->addPort("data_out", data_out->getSize(), VHDLInterface::Port::OUTPUT, data_out->getLLVMValue(), false);
  def->map(data_out, tmp); data_out = tmp;
  tmp = dec->addPort("read_enable_in", 1, VHDLInterface::Port::INPUT, NULL, false);
  def->map(read_enable_in, tmp); read_enable_in = tmp;
  tmp = dec->addPort("empty_out", 1, VHDLInterface::Port::OUTPUT, NULL, false);
  def->map(empty_out, tmp); empty_out = tmp;
  e = ent;

  // Add assignment statements to the VHDL component so everything is driven.
  //  These won't be output but are here just to make everything work.
  VHDLInterface::AssignmentStatement* emptyStmt =
    e->createSynchronousStatement(empty_out) ;
  emptyStmt->addCase(VHDLInterface::ConstantInt::get(1)) ; 
       
  VHDLInterface::AssignmentStatement* fullStmt =
    e->createSynchronousStatement(full_out) ;
  fullStmt->addCase(VHDLInterface::ConstantInt::get(1)) ;

  VHDLInterface::AssignmentStatement* dataStmt =
    e->createSynchronousStatement(data_out) ;
  dataStmt->addCase(VHDLInterface::ConstantInt::get(1)) ;

  // ----------------- The Verilog section --------------------
  std::ofstream fout((e->getDeclaration()->getName()+".v").c_str());

  fout << "`timescale 100 ps / 10 ps\n\n" ;
  fout << "module " << e->getDeclaration()->getName() << "\n" ;
  fout << "(\n" ;
  fout << "  clk, rst, data_in, valid_in, full_out, data_out, read_enable_in,"
          " empty_out\n" ;
  fout << ") ;\n\n" ;
  fout << "  parameter DATA_WIDTH = " << DATA_WIDTH << " ;\n" ;
  fout << "  parameter ADDRESS_WIDTH = " << ADDRESS_WIDTH << " ;\n" ;
  fout << "  parameter ALMOST_FULL_COUNT = " << ALMOST_FULL_COUNT << " ;\n" ;
  fout << "  parameter ALMOST_EMPTY_COUNT = " << ALMOST_EMPTY_COUNT << " ;\n" ;
  fout << "  input clk ;\n" ;
  fout << "  input rst ;\n" ;
  fout << "  input [DATA_WIDTH-1:0] data_in ;\n" ;
  fout << "  input valid_in ;\n" ;
  fout << "  output full_out ;\n" ;
  fout << "  output [DATA_WIDTH-1:0] data_out ;\n" ;
  fout << "  input read_enable_in ;\n" ;
  fout << "  output empty_out ;\n\n" ;
  fout << "  reg [7:0] mem [DATA_WIDTH-1:0] ;\n" ;
  fout << "  reg [2:0] rd_addr ;\n" ;
  fout << "  reg [2:0] wr_addr ;\n" ;
  fout << "  reg [3:0] count ;\n\n" ;
  fout << "  assign empty_out = (rd_addr == wr_addr) ;\n" ;
  fout << "  assign full_out = (count > ALMOST_FULL_COUNT) ;\n" ;
  fout << "  assign data_out = mem[rd_addr] ;\n\n" ;
  fout << "  always @(posedge clk)\n" ;
  fout << "    begin\n" ;
  fout << "      if (rst == 1\'b1)\n" ;
  fout << "        begin\n" ;
  fout << "          rd_addr <= 3\'b0 ;\n" ;
  fout << "          wr_addr <= 3\'b0 ;\n" ;
  fout << "          count   <= 4\'b0 ;\n" ;
  fout << "        end\n" ;
  fout << "      else\n" ;
  fout << "        begin\n" ;
  fout << "          if (valid_in)\n" ;
  fout << "            begin\n" ;
  fout << "              mem[rd_addr] <= data_in ;\n" ;
  fout << "              rd_addr <= rd_addr + 1 ;\n" ;
  fout << "            end\n" ;
  fout << "          if (read_enable_in)\n" ;
  fout << "            wr_addr <= wr_addr + 1 ;\n" ;
  fout << "          if (valid_in == 1\'b1 && read_enable_in == 1\'b0)\n" ;
  fout << "            count <= count + 1 ;\n" ;
  fout << "          if (valid_in == 1\'b0 && read_enable_in == 1\'b1)\n" ;
  fout << "            count <= count - 1 ;\n" ;
  fout << "        end\n" ;
  fout << "    end\n" ;
  fout << "endmodule\n" ;

  fout.close() ;

}

void MicroFifo::generateCode(VHDLInterface::Entity* e)
{
  assert(e);

  // Determine if we have to generate the workaround for Virtex 6 if
  //  the size is too big.  
  //  TODO - Fix this magic number to something that actually represents
  //         the limit on the Virtex 6 tools.
  if (DATA_WIDTH * getNumElements() > 10000)
  {
    virtex6Workaround(e) ;
    return ;
  }

#ifndef INLINE_VHDL
  //instead of using the values directly, lets create a MicroFifo component and output it
  std::stringstream ss;
  ss << "MicroFifo" << DATA_WIDTH;
  VHDLInterface::Entity* ent = new VHDLInterface::Entity(ss.str());
  VHDLInterface::ComponentDeclaration* dec = ent->getDeclaration();
  dec->getPorts().clear();
  VHDLInterface::ComponentDefinition* def = e->createComponent(name, dec);
  def->mapGeneric(VHDLInterface::IntegerGeneric::getTypeMatchedInteger(ADDRESS_WIDTH), dec->addGeneric(new VHDLInterface::IntegerGeneric(dec, "ADDRESS_WIDTH", 0)));
  def->mapGeneric(VHDLInterface::IntegerGeneric::getTypeMatchedInteger(ALMOST_FULL_COUNT), dec->addGeneric(new VHDLInterface::IntegerGeneric(dec, "ALMOST_FULL_COUNT", 0)));
  def->mapGeneric(VHDLInterface::IntegerGeneric::getTypeMatchedInteger(ALMOST_EMPTY_COUNT), dec->addGeneric(new VHDLInterface::IntegerGeneric(dec, "ALMOST_EMPTY_COUNT", 0)));
  VHDLInterface::Port* tmp;
  tmp = dec->addPort("clk", 1, VHDLInterface::Port::INPUT, NULL, false);
  def->map(clk, tmp); clk = tmp;
  tmp = dec->addPort("rst", 1, VHDLInterface::Port::INPUT, NULL, false);
  def->map(rst, tmp); rst = tmp;
  tmp = dec->addPort("data_in", data_in->getSize(), VHDLInterface::Port::INPUT, data_in->getLLVMValue(), false);
  def->map(data_in, tmp); data_in = tmp;
  tmp = dec->addPort("valid_in", 1, VHDLInterface::Port::INPUT, NULL, false);
  def->map(valid_in, tmp); valid_in = tmp;
  tmp = dec->addPort("full_out", 1, VHDLInterface::Port::OUTPUT, NULL, false);
  def->map(full_out, tmp); full_out = tmp;
  tmp = dec->addPort("data_out", data_out->getSize(), VHDLInterface::Port::OUTPUT, data_out->getLLVMValue(), false);
  def->map(data_out, tmp); data_out = tmp;
  tmp = dec->addPort("read_enable_in", 1, VHDLInterface::Port::INPUT, NULL, false);
  def->map(read_enable_in, tmp); read_enable_in = tmp;
  tmp = dec->addPort("empty_out", 1, VHDLInterface::Port::OUTPUT, NULL, false);
  def->map(empty_out, tmp); empty_out = tmp;
  e = ent;
#endif

  VHDLInterface::Signal* rd_addr = e->createSignal<VHDLInterface::Signal>(name+"_rd_addr", ADDRESS_WIDTH);
  VHDLInterface::Signal* wr_addr = e->createSignal<VHDLInterface::Signal>(name+"_wr_addr", ADDRESS_WIDTH);
  VHDLInterface::Signal* has_data = e->createSignal<VHDLInterface::Signal>(name+"_has_data", 1);
  VHDLInterface::Signal* count = e->createSignal<VHDLInterface::Signal>(name+"_count", ADDRESS_WIDTH+1);
  Array* ram = e->createSignal<Array>(name+"_ram", DATA_WIDTH);
  ram->setNumElements(getNumElements());
  VHDLInterface::Signal* full = e->createSignal<VHDLInterface::Signal>(name+"_full", 1);
  VHDLInterface::Signal* empty = e->createSignal<VHDLInterface::Signal>(name+"_empty", 1);
  VHDLInterface::Signal* push = e->createSignal<VHDLInterface::Signal>(name+"_push", 1);
  VHDLInterface::Signal* pop = e->createSignal<VHDLInterface::Signal>(name+"_pop", 1);
  
  {//create a local namespace
    VHDLInterface::AssignmentStatement* a = e->createSynchronousStatement(empty);
    a->addCase(VHDLInterface::ConstantInt::get(1), VHDLInterface::Wrap(has_data) == VHDLInterface::ConstantInt::get(0));
    a->addCase(VHDLInterface::ConstantInt::get(0));
  }
  {//create a local namespace
    VHDLInterface::AssignmentStatement* a = e->createSynchronousStatement(empty_out);
    a->addCase(VHDLInterface::ConstantInt::get(1), VHDLInterface::Wrap(count) <= VHDLInterface::ConstantInt::get(ALMOST_EMPTY_COUNT));
    a->addCase(VHDLInterface::ConstantInt::get(0));
  }
  {//create a local namespace
    VHDLInterface::AssignmentStatement* a = e->createSynchronousStatement(full);
    a->addCase(VHDLInterface::ConstantInt::get(1), VHDLInterface::Wrap(has_data) == VHDLInterface::ConstantInt::get(1) and VHDLInterface::Wrap(rd_addr) == VHDLInterface::Wrap(wr_addr));
    a->addCase(VHDLInterface::ConstantInt::get(0));
  }
  {//create a local namespace
    VHDLInterface::AssignmentStatement* a = e->createSynchronousStatement(full_out);
    a->addCase(VHDLInterface::ConstantInt::get(1), VHDLInterface::ConstantInt::get(getNumElements()-ALMOST_FULL_COUNT) <= VHDLInterface::Wrap(count));
    a->addCase(VHDLInterface::ConstantInt::get(0));
  }
  {//create a local namespace
    VHDLInterface::AssignmentStatement* a = e->createSynchronousStatement(push);
    a->addCase(VHDLInterface::ConstantInt::get(1), VHDLInterface::Wrap(full) == VHDLInterface::ConstantInt::get(0) and VHDLInterface::Wrap(valid_in) == VHDLInterface::ConstantInt::get(1));
    a->addCase(VHDLInterface::ConstantInt::get(0));
  }
  {//create a local namespace
    VHDLInterface::AssignmentStatement* a = e->createSynchronousStatement(pop);
    a->addCase(VHDLInterface::ConstantInt::get(1), VHDLInterface::Wrap(empty) == VHDLInterface::ConstantInt::get(0) and VHDLInterface::Wrap(read_enable_in) == VHDLInterface::ConstantInt::get(1));
    a->addCase(VHDLInterface::ConstantInt::get(0));
  }
  VHDLInterface::MultiStatementProcess* p = e->createProcess<MicroFifoProcess>(clk);

  p->addStatement(new VHDLInterface::IfStatement(p, VHDLInterface::Wrap(push) == VHDLInterface::ConstantInt::get(1), 
                    (new VHDLInterface::MultiStatement(p))->addStatement(
			 new VHDLInterface::AssignmentStatement(ram->getElement(wr_addr), data_in, p)
                    )->addStatement(
                         new VHDLInterface::AssignmentStatement(wr_addr, VHDLInterface::Wrap(wr_addr) + VHDLInterface::ConstantInt::get(1), p)
                    )
                 ));
  p->addStatement(new VHDLInterface::AssignmentStatement(data_out, ram->getElement(rd_addr), p));
  p->addStatement(new VHDLInterface::IfStatement(p, VHDLInterface::Wrap(pop) == VHDLInterface::ConstantInt::get(1),
                    new VHDLInterface::AssignmentStatement(rd_addr, VHDLInterface::Wrap(rd_addr) + VHDLInterface::ConstantInt::get(1), p)
                 ));
  p->addStatement(new VHDLInterface::IfStatement(p, VHDLInterface::Wrap(push) == VHDLInterface::ConstantInt::get(1) and VHDLInterface::Wrap(pop) == VHDLInterface::ConstantInt::get(0),
                    //if true
                    (new VHDLInterface::MultiStatement(p))->addStatement(
                         new VHDLInterface::AssignmentStatement(count, VHDLInterface::Wrap(count) + VHDLInterface::ConstantInt::get(1), p)
                    )->addStatement(
                         new VHDLInterface::AssignmentStatement(has_data, VHDLInterface::ConstantInt::get(1), p)
                    ),
                    //if false
                    new VHDLInterface::IfStatement(p, VHDLInterface::Wrap(pop) == VHDLInterface::ConstantInt::get(1) and VHDLInterface::Wrap(push) == VHDLInterface::ConstantInt::get(0),
                    (new VHDLInterface::MultiStatement(p))->addStatement(
                         new VHDLInterface::AssignmentStatement(count, VHDLInterface::Wrap(count) - VHDLInterface::ConstantInt::get(1), p)
                    )->addStatement(
                         (new VHDLInterface::AssignmentStatement(has_data, p))
                              ->addCase(VHDLInterface::ConstantInt::get(1), VHDLInterface::Wrap(count) != VHDLInterface::ConstantInt::get(1))
                              ->addCase(VHDLInterface::ConstantInt::get(0))
                    ))
                 ));
#ifndef INLINE_VHDL
  std::ofstream fout((e->getDeclaration()->getName()+".vhdl").c_str());
  fout << e->generateCode();
#endif
}
void MicroFifo::mapInputAndOutputVector(std::vector<VHDLInterface::Variable*> input, std::vector<VHDLInterface::Variable*> output, VHDLInterface::Entity* e)
{
  //map the input
  VHDLInterface::Value* vdati = NULL;
  int input_bit_size = 0;
  for(std::vector<VHDLInterface::Variable*>::iterator II = input.begin(); II != input.end(); ++II)
  {
    input_bit_size += (*II)->getSize();
    if( vdati == NULL )
      vdati = *II;
    else
      vdati = bitwise_concat(*II, vdati);
  }
  assert(input_bit_size > 0);
  VHDLInterface::Variable* dati = e->createSignal<VHDLInterface::Signal>(name+"_dati_tmp", input_bit_size);
  e->createSynchronousStatement(dati, vdati);
  this->mapDataWidth(input_bit_size);
  this->mapDataIn(dati);
  //map the output
  VHDLInterface::Signal* dato = e->createSignal<VHDLInterface::Signal>(name+"_dato_tmp", input_bit_size);
  this->mapDataOut(dato);
  int count = 0;
  for(std::vector<VHDLInterface::Variable*>::iterator II = output.begin(); II != output.end(); ++II)
  {
    e->createSynchronousStatement(*II, VHDLInterface::BitRange::get(dato, count+(*II)->getSize()-1, count));
    count += (*II)->getSize();
  }
}
