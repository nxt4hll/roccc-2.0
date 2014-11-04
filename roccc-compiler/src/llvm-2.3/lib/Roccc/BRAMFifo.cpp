#include "rocccLibrary/VHDLComponents/BRAMFifo.h"
#include "rocccLibrary/FileInfo.h"
#include "rocccLibrary/DatabaseHelpers.h"
#include <sstream>
#include <fstream>
#include <cstdlib>

using namespace VHDLInterface;

class BRAMFifo::BRAMFifo_impl {
public:
  int data_width;
  int data_depth;
  BRAMFifo_impl(int dw, int dd) : data_width(dw), data_depth(dd) {}
};

class BRAMDriverOwner : public VHDLInterface::ValueOwner {
public:
  virtual std::string getName(); //from ValueOwner
};
std::string BRAMDriverOwner::getName() //from ValueOwner
{
  return "BRAM_DRIVER_OWNER";
}
std::string int2str(int data_width)
{
  std::stringstream ss;
  ss << data_width;
  return ss.str();
}
BRAMFifo::BRAMFifo(int data_width, int data_depth) : ComponentDeclaration("InferredBRAMFifo"+int2str(data_width)), impl(new BRAMFifo::BRAMFifo_impl(data_width, data_depth))
{
  //delete all the builtin ports
  this->getPorts().clear();
  //add in the new ports
  this->addPort("Data_out", data_width, VHDLInterface::Port::OUTPUT, NULL, false);
  this->addPort("Empty_out", 1, VHDLInterface::Port::OUTPUT, NULL, false);
  this->addPort("ReadEn_in", 1, VHDLInterface::Port::INPUT, NULL, false);
  this->addPort("RClk", 1, VHDLInterface::Port::INPUT, NULL, false);
  this->addPort("Data_in", data_width, VHDLInterface::Port::INPUT, NULL, false);
  this->addPort("Full_out", 1, VHDLInterface::Port::OUTPUT, NULL, false);
  this->addPort("WriteEn_in", 1, VHDLInterface::Port::INPUT, NULL, false);
  this->addPort("WClk", 1, VHDLInterface::Port::INPUT, NULL, false);
  this->addPort("Clear_in", 1, VHDLInterface::Port::INPUT, NULL, false);
  //add the generics
  addGeneric(new IntegerGeneric(this, "DATA_DEPTH", 8));
  //set all the ports as owned and driven
  for(std::vector<VHDLInterface::Port*>::iterator PI = this->getPorts().begin(); PI != this->getPorts().end(); ++PI)
  {
    static BRAMDriverOwner BRAM_DRIVER_OWNER;
    BRAM_DRIVER_OWNER.add(*PI);
  }
  //create the actual file instantiating this BRAM
  char buff[1024];
  if( !getcwd(buff, 1024) )
  {
    llvm::cout << "Could not get current directory!\n";
    assert(0);
    exit(0);
  }
  std::string outputDirectory(buff);
  outputDirectory += "/vhdl/";
  std::string outputFileName ; 
  outputFileName = this->getName() + ".vhdl" ;
  std::string outputFullPath = outputDirectory + outputFileName;
  std::ofstream fout;
  fout.open(outputFullPath.c_str(), std::ios_base::out) ;
  if (!fout)
  {
    llvm::cerr << "Cannot open \'" << outputFullPath << "\' for BRAM fifo!" << std::endl ;
    assert(0 and "Could not open critical file") ;
  }
  //Database::FileInfoInterface::addFileInfo(Database::getCurrentID(), Database::FileInfo(outputFileName, Database::FileInfo::INFERRED_FIFO, outputDirectory));
  fout << "\
library UNISIM;\n\
use UNISIM.vcomponents.all;\n\
library ieee;\n\
use ieee.std_logic_1164.all;\n\
use ieee.std_logic_unsigned.all;\n\
\n\
entity " << this->getName() << " is\n\
  generic (DATA_DEPTH : integer);\n\
  port(\n\
    Data_out : out std_logic";
  if( data_width > 1 )
    fout << "_vector(" << data_width-1 << " downto 0)";
  fout << ";\n\
    Empty_out : out std_logic;\n\
    ReadEn_in : in std_logic;\n\
    RClk : in std_logic;\n\
    Data_in : in std_logic";
  if( data_width > 1 )
    fout << "_vector(" << data_width-1 << " downto 0)";
  fout << ";\n\
    Full_out : out std_logic;\n\
    WriteEn_in : in std_logic;\n\
    WClk : in std_logic;\n\
    Clear_in : in std_logic\n\
  );\n\
end entity;\n\
architecture rtl of " << this->getName() << " is\n\
signal fifo_rst : STD_LOGIC;\n";
  for(int fifo_count = 0; fifo_count * 36 < data_width; ++fifo_count)
  {
    fout << "\
signal FIFO18E1_inst" << fifo_count << "_DI : STD_LOGIC_VECTOR(31 downto 0);\n\
signal FIFO18E1_inst" << fifo_count << "_DO : STD_LOGIC_VECTOR(31 downto 0);\n\
signal FIFO18E1_inst" << fifo_count << "_DIP : STD_LOGIC_VECTOR(3 downto 0);\n\
signal FIFO18E1_inst" << fifo_count << "_DOP : STD_LOGIC_VECTOR(3 downto 0);\n\
signal FIFO18E1_inst" << fifo_count << "_FULL : STD_LOGIC;\n\
signal FIFO18E1_inst" << fifo_count << "_EMPTY : STD_LOGIC;\n";
  }
fout << "\
begin\n\
process( WClk )\n\
variable rst_buffer : STD_LOGIC_VECTOR(4 downto 0);\n\
begin\n\
  if( WClk'event and WClk = '1' )\n\
  then\n\
    rst_buffer(4 downto 0) := rst_buffer(3 downto 0) & Clear_in;\n\
    fifo_rst <= rst_buffer(4) and rst_buffer(3) and rst_buffer(2) and rst_buffer(1) and rst_buffer(0);\n\
  end if;\n\
end process;\n";
  for(int fifo_count = 0; fifo_count * 36 < data_width; ++fifo_count)
  {
    int cur_start = fifo_count * 36;
    int cur_end = cur_start + 36;
    if( cur_end > data_width )
    {
      cur_end = data_width;
    }
    int size_of_parity = 0;
    if( cur_end - cur_start > 32 )
    {
      size_of_parity = cur_end - cur_start - 32;
      cur_end = cur_start + 32;
    }
    if( cur_end - cur_start > 0 )
    {
      fout << "FIFO18E1_inst" << fifo_count << "_DI(" << cur_end - cur_start - 1 << " downto 0) <= Data_in";
      if( data_width > 1 )
        fout << "(" << cur_end - 1 << " downto " << cur_start << ")";
      fout << ";\n";
      fout << "Data_out";
      if( data_width > 1 )
        fout << "(" << cur_end - 1 << " downto " << cur_start << ")";
      fout << " <= FIFO18E1_inst" << fifo_count << "_DO(" << cur_end - cur_start - 1 << " downto 0);\n"; 
    }
    if( size_of_parity > 0 )
    {
      fout << "FIFO18E1_inst" << fifo_count << "_DIP(" << size_of_parity - 1 << " downto 0) <= Data_in(" << cur_end + size_of_parity - 1 << " downto " << cur_end << ");\n";
      fout << "Data_out(" << cur_end + size_of_parity - 1 << " downto " << cur_end << ") <= FIFO18E1_inst" << fifo_count << "_DOP(" << size_of_parity - 1 << " downto 0);\n";
    }
  }
  fout << "\
process( WClk )\n\
begin\n\
  if( WClk'event and WClk = '1' )\n\
  then\n";  
  fout << "    Full_out <= ";
  for(int fifo_count = 0; fifo_count * 36 < data_width; ++fifo_count)
  {
    if( fifo_count != 0 )
      fout << " or ";
    fout << "FIFO18E1_inst" << fifo_count << "_FULL";
  }
  fout << ";\n";
  fout << "\
  end if;\n\
end process;\n";
  fout << "Empty_out <= ";
  for(int fifo_count = 0; fifo_count * 36 < data_width; ++fifo_count)
  {
    if( fifo_count != 0 )
      fout << " or ";
    fout << "FIFO18E1_inst" << fifo_count << "_EMPTY";
  }
  fout << ";\n";
  for(int fifo_count = 0; fifo_count * 36 < data_width; ++fifo_count)
  {
    fout << "\
FIFO18E1_inst" << fifo_count << " : FIFO18E1 \
generic map (\n\
  DO_REG => 1, \n\
  EN_SYN => FALSE, \n\
  FIFO_MODE => \"FIFO18_36\", \n\
  ALMOST_FULL_OFFSET => X\"0080\", \n\
  ALMOST_EMPTY_OFFSET => X\"0080\", \n\
  DATA_WIDTH => 36, \n\
  FIRST_WORD_FALL_THROUGH => FALSE) \n\
port map (\n\
  ALMOSTEMPTY => open, \n\
  ALMOSTFULL => FIFO18E1_inst" << fifo_count << "_FULL, \n\
  DI => FIFO18E1_inst" << fifo_count << "_DI,\n\
  DO => FIFO18E1_inst" << fifo_count << "_DO,\n\
  DIP => FIFO18E1_inst" << fifo_count << "_DIP,\n\
  DOP => FIFO18E1_inst" << fifo_count << "_DOP,\n\
  EMPTY => FIFO18E1_inst" << fifo_count << "_EMPTY, \n\
  FULL => open,\n\
  RDCOUNT => open, \n\
  RDERR => open, \n\
  WRCOUNT => open, \n\
  WRERR => open, \n\
  REGCE => \'1\', \n\
  RSTREG => fifo_rst, \n\
  RDCLK => RClk, \n\
  RDEN => ReadEn_in and not fifo_rst, \n\
  RST => fifo_rst, \n\
  WRCLK => WClk, \n\
  WREN => WriteEn_in and not fifo_rst\n\
);\n";
  }
  fout << "end architecture;\n";
}
VHDLInterface::Port* BRAMFifo::getRst()
{
  return this->getPort("Clear_in")[0];
}
VHDLInterface::Port* BRAMFifo::getRClk()
{
  return this->getPort("RClk")[0];
}
VHDLInterface::Port* BRAMFifo::getEmptyOut()
{
  return this->getPort("Empty_out")[0];
}
VHDLInterface::Port* BRAMFifo::getReadEnIn()
{
  return this->getPort("ReadEn_in")[0];
}
VHDLInterface::Port* BRAMFifo::getDataOut()
{
  return this->getPort("Data_out")[0];
}
VHDLInterface::Port* BRAMFifo::getWClk()
{
  return this->getPort("WClk")[0];
}
VHDLInterface::Port* BRAMFifo::getFullOut()
{
  return this->getPort("Full_out")[0];
}
VHDLInterface::Port* BRAMFifo::getWriteEnIn()
{
  return this->getPort("WriteEn_in")[0];
}
VHDLInterface::Port* BRAMFifo::getDataIn()
{
  return this->getPort("Data_in")[0];
}
ComponentDefinition* BRAMFifo::getInstantiation(VHDLInterface::Entity* e)
{
  std::stringstream ss;
  ss << "fifo" << this;
  ComponentDefinition* ret = e->createComponent(ss.str(), this);
  ret->mapGeneric(VHDLInterface::IntegerTypedValue::get(impl->data_depth), "DATA_DEPTH");
  return ret;
}
