// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details

/*

  This file connects systems.

 */
#include "rocccLibrary/passes/SystemToSystemPass.h"

#include "llvm/Constants.h"

#include <vector>
#include <assert.h>
#include <sstream>
#include <map>

#include "rocccLibrary/SizeInBits.h"
#include "rocccLibrary/GetValueName.h"
#include "rocccLibrary/InternalWarning.h"
#include "rocccLibrary/ROCCCNames.h"
#include "rocccLibrary/DatabaseInterface.h"
#include "rocccLibrary/FunctionType.h"
#include "rocccLibrary/VHDLInterface.h"
#include "rocccLibrary/StreamVariable.h"
#include "rocccLibrary/FileInfo.h"
#include "rocccLibrary/DatabaseHelpers.h"
#include "rocccLibrary/MessageLogger.h"

using namespace llvm ;
using namespace VHDLInterface;
using namespace Database;

// The call to the constructor that registers this pass
char ROCCCSystemToSystemPass::ID = 0 ;

static RegisterPass<ROCCCSystemToSystemPass> X("systemToSystem", "SystemToSystem Pass") ;

ROCCCSystemToSystemPass::ROCCCSystemToSystemPass() : FunctionPass((intptr_t)&ID), entity(NULL)
{
}

ROCCCSystemToSystemPass::~ROCCCSystemToSystemPass() 
{
 // Nothing to clean up yet
}

bool isLLVMValueStream(llvm::Value* v)
{
  const llvm::Type* T = v->getType();
  return ( T->getTypeID() == Type::PointerTyID or T->getTypeID() == Type::ArrayTyID );
}

bool isOperandOfCallAnInput(CallInst* CI, User::op_iterator OP)
{
  if( isROCCCFunctionCall(CI, ROCCCNames::InputScalar) )
  {
    return false;
  }
  else if( isROCCCFunctionCall(CI, ROCCCNames::OutputScalar) )
  {
    return true;
  }
  else if( isROCCCFunctionCall(CI, ROCCCNames::InputStream) )
  {
    return false;
  }
  else if( isROCCCFunctionCall(CI, ROCCCNames::OutputStream) )
  {
    return true;
  }
  else if( isROCCCFunctionCall(CI, ROCCCNames::DebugScalarOutput) )
  {
    return true;
  }
  else if( isROCCCFunctionCall(CI, ROCCCNames::InvokeHardware) )
  {
    LibraryEntry currentEntry = DatabaseInterface::getInstance()->LookupEntry(getComponentNameFromCallInst(CI)) ;
    const std::list<Database::Port*> ports = currentEntry.getNonDebugScalarPorts();
    std::list<Database::Port*>::const_iterator PI = ports.begin();
    const std::list<Database::Stream> streams = currentEntry.getStreams();
    std::list<Database::Stream>::const_iterator SI = streams.begin();
    for(User::op_iterator OP2 = CI->op_begin()+2; OP2 != CI->op_end(); ++OP2)
    {
      if( OP == OP2 )
      {
        if( isLLVMValueStream(*OP2) )
        {
          assert(SI != streams.end() and "Too many streams!");
          return SI->isInput();
        }
        else
        {
          assert(PI != ports.end() and "Too many non-streams!");
          return !(*PI)->isOutput();
        }
      }
      if( isLLVMValueStream(*OP2) )
        assert(SI++ != streams.end() and "Too many streams!");
      else
        assert(PI++ != ports.end() and "Too many non-streams!");
    }
    return false;
  }
  else if( isROCCCFunctionCall(CI, ROCCCNames::FunctionType) or
           isROCCCFunctionCall(CI, ROCCCNames::VariableSize) or
           isROCCCFunctionCall(CI, ROCCCNames::VariableName) or
           isROCCCFunctionCall(CI, ROCCCNames::VariableSigned) or
           isROCCCFunctionCall(CI, ROCCCNames::NumberOfDataChannels) or
           isROCCCFunctionCall(CI, ROCCCNames::NumberOfAddressChannels) or
           isROCCCFunctionCall(CI, ROCCCNames::NumberOfOutstandingMemoryRequests) )
  {
    return false;
  }
  INTERNAL_WARNING("Cannot decide whether operand is input in " << *CI << "!\n");
  return false;
}
bool isOperandOfCallAnOutput(CallInst* CI, User::op_iterator OP)
{
  return !isOperandOfCallAnInput(CI, OP);
}

class LibraryComponentOwner : public VHDLInterface::ValueOwner {
public:
  virtual std::string getName(); //from ValueOwner
};

VHDLInterface::ComponentDeclaration* getDeclarationForComponent(CallInst* CI, VHDLInterface::Entity* e)
{
  LibraryEntry currentEntry = DatabaseInterface::getInstance()->LookupEntry(getComponentNameFromCallInst(CI)) ;
  VHDLInterface::ComponentDeclaration* cd = new VHDLInterface::ComponentDeclaration(currentEntry.getName());
  std::list<Database::Port*> p = currentEntry.getAllPorts();
  for( std::list<Database::Port*>::iterator PI = p.begin(); PI != p.end(); ++PI)
  {
    cd->addPort( (*PI)->getName(), (*PI)->getBitSize(), (*PI)->isOutput()?VHDLInterface::Port::OUTPUT:VHDLInterface::Port::INPUT, NULL, false );
  }
  for(std::vector<VHDLInterface::Port*>::iterator PI = cd->getPorts().begin(); PI != cd->getPorts().end(); ++PI)
  {
    static LibraryComponentOwner LIBRARY_COMPONENT_OWNER;
    LIBRARY_COMPONENT_OWNER.add(*PI);
  }
  return cd;
}

std::string getConstValuePrintableName(llvm::Value* V);

VHDLInterface::VWrap LLVMValueToVHDLValue(VHDLInterface::Entity* e, llvm::Value* v)
{
  if( e->findSignal(v).size() > 0 )
    return VHDLInterface::VWrap(e->findSignal(v).at(0));
  else if (dynamic_cast<Constant*>(v) != NULL)
  {
    llvm::ConstantInt* ci = dynamic_cast<llvm::ConstantInt*>(v) ;
    llvm::ConstantFP* cf = dynamic_cast<llvm::ConstantFP*>(v) ;
    std::string name = getConstValuePrintableName(v);
    if (ci != NULL)
    {
      VHDLInterface::Signal* val = new VHDLInterface::NamedConstant(name, static_cast<int>(ci->getValue().getSExtValue()), ci);
      e->addSignal(val);
      return VHDLInterface::VWrap(val);
    }
    else if (cf != NULL)
    {
      VHDLInterface::Signal* val = new VHDLInterface::NamedConstant(name,cf->getValueAPF().convertToFloat(), cf);
      e->addSignal(val);
      return VHDLInterface::VWrap(val);
    }
    else
    {
      assert(0) ;
    }
  }
  return VHDLInterface::VWrap(NULL);
}

const Database::Stream* getDatabaseStreamFromLLVMStream(llvm::Value* v)
{
  const Database::Stream* ret = NULL;
  for(llvm::Value::use_iterator UI = v->use_begin(); UI != v->use_end(); ++UI)
  {
    llvm::CallInst* CI = dynamic_cast<llvm::CallInst*>(*UI);
    if( isROCCCFunctionCall(CI, ROCCCNames::InvokeHardware) )
    {
      LibraryEntry currentEntry = DatabaseInterface::getInstance()->LookupEntry(getComponentNameFromCallInst(CI)) ;
      const std::list<Database::Stream> streams = currentEntry.getStreams();
      std::list<Database::Stream>::const_iterator SI = streams.begin();
      for(User::op_iterator OP = CI->op_begin()+2; OP != CI->op_end(); ++OP)
      {
        if( *OP == v )
        {
          if( SI == streams.end() )
            INTERNAL_ERROR("Stream " << currentEntry.getName() << " does not have enough streams.\n");
          assert(SI != streams.end() and "Incorrect number of streams!");
          if( ret == NULL )
            ret = new Stream(*SI);
          else
          {
            if( SI->getDataChannels().size() != ret->getDataChannels().size() )
            {
              INTERNAL_ERROR("One definition of stream " << v->getName() << " has " << ret->getDataChannels().size() << " data channels, but an instantiation of " << getComponentNameFromCallInst(CI) << "() defines " << v->getName() << " as having " << SI->getDataChannels().size() << " data channels!\n");
            }
            assert( SI->getDataChannels().size() == ret->getDataChannels().size() and "Streams passed to components must all share number of data channels!" );
            if( SI->getAddressChannelsBase().size() != ret->getAddressChannelsBase().size() )
            {
              INTERNAL_ERROR("One definition of stream " << v->getName() << " has " << ret->getAddressChannelsBase().size() << " address channels, but an instantiation of " << getComponentNameFromCallInst(CI) << "() defines " << v->getName() << " as having " << SI->getAddressChannelsBase().size() << " address channels!\n");
            }
            assert( SI->getAddressChannelsBase().size() == ret->getAddressChannelsBase().size() and "Streams passed to components must all share number of address channels!" );
          }
        }
        else if( isLLVMValueStream(*OP) )
        {
          ++SI;
        }
      }
    }
  }
  if( ret == NULL )
  {
    INTERNAL_ERROR("Stream " << getValueName(v) << " does not have an InvokeHardware use!\n");
    assert(0 and "Could not find a use of stream!");
  }
  return ret;
}

int S2SgetNumDataChannels(llvm::Value* v)
{
  int ret = getDatabaseStreamFromLLVMStream(v)->getDataChannels().size();
  return ret;
}

int S2SgetNumAddressChannels(llvm::Value* v)
{
  int ret = getDatabaseStreamFromLLVMStream(v)->getAddressChannelsBase().size();
  return ret;
}

std::vector<StreamVariable>::iterator ROCCCSystemToSystemPass::streamBegin()
{
  return streams.begin();
}
std::vector<StreamVariable>::iterator ROCCCSystemToSystemPass::streamEnd()
{
  return streams.end();
}

StreamVariable getStreamVariableSignalsFromPorts(StreamVariable svp, VHDLInterface::ComponentDefinition* cd, VHDLInterface::Entity* p)
{
  StreamVariable svs(svp.readableName, svp.value);
  svs.cross_clk = p->createSignal<Signal>(cd->getName()+"_"+svp.cross_clk->getName(), svp.cross_clk->getSize());
  svs.stop_access = p->createSignal<Signal>(cd->getName()+"_"+svp.stop_access->getName(), svp.stop_access->getSize());
  svs.enable_access = p->createSignal<Signal>(cd->getName()+"_"+svp.enable_access->getName(), svp.stop_access->getSize());
  for(std::vector<VHDLInterface::Variable*>::iterator DCI = svp.data_channels.begin(); DCI != svp.data_channels.end(); ++DCI)
  {
    svs.data_channels.push_back(p->createSignal<Signal>(cd->getName()+"_"+(*DCI)->getName(), (*DCI)->getSize()));
  }
  svs.address_clk = p->createSignal<Signal>(cd->getName()+"_"+svp.address_clk->getName(), svp.address_clk->getSize());
  svs.address_stall = p->createSignal<Signal>(cd->getName()+"_"+svp.address_stall->getName(), svp.address_stall->getSize());
  svs.address_rdy = p->createSignal<Signal>(cd->getName()+"_"+svp.address_rdy->getName(), svp.address_rdy->getSize());
  for(std::vector<std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*> >::iterator ACI = svp.address_channels.begin(); ACI != svp.address_channels.end(); ++ACI)
  {
    VHDLInterface::Variable* base = p->createSignal<Signal>(cd->getName()+"_"+ACI->first->getName(), ACI->first->getSize());
    VHDLInterface::Variable* count = p->createSignal<Signal>(cd->getName()+"_"+ACI->second->getName(), ACI->second->getSize());
    svs.address_channels.push_back(std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*>(base, count));
  }
  return svs;
}

StreamVariable getStreamVHDLPortsFromDatabasePorts(Database::Stream ds, VHDLInterface::ComponentDefinition* cdef)
{
  VHDLInterface::ComponentDeclaration* cd = cdef->getDeclaration();
  StreamVariable svp(ds.getName(), NULL);
  assert( cd->getPort(ds.getCrossClk()->getName()).size() == 1 and "Component must have cross clock port!" );
  svp.cross_clk = cd->getPort(ds.getCrossClk()->getName()).at(0);
  assert( cd->getPort(ds.getStopAccess()->getName()).size() == 1 and "Component must have access stop port!" );
  svp.stop_access = cd->getPort(ds.getStopAccess()->getName()).at(0);
  assert( cd->getPort(ds.getEnableAccess()->getName()).size() == 1 and "Component must have access enable port!" );
  svp.enable_access = cd->getPort(ds.getEnableAccess()->getName()).at(0);
  assert( cd->getPort(ds.getAddressClk()->getName()).size() == 1 and "Component must have address clock port!" );
  svp.address_clk = cd->getPort(ds.getAddressClk()->getName()).at(0);
  assert( cd->getPort(ds.getAddressReady()->getName()).size() == 1 and "Component must have address ready port!" );
  svp.address_rdy = cd->getPort(ds.getAddressReady()->getName()).at(0);
  assert( cd->getPort(ds.getAddressStall()->getName()).size() == 1 and "Component must have address stall port!" );
  svp.address_stall = cd->getPort(ds.getAddressStall()->getName()).at(0);
  for(std::list<Database::Port*>::const_iterator DCI = ds.getDataChannels().begin(); DCI != ds.getDataChannels().end(); ++DCI)
  {
    assert( cd->getPort((*DCI)->getName()).size() == 1 and "Component must have data channel port!" );
    svp.data_channels.push_back(cd->getPort((*DCI)->getName()).at(0));
  }
  assert( ds.getAddressChannelsBase().size() == ds.getAddressChannelsCount().size() and "Base and count address's must have same number of channels!" );
  std::list<Database::Port*>::const_iterator ACBI = ds.getAddressChannelsBase().begin();
  std::list<Database::Port*>::const_iterator ACCI = ds.getAddressChannelsCount().begin();
  for(; ACBI != ds.getAddressChannelsBase().end() and ACCI != ds.getAddressChannelsCount().end(); ++ACBI, ++ACCI)
  {
    assert( cd->getPort((*ACBI)->getName()).size() == 1 and "Component must have base address port!" );
    assert( cd->getPort((*ACCI)->getName()).size() == 1 and "Component must have count address port!" );
    svp.address_channels.push_back(std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*>(cd->getPort((*ACBI)->getName()).at(0),cd->getPort((*ACCI)->getName()).at(0)));
  }
  return svp;
}

void connectStreamInterface(/*stream variable ports*/StreamVariable svp, /*stream variable signals*/StreamVariable svs, VHDLInterface::ComponentDefinition* cd)
{
  cd->map(svs.cross_clk, dynamic_cast<VHDLInterface::Port*>(svp.cross_clk));
  cd->map(svs.stop_access, dynamic_cast<VHDLInterface::Port*>(svp.stop_access));
  cd->map(svs.enable_access, dynamic_cast<VHDLInterface::Port*>(svp.enable_access));
  if( svp.data_channels.size() != svs.data_channels.size() )
  {
    INTERNAL_ERROR("Attempting to map stream ports " << svp.readableName << " with " << svp.data_channels.size() << " data channels, to stream signals " << svs.readableName << " with " << svs.data_channels.size() << "!\n");
  }
  assert( svp.data_channels.size() == svs.data_channels.size() and "Cannot map streams with different numbers of data channels!" );
  std::vector<VHDLInterface::Variable*>::iterator SDCI = svs.data_channels.begin();
  std::vector<VHDLInterface::Variable*>::iterator PDCI = svp.data_channels.begin();
  for(; SDCI != svs.data_channels.end() and PDCI != svp.data_channels.end(); ++SDCI, ++PDCI)
  {
    cd->map(*SDCI, dynamic_cast<VHDLInterface::Port*>(*PDCI));
  }
  cd->map(svs.address_clk, dynamic_cast<VHDLInterface::Port*>(svp.address_clk));
  cd->map(svs.address_stall, dynamic_cast<VHDLInterface::Port*>(svp.address_stall));
  cd->map(svs.address_rdy, dynamic_cast<VHDLInterface::Port*>(svp.address_rdy));
  assert( svp.address_channels.size() == svs.address_channels.size() and "Cannot map streams with different numbers of address channels!" );
  std::vector<std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*> >::iterator SACI = svs.address_channels.begin();
  std::vector<std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*> >::iterator PACI = svp.address_channels.begin();
  for(; SACI != svs.address_channels.end() and PACI != svp.address_channels.end(); ++SACI, ++PACI)
  {
    cd->map(SACI->first, dynamic_cast<VHDLInterface::Port*>(PACI->first));
    cd->map(SACI->second, dynamic_cast<VHDLInterface::Port*>(PACI->second));
  }
}

class StreamAccessIntrinsic : public VHDLInterface::ComponentDefinition {
  StreamVariable inputAccess, outputAccess;
  VHDLInterface::ComponentDeclaration* getDecl(llvm::Value* streamValue)
  {
    VHDLInterface::ComponentDeclaration* ret = new VHDLInterface::ComponentDeclaration("streamIntrinsic");
    ret->getPorts().clear();
    return ret;
  }
  VHDLInterface::Port* rst;
public:
  StreamAccessIntrinsic(llvm::Value* streamValue) : VHDLInterface::ComponentDefinition(getValueName(streamValue)+"_streamIntrinsic", getDecl(streamValue)), inputAccess(getValueName(streamValue), streamValue), outputAccess(getValueName(streamValue), streamValue), rst(NULL)
  {
    ComponentDeclaration* ret = this->getDeclaration();
    rst = ret->addPort("rst", 1, VHDLInterface::Port::INPUT);
    inputAccess.cross_clk = ret->addPort("input_RClk", 1, VHDLInterface::Port::INPUT);
    inputAccess.stop_access = ret->addPort("input_Empty", 1, VHDLInterface::Port::INPUT);
    inputAccess.enable_access = ret->addPort("input_ReadEn", 1, VHDLInterface::Port::OUTPUT);
    inputAccess.address_clk = ret->addPort("input_address_clk", 1, VHDLInterface::Port::INPUT);
    inputAccess.address_stall = ret->addPort("input_address_stall", 1, VHDLInterface::Port::OUTPUT);
    inputAccess.address_rdy = ret->addPort("input_address_rdy", 1, VHDLInterface::Port::INPUT);
    int numDataChannels = S2SgetNumDataChannels(streamValue);
    for(int n = 0; n < numDataChannels; ++n)
    {
      std::stringstream channelNum;
      channelNum << "input_channel" << n;
      VHDLInterface::Variable* data = ret->addPort(channelNum.str(), getSizeInBits(streamValue), VHDLInterface::Port::INPUT);
      inputAccess.data_channels.push_back(data);
    }
    int numAddressChannels = S2SgetNumAddressChannels(streamValue);
    for(int n = 0; n < numAddressChannels; ++n)
    {
      std::stringstream channelNum;
      channelNum << "input_channel" << n;
      VHDLInterface::Variable* address_base = ret->addPort(channelNum.str()+"_address_base", 32, VHDLInterface::Port::INPUT);
      VHDLInterface::Variable* address_count = ret->addPort(channelNum.str()+"_address_count", 32, VHDLInterface::Port::INPUT);
      inputAccess.address_channels.push_back(std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*>(address_base, address_count));
    }
    outputAccess.cross_clk = ret->addPort("output_WClk", 1, VHDLInterface::Port::INPUT);
    outputAccess.stop_access = ret->addPort("output_Full", 1, VHDLInterface::Port::INPUT);
    outputAccess.enable_access = ret->addPort("output_WriteEn", 1, VHDLInterface::Port::OUTPUT);
    outputAccess.address_clk = ret->addPort("output_address_clk", 1, VHDLInterface::Port::INPUT);
    outputAccess.address_stall = ret->addPort("output_address_stall", 1, VHDLInterface::Port::OUTPUT);
    outputAccess.address_rdy = ret->addPort("output_address_rdy", 1, VHDLInterface::Port::INPUT);
    numDataChannels = S2SgetNumDataChannels(streamValue);
    for(int n = 0; n < numDataChannels; ++n)
    {
      std::stringstream channelNum;
      channelNum << "output_channel" << n;
      VHDLInterface::Variable* data = ret->addPort(channelNum.str(), getSizeInBits(streamValue), VHDLInterface::Port::OUTPUT);
      outputAccess.data_channels.push_back(data);
    }
    numAddressChannels = S2SgetNumAddressChannels(streamValue);
    for(int n = 0; n < numAddressChannels; ++n)
    {
      std::stringstream channelNum;
      channelNum << "output_channel" << n;
      VHDLInterface::Variable* address_base = ret->addPort(channelNum.str()+"_address_base", 32, VHDLInterface::Port::INPUT);
      VHDLInterface::Variable* address_count = ret->addPort(channelNum.str()+"_address_count", 32, VHDLInterface::Port::INPUT);
      outputAccess.address_channels.push_back(std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*>(address_base, address_count));
    }
    for(std::vector<VHDLInterface::Port*>::iterator PI = ret->getPorts().begin(); PI != ret->getPorts().end(); ++PI)
    {
      static LibraryComponentOwner LIBRARY_COMPONENT_OWNER;
      LIBRARY_COMPONENT_OWNER.add(*PI);
    }
  }
  StreamVariable getInputAccessPorts()
  {
    return inputAccess;
  }
  StreamVariable getOutputAccessPorts()
  {
    return outputAccess;
  }
  virtual void setParent(Entity* e) //from ComponentDefinition
  {
    //when we get a parent entity, map our rst to the parent's rst
    this->map(e->getDeclaration()->getStandardPorts().rst, rst);
    //then call the base setParent()
    ComponentDefinition::setParent(e);
  }
};

bool isInternalStream(llvm::Value* stream)
{
  if( !isLLVMValueStream(stream) )
    return false;
  if( !dynamic_cast<llvm::Instruction*>(stream) )
    return false;
  llvm::Function* F = dynamic_cast<llvm::Instruction*>(stream)->getParent()->getParent();
  for(Function::iterator BB = F->begin(); BB != F->end(); ++BB)
  {
    for(BasicBlock::iterator II = BB->begin(); II != BB->end(); ++II)
    {
      if( CallInst* CI = dynamic_cast<CallInst*>(&*II) )
      {
        if( isROCCCFunctionCall(CI, ROCCCNames::InputStream) or
            isROCCCFunctionCall(CI, ROCCCNames::OutputStream) )
        {
          for(User::op_iterator OP = CI->op_begin()+1; OP != CI->op_end(); ++OP)
          {
            if( *OP == stream )
            {
              return false;
            }
          }
        }
      }
    }
  }
  return true;
}

// This is the entry point to our pass and where all of our work gets done
bool ROCCCSystemToSystemPass::runOnFunction(Function& F)
{
  CurrentFile::set(__FILE__);
  bool changed = false;

  if( ROCCC::ROCCCFunctionType(&F) != ROCCC::SYSTEM )
  {
    return changed;
  }
  
  entity = new Entity(F.getName());
  //Each scalar value or stream has to come from some source. We want to know when
  //  that source is finished processing, so we can start processing.
  //  This map maps each scalar or stream to the signal (could also be port, if incoming value)
  //  that says that the source is finished.
  std::map<llvm::Value*, VHDLInterface::Variable*> triggerMap;
  //We will go through the function multiple times, so we need to have an end condition
  bool isDone = true;
  //Also keep track of whether each instruction has been processed or not.
  std::map<llvm::Instruction*, bool> instructionHasBeenProcessed;
  //Map each variable that is deemed to be a stream to the set of variables associated with it
  std::map<llvm::Value*, StreamVariable> streamVariableMap;
  //we need to keep track off all done signals from subcomponents that are connected to output ports,
  //   so as to be able to set the done high when all the output sources are done.
  //The done trigger, at minimum, requires inputReady to go high.
  VHDLInterface::Wrap doneTrigger(entity->getStandardPorts().inputReady);
  //search for all internal streams, then create intrinsics for them
  std::map<llvm::Value*,StreamAccessIntrinsic*> internalIntrinsics;
  for(Function::iterator BB = F.begin(); BB != F.end(); ++BB)
  {
    for(BasicBlock::iterator II = BB->begin(); II != BB->end(); ++II)
    {
      if( isInternalStream(&*II) )
      {
        internalIntrinsics[&*II] = new StreamAccessIntrinsic(&*II);
        INTERNAL_MESSAGE("Adding intrinsic component for internal stream " << getValueName(&*II) << "\n");
        entity->addComponent(internalIntrinsics[&*II]);
      }
    }
  }
  do {
    //Assume we are done; if any statement cant be processed, change this to false
    isDone = true;
    //Assume we havent changed anything; if we process any statement, change this to true
    bool hasChanged = false;
    //Process all of the statements in the function. Some of them create
    //connections, some of them invoke hardware.
    for(Function::iterator BB = F.begin(); BB != F.end(); ++BB)
    {
      for(BasicBlock::iterator II = BB->begin(); II != BB->end(); ++II)
      {
        //dont reprocess already processed instructions
        if( instructionHasBeenProcessed[&*II] )
          continue;
        if( CallInst* CI = dynamic_cast<CallInst*>(&*II) )
        {
          //check that all inputs have been defined
          bool allDefined = true;
          for(User::op_iterator OP = CI->op_begin()+1; OP != CI->op_end() and allDefined; ++OP)
          {
            if( isOperandOfCallAnOutput(CI,OP) )
              continue;
            if( isLLVMValueStream(*OP) )
            {
              if( streamVariableMap.find(*OP) == streamVariableMap.end() )
              {
                INTERNAL_MESSAGE("Delayed processing " << *CI << " due to " << (*OP)->getName() << " being unprocessed.\n");
                isDone = false;
                allDefined = false;
              }
            }
            else if( dynamic_cast<llvm::Constant*>(&**OP) )
            {
              //ignore constants
            }
            else
            {
              if( entity->findSignal(*OP).size() == 0 )
              {
                INTERNAL_MESSAGE("Delayed processing " << *CI << " due to " << (*OP)->getName() << " being unprocessed.\n");
                isDone = false;
                allDefined = false;
              }
            }
          }
          if( !allDefined )
            continue;
          //if its an InputScalar, add the scalars to the entity
          if( isROCCCFunctionCall(CI, ROCCCNames::InputScalar) )
          {
            for(User::op_iterator OP = CI->op_begin()+1; OP != CI->op_end(); ++OP)
            {
              entity->createSynchronousStatement(entity->createSignal<Signal>((*OP)->getName(), getSizeInBits(*OP), *OP), entity->addPort(getValueName(*OP), getSizeInBits(*OP), VHDLInterface::Port::INPUT, *OP));
              //input scalars are valid as soon as the inputReady is set high
              triggerMap[*OP] = entity->getStandardPorts().inputReady;
            }
            hasChanged = true;
            instructionHasBeenProcessed[CI] = true;
          }
          //if its an OutputScalar, check to see if the operands have been defined, then add them to the entity
          if( isROCCCFunctionCall(CI, ROCCCNames::OutputScalar) )
          {
            //output scalars by default need the inputReady signal to have gone high at least once
            VHDLInterface::Wrap trigger(entity->getStandardPorts().inputReady);
            for(User::op_iterator OP = CI->op_begin()+1; OP != CI->op_end(); ++OP)
            {
              if( !dynamic_cast<llvm::Constant*>(&**OP) )
              {
                trigger = trigger & VHDLInterface::Wrap(triggerMap[*OP]);
                doneTrigger = doneTrigger & VHDLInterface::Wrap(triggerMap[*OP]);
              }
              VHDLInterface::Value* val = LLVMValueToVHDLValue(entity, *OP);
              entity->createSynchronousStatement(entity->addPort(val->getName(), val->getSize(), VHDLInterface::Port::OUTPUT, *OP), val);
            }
            entity->createSynchronousStatement(entity->getStandardPorts().outputReady, trigger);
            hasChanged = true;
            instructionHasBeenProcessed[CI] = true;
          }
          //if its an DebugScalarOutput, check to see if the operands have been defined, then add them to the entity
          if( isROCCCFunctionCall(CI, ROCCCNames::DebugScalarOutput) )
          {
            //output scalars by default need the inputReady signal to have gone high at least once
            VHDLInterface::Wrap trigger(entity->getStandardPorts().inputReady);
            for(User::op_iterator OP = CI->op_begin()+1; OP != CI->op_end(); ++OP)
            {
              if( !dynamic_cast<llvm::Constant*>(&**OP) )
              {
                trigger = trigger & VHDLInterface::Wrap(triggerMap[*OP]);
                doneTrigger = doneTrigger & VHDLInterface::Wrap(triggerMap[*OP]);
              }
              VHDLInterface::Value* val = LLVMValueToVHDLValue(entity, *OP);
              entity->createSynchronousStatement(entity->addPort(val->getName(), val->getSize(), VHDLInterface::Port::OUTPUT, *OP), val);
            }
            hasChanged = true;
            instructionHasBeenProcessed[CI] = true;
          }
          //if its an input stream, create the signals that this input stream deals with
          if( isROCCCFunctionCall(CI, ROCCCNames::InputStream) )
          {
            for(User::op_iterator OP = CI->op_begin()+1; OP != CI->op_end(); ++OP)
            {
              StreamVariable sv(getValueName(*OP), *OP);
              StreamVariable esv(getValueName(*OP), *OP);
              sv.cross_clk = entity->createSignal<Signal>(sv.readableName+"_WClk", 1);
              esv.cross_clk = entity->addPortCopy(sv.cross_clk, VHDLInterface::Port::INPUT);
              entity->createSynchronousStatement(sv.cross_clk, esv.cross_clk);
              sv.stop_access = entity->createSignal<Signal>(sv.readableName+"_Full", 1);
              esv.stop_access = entity->addPortCopy(sv.stop_access, VHDLInterface::Port::OUTPUT);
              entity->createSynchronousStatement(esv.stop_access, sv.stop_access);
              sv.enable_access = entity->createSignal<Signal>(sv.readableName+"_WriteEn", 1);
              esv.enable_access = entity->addPortCopy(sv.enable_access, VHDLInterface::Port::INPUT);
              entity->createSynchronousStatement(sv.enable_access, esv.enable_access);
              sv.address_clk = entity->createSignal<Signal>(sv.readableName+"_address_clk", 1);
              esv.address_clk = entity->addPortCopy(sv.address_clk, VHDLInterface::Port::INPUT);
              entity->createSynchronousStatement(sv.address_clk, esv.address_clk);
              sv.address_stall = entity->createSignal<Signal>(sv.readableName+"_address_stall", 1);
              esv.address_stall = entity->addPortCopy(sv.address_stall, VHDLInterface::Port::INPUT);
              entity->createSynchronousStatement(sv.address_stall, esv.address_stall);
              sv.address_rdy = entity->createSignal<Signal>(sv.readableName+"_address_rdy", 1);
              esv.address_rdy = entity->addPortCopy(sv.address_rdy, VHDLInterface::Port::OUTPUT);
              entity->createSynchronousStatement(esv.address_rdy, sv.address_rdy);
              int numDataChannels = S2SgetNumDataChannels(*OP);
              for(int n = 0; n < numDataChannels; ++n)
              {
                std::stringstream channelNum;
                channelNum << "_channel" << n;
                VHDLInterface::Variable* data = entity->createSignal<Signal>(sv.readableName+channelNum.str(), getSizeInBits(*OP), *OP);
                sv.data_channels.push_back(data);
                VHDLInterface::Variable* data2 = entity->addPortCopy(data, VHDLInterface::Port::INPUT);
                esv.data_channels.push_back(data2);
                entity->createSynchronousStatement(data, data2);
              }
              int numAddressChannels = S2SgetNumAddressChannels(*OP);
              for(int n = 0; n < numAddressChannels; ++n)
              {
                std::stringstream channelNum;
                channelNum << "_channel" << n;
                VHDLInterface::Variable* address_base = entity->createSignal<Signal>(sv.readableName+channelNum.str()+"_address_base", 32);
                VHDLInterface::Variable* address_count = entity->createSignal<Signal>(sv.readableName+channelNum.str()+"_address_count", 32);
                sv.address_channels.push_back(std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*>(address_base, address_count));
                VHDLInterface::Variable* address2_base = entity->addPortCopy(address_base, VHDLInterface::Port::OUTPUT);
                VHDLInterface::Variable* address2_count = entity->addPortCopy(address_count, VHDLInterface::Port::OUTPUT);
                esv.address_channels.push_back(std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*>(address2_base, address2_count));
                entity->createSynchronousStatement(address2_base, address_base);
                entity->createSynchronousStatement(address2_count, address_count);
              }
              streamVariableMap[*OP] = sv;
              streams.push_back(esv);
            }
            hasChanged = true;
            instructionHasBeenProcessed[CI] = true; 
          }
          //if its an output stream, create the signals dealing with the output stream
          if( isROCCCFunctionCall(CI, ROCCCNames::OutputStream) )
          {
            for(User::op_iterator OP = CI->op_begin()+1; OP != CI->op_end(); ++OP)
            {
              StreamVariable sv = streamVariableMap[*OP];
              StreamVariable esv(sv.readableName, sv.value);
              sv.readableName = getValueName(*OP);
              esv.cross_clk = entity->addPort(sv.readableName+"_RClk", 1, VHDLInterface::Port::INPUT);
              entity->createSynchronousStatement(sv.cross_clk, esv.cross_clk);
              esv.stop_access = entity->addPort(sv.readableName+"_Empty", 1, VHDLInterface::Port::OUTPUT);
              entity->createSynchronousStatement(esv.stop_access, sv.stop_access);
              esv.enable_access = entity->addPort(sv.readableName+"_ReadEn", 1, VHDLInterface::Port::INPUT);
              entity->createSynchronousStatement(sv.enable_access, esv.enable_access);
              esv.address_clk = entity->addPort(sv.readableName+"_address_clk", 1, VHDLInterface::Port::INPUT);
              entity->createSynchronousStatement(sv.address_clk, esv.address_clk);
              esv.address_stall = entity->addPort(sv.readableName+"_address_stall", 1, VHDLInterface::Port::INPUT);
              entity->createSynchronousStatement(sv.address_stall, esv.address_stall);
              esv.address_rdy = entity->addPort(sv.readableName+"_address_rdy", 1, VHDLInterface::Port::OUTPUT);
              entity->createSynchronousStatement(esv.address_rdy, sv.address_rdy);
              int count = 0;
              for(std::vector<VHDLInterface::Variable*>::iterator CHI = sv.data_channels.begin(); CHI != sv.data_channels.end(); ++CHI, ++count)
              {
                std::stringstream channelNum;
                channelNum << "_channel" << count;
                VHDLInterface::Variable* chi = entity->addPort(sv.readableName+channelNum.str(), (*CHI)->getSize(), VHDLInterface::Port::OUTPUT, *OP);
                esv.data_channels.push_back(chi);
                entity->createSynchronousStatement(chi, *CHI);
              }
              count = 0;
              for(std::vector<std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*> >::iterator OII = sv.address_channels.begin(); OII != sv.address_channels.end(); ++OII)
              {
                std::stringstream channelNum;
                channelNum << "_channel" << count;
                VHDLInterface::Variable* oii_b = entity->addPort(sv.readableName+channelNum.str()+"_address_base", 32, VHDLInterface::Port::OUTPUT);
                VHDLInterface::Variable* oii_c = entity->addPort(sv.readableName+channelNum.str()+"_address_count", 32, VHDLInterface::Port::OUTPUT);
                esv.address_channels.push_back(std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*>(oii_b, oii_c));
                entity->createSynchronousStatement(oii_b, OII->first);
                entity->createSynchronousStatement(oii_c, OII->second);
              }
              streams.push_back(esv);
              doneTrigger = doneTrigger & VHDLInterface::Wrap(triggerMap[*OP]);
            }
            hasChanged = true;
            instructionHasBeenProcessed[CI] = true;
          }
          //if its an invoke hardware, create the subcomponent and map the requisite signals
          if( isROCCCFunctionCall(CI, ROCCCNames::InvokeHardware) )
          {
            //create a subcomponent that is the same as the database entry
            std::stringstream instanceName;
            static int count = 0;
            instanceName << "U" << count++;
            ComponentDefinition* cd = entity->createComponent(instanceName.str(),getDeclarationForComponent(CI, entity));
            entity->mapPortToSubComponentPort(entity->getStandardPorts().clk, cd, cd->getDeclaration()->getStandardPorts().clk);
            entity->mapPortToSubComponentPort(entity->getStandardPorts().rst, cd, cd->getDeclaration()->getStandardPorts().rst);
            entity->mapPortToSubComponentPort(entity->getStandardPorts().stall, cd, cd->getDeclaration()->getStandardPorts().stall);
            //create a trigger for the inputReady, which is based on all of the inputs, and by default the inputReady of the entity
            VHDLInterface::Wrap trigger(entity->getStandardPorts().inputReady);
            //get the stream and non-stream ports
            LibraryEntry currentEntry = DatabaseInterface::getInstance()->LookupEntry(getComponentNameFromCallInst(CI)) ;
            const std::list<Database::Port*> ports = currentEntry.getNonDebugScalarPorts();
            std::list<Database::Port*>::const_iterator PI = ports.begin();
            const std::list<Database::Stream> streams = currentEntry.getStreams();
            std::list<Database::Stream>::const_iterator SI = streams.begin();
            for(User::op_iterator OP = CI->op_begin()+2; OP != CI->op_end(); ++OP)
            {
              if( isLLVMValueStream(*OP) )
              {
                StreamVariable svp(getValueName(*OP), *OP);
                svp = getStreamVHDLPortsFromDatabasePorts(*SI, cd);
                StreamVariable sv(getValueName(*OP), *OP);
                if( isInternalStream(*OP) )
                {
                  sv = getStreamVariableSignalsFromPorts(svp, cd, entity);
                  connectStreamInterface(svp, sv, cd);
                  std::string connectionName = "_sink";
                  if( isOperandOfCallAnInput(CI,OP) )
                  {
                    connectionName = "_source";
                  }
                  VHDLInterface::Port* data_clk = entity->addPort(getValueName(*OP)+connectionName+"_data_clk", 1, VHDLInterface::Port::INPUT);
                  entity->createSynchronousStatement(sv.cross_clk, data_clk);
                  VHDLInterface::Port* address_clk = entity->addPort(getValueName(*OP)+connectionName+"_address_clk", 1, VHDLInterface::Port::INPUT);
                  entity->createSynchronousStatement(sv.address_clk, address_clk);
                  if( !entity->getAttribute("port_type") )
                    entity->addAttribute(new VHDLInterface::Attribute("port_type"));
                  data_clk->addAttribute(entity->getAttribute("port_type"), "INTERNAL_CROSS_CLK");
                  address_clk->addAttribute(entity->getAttribute("port_type"), "INTERNAL_ADDRESS_CLK");
                  StreamAccessIntrinsic* intrinsic = internalIntrinsics[*OP];
                  assert( intrinsic and "Intrinsic for stream was not created!" );
                  if( isOperandOfCallAnInput(CI, OP) )
                  {
                    connectStreamInterface(intrinsic->getOutputAccessPorts(), sv, intrinsic);
                  }
                  else
                  {
                    connectStreamInterface(intrinsic->getInputAccessPorts(), sv, intrinsic);
                  }
                }
                else
                {
                  if( isOperandOfCallAnInput(CI, OP) )
                  {
                    sv = streamVariableMap[*OP];
                  }
                  else
                  {
                    sv = getStreamVariableSignalsFromPorts(svp, cd, entity);
                  }
                  connectStreamInterface(svp, sv, cd);
                }
                streamVariableMap[*OP] = sv;
                triggerMap[*OP] = entity->getVariableMappedTo(cd, cd->getDeclaration()->getStandardPorts().done);
                ++SI;
              }
              else
              {
                VHDLInterface::Variable* val = NULL;
                if( isOperandOfCallAnInput(CI, OP) )
                {
                  if( !dynamic_cast<llvm::Constant*>(&**OP) )
                  {
                    trigger = trigger & VHDLInterface::Wrap(triggerMap[*OP]);
                  }
                  val = LLVMValueToVHDLValue(entity, *OP);
                }
                else
                {
                  val = entity->createSignal<Signal>((*OP)->getName(), getSizeInBits(*OP), *OP);
                  //its an output scalar, so we need to make sure the triggerMap gets updated
                  triggerMap[*OP] = entity->getVariableMappedTo(cd, cd->getDeclaration()->getStandardPorts().done);
                }
                //resize the constant, if its a constant, to get mapped to the port
                if( dynamic_cast<VHDLInterface::NamedConstant*>(val) )
                  dynamic_cast<VHDLInterface::NamedConstant*>(val)->resize((*PI)->getBitSize());
                cd->map(val, (*PI)->getName());
                ++PI;
              }
            }
            //map the trigger to the inputReady
            entity->createSynchronousStatement(entity->getVariableMappedTo(cd, cd->getDeclaration()->getStandardPorts().inputReady), trigger);
            hasChanged = true;
            instructionHasBeenProcessed[CI] = true;
          }
        }
      }
    }
    assert( hasChanged and "Processing system iteratively, but nothing has changed since last iteration!" );
  } while( !isDone );
  //map the done signal to the trigger we have been creating
  entity->createSynchronousStatement(entity->getStandardPorts().done, doneTrigger);
  
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
  outputFileName = F.getName() ;
  outputFileName += ".vhdl" ;
  std::string outputFullPath = outputDirectory + outputFileName;
  std::ofstream fout;
  fout.open(outputFullPath.c_str(), std::ios_base::out) ;
  if (!fout)
  {
    llvm::cerr << "Cannot open output file:" << outputFullPath << std::endl ;
    assert(0) ;
  }
  llvm::cout << "Writing system to \'" << outputFullPath << "\'\n";
  fout << entity->generateCode();
  fout.close() ;
  Database::FileInfoInterface::addFileInfo(Database::getCurrentID(), Database::FileInfo(outputFileName, Database::FileInfo::VHDL_SOURCE, outputDirectory));
  LOG_MESSAGE2("VHDL Generation", "Datapath", "Datapath written to <a href=\'" << outputFullPath << "\'>" << outputFileName << "</a>.\n");
  return changed ; 
}

void ROCCCSystemToSystemPass::getAnalysisUsage(AnalysisUsage &AU) const
{
  AU.setPreservesAll();
}

