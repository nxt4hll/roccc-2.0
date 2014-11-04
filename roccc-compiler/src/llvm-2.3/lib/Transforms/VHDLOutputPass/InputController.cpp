// The ROCCC Compiler Infrastructure 
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

/*

This pass generates the component that handles the timing of all inputs.
In particular, this pass manages generating addresses for all input streams,
windows the streams so as to reuse elements as possible, generates code
for the loadPrevious, and times everything so that data is only pushed onto
the datapath when all incoming elements are ready.  
   
*/

#include "rocccLibrary/passes/InputController.h"

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
#include "rocccLibrary/FunctionType.h"
#include "rocccLibrary/DefinitionInst.h"
#include "rocccLibrary/FileInfo.h"
#include "rocccLibrary/DatabaseHelpers.h"
#include "rocccLibrary/VHDLComponents/BRAMFifo.h"
#include "rocccLibrary/VHDLComponents/InputSmartBuffer.h"
#include "rocccLibrary/VHDLComponents/ShiftBuffer.h"
#include "rocccLibrary/VHDLComponents/LoopInductionVariableHandler.h"

using namespace llvm ;
using namespace VHDLInterface;
char InputControllerPass::ID = 0 ;

static RegisterPass<InputControllerPass> X("inputController",
					  "Generate the ROCCC input control") ;

InputControllerPass::InputControllerPass() : FunctionPass((intptr_t)&ID)//, done_internal(NULL), pop(NULL)
{
}

InputControllerPass::~InputControllerPass()
{
}

//used when sorting the buffer values for n-D arrays in ascending order
namespace InputControllerPassNS {
bool compare_nD(std::pair<llvm::Value*,std::vector<int> > a, std::pair<llvm::Value*,std::vector<int> > b)
{
  return( a.second < b.second );
}
}

/*
The number of incoming memory channels is the number of values
that will get pushed from the outside for one valid signal. This value
is used by the InputProcess to control how many values we push onto the shift
buffers.

Because every valid signal means there are N values ready to be read, 
N needs to be both a factor of the window size, and of the step window size.
This can easily be just the window size divided by the window's x size, ie the
height of the window, since both the step window and the main window will only
differ in length along the x axis.
*/
int getNumAddressChannels(llvm::Value* v)
{
  assert( v );
  static std::map<llvm::Value*, int> requestMap;
  //if this is the first time through the map, fill the map with the values we
  //  find from the NumberOfIncomingMemoryWords calls
  if( requestMap.empty() )
  {
    if(Instruction* II = dynamic_cast<Instruction*>(v))
    {
      Function* f = II->getParent()->getParent();
      for(Function::iterator BB = f->begin(); BB != f->end(); ++BB)
      {
        for(BasicBlock::iterator II = BB->begin(); II != BB->end(); ++II)
        {
          if(CallInst* CI = dynamic_cast<CallInst*>(&*II))
          {
            if( isROCCCFunctionCall(CI, ROCCCNames::NumberOfAddressChannels) )
            {
              assert( CI->getNumOperands() == 3 );
              llvm::ConstantInt* constInt = dynamic_cast<llvm::ConstantInt*>( CI->getOperand(1));
              assert( constInt and "Number of address channels must be a constant int!" );
              requestMap[CI->getOperand(2)] = constInt->getValue().getSExtValue();
            }
          }
        }
      }
    }
  }
  if( requestMap.find(v) == requestMap.end() )
  {
    INTERNAL_WARNING("No " << ROCCCNames::NumberOfAddressChannels << " found for stream " << v->getName() << "! Using default (1)!\n");
    requestMap[v] = 1;
  }
  return requestMap.find(v)->second;
}
int getNumDataChannels(llvm::Value* v)
{
  assert( v );
  static std::map<llvm::Value*, int> requestMap;
  //if this is the first time through the map, fill the map with the values we
  //  find from the NumberOfIncomingMemoryWords calls
  if( requestMap.empty() )
  {
    if(Instruction* II = dynamic_cast<Instruction*>(v))
    {
      Function* f = II->getParent()->getParent();
      for(Function::iterator BB = f->begin(); BB != f->end(); ++BB)
      {
        for(BasicBlock::iterator II = BB->begin(); II != BB->end(); ++II)
        {
          if(CallInst* CI = dynamic_cast<CallInst*>(&*II))
          {
            if( isROCCCFunctionCall(CI, ROCCCNames::NumberOfDataChannels) )
            {
              assert( CI->getNumOperands() == 3 );
              llvm::ConstantInt* constInt = dynamic_cast<llvm::ConstantInt*>( CI->getOperand(1));
              assert( constInt and "Number of incoming data channels must be a constant int!" );
              requestMap[CI->getOperand(2)] = constInt->getValue().getSExtValue();
            }
          }
        }
      }
    }
  }
  if( requestMap.find(v) == requestMap.end() )
  {
    INTERNAL_WARNING("No " << ROCCCNames::NumberOfDataChannels << " found for stream " << v->getName() << "! Using default (1)!\n");
    requestMap[v] = 1;
  }
  return requestMap.find(v)->second;
}

class CounterProcess : public Process {
  VHDLInterface::Statement* steadyState;
protected:
  virtual std::string generateSteadyState(int level) //from Process
  {
    std::stringstream ss;
    ss << steadyState->generateCode(level);
    return ss.str();
  }
public:
  CounterProcess(Entity* e) : Process(e), steadyState(NULL) {}
  void initialize(VHDLInterface::Signal* counter, int maxValue, int increment)
  {
    VHDLInterface::AssignmentStatement* counterAssign = new VHDLInterface::AssignmentStatement( counter, this );
    counterAssign->addCase( VHDLInterface::ConstantInt::get(0), (Wrap(counter) >= VHDLInterface::ConstantInt::get(maxValue)) );
    counterAssign->addCase( Wrap(counter) + VHDLInterface::ConstantInt::get(increment) );
    steadyState = new VHDLInterface::IfStatement(this, Wrap(getParent()->getStandardPorts().stall) != VHDLInterface::ConstantInt::get(1), counterAssign);
  }
};

//FIXME- located in OutputController.cpp
llvm::Value* getLastCopiedValue(llvm::Value* v);

std::pair<bool,int> getFeedbackLengthIfItExists(llvm::Function* df)
{
  unsigned int feedbackLength = 0;
  bool has_feedback = false;
  //find the systolicPrevious
  std::map<llvm::Value*,llvm::CallInst*> systolicFeedbackMap;
  for ( Function::iterator BB = df->begin(); BB != df->end(); ++BB )
  {
    for ( BasicBlock::iterator II = BB->begin(); II != BB->end(); ++II )
    {
      llvm::CallInst* CI = dynamic_cast<llvm::CallInst*>(&*II);
      if( isROCCCFunctionCall(CI, ROCCCNames::SystolicPrevious) )
      {
        assert(CI->getNumOperands() == 3) ;
        llvm::Value* source = CI->getOperand(2);
        systolicFeedbackMap[source] = CI;
      }
    }
  }
  //find the matching systolicNext
  for ( Function::iterator BB2 = df->begin(); BB2 != df->end(); ++BB2 )
  {
    for ( BasicBlock::iterator II2 = BB2->begin(); II2 != BB2->end(); ++II2 )
    {
      llvm::CallInst* CI2 = dynamic_cast<llvm::CallInst*>(&*II2);
      if( isROCCCFunctionCall(CI2, ROCCCNames::SystolicNext) )
      {
        assert(CI2->getNumOperands() == 3);
        llvm::Value* source = CI2->getOperand(1);
        source = getLastCopiedValue(source);
        assert( systolicFeedbackMap.find(source) != systolicFeedbackMap.end() );
        int prevLevel = systolicFeedbackMap[source]->getParent()->getDFBasicBlock()->getPipelineLevel();
        llvm::Instruction* defIncInst = getDefinitionInstruction(dynamic_cast<llvm::Instruction*>(CI2->getOperand(2)), CI2->getParent());
        assert(defIncInst);
        int nextLevel = defIncInst->getParent()->getDFBasicBlock()->getPipelineLevel()-1;
          unsigned curLength = prevLevel - nextLevel;
        if( !has_feedback )
        {
          has_feedback = true;
          feedbackLength = curLength;
        }
        feedbackLength = std::max(feedbackLength, curLength);
      }
    }
  }
  //now find the longest lut path
  std::map<llvm::Value*,llvm::CallInst*> lutAccessMaxLevelMap;
  std::map<llvm::Value*,llvm::CallInst*> lutAccessMinLevelMap;
  for ( Function::iterator BB = df->begin(); BB != df->end(); ++BB )
  {
    for ( BasicBlock::iterator II = BB->begin(); II != BB->end(); ++II )
    {
      llvm::CallInst* CI = dynamic_cast<llvm::CallInst*>(&*II);
      if( isROCCCFunctionCall(CI, ROCCCNames::LUTRead) or
          isROCCCFunctionCall(CI, ROCCCNames::LUTWrite) )
      {
        assert(CI->getNumOperands() >= 2);
        llvm::Value* source = CI->getOperand(1);
        if( !lutAccessMaxLevelMap[source] )
        {
          lutAccessMaxLevelMap[source] = CI;
        }
        if( !lutAccessMinLevelMap[source] )
        {
          lutAccessMinLevelMap[source] = CI;
        }
        int old_max_level = lutAccessMaxLevelMap[source]->getParent()->getDFBasicBlock()->getPipelineLevel();
        int old_min_level = lutAccessMinLevelMap[source]->getParent()->getDFBasicBlock()->getPipelineLevel();
        int cur_level = CI->getParent()->getDFBasicBlock()->getPipelineLevel();
        if( cur_level > old_max_level and cur_level >= 0 )
        {
          old_max_level = cur_level;
          lutAccessMaxLevelMap[source] = CI;
        }
        if( cur_level < old_min_level and cur_level >= 0 )
        {
          old_min_level = cur_level;
          lutAccessMinLevelMap[source] = CI;
        }
        unsigned curLength = old_max_level - old_min_level + 1;
        if( !has_feedback )
        {
          has_feedback = true;
          feedbackLength = curLength;
        }
        feedbackLength = std::max(feedbackLength, curLength);
      }
    }
  }
  assert( (!has_feedback or feedbackLength > 0) and "Malformed feedbacks!" );
  return std::pair<bool,int>(has_feedback,feedbackLength);
}

int getNumOutstandingMemoryRequests(llvm::Value* v)
{
  assert( v );
  static std::map<llvm::Value*, int> requestMap;
  //if this is the first time through the map, fill the map with the values we
  //  find from the NumberOfSimultaneousMemoryRequests calls
  if( requestMap.empty() )
  {
    if(Instruction* II = dynamic_cast<Instruction*>(v))
    {
      Function* f = II->getParent()->getParent();
      for(Function::iterator BB = f->begin(); BB != f->end(); ++BB)
      {
        for(BasicBlock::iterator II = BB->begin(); II != BB->end(); ++II)
        {
          if(CallInst* CI = dynamic_cast<CallInst*>(&*II))
          {
            if( isROCCCFunctionCall(CI, ROCCCNames::NumberOfOutstandingMemoryRequests) )
            {
              assert( CI->getNumOperands() == 3 );
              llvm::ConstantInt* constInt = dynamic_cast<llvm::ConstantInt*>( CI->getOperand(1));
              assert( constInt and "Number of outstanding memory requests must be a constant int!" );
              requestMap[CI->getOperand(2)] = constInt->getValue().getSExtValue();
            }
          }
        }
      }
    }
  }
  if( requestMap.find(v) == requestMap.end() )
  {
    INTERNAL_WARNING("No " << ROCCCNames::NumberOfOutstandingMemoryRequests << " found for stream " << v->getName() << "! Using default (1)!\n");
    requestMap[v] = 1;
  }
  return requestMap.find(v)->second;
}

bool InputControllerPass::runOnFunction(Function& f)
{
  CurrentFile::set(__FILE__);
  
  // Make sure this is a function that we can use
  if (f.isDeclaration() || !f.isDFFunction() )
  {
    return false ;
  }
  DFFunction* df = f.getDFFunction() ;

  // Only functions with inputs need a input controller
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
  outputFileName += "_InputController.vhdl" ;
  std::string outputFullPath = outputDirectory + outputFileName;
  std::ofstream fout;
  fout.open(outputFullPath.c_str(), std::ios_base::out) ;
  if (!fout)
  {
    llvm::cerr << "Cannot open \'" << outputFullPath << "\' for input controller output!" << std::endl ;
    assert(0 and "Could not open critical file") ;
  }
  llvm::cout << "Writing input controller to \'" << outputFullPath << "\'\n";
  Database::FileInfoInterface::addFileInfo(Database::getCurrentID(), Database::FileInfo(outputFileName, Database::FileInfo::VHDL_INPUTCONTROLLER, outputDirectory));
  LOG_MESSAGE2("VHDL Generation", "InputController", "InputController written to <a href=\'" << outputFullPath << "\'>" << outputFileName << "</a>.\n");
  
  //setup the vhdl interface
  inputEntity = new Entity(df->getName() + "_InputController");
  InputSmartBuffer isb;
  ROCCCLoopInformation loopInfo = df->getROCCCLoopInfo();
  std::vector<VHDLInterface::Variable*> dat_out;
  for(std::vector<Value*>::iterator BI = loopInfo.inputBuffers.begin(); BI != loopInfo.inputBuffers.end(); ++BI)
  {
    //create an inputstream struct so that any classes that instantiate the inputcontroller will be able to find the streams connected
    InputStream stream;
    stream.stream = new StreamVariable();
    stream.stream->readableName = getValueName(*BI);
    stream.stream->value = *BI;
    //create the bram fifo that will be feeding the stream of the inputsmartbuffer component
    VHDLInterface::BRAMFifo* bramDecl = new VHDLInterface::BRAMFifo(getSizeInBits(*BI) * getNumDataChannels(*BI), 512);
    ComponentDefinition* bramDef = bramDecl->getInstantiation(inputEntity);
    inputEntity->mapPortToSubComponentPort(inputEntity->getStandardPorts().rst, bramDef, bramDecl->getRst());
    //connect the writing side of the fifo to the outside
    inputEntity->mapPortToSubComponentPort(inputEntity->addPort(getValueName(*BI)+"_WClk", 1, VHDLInterface::Port::INPUT), bramDef, bramDecl->getWClk());
    stream.stream->cross_clk = inputEntity->getVariableMappedTo(bramDef, bramDecl->getWClk());
    inputEntity->mapPortToSubComponentPort(inputEntity->addPort(getValueName(*BI)+"_full", 1, VHDLInterface::Port::OUTPUT), bramDef, bramDecl->getFullOut());
    stream.stream->stop_access = inputEntity->getVariableMappedTo(bramDef, bramDecl->getFullOut());
    inputEntity->mapPortToSubComponentPort(inputEntity->addPort(getValueName(*BI)+"_writeEn", 1, VHDLInterface::Port::INPUT), bramDef, bramDecl->getWriteEnIn());
    stream.stream->enable_access = inputEntity->getVariableMappedTo(bramDef, bramDecl->getWriteEnIn());
    for(int count = 0; count < getNumDataChannels(*BI); ++count)
    {
      //and also the fifo data in
      VHDLInterface::Variable* val = inputEntity->getVariableMappedTo(bramDef, bramDecl->getDataIn());
      val = VHDLInterface::BitRange::get(val, (count+1) * getSizeInBits(*BI) - 1, count * getSizeInBits(*BI));
      std::stringstream ss;
      ss << getValueName(*BI)+"_data_channel" << count;
      VHDLInterface::Port* channel = inputEntity->addPort(ss.str(), getSizeInBits(*BI), VHDLInterface::Port::INPUT);
      inputEntity->createSynchronousStatement(val, channel);
      stream.stream->data_channels.push_back(channel);
    }
    //also connect the address ports to the outside
    stream.stream->address_stall = inputEntity->addPort(getValueName(*BI)+"_address_stall", 1, VHDLInterface::Port::INPUT);
    stream.stream->address_rdy = inputEntity->addPort(getValueName(*BI)+"_address_rdy", 1, VHDLInterface::Port::OUTPUT);
    for(int c = 0; c < getNumAddressChannels(*BI); ++c)
    {
      std::stringstream ss;
      ss << getValueName(*BI)+"_address_channel" << c;
      stream.stream->address_channels.push_back(
                    std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*>(
                             inputEntity->addPort(ss.str()+"_base", 32, VHDLInterface::Port::OUTPUT),
                             inputEntity->addPort(ss.str()+"_count", 32, VHDLInterface::Port::OUTPUT)
                         )
                    );
    }
    stream.stream->address_clk = inputEntity->addPort(getValueName(*BI)+"_address_clk", 1, VHDLInterface::Port::INPUT);
    isb.setAddressVariables(*BI,
                    stream.stream->address_rdy,
                    stream.stream->address_stall,
                    stream.stream->address_channels,
                    stream.stream->address_clk);
    //then set the output of the fifo to the smartbuffer
    inputEntity->mapPortToSubComponentPort(inputEntity->getStandardPorts().clk, bramDef, bramDecl->getRClk());
    std::vector<VHDLInterface::Variable*> fifo_data_outs;
    for(int count = 0; count < getNumDataChannels(*BI); ++count)
    {
      //map the fifo data out
      VHDLInterface::Variable* val = inputEntity->getVariableMappedTo(bramDef, bramDecl->getDataOut());
      val = VHDLInterface::BitRange::get(val, (count+1) * getSizeInBits(*BI) - 1, count * getSizeInBits(*BI));
      fifo_data_outs.push_back(val);
    }
    isb.initializeInputInterfacePorts(*BI,
                    inputEntity->getVariableMappedTo(bramDef, bramDecl->getReadEnIn()),
                    inputEntity->getVariableMappedTo(bramDef, bramDecl->getEmptyOut()),
                    fifo_data_outs);
    for(std::vector<std::pair<Value*,std::vector<int> > >::iterator BII = loopInfo.inputBufferIndexes[*BI].begin(); BII != loopInfo.inputBufferIndexes[*BI].end(); ++BII)
    {
      dat_out.push_back(inputEntity->addPort(getValueName(BII->first), getSizeInBits(BII->first), VHDLInterface::Port::OUTPUT, BII->first));
    }
    this->input_streams.push_back(stream);
  }
  {//create the standard fifo reader
    VHDLInterface::Variable* inputValueValid = inputEntity->getSignalMappedToPort(inputEntity->getDeclaration()->getStandardPorts().outputReady);
    VHDLInterface::Variable* outputStallIn = inputEntity->getStandardPorts().stall;
    VHDLInterface::Signal* readEnable = inputEntity->createSignal<VHDLInterface::Signal>("inputController_read_enable_t", 1);
    VHDLInterface::Signal* empty = inputEntity->createSignal<VHDLInterface::Signal>("inputController_empty_t", 1);
    VHDLInterface::AssignmentStatement* read_ass = inputEntity->createSynchronousStatement(readEnable);
    read_ass->addCase(VHDLInterface::ConstantInt::get(0), VHDLInterface::Wrap(outputStallIn) == VHDLInterface::ConstantInt::get(1));
    read_ass->addCase(VHDLInterface::ConstantInt::get(0), VHDLInterface::Wrap(empty) == VHDLInterface::ConstantInt::get(1));
    //if there are any systolics, make a counter and only read when the counter is at 0
    unsigned systolicLength = getFeedbackLengthIfItExists(df).second;
    bool has_systolics = getFeedbackLengthIfItExists(df).first;
    if( has_systolics )
    {
      Signal* counter = inputEntity->createSignal<Signal>("counter", 32);
      CounterProcess* counterProcess = inputEntity->createProcess<CounterProcess>();
      counterProcess->initialize(counter, systolicLength, 1);
      LOG_MESSAGE2("VHDL Generation", "InputController", "Initialization interval restricted to " << systolicLength << " due to feedback in the datapath.\n");
      read_ass->addCase( VHDLInterface::ConstantInt::get(0), Wrap(counter) != VHDLInterface::ConstantInt::get(0) );
    }
    read_ass->addCase(VHDLInterface::ConstantInt::get(1));
    ShiftBuffer sb("inputController_outputShiftBuffer");
    sb.mapDataWidth(1);
    sb.mapShiftDepth(1);
    sb.mapRst(inputEntity->getStandardPorts().rst);
    sb.mapClk(inputEntity->getStandardPorts().clk);
    sb.mapDataIn(readEnable);
    sb.mapDataOut(inputValueValid);
    sb.generateCode(inputEntity);
    //set the output ports of the smartbuffer
    isb.initializeOutputInterfacePorts(0, 
                  readEnable,
                  empty,
                  dat_out);
  }
  //generate vhdl
  isb.generateVHDL(inputEntity, loopInfo);
  //create a process for the datapath LIV variables
  LoopInductionVariableHandler livHandler;
  livHandler.setIndexes(loopInfo.indexes);
  livHandler.setChildren(loopInfo.loopChildren);
  livHandler.setEndValues(loopInfo.endValues);
  for(std::vector<llvm::Value*>::iterator LIVI = loopInfo.indexes.begin(); LIVI != loopInfo.indexes.end(); ++LIVI)
  {
    VHDLInterface::Variable* new_endValue = NULL;
    if( livHandler.getEndValueVHDLValue(*LIVI) == NULL and not livHandler.isEndValueInfinity(*LIVI) )
    {
      llvm::Value* ev = loopInfo.endValues[*LIVI];
      if( inputEntity->getPort(ev).size() == 0 )
        inputEntity->addPort(getValueName(ev), getSizeInBits(ev), VHDLInterface::Port::INPUT, ev);
      new_endValue = inputEntity->getPort(ev).at(0);
    }
    VHDLInterface::Signal* sig = inputEntity->createSignal<VHDLInterface::Signal>("datapath_"+getValueName(*LIVI),getSizeInBits(*LIVI));
    inputEntity->createSynchronousStatement(inputEntity->addPort(getValueName(*LIVI),getSizeInBits(*LIVI),VHDLInterface::Port::OUTPUT,*LIVI), sig);
    livHandler.setVHDLInterface(*LIVI, sig, new_endValue);
  }
  VHDLInterface::Signal* datapath_process_done = inputEntity->createSignal<VHDLInterface::Signal>("datapath_process_done", 1);
  VHDLInterface::MultiStatementProcess* livProcess = inputEntity->createProcess<VHDLInterface::MultiStatementProcess>();
  VHDLInterface::CWrap outputReadyCondition = VHDLInterface::Wrap(inputEntity->getSignalMappedToPort(inputEntity->getDeclaration()->getStandardPorts().outputReady)) == VHDLInterface::ConstantInt::get(1);
  livProcess->addStatement(new VHDLInterface::IfStatement(livProcess, outputReadyCondition, livHandler.getIncrementStatement(livProcess)));
  livProcess->addStatement(new VHDLInterface::IfStatement(livProcess, outputReadyCondition and livHandler.getDoneCondition(), new VHDLInterface::AssignmentStatement(datapath_process_done, VHDLInterface::ConstantInt::get(1), livProcess)));
  //map the done port
  VHDLInterface::AssignmentStatement* done_assign = inputEntity->createSynchronousStatement(inputEntity->getDeclaration()->getStandardPorts().done);
  //done_assign->addCase(VHDLInterface::ConstantInt::get(1), livHandler.getDoneCondition());
  done_assign->addCase(VHDLInterface::ConstantInt::get(1), VHDLInterface::Wrap(datapath_process_done) == VHDLInterface::ConstantInt::get(1));
  done_assign->addCase(VHDLInterface::ConstantInt::get(0));
  
  //finally, generate the inputcontroller entity code
  fout << inputEntity->generateCode();
  
  // I shouldn't have changed anything
  return false ;
}

void InputControllerPass::getAnalysisUsage(AnalysisUsage& AU) const
{
  AU.setPreservesAll();
}
