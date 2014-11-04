// The ROCCC Compiler Infrastructure 
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*

This pass generates the component that handles the timing of all outputs.
In particular, this pass manages generating addresses for all output streams,
windows the streams so as to reuse elements as possible, and times everything
so that data is only taken from the datapath when all outgoing streams
are ready.
   
*/

#include "rocccLibrary/passes/OutputController.h"

#include "llvm/Constants.h"

#include <sstream>
#include <algorithm>

#include "rocccLibrary/DFBasicBlock.h"
#include "rocccLibrary/GetValueName.h"
#include "rocccLibrary/SizeInBits.h"
#include "rocccLibrary/MultiForVar.hpp"
#include "rocccLibrary/InternalWarning.h"
#include "rocccLibrary/MessageLogger.h"
#include "rocccLibrary/ROCCCNames.h"
#include "rocccLibrary/CopyValue.h"
#include "rocccLibrary/FunctionType.h"
#include "rocccLibrary/FileInfo.h"
#include "rocccLibrary/DatabaseHelpers.h"
#include "rocccLibrary/VHDLComponents/BRAMFifo.h"
#include "rocccLibrary/VHDLComponents/MicroFifo.h"
#include "rocccLibrary/VHDLComponents/OutputSmartBuffer.h"
#include "rocccLibrary/VHDLComponents/ShiftBuffer.h"

using namespace llvm ;
using namespace VHDLInterface;
char OutputControllerPass::ID = 0 ;

static RegisterPass<OutputControllerPass> X("outputController",
					  "Generate the ROCCC output control") ;

llvm::Value* getLastCopiedValue(llvm::Value* v)
{
  if(!v)
    return v;
  //check if any of the successors are copy statements, and if so, return that instead
  for(llvm::Value::use_iterator UI = v->use_begin(); UI != v->use_end(); ++UI)
  {
    if( isCopyValue(*UI) )
      return getLastCopiedValue(*UI);
  }
  return v;
}

//DEFINED IN InputController.cpp - @TODO - lump these together
int getNumDataChannels(llvm::Value* v);
int getNumAddressChannels(llvm::Value* v);
int getNumOutstandingMemoryRequests(llvm::Value* v);

OutputControllerPass::OutputControllerPass() : FunctionPass((intptr_t)&ID)//, stall_internal(NULL), outputEntity(NULL)
{
}

OutputControllerPass::~OutputControllerPass()
{
}

void OutputControllerPass::getAnalysisUsage(AnalysisUsage& AU) const
{
  AU.setPreservesAll();
}

bool OutputControllerPass::runOnFunction(Function& f)
{
  CurrentFile::set(__FILE__);
  // Make sure this is a function that we can use
  if (f.isDeclaration() || !f.isDFFunction() )
  {
    return false ;
  }
  DFFunction* df = f.getDFFunction() ;

  // Only functions with outputs need a output controller
  if (df->getFunctionType() != ROCCC::MODULE)
  {
    return false ;
  }
  //output the generated entity
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
  outputFileName = f.getName() ;
  outputFileName += "_OutputController.vhdl" ;
  std::string outputFullPath = outputDirectory + outputFileName;
  std::ofstream fout;
  fout.open(outputFullPath.c_str(), std::ios_base::out) ;
  if (!fout)
  {
    llvm::cerr << "Cannot open \'" << outputFullPath << "\' for output controller output!" << std::endl ;
    assert(0 and "Could not open critical file") ;
  }
  llvm::cout << "Writing output controller to \'" << outputFullPath << "\'\n";
  Database::FileInfoInterface::addFileInfo(Database::getCurrentID(), Database::FileInfo(outputFileName, Database::FileInfo::VHDL_OUTPUTCONTROLLER, outputDirectory));
  LOG_MESSAGE2("VHDL Generation", "OutputController", "OutputController written to <a href=\'" << outputFullPath << "\'>" << outputFileName << "</a>.\n");
  
  //setup the vhdl interface
  outputEntity = new Entity(df->getName() + "_OutputController");
  OutputSmartBuffer osb;
  ROCCCLoopInformation loopInfo = df->getROCCCLoopInfo();
  //first, create a micro fifo that takes the input from the datapath and fifos it. To do this,
  //   we need to create the datapath ports.
  std::vector<VHDLInterface::Variable*> dat_in;
  std::vector<VHDLInterface::Variable*> dat_in_fifoed;
  for(std::vector<Value*>::iterator BI = loopInfo.outputBuffers.begin(); BI != loopInfo.outputBuffers.end(); ++BI)
  {
    for(std::vector<std::pair<Value*,std::vector<int> > >::iterator BII = loopInfo.outputBufferIndexes[*BI].begin(); BII != loopInfo.outputBufferIndexes[*BI].end(); ++BII)
    {
      dat_in.push_back(outputEntity->addPort(getValueName(getLastCopiedValue(BII->first)), getSizeInBits(getLastCopiedValue(BII->first)), VHDLInterface::Port::INPUT, getLastCopiedValue(BII->first)));
      dat_in_fifoed.push_back(outputEntity->createSignal<VHDLInterface::Signal>(getValueName(BII->first)+"_fifoed", getSizeInBits(BII->first), BII->first));
    }
  }
  if( loopInfo.outputBuffers.size() > 0 ) //if we have output streams, we need to do this
  {
    //create the bram fifo that will be fed from the datapath
    //and then connect it to the input ports of the output smart buffer
    MicroFifo mf("outputController_datapath_micro_fifo");
    mf.mapAddressWidth(3);
    mf.mapAlmostFullCount(5);
    mf.mapAlmostEmptyCount(0);
    mf.mapClk(outputEntity->getStandardPorts().clk);
    mf.mapRst(outputEntity->getStandardPorts().rst);
    mf.mapValidIn(outputEntity->getStandardPorts().inputReady);
    //need to create the stall_internal port
    this->stall_internal = outputEntity->addPort("stall_internal", 1, VHDLInterface::Port::OUTPUT);
    mf.mapFullOut(this->stall_internal);
    VHDLInterface::Variable* read_en = outputEntity->createSignal<VHDLInterface::Signal>("outputController_datapath_micro_fifo0_read_enable", 1);
    mf.mapReadEnableIn(read_en);
    VHDLInterface::Variable* empty = outputEntity->createSignal<VHDLInterface::Signal>("outputController_datapath_micro_fifo0_empty", 1);
    mf.mapEmptyOut(empty);
    mf.mapInputAndOutputVector(dat_in, dat_in_fifoed, outputEntity);
    mf.generateCode(outputEntity);
    osb.initializeInputInterfacePorts(0,
                    read_en,
                    empty,
                    dat_in_fifoed);
  }
  else //we dont have output streams, still need to create stall_internal port!
  {
    //need to create the stall_internal port
    this->stall_internal = outputEntity->addPort("stall_internal", 1, VHDLInterface::Port::OUTPUT);
    outputEntity->createSynchronousStatement(this->stall_internal, outputEntity->getDeclaration()->getStandardPorts().stall);
  }
  //now, create the streams that the output smart buffer will be outputting
  for(std::vector<Value*>::iterator BI = loopInfo.outputBuffers.begin(); BI != loopInfo.outputBuffers.end(); ++BI)
  {
  //create an outputstream struct so that any classes that instantiate the outputcontroller will be able to find the streams connected
    OutputStream stream;
    stream.stream = new StreamVariable();
    stream.stream->readableName = getValueName(*BI);
    stream.stream->value = *BI;
    //create the BRAM fifo
    VHDLInterface::BRAMFifo* bramDecl = new VHDLInterface::BRAMFifo(getSizeInBits(*BI) * getNumDataChannels(*BI), 512);
    ComponentDefinition* bramDef = bramDecl->getInstantiation(outputEntity);
    outputEntity->mapPortToSubComponentPort(outputEntity->getStandardPorts().rst, bramDef, bramDecl->getRst());
    //connect the reading side of the fifo to the outside
    outputEntity->mapPortToSubComponentPort(outputEntity->addPort(getValueName(*BI)+"_RClk", 1, VHDLInterface::Port::INPUT), bramDef, bramDecl->getRClk());
    stream.stream->cross_clk = outputEntity->getVariableMappedTo(bramDef, bramDecl->getRClk());
    outputEntity->mapPortToSubComponentPort(outputEntity->addPort(getValueName(*BI)+"_empty", 1, VHDLInterface::Port::OUTPUT), bramDef, bramDecl->getEmptyOut());
    stream.stream->stop_access = outputEntity->getVariableMappedTo(bramDef, bramDecl->getEmptyOut());
    outputEntity->mapPortToSubComponentPort(outputEntity->addPort(getValueName(*BI)+"_read", 1, VHDLInterface::Port::INPUT), bramDef, bramDecl->getReadEnIn());
    stream.stream->enable_access = outputEntity->getVariableMappedTo(bramDef, bramDecl->getReadEnIn());
    //connect the writing side internally; create the fifo reading shiftbuffer
    outputEntity->mapPortToSubComponentPort(outputEntity->getStandardPorts().clk, bramDef, bramDecl->getWClk());
    //read from the output smart buffer's buffer
    VHDLInterface::Variable* inputValueValid = outputEntity->getVariableMappedTo(bramDef, bramDecl->getWriteEnIn());
    VHDLInterface::Variable* outputFullIn = outputEntity->getVariableMappedTo(bramDef, bramDecl->getFullOut());
    VHDLInterface::Signal* inputEmptyIn = outputEntity->createSignal<VHDLInterface::Signal>(getValueName(*BI)+"_fiforead_empty", 1);
    VHDLInterface::Signal* readEnable = outputEntity->createSignal<VHDLInterface::Signal>(getValueName(*BI)+"_fiforead_read_enable", 1);
    {//none of this needs to be exposed to the rest of the body
      VHDLInterface::AssignmentStatement* read_ass = outputEntity->createSynchronousStatement(readEnable);
      read_ass->addCase(VHDLInterface::ConstantInt::get(0), VHDLInterface::Wrap(outputFullIn) == VHDLInterface::ConstantInt::get(1) or VHDLInterface::Wrap(inputEmptyIn) == VHDLInterface::ConstantInt::get(1));
      read_ass->addCase(VHDLInterface::ConstantInt::get(1));
      ShiftBuffer sb(getValueName(*BI)+"_output_fiforead_shiftbuffer");
      sb.mapDataWidth(1);
      sb.mapShiftDepth(1);
      sb.mapRst(outputEntity->getStandardPorts().rst);
      sb.mapClk(outputEntity->getStandardPorts().clk);
      sb.mapDataIn(readEnable);
      sb.mapDataOut(inputValueValid);
      sb.generateCode(outputEntity);
    }
    //base this off of number of incoming data channels?
    std::vector<VHDLInterface::Variable*> dat;
    for(int count = 0; count < getNumDataChannels(*BI); ++count)
    {
      //map the fifo data in
      VHDLInterface::Variable* val = outputEntity->getVariableMappedTo(bramDef, bramDecl->getDataIn());
      val = VHDLInterface::BitRange::get(val, (count+1) * getSizeInBits(*BI) - 1, count * getSizeInBits(*BI));
      dat.push_back(val);
      //and also the fifo data out
      val = outputEntity->getVariableMappedTo(bramDef, bramDecl->getDataOut());
      val = VHDLInterface::BitRange::get(val, (count+1) * getSizeInBits(*BI) - 1, count * getSizeInBits(*BI));
      std::stringstream ss;
      ss << getValueName(*BI)+"_data_channel" << count;
      VHDLInterface::Port* channel = outputEntity->addPort(ss.str(), getSizeInBits(*BI), VHDLInterface::Port::OUTPUT);
      outputEntity->createSynchronousStatement(channel, val);
      stream.stream->data_channels.push_back(channel);
    }
    osb.initializeOutputInterfacePorts(*BI,
                           readEnable,
                           inputEmptyIn,
                           dat
                                     );
    //also connect the address ports to the outside
    stream.stream->address_stall = outputEntity->addPort(getValueName(*BI)+"_address_stall", 1, VHDLInterface::Port::INPUT);
    stream.stream->address_rdy = outputEntity->addPort(getValueName(*BI)+"_address_rdy", 1, VHDLInterface::Port::OUTPUT);
    for(int c = 0; c < getNumAddressChannels(*BI); ++c)
    {
      std::stringstream ss;
      ss << getValueName(*BI)+"_address_channel" << c;
      stream.stream->address_channels.push_back(
                    std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*>(
                             outputEntity->addPort(ss.str()+"_base", 32, VHDLInterface::Port::OUTPUT),
                             outputEntity->addPort(ss.str()+"_count", 32, VHDLInterface::Port::OUTPUT)
                         )
                    );
    }
    stream.stream->address_clk = outputEntity->addPort(getValueName(*BI)+"_address_clk", 1, VHDLInterface::Port::INPUT);
    osb.setAddressVariables(*BI,
                    stream.stream->address_rdy,
                    stream.stream->address_stall,
                    stream.stream->address_channels,
                    stream.stream->address_clk);
    this->output_streams.push_back(stream);
  }
  //map the outputReady port
  outputEntity->createSynchronousStatement(outputEntity->getDeclaration()->getStandardPorts().outputReady, VHDLInterface::ConstantInt::get(0));
  osb.generateVHDL(outputEntity, loopInfo);
  fout << outputEntity->generateCode();
  
  return false;
}
