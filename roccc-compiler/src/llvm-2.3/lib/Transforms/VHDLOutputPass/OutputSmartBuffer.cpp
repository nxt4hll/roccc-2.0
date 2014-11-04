#include "rocccLibrary/VHDLComponents/OutputSmartBuffer.h"

#include <sstream>
#include <fstream>
#include <vector>
#include <map>

#include "rocccLibrary/SizeInBits.h"
#include "rocccLibrary/VHDLInterface.h"
#include "rocccLibrary/GetValueName.h"
#include "rocccLibrary/MultiForVar.hpp"
#include "rocccLibrary/VHDLComponents/FifoInterfaceBlock.hpp"
#include "rocccLibrary/VHDLComponents/LoopInductionVariableHandler.h"
#include "rocccLibrary/VHDLComponents/ShiftBuffer.h"
#include "rocccLibrary/VHDLComponents/MicroFifo.h"
#include "rocccLibrary/VHDLComponents/AddressGenerator.h"

#include "rocccLibrary/InternalWarning.h"

//FIXME: From InputSmartBuffer.cpp - organize!
//helper function to get a vector's worth of values from a map
template<class T, class V>
std::vector<V> getVectorOfMappedValues(std::map<T,V> mmap, std::vector<T> in)
{
  std::vector<V> ret;
  for(typename std::vector<T>::iterator VI = in.begin(); VI != in.end(); ++VI)
  {
    typename std::map<T,V>::iterator MI = mmap.find(*VI);
    assert(MI != mmap.end() and "Value in input vector does not exist in map!");
    ret.push_back(MI->second);
  }
  assert(ret.size() == in.size());
  return ret;
}
//helper function to call getVectorOfMappedValues() correctly
llvm::MultiForVar<int> getMultiForVarForWindow(std::map<llvm::Value*,int> window, std::vector<llvm::Value*> accessOrder);
//helper function to call getVectorOfMappedValues() on both start and end values
llvm::MultiForVar<int> getMultiForVarForWindowWithStart(std::map<llvm::Value*,int> start_window, std::map<llvm::Value*,int> end_window, std::vector<llvm::Value*> accessOrder)
{
  std::vector<int> start = getVectorOfMappedValues(start_window, accessOrder);
  for(unsigned c = 0; c < start.size()-1; ++c)
    start[c] = 0;
  return llvm::MultiForVar<int>(start, getVectorOfMappedValues(end_window, accessOrder));
}
//helper function to print a vector to a string
template<class T>
std::string getVectorAsString(std::vector<T> v)
{
  std::stringstream ss;
  for(unsigned c = 0; c < v.size(); ++c)
  {
    ss << "_" << v[c];
  }
  return ss.str();
}

class OutputSmartBufferImpl : public FifoInterfaceBlock<llvm::Value*,llvm::Value*> {
  typedef llvm::Value* INPUT_TYPE;
  typedef llvm::Value* OUTPUT_TYPE;
  //what stream is this, anyways?!
  llvm::Value* stream_value;
  //map loop induction variables (LIV) to the size of the window they access
  std::map<llvm::Value*,int> window_dimensions;
  std::map<llvm::Value*,int> step_dimensions;
  std::vector<llvm::Value*> accessed_LIV;
  std::vector<llvm::Value*> written_LIV;
  std::vector<llvm::Value*> getLIVInAccessedOrder()
  {
    return accessed_LIV;
  }
  std::vector<llvm::Value*> getLIVInWrittenOrder()
  {
    return written_LIV;
  }
  llvm::Value* getBufferedLoopInductionVariable()
  {
    assert(accessed_LIV.size() > 0);
    return *accessed_LIV.begin();
  }
  VHDLInterface::StateVar* state;
  VHDLInterface::State* s_ready;
  VHDLInterface::State* s_read;
  VHDLInterface::State* s_done;
  std::map<std::vector<int>,VHDLInterface::State*> window_states;
  std::map<std::vector<int>,VHDLInterface::State*> step_states;
  bool isWindowBufferState(VHDLInterface::State* s)
  {
    for(std::map<std::vector<int>,VHDLInterface::State*>::iterator SI = window_states.begin(); SI != window_states.end(); ++SI)
    {
      if( SI->second == s )
      {
        return true;
      }
    }
    return false;
  }
  bool isStepBufferState(VHDLInterface::State* s)
  {
    for(std::map<std::vector<int>,VHDLInterface::State*>::iterator SI = step_states.begin(); SI != step_states.end(); ++SI)
    {
      if( SI->second == s )
      {
        return true;
      }
    }
    return false;
  }
  llvm::MultiForVar<int> getMultiForVarForState(VHDLInterface::State* s)
  {
    for(std::map<std::vector<int>,VHDLInterface::State*>::iterator SI = step_states.begin(); SI != step_states.end(); ++SI)
    {
      if( SI->second == s )
      {
        llvm::MultiForVar<int> ret = getMultiForVarForWindow(step_dimensions, getLIVInWrittenOrder());
        std::vector<int> raw = SI->first;
        //for(unsigned c = 0; c < raw.size(); ++c)
        //  raw[c] += ret.getBegin()[c];
        ret.getRaw() = raw;
        return ret;
      }
    }
    for(std::map<std::vector<int>,VHDLInterface::State*>::iterator SI = window_states.begin(); SI != window_states.end(); ++SI)
    {
      if( SI->second == s )
      {
        llvm::MultiForVar<int> ret = getMultiForVarForWindowWithStart(step_dimensions, window_dimensions, getLIVInWrittenOrder());
        std::vector<int> raw = SI->first;
        //for(unsigned c = 0; c < raw.size(); ++c)
        //  raw[c] += ret.getBegin()[c];
        ret.getRaw() = raw;
        return ret;
      }
    }
    assert(0 and "Could not find matching buffer index for state!");
  }
  bool isLoadState(VHDLInterface::State* s)
  {
    if( s == s_ready or
        s == s_read )
    {
      return false;
    }
    llvm::MultiForVar<int> var = getMultiForVarForState(s);
    //is it the starting state?
    return (isStepBufferState(s) and var.getRaw() == var.getBegin());
  }
  VHDLInterface::Variable* has_made_read_request;
  VHDLInterface::Variable* outfifo_full;
  std::vector<VHDLInterface::Variable*> outfifo_data_in;
  VHDLInterface::Statement* getFifoRequestDataStatement(VHDLInterface::Process* p)
  {
    llvm::Value* index = stream_value;
    /*
    if( sb0_full_in = '0' and fifo0_micro_empty_out = '0' ) then
		  fifo0_micro_read_enable_in <= '1';
		  data0_state <= S_READ;
		  has_made_read_request := true;
		end if;
		*/
    VHDLInterface::MultiStatement* ms = new VHDLInterface::MultiStatement(p);
    ms->addStatement(new VHDLInterface::AssignmentStatement(inputs[index].read_enable_out, VHDLInterface::ConstantInt::get(1), p));
    ms->addStatement(new VHDLInterface::AssignmentStatement(has_made_read_request, VHDLInterface::ConstantInt::get(1), p));
    return new VHDLInterface::IfStatement(p, VHDLInterface::Wrap(outfifo_full) == VHDLInterface::ConstantInt::get(0) and VHDLInterface::Wrap(inputs[index].empty_in) == VHDLInterface::ConstantInt::get(0), ms);
  }
  std::map<std::vector<int>,VHDLInterface::Variable*> buffers;
  VHDLInterface::Statement* getFifoLoadDataStatement(VHDLInterface::Process* p)
  {
    llvm::Value* index = stream_value;
    VHDLInterface::MultiStatement* ret = new VHDLInterface::MultiStatement(p);
    //grab the data from the datapath and load it into the buffer
    int count = 0;
    for(llvm::MultiForVar<int> i = getMultiForVarForWindow(window_dimensions, getLIVInWrittenOrder()); !i.done(); ++i)
    {
      ret->addStatement(new VHDLInterface::AssignmentStatement(buffers[i.getRaw()], inputs[index].data_in.at(count), p));
      ++count;
    }
    ret->addStatement(new VHDLInterface::AssignmentStatement(has_made_read_request, VHDLInterface::ConstantInt::get(0), p));
    return ret;
  }
  VHDLInterface::Statement* getBufferStoreElementStatement(VHDLInterface::Variable* outputValueValid, VHDLInterface::Process* p, VHDLInterface::State* s)
  {
    VHDLInterface::MultiStatement* ret = new VHDLInterface::MultiStatement(p);
    ret->addStatement(new VHDLInterface::AssignmentStatement(outputValueValid, VHDLInterface::ConstantInt::get(1), p));
    llvm::MultiForVar<int> mfv = getMultiForVarForState(s);
    for(std::vector<VHDLInterface::Variable*>::iterator VI = outfifo_data_in.begin(); VI != outfifo_data_in.end(); ++VI)
    {
      if( buffers.find(mfv.getRaw()) == buffers.end() )
      {
        INTERNAL_MESSAGE("In state " << s->getName() << ", buffer element " << getVectorAsString(mfv.getRaw()) << " does not exist.\n");
        assert(0 and "Number of channels must be factor of buffer window!");
      }
      ret->addStatement(new VHDLInterface::AssignmentStatement(*VI, buffers[mfv.getRaw()], p));
      ++mfv;
    }
    return ret;
  }
public:
  OutputSmartBufferImpl(llvm::Value* v) : FifoInterfaceBlock<INPUT_TYPE,OUTPUT_TYPE>(), stream_value(v), state(NULL), s_ready(NULL), s_read(NULL), has_made_read_request(NULL), outfifo_full(NULL)
  {
    assert(v);
  }
  void setWindowDimensions(std::map<llvm::Value*,int> wd)
  {
    window_dimensions = wd;
  }
  void setStepDimensions(std::map<llvm::Value*,int> sd)
  {
    step_dimensions = sd;
  }
  void setAccessedLIV(std::vector<llvm::Value*> aliv)
  {
    accessed_LIV = aliv;
  }
  void setWrittenLIV(std::vector<llvm::Value*> wliv)
  {
    written_LIV = wliv;
  }
  VHDLInterface::CWrap getDoneCondition()
  {
    return VHDLInterface::Wrap(state) == VHDLInterface::Wrap(s_done);
  }
  void createVHDL(VHDLInterface::Entity* parent, LoopInductionVariableHandler* livHandler)
  {
    llvm::Value* index = stream_value;
    bool is_single_element = true;
    for(std::vector<llvm::Value*>::iterator ALIVI = accessed_LIV.begin(); ALIVI != accessed_LIV.end(); ++ALIVI)
    {
      if( window_dimensions[*ALIVI] > 1 or step_dimensions[*ALIVI] > 1 )
        is_single_element = false;
    }
    if( is_single_element ) //if its a single element, just do pass through
    {
      assert( outputs.size() == inputs.size() );
      assert( outputs.size() == 1 );
      assert(inputs.find(index) != inputs.end());
      assert(outputs[index].read_enable_in);
      assert(inputs[index].read_enable_out);
      parent->createSynchronousStatement(inputs[index].read_enable_out, outputs[index].read_enable_in);
      assert(outputs[index].empty_out);
      assert(inputs[index].empty_in);
      parent->createSynchronousStatement(outputs[index].empty_out, inputs[index].empty_in);
      assert(outputs[index].data_out.size() == inputs[index].data_in.size());
      std::vector<VHDLInterface::Variable*>::iterator DOI = outputs[index].data_out.begin();
      for(std::vector<VHDLInterface::Variable*>::iterator DII = inputs[index].data_in.begin(); DOI != outputs[index].data_out.end() and DII != inputs[index].data_in.end(); ++DOI, ++DII)
      {
        parent->createSynchronousStatement(*DOI, *DII);
      }
      //even though we made direct connections, we need to create a counter process
      //  that will eventually set done.
      VHDLInterface::MultiStatementProcess* p = parent->createProcess<VHDLInterface::MultiStatementProcess>();
      //create the shift register for the read_enable signal
      VHDLInterface::Signal* shift_reg = parent->createSignal<VHDLInterface::Signal>(getValueName(stream_value)+"_shift_reg", 4);
      p->addStatement(new VHDLInterface::AssignmentStatement(shift_reg, VHDLInterface::bitwise_concat(outputs[index].read_enable_in, VHDLInterface::BitRange::get(shift_reg, 3, 1)), p));
      //create the state machine for the done signal
      state = parent->createSignal<VHDLInterface::StateVar>(getValueName(stream_value)+"_count",0);
      VHDLInterface::CaseStatement* cs = new VHDLInterface::CaseStatement(p, state);
      p->addStatement( cs );
      //create the ready state
      s_ready = state->addState("S_READY");
      //Create the done state
      s_done = state->addState("S_DONE");
      //create the body of the ready state
      VHDLInterface::MultiStatement* ms = new VHDLInterface::MultiStatement(p);
      cs->addStatement(s_ready, new VHDLInterface::IfStatement(p, VHDLInterface::BitRange::get(shift_reg, 0, 0) == VHDLInterface::ConstantInt::get(1), ms));
      ms->addStatement(new VHDLInterface::IfStatement(p, livHandler->getDoneCondition(), new VHDLInterface::AssignmentStatement(state, s_done, p)));
      ms->addStatement(livHandler->getIncrementStatement(p));
      //create the body of the done state
      cs->addStatement(s_done, new VHDLInterface::AssignmentStatement(state, s_done, p));
    }
    //if the innermost access LIV is also the innermost loop LIV,
    //  then use the developed smart buffering approach.
    else if( livHandler->getInnermostLIV() == getBufferedLoopInductionVariable() )
    {
      //create the output fifo signals that the smart buffer writes to
      outfifo_full = parent->createSignal<VHDLInterface::Signal>(getValueName(stream_value)+"_outfifo_full_t", 1);
      int count = 0;
      for(std::vector<VHDLInterface::Variable*>::iterator VI = outputs[index].data_out.begin(); VI != outputs[index].data_out.end(); ++VI)
      {
        std::stringstream ss;
        ss << (*VI)->getName() << "_prefifo" << count++;
        VHDLInterface::Variable* temp = parent->createSignal<VHDLInterface::Signal>(ss.str(), (*VI)->getSize(), (*VI)->getLLVMValue());
        outfifo_data_in.push_back(temp);
      }
      //create the smart buffering stage
      VHDLInterface::MultiStatementProcess* p = parent->createProcess<VHDLInterface::MultiStatementProcess>();
      std::map<VHDLInterface::Variable*, VHDLInterface::Variable*> bufferToSignal;
      VHDLInterface::Variable* outputValueValid = parent->createSignal<VHDLInterface::Signal>(getValueName(stream_value)+"_valid1",1);
      //steady state
      p->addStatement(new VHDLInterface::AssignmentStatement(outputValueValid, VHDLInterface::ConstantInt::get(0), p));
      p->addStatement(new VHDLInterface::AssignmentStatement(inputs[index].read_enable_out, VHDLInterface::ConstantInt::get(0), p));
      state = parent->createSignal<VHDLInterface::StateVar>(getValueName(stream_value)+"_buff_state",0);
      VHDLInterface::CaseStatement* cs = new VHDLInterface::CaseStatement(p, state);
      p->addStatement( cs );
      //save the number of data channels
      int num_channels = outfifo_data_in.size();
      //Create the ready and read states
      s_ready = state->addState("S_READY");
      s_read = state->addState("S_READ");
      //Create the done state
      s_done = state->addState("S_DONE");
      cs->addStatement(s_done, new VHDLInterface::AssignmentStatement(state, s_done, p));
      //create the psuedo-state variable
      has_made_read_request = new VHDLInterface::ProcessVariable(getValueName(stream_value)+"_has_made_read_request", 1);
      //just for ease-of-access, save the MultiForVar that is the start of the step buffer and the window buffer, respectively
      llvm::MultiForVar<int> stepBufferStart = getMultiForVarForWindow(step_dimensions, getLIVInWrittenOrder());
      llvm::MultiForVar<int> windowBufferStart = getMultiForVarForWindowWithStart(step_dimensions, window_dimensions, getLIVInWrittenOrder());
      //the output buffer is a step-first architecture, so create the step states first
      for(llvm::MultiForVar<int> i = stepBufferStart; !i.done(); ++i)
      {
        std::string name = getValueName(stream_value) + getVectorAsString(i.getRaw());
        step_states[i.getRaw()] = state->addState("S_STEP_"+name);
        //create the buffers
        buffers[i.getRaw()] = new VHDLInterface::ProcessVariable(name+"_v", getSizeInBits(stream_value));
        bufferToSignal[buffers[i.getRaw()]] = parent->createSignal<VHDLInterface::Signal>(name, getSizeInBits(stream_value));
      }
      for(llvm::MultiForVar<int> i = windowBufferStart; !i.done(); ++i)
      {
        std::string name = getValueName(stream_value) + getVectorAsString(i.getRaw());
        window_states[i.getRaw()] = state->addState("S_WINDOW_"+name);
        //create the extra buffers
        buffers[i.getRaw()] = new VHDLInterface::ProcessVariable(name+"_v", getSizeInBits(stream_value));
        bufferToSignal[buffers[i.getRaw()]] = parent->createSignal<VHDLInterface::Signal>(name, getSizeInBits(stream_value));
      }
      //create the actual cases for the ready and read state
      cs->addStatement(s_ready, getFifoRequestDataStatement(p));
      cs->addStatement(s_ready, new VHDLInterface::IfStatement(p, VHDLInterface::Wrap(has_made_read_request) == VHDLInterface::ConstantInt::get(1),
                                                new VHDLInterface::AssignmentStatement(state, s_read, p)
                                                ));
      cs->addStatement(s_read, new VHDLInterface::AssignmentStatement(state, step_states[stepBufferStart.getRaw()], p));
      //the output buffer is a step-first architecture, so process the step buffer first
      VHDLInterface::MultiStatement* curStateMultiBlock = NULL;
      for(llvm::MultiForVar<int> i = stepBufferStart; !i.done(); i+=num_channels)
      {
        curStateMultiBlock = new VHDLInterface::MultiStatement(p);
        cs->addStatement(step_states[i.getRaw()], curStateMultiBlock);
        //check if we are a load state
        if( isLoadState(step_states[i.getRaw()]) )
          curStateMultiBlock->addStatement(getFifoLoadDataStatement(p));
        //output the buffer read
        curStateMultiBlock->addStatement(getBufferStoreElementStatement(outputValueValid, p, step_states[i.getRaw()]));
        //check the next state
        llvm::MultiForVar<int> n = i;
        n+=num_channels;
        //check whether we are a read state
        llvm::MultiForVar<int> nn = n;
        nn+=num_channels;
        if( (n.done() and i.getRaw() == stepBufferStart.getRaw()) //if its the only step state
            or
            (!n.done() and nn.done() ) //or its the element right before the last step state
          )
        {
          curStateMultiBlock->addStatement(getFifoRequestDataStatement(p));
        }
        //check whether we are the last step_state
        if( !n.done() )
        {
          //its not the end of the step states
          curStateMultiBlock->addStatement(new VHDLInterface::AssignmentStatement(state, step_states[n.getRaw()], p));
        }
        else
        {
          //its the end of the step states
          //increment indexes
          assert(step_states.find(stepBufferStart.getRaw()) != step_states.end());
          VHDLInterface::State* axisContinueState = step_states[stepBufferStart.getRaw()];
          VHDLInterface::State* axisDoneState = step_states[stepBufferStart.getRaw()];
          if(window_states.find(windowBufferStart.getRaw()) != window_states.end())
            axisDoneState = window_states[windowBufferStart.getRaw()];
          //if the next state is ever the current state, we cant just jump straight there; we need to go to the read state instead
          if( axisContinueState == step_states[i.getRaw()] )
            axisContinueState = s_read;
          if( axisDoneState == step_states[i.getRaw()] )
            axisDoneState = s_read;
          //if either of the next states are the load state, we need to make the next state assignment conditional
          VHDLInterface::Statement* axisContinueStatement = new VHDLInterface::AssignmentStatement(state, axisContinueState, p);
          VHDLInterface::Statement* axisDoneStatement = new VHDLInterface::AssignmentStatement(state, axisDoneState, p);
          if( axisContinueState == s_read or isLoadState(axisContinueState) )
          {
            axisContinueStatement = new VHDLInterface::IfStatement(p, VHDLInterface::Wrap(has_made_read_request) == VHDLInterface::ConstantInt::get(1),
                                                axisContinueStatement,
                                                new VHDLInterface::AssignmentStatement(state, s_ready, p)
                                                );
          }
          if( axisDoneState == s_read or isLoadState(axisDoneState) )
          {
            axisDoneStatement = new VHDLInterface::IfStatement(p, VHDLInterface::Wrap(has_made_read_request) == VHDLInterface::ConstantInt::get(1),
                                                new VHDLInterface::AssignmentStatement(state, axisDoneState, p),
                                                new VHDLInterface::AssignmentStatement(state, s_ready, p)
                                                );
          }
          curStateMultiBlock->addStatement(new VHDLInterface::IfStatement(p, livHandler->shouldIncrement(livHandler->getInnermostLIV()),
                                                                             (new VHDLInterface::MultiStatement(p))->addStatement(axisContinueStatement)->addStatement(livHandler->getIncrementStatement(p)),
                                                                             axisDoneStatement));
        }
      }
      //now process the window buffer
      for(llvm::MultiForVar<int> i = windowBufferStart; !i.done(); i+=num_channels)
      {
        curStateMultiBlock = new VHDLInterface::MultiStatement(p);
        cs->addStatement(window_states[i.getRaw()], curStateMultiBlock);
        //output the buffer read
        curStateMultiBlock->addStatement(getBufferStoreElementStatement(outputValueValid, p, window_states[i.getRaw()]));
        //check the next state
        llvm::MultiForVar<int> n = i;
        n+=num_channels;
        //check whether we are a read state
        llvm::MultiForVar<int> nn = n;
        nn+=num_channels;
        if( (n.done() and i.getRaw() == windowBufferStart.getRaw()) //if its the only window state
            or
            (!n.done() and nn.done() ) //or its the element right before the last step state
          )
        {
          curStateMultiBlock->addStatement(getFifoRequestDataStatement(p));
        }
        //check whether we are the last window_state
        if( !n.done() )
        {
          //its not the end of the window states
          curStateMultiBlock->addStatement(new VHDLInterface::AssignmentStatement(state, window_states[n.getRaw()], p));
        }
        else
        {
          //its the end of the window states
          //go back to the start of the step buffer
          assert(step_states.find(stepBufferStart.getRaw()) != step_states.end());
          curStateMultiBlock->addStatement(
                          new VHDLInterface::IfStatement(p, VHDLInterface::Wrap(has_made_read_request) == VHDLInterface::ConstantInt::get(1),
                                                new VHDLInterface::AssignmentStatement(state, step_states[stepBufferStart.getRaw()], p),
                                                new VHDLInterface::AssignmentStatement(state, s_ready, p)
                                                )
                          );
        }
      }
      assert( curStateMultiBlock );
      curStateMultiBlock->addStatement(new VHDLInterface::IfStatement(p, livHandler->getDoneCondition(), new VHDLInterface::AssignmentStatement(state, s_done, p)));
      curStateMultiBlock->addStatement(livHandler->getIncrementStatement(p));
      //push the data onto the fifo 
      MicroFifo outfifo(getValueName(stream_value)+"_outfifo");
      int numElementsPushed = inputs[index].data_in.size() + 2; //for overflow
      int smallestPowerOfTwo = 1;
      int smallestPowerOfTwoExponent = 0; //2^0 == 1
      while( smallestPowerOfTwo <= numElementsPushed )
      {
        smallestPowerOfTwo *= 2;
        smallestPowerOfTwoExponent += 1;
      }
      outfifo.mapAddressWidth(smallestPowerOfTwoExponent);
      outfifo.mapAlmostFullCount(numElementsPushed);
      outfifo.mapAlmostEmptyCount(0);
      outfifo.mapClk(parent->getStandardPorts().clk);
      outfifo.mapRst(parent->getStandardPorts().rst);
      outfifo.mapValidIn(outputValueValid);
      outfifo.mapFullOut(outfifo_full);
      outfifo.mapReadEnableIn(outputs[index].read_enable_in);
      outfifo.mapEmptyOut(outputs[index].empty_out);
      outfifo.mapInputAndOutputVector(outfifo_data_in, outputs[index].data_out, parent);
      outfifo.generateCode(parent);
    }
    else //its not a single element, and it doesnt walk along the innermost LIV - handle custom
    {
      assert(0 and "Cannot have output streams with innermost accessed LIV not the same as innermost written LIV!");
    }
  }
};

OutputSmartBuffer::AddressVariables::AddressVariables() : address_rdy_out(NULL), address_stall_in(NULL), clk(NULL)
{
}
OutputSmartBuffer::OutputSmartBuffer() : FifoInterfaceBlock<int,llvm::Value*>()
{
}
void OutputSmartBuffer::setAddressVariables(llvm::Value* stream, VHDLInterface::Variable* rdy, VHDLInterface::Variable* stall, std::vector<std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*> > base_count_pair, VHDLInterface::Variable* clk)
{
  addressVariables[stream].address_rdy_out = rdy;
  addressVariables[stream].address_stall_in = stall;
  addressVariables[stream].address_out = base_count_pair;
  addressVariables[stream].clk = clk;
}
void OutputSmartBuffer::generateVHDL(VHDLInterface::Entity* parent, llvm::ROCCCLoopInformation &loopInfo)
{
  if( loopInfo.outputBuffers.size() == 0 ) //if we dont have any streams - set done manually, and return
  {
    //create a process for the datapath LIV variables
    LoopInductionVariableHandler dp_livHandler;
    dp_livHandler.setIndexes(loopInfo.indexes);
    dp_livHandler.setChildren(loopInfo.loopChildren);
    dp_livHandler.setEndValues(loopInfo.endValues);
    for(std::vector<llvm::Value*>::iterator LIVI = loopInfo.indexes.begin(); LIVI != loopInfo.indexes.end(); ++LIVI)
    {
      VHDLInterface::Variable* new_endValue = NULL;
      if( dp_livHandler.getEndValueVHDLValue(*LIVI) == NULL and not dp_livHandler.isEndValueInfinity(*LIVI) )
      {
        llvm::Value* ev = loopInfo.endValues[*LIVI];
        if( parent->getPort(ev).size() == 0 )
          parent->addPort(getValueName(ev), getSizeInBits(ev), VHDLInterface::Port::INPUT, ev);
        new_endValue = parent->getPort(ev).at(0);
      }
      VHDLInterface::Signal* sig = parent->createSignal<VHDLInterface::Signal>("datapath_"+getValueName(*LIVI),getSizeInBits(*LIVI));
      dp_livHandler.setVHDLInterface(*LIVI, sig, new_endValue);
    }
    VHDLInterface::MultiStatementProcess* livProcess = parent->createProcess<VHDLInterface::MultiStatementProcess>();
    livProcess->addStatement(new VHDLInterface::IfStatement(livProcess, VHDLInterface::Wrap(parent->getDeclaration()->getStandardPorts().inputReady) == VHDLInterface::ConstantInt::get(1), dp_livHandler.getIncrementStatement(livProcess)));
    //assign done
    VHDLInterface::AssignmentStatement* done_assign = parent->createSynchronousStatement(parent->getSignalMappedToPort(parent->getStandardPorts().done));
    done_assign->addCase(VHDLInterface::ConstantInt::get(0), VHDLInterface::Wrap(parent->getStandardPorts().rst) == VHDLInterface::ConstantInt::get(1));
    done_assign->addCase(VHDLInterface::ConstantInt::get(1), /*VHDLInterface::Wrap(parent->getStandardPorts().inputReady) == VHDLInterface::ConstantInt::get(1) and*/ dp_livHandler.getDoneCondition());
    done_assign->addCase(parent->getSignalMappedToPort(parent->getStandardPorts().done));
    return;
  }
  assert(this->getInputIndexes().size() == 1);
  int in_index = this->getInputIndexes().at(0);
  bool has_multiple_output_streams = (loopInfo.outputBuffers.size() > 1);
  //create the input valid signal
  VHDLInterface::Signal* inputValueValid = NULL;
  VHDLInterface::Signal* outputStallIn = NULL;
  VHDLInterface::AssignmentStatement* outputStallInAssign = NULL;
  if( has_multiple_output_streams ) //only create the fifo reading signals if there are multiple streams
  {
    inputValueValid = parent->createSignal<VHDLInterface::Signal>("data_body_valid0", 1);
    outputStallIn = parent->createSignal<VHDLInterface::Signal>("data_body_output_stall", 1);
    outputStallInAssign = parent->createSynchronousStatement(outputStallIn);
    VHDLInterface::Signal* readEnable = parent->createSignal<VHDLInterface::Signal>("data_body_read_enable_t", 1);
    parent->createSynchronousStatement(inputs[in_index].read_enable_out, readEnable);
    VHDLInterface::AssignmentStatement* read_ass = parent->createSynchronousStatement(readEnable);
    read_ass->addCase(VHDLInterface::ConstantInt::get(0), VHDLInterface::Wrap(outputStallIn) == VHDLInterface::ConstantInt::get(1) or VHDLInterface::Wrap(inputs[in_index].empty_in) == VHDLInterface::ConstantInt::get(1));
    read_ass->addCase(VHDLInterface::ConstantInt::get(1));
    ShiftBuffer sb("outputShiftBuffer");
    sb.mapDataWidth(1);
    sb.mapShiftDepth(1);
    sb.mapRst(parent->getStandardPorts().rst);
    sb.mapClk(parent->getStandardPorts().clk);
    sb.mapDataIn(readEnable);
    sb.mapDataOut(inputValueValid);
    sb.generateCode(parent);
  }
  //each seperate fifo that feeds into the output smart buffers needs to write its full to the stall and use the
  //  valid as its valid
  std::vector<llvm::Value*> inBuffs = loopInfo.outputBuffers;
  //create the condition that will be written to the done signal
  VHDLInterface::CWrap done_condition = VHDLInterface::ConstCondition::get(true);
  for(std::vector<llvm::Value*>::iterator II = inBuffs.begin(); II != inBuffs.end(); ++II)
  {
    LoopInductionVariableHandler livHandler, address_livHandler;
    livHandler.setIndexes(loopInfo.indexes);
    livHandler.setChildren(loopInfo.loopChildren);
    livHandler.setEndValues(loopInfo.endValues);
    address_livHandler.setIndexes(loopInfo.indexes);
    address_livHandler.setChildren(loopInfo.loopChildren);
    address_livHandler.setEndValues(loopInfo.endValues);
    //create the microFifo that the output smart buffer will read from,
    //   making sure to connect the full to the stall signal
    VHDLInterface::Variable* micro_read = this->getInputReadEnableOut(in_index);
    VHDLInterface::Variable* micro_empty = this->getInputEmptyIn(in_index);
    std::vector<VHDLInterface::Variable*> micro_data = this->getInputDataIn(in_index);
    if( has_multiple_output_streams ) //only create a fifo for each stream if there are multiple streams
    {
      micro_read = parent->createSignal<VHDLInterface::Signal>("outsb_"+getValueName(*II)+"_micro_fifo0_read_in", 1);
      micro_empty = parent->createSignal<VHDLInterface::Signal>("outsb_"+getValueName(*II)+"_micro_fifo0_empty_out", 1);
      micro_data.clear();
      MicroFifo mf("outsb_" + getValueName(*II)+"_micro_fifo0");
      mf.mapClk(parent->getStandardPorts().clk);
      mf.mapRst(parent->getStandardPorts().rst);
      mf.mapAddressWidth(3);
      //calculate the total width of the packed fifo values
      int data_width = 0;
      std::vector<VHDLInterface::Variable*> data_in_vec = this->getInputDataIn(in_index);
      for(std::vector<VHDLInterface::Variable*>::iterator DI = data_in_vec.begin(); DI != data_in_vec.end(); ++DI)
      {
        data_width += (*DI)->getSize();
      }
      VHDLInterface::Variable* micro_data_in = parent->createSignal<VHDLInterface::Signal>("outsb_"+getValueName(*II)+"_micro_fifo0_data_in", data_width);
      VHDLInterface::Variable* micro_data_out = parent->createSignal<VHDLInterface::Signal>("outsb_"+getValueName(*II)+"_micro_fifo0_data_out", data_width);
      VHDLInterface::Value* micro_data_in_concat = NULL;
      //fill the data_in and data_out
      data_width = 0;
      for(std::vector<VHDLInterface::Variable*>::iterator DI = data_in_vec.begin(); DI != data_in_vec.end(); ++DI)
      {
        int next_data_width = data_width + (*DI)->getSize();
        if(micro_data_in_concat == NULL)
          micro_data_in_concat = *DI;
        else
          micro_data_in_concat = VHDLInterface::bitwise_concat(*DI, micro_data_in_concat);
        micro_data.push_back(VHDLInterface::BitRange::get(micro_data_out, next_data_width-1, data_width));
        data_width = next_data_width;
      }
      parent->createSynchronousStatement(micro_data_in, micro_data_in_concat);
      mf.mapDataWidth(data_width);
      mf.mapAlmostFullCount(5);
      mf.mapAlmostEmptyCount(0);
      //map the input side
      mf.mapDataIn(micro_data_in);
      mf.mapValidIn(inputValueValid);
      VHDLInterface::Variable* input_full = parent->createSignal<VHDLInterface::Signal>("outsb_"+getValueName(*II)+"_micro_fifo0_full_out", 1);
      mf.mapFullOut(input_full);
      //connect the full to the stall
      outputStallInAssign->addCase(VHDLInterface::ConstantInt::get(1), VHDLInterface::Wrap(input_full) == VHDLInterface::ConstantInt::get(1));
      //map the output side
      mf.mapDataOut(micro_data_out);
      mf.mapReadEnableIn(micro_read);
      mf.mapEmptyOut(micro_empty);
      mf.generateCode(parent);
    }
    //calculate the window dimensions
    std::vector<llvm::Value*> indexes = loopInfo.outputBufferLoopIndexes[*II];
    std::map<llvm::Value*,int> window_dimensions;
    std::vector<std::pair<llvm::Value*,std::vector<int> > > bufferElements = loopInfo.outputBufferIndexes[*II];
    for(unsigned c = 0; c < indexes.size(); ++c)
    {
      int max = -1, min = 10000000;
      //calc max and min of the bufferElements
      for(std::vector<std::pair<llvm::Value*,std::vector<int> > >::iterator VVPI = bufferElements.begin(); VVPI != bufferElements.end(); ++VVPI)
      {
        if( VVPI->second[c] > max )
          max = VVPI->second[c];
        if( VVPI->second[c] < min )
          min = VVPI->second[c];
      }
      //then set the window_dimension based on that max and min
      assert( max - min >= 0 );
      window_dimensions[indexes[c]] = max /*- min*/ + 1;
    }
    std::vector<llvm::Value*> writtenIndexesUsed;
    for(std::vector<llvm::Value*>::iterator LII = loopInfo.indexes.begin(); LII != loopInfo.indexes.end(); ++LII)
    {
      if( window_dimensions.find(*LII) != window_dimensions.end() )
        writtenIndexesUsed.push_back(*LII);
    }
    //the data in values are either real data ins, or need to have a temporary
    //  created for them; this will check each buffer element, find the llvm value
    //  that exists at that window point if the element exists, and if it exists,
    //  finds the matching Variable* from the vector of input elements and passes
    //  it along; if the element doesnt exist, ie is a dummy buffer element, then
    //  create a dummy signal to hold it.
    std::vector<VHDLInterface::Variable*> data_ins;
    for(llvm::MultiForVar<int> i = getMultiForVarForWindow(window_dimensions, std::vector<llvm::Value*>(indexes.rbegin(),indexes.rend())); !i.done(); ++i)
    {
      std::stringstream ss;
      ss << getValueName(*II) << "_s" << getVectorAsString(i.getRaw());
      llvm::Value* val = NULL;
      for(std::vector<std::pair<llvm::Value*,std::vector<int> > >::iterator IBI = loopInfo.outputBufferIndexes[*II].begin(); IBI != loopInfo.outputBufferIndexes[*II].end(); ++IBI)
      {
        if( std::vector<int>(IBI->second.rbegin(),IBI->second.rend()) == i.getRaw() )
          val = IBI->first;
      }
      bool found = false;
      std::vector<VHDLInterface::Variable*>::iterator D0I = micro_data.begin();
      std::vector<VHDLInterface::Variable*> true_input = this->getInputDataIn(in_index);
      for(std::vector<VHDLInterface::Variable*>::iterator DI = true_input.begin(); DI != true_input.end() and D0I != micro_data.end(); ++DI, ++D0I)
      {
        if( val != NULL and (*DI)->getLLVMValue() == val)
        {
          data_ins.push_back(*D0I);
          found = true;
        }       
      }
      assert(found or val == NULL);
      if( !found )
      {
        VHDLInterface::Signal* dummy = parent->createSignal<VHDLInterface::Signal>(ss.str(), getSizeInBits(*II));
        parent->createSynchronousStatement(dummy, VHDLInterface::ConstantInt::get(0));
        data_ins.push_back(dummy);
      }
    }
    //connect the microFifo with the smart buffer
    OutputSmartBufferImpl* isb = new OutputSmartBufferImpl(*II);
    isb->initializeInputInterfacePorts(*II,
                         micro_read,
                         micro_empty,
                         data_ins
                                     );
    //create the address generator
    AddressGenerator* ag = new OutputAddressGenerator(*II);
    ag->initializeVHDLInterface(
                        addressVariables[*II].address_rdy_out,
                        addressVariables[*II].address_stall_in,
                        addressVariables[*II].address_out
                                     );
    //set the loop induction variables
    isb->setAccessedLIV(indexes);
    isb->setWrittenLIV(loopInfo.indexes);
    for(std::vector<llvm::Value*>::iterator LIVI = loopInfo.indexes.begin(); LIVI != loopInfo.indexes.end(); ++LIVI)
    {
      VHDLInterface::Variable* new_endValue = NULL;
      if( livHandler.getEndValueVHDLValue(*LIVI) == NULL and not livHandler.isEndValueInfinity(*LIVI) )
      {
        assert( address_livHandler.getEndValueVHDLValue(*LIVI) == NULL and not address_livHandler.isEndValueInfinity(*LIVI) );
        llvm::Value* ev = loopInfo.endValues[*LIVI];
        if( parent->getPort(ev).size() == 0 )
          parent->addPort(getValueName(ev), getSizeInBits(ev), VHDLInterface::Port::INPUT, ev);
        new_endValue = parent->getPort(ev).at(0);
      }
      livHandler.setVHDLInterface(*LIVI, parent->createSignal<VHDLInterface::Signal>(getValueName(*II)+"_"+getValueName(*LIVI),getSizeInBits(*LIVI)), new_endValue);
      address_livHandler.setVHDLInterface(*LIVI, parent->createSignal<VHDLInterface::Signal>(getValueName(*II)+"_address_"+getValueName(*LIVI),getSizeInBits(*LIVI)), new_endValue);
    }
    //calculate the step dimensions 
    std::map<llvm::Value*,int> step_dimensions;
    step_dimensions = window_dimensions;
    if( window_dimensions.find(livHandler.getInnermostLIV()) != window_dimensions.end() and 
        window_dimensions[livHandler.getInnermostLIV()] >= loopInfo.outputBufferNumInvalidated[*II] )
    {
      step_dimensions[livHandler.getInnermostLIV()] = loopInfo.outputBufferNumInvalidated[*II];
    }
    //now set the calculated window and step dimensions
    isb->setWindowDimensions(window_dimensions);
    isb->setStepDimensions(step_dimensions);                           
    //attach the output ports
    assert(this->getOutputReadEnableIn(*II));
    isb->initializeOutputInterfacePorts(*II,
                         this->getOutputReadEnableIn(*II),
                         this->getOutputEmptyOut(*II),
                         this->getOutputDataOut(*II)
                                     );
    isb->createVHDL(parent, &livHandler);
    BufferSpaceAccesser window_accesser = getWindowBufferSpaceAccessor(window_dimensions, std::vector<llvm::Value*>(indexes.rbegin(),indexes.rend()), writtenIndexesUsed, addressVariables[*II].address_out.size());
    BufferSpaceAccesser step_accesser = getStepBufferSpaceAccessor(window_dimensions, std::vector<llvm::Value*>(indexes.rbegin(),indexes.rend()), writtenIndexesUsed, addressVariables[*II].address_out.size());
    //move the window space over a number of elements equal to the size of the step space along the innermost written LIV
    window_accesser.getBufferSpace()->getTopLeft()->setAbsolute(writtenIndexesUsed.back(), 
                                                  window_accesser.getBufferSpace()->getTopLeft()->getAbsolute(writtenIndexesUsed.back()) + step_accesser.getBufferSpace()->getBottomRight()->getAbsolute(writtenIndexesUsed.back()) ); 
    ag->createVHDL(parent, &address_livHandler, window_accesser, step_accesser, addressVariables[*II].clk);
    //set the done condition
    done_condition = done_condition and isb->getDoneCondition();
  }
  VHDLInterface::Signal* temp_done = parent->createSignal<VHDLInterface::Signal>("done_temp", 1);
  VHDLInterface::AssignmentStatement* done_assign = parent->createSynchronousStatement(temp_done);
  done_assign->addCase(VHDLInterface::ConstantInt::get(0), VHDLInterface::Wrap(parent->getStandardPorts().rst) == VHDLInterface::ConstantInt::get(1));
  done_assign->addCase(VHDLInterface::ConstantInt::get(1), done_condition);
  done_assign->addCase(temp_done);
  ShiftBuffer done_shift("done_shift");
  done_shift.mapDataWidth(1);
  done_shift.mapShiftDepth(10);
  done_shift.mapRst(parent->getStandardPorts().rst);
  done_shift.mapClk(parent->getStandardPorts().clk);
  done_shift.mapDataIn(temp_done);
  done_shift.mapDataOut(parent->getStandardPorts().done);
  done_shift.generateCode(parent);
  if( has_multiple_output_streams ) //only need to finalize the datapath fifo reading signals if there are multiple streams
    outputStallInAssign->addCase(VHDLInterface::ConstantInt::get(0));
}

