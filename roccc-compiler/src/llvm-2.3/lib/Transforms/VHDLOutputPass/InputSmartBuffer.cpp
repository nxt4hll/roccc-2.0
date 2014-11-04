#include "rocccLibrary/VHDLComponents/InputSmartBuffer.h"

#include <sstream>
#include <vector>
#include <map>
#include <list>
#include <algorithm>

#include "rocccLibrary/SizeInBits.h"
#include "rocccLibrary/GetValueName.h"
#include "rocccLibrary/MultiForVar.hpp"
#include "rocccLibrary/VHDLComponents/FifoInterfaceBlock.hpp"
#include "rocccLibrary/VHDLComponents/LoopInductionVariableHandler.h"
#include "rocccLibrary/VHDLComponents/ShiftBuffer.h"
#include "rocccLibrary/VHDLComponents/MicroFifo.h"
#include "rocccLibrary/VHDLComponents/AddressGenerator.h"

#include "rocccLibrary/InternalWarning.h"

#include "rocccLibrary/Window/RelativeLocation.h"

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

//helper function to create all the MultiForVars we need to walk the buffers
llvm::MultiForVar<int> getMultiForVarForWindow(std::map<llvm::Value*,int> window, std::vector<llvm::Value*> accessOrder)
{
  assert(window.size() == accessOrder.size());
  return llvm::MultiForVar<int>(
              std::vector<int>(window.size()), //starts at dimensions all 0
              getVectorOfMappedValues(window, accessOrder)
            );
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
//specialization for llvm::Value*
template<>
std::string getVectorAsString<llvm::Value*>(std::vector<llvm::Value*> v)
{
  std::stringstream ss;
  for(unsigned c = 0; c < v.size(); ++c)
  {
    assert(v[c]);
    ss << "_" << v[c]->getName();
  }
  return ss.str();
}

VHDLInterface::Statement* getBufferShiftStatement(VHDLInterface::Process* p, std::map<std::vector<int>,VHDLInterface::Variable*> buffers, CountingPointer<Window::Window> window, Window::Location::DIMENSION_INDEX_TYPE stepDim, CountingPointer<Window::Location> cur_loc, VHDLInterface::Variable* DAT_IN, Window::LocationIterator::ACCESS_ORDER_TYPE access_order)
{
  VHDLInterface::MultiStatement* ret = new VHDLInterface::MultiStatement(p);
  Window::LocationIterator::ACCESS_ORDER_TYPE ao;
  ao.push_back(stepDim);
  Window::LocationIterator EI = Window::getElementIterator<Window::LocationIterator>(window, ao);
  Window::setLocationDimensionsToLocation(EI.getIteratedLocation(), cur_loc);
  if( window->getBottomRight()->getRelative(stepDim) > 0 )
    EI.getIteratedLocation()->setAbsolute(stepDim, 1);
  VHDLInterface::Variable* last_val = NULL;
  for(; !EI.isDone(); ++EI)
  {
    CountingPointer<Window::Location> tloc = EI.getIteratedLocation();
    CountingPointer<Window::RelativeLocation> cp = tloc.dyn_cast<Window::RelativeLocation>();
    if( !cp.isNull() )
    {
      tloc = cp->getLocus();
    }
    std::vector<int> loc_vec = getVectorOfMappedValues(Window::getAbsoluteDimensionMapFromLocation(tloc), access_order);
    if( last_val )
      ret->addStatement(new VHDLInterface::AssignmentStatement(last_val, buffers[loc_vec], p));
    last_val = buffers[loc_vec];
  }
  assert(last_val and "Could not create shift buffer! Need at least 1 element!");
  ret->addStatement(new VHDLInterface::AssignmentStatement(last_val, DAT_IN, p));
  return ret;
}

CountingPointer<Window::Location> getRelatedLocation(CountingPointer<Window::Location> cur_loc)
{
  //if its a relative location (and it should be if its from a window!) we actually want the locus, not the relative corner.
  CountingPointer<Window::RelativeLocation> cp = cur_loc.dyn_cast<Window::RelativeLocation>();
  if( !cp.isNull() )
  {
    cur_loc = cp->getLocus();
  }
  return cur_loc;
}

template<class S, class T>
S& operator << (S& st, const std::vector<T> m)
{
  st << "{";
  for(typename std::vector<T>::const_iterator MI = m.begin(); MI != m.end(); ++MI)
  {
    if( MI != m.begin() )
      st << ", ";
    st << *MI;
  }
  st << "}";
  return st;
}

template<class S, class T, class D>
S& operator << (S& st, const std::map<T,D> m)
{
  for(typename std::map<T,D>::const_iterator MI = m.begin(); MI != m.end(); ++MI)
  {
    if( MI != m.begin() )
      st << ", ";
    st << MI->first << " = " << MI->second;
  }
  return st;
}

//TODO: located in AddressGenerator.cpp - move together
VHDLInterface::State* getStateNamed(VHDLInterface::StateVar* state, std::string name);
std::string getLocationAsString(CountingPointer<Window::Location> loc, Window::LocationIterator::ACCESS_ORDER_TYPE ao);

class InputSmartBufferImpl : public FifoInterfaceBlock<llvm::Value*,llvm::Value*> {
  typedef llvm::Value* INPUT_TYPE;
  typedef llvm::Value* OUTPUT_TYPE;
  //what stream is this, anyways?!
  llvm::Value* stream_value;
public:
  InputSmartBufferImpl(llvm::Value* v) : FifoInterfaceBlock<INPUT_TYPE,OUTPUT_TYPE>(), stream_value(v)
  {
    assert(v);
  }
  void createVHDL(VHDLInterface::Entity* parent, LoopInductionVariableHandler* livHandler, BufferSpaceAccesser window_accesser, BufferSpaceAccesser step_accesser, std::vector<llvm::Value*> written_LIV)
  {
    llvm::Value* index = stream_value;
    bool is_single_element = true;
    Window::LocationIterator::ACCESS_ORDER_TYPE access_order = window_accesser.getTimeIterator().getAccessOrder();
    Window::LocationIterator::ACCESS_ORDER_TYPE r_access_order(access_order.rbegin(), access_order.rend());
    for(std::vector<llvm::Value*>::iterator ALIVI = access_order.begin(); ALIVI != access_order.end(); ++ALIVI)
    {
      if( window_accesser.getBufferSpace()->getBottomRight()->getAbsolute(*ALIVI) > 1 or step_accesser.getBufferSpace()->getBottomRight()->getAbsolute(*ALIVI) > 1 )
        is_single_element = false;
    }
    if( is_single_element ) //if its a single element, just do pass through
    {
      assert( outputs.size() == inputs.size() );
      assert( outputs.size() == 1 );
      assert(inputs.find(index) != inputs.end());
      assert(outputs.find(index) != outputs.end());
      assert(outputs[index].read_enable_in);
      assert(inputs[index].read_enable_out);
      parent->createSynchronousStatement(inputs[index].read_enable_out, outputs[index].read_enable_in);
      assert(outputs[index].empty_out);
      assert(inputs[index].empty_in);
      parent->createSynchronousStatement(outputs[index].empty_out, inputs[index].empty_in);
      if( outputs[index].data_out.size() != inputs[index].data_in.size() )
      {
        for(std::vector<llvm::Value*>::iterator ALIVI = access_order.begin(); ALIVI != access_order.end(); ++ALIVI)
        {
          INTERNAL_ERROR("window[" << (*ALIVI)->getName() << "] = " << window_accesser.getBufferSpace()->getBottomRight()->getAbsolute(*ALIVI) << "\n");
        }
        INTERNAL_ERROR("outputs[index].data_out.size() = " << outputs[index].data_out.size() << ", but inputs[index].data_in.size() = " << inputs[index].data_in.size() << "!\n");
        for(std::vector<VHDLInterface::Variable*>::iterator DI = this->inputs[index].data_in.begin(); DI != this->inputs[index].data_in.end(); ++DI)
        {
          INTERNAL_MESSAGE("val_in:" << (*DI)->getInternalName() << "\n");
        }
        for(std::vector<VHDLInterface::Variable*>::iterator DI = this->outputs[index].data_out.begin(); DI != this->outputs[index].data_out.end(); ++DI)
        {
          INTERNAL_MESSAGE("val_out:" << (*DI)->getInternalName() << "\n");
        }
      }
      assert(outputs[index].data_out.size() == inputs[index].data_in.size());
      std::vector<VHDLInterface::Variable*>::iterator DOI = outputs[index].data_out.begin();
      for(std::vector<VHDLInterface::Variable*>::iterator DII = inputs[index].data_in.begin(); DOI != outputs[index].data_out.end() and DII != inputs[index].data_in.end(); ++DOI, ++DII)
      {
        parent->createSynchronousStatement(*DOI, *DII);
      }
    }
    // its not passthrough, so use the developed smart buffering approach.
    else
    {
      //create the input valid signal
      VHDLInterface::Signal* inputValueValid = parent->createSignal<VHDLInterface::Signal>(getValueName(stream_value)+"_valid0", 1);
      VHDLInterface::Signal* outputStallIn = parent->createSignal<VHDLInterface::Signal>(getValueName(stream_value)+"_output_stall", 1);
      {//none of this needs to be exposed to the rest of the body
        VHDLInterface::Signal* readEnable = parent->createSignal<VHDLInterface::Signal>(getValueName(stream_value)+"_read_enable_t", 1);
        parent->createSynchronousStatement(inputs[index].read_enable_out, readEnable);
        VHDLInterface::AssignmentStatement* read_ass = parent->createSynchronousStatement(readEnable);
        read_ass->addCase(VHDLInterface::ConstantInt::get(0), VHDLInterface::Wrap(outputStallIn) == VHDLInterface::ConstantInt::get(1) or VHDLInterface::Wrap(inputs[index].empty_in) == VHDLInterface::ConstantInt::get(1));
        read_ass->addCase(VHDLInterface::ConstantInt::get(1));
        ShiftBuffer sb(getValueName(stream_value)+"_inputShiftBuffer");
        sb.mapDataWidth(1);
        sb.mapShiftDepth(1);
        sb.mapRst(parent->getStandardPorts().rst);
        sb.mapClk(parent->getStandardPorts().clk);
        sb.mapDataIn(readEnable);
        sb.mapDataOut(inputValueValid);
        sb.generateCode(parent);
      }
      //create the smart buffering stage
      VHDLInterface::MultiStatementProcess* p = parent->createProcess<VHDLInterface::MultiStatementProcess>();
      std::map<std::vector<int>,VHDLInterface::Variable*> buffers;
      std::map<VHDLInterface::Variable*, VHDLInterface::Variable*> bufferToSignal;
      VHDLInterface::Variable* outputValueValid = parent->createSignal<VHDLInterface::Signal>(getValueName(stream_value)+"_valid1",1);
      p->addStatement(new VHDLInterface::AssignmentStatement(outputValueValid, VHDLInterface::ConstantInt::get(0), p));
      VHDLInterface::StateVar* state = parent->createSignal<VHDLInterface::StateVar>(getValueName(stream_value)+"_buff_state",0);
      VHDLInterface::MultiStatement* ms = new VHDLInterface::MultiStatement(p);
      p->addStatement(new VHDLInterface::IfStatement(p, VHDLInterface::Wrap(inputValueValid) == VHDLInterface::ConstantInt::get(1), ms));
      VHDLInterface::CaseStatement* cs = new VHDLInterface::CaseStatement(p, state);
      ms->addStatement( cs );
      //create the buffer-state buffers
      for(Window::LocationIterator TI = window_accesser.getTimeIterator(); !TI.isDone(); ++TI)
      {
        //iterate across the channel window
        for(Window::LocationIterator BI = Window::getElementIterator<Window::LocationIterator>(window_accesser.getChannelSpace(), access_order); !BI.isDone(); ++BI)
        {
          //and iterate across the address windows
          for(Window::LocationIterator CI = window_accesser.getChannelIterator(); !CI.isDone(); ++CI)
          {
            //create the buffer, and the signals
            CountingPointer<Window::Location> cur_loc = BI.getIteratedLocation();
            //if its a relative location (and it should be if its from a window!) we actually want the locus, not the relative corner.
            CountingPointer<Window::RelativeLocation> cp = cur_loc.dyn_cast<Window::RelativeLocation>();
            if( !cp.isNull() )
            {
              cur_loc = cp->getLocus();
            }
            std::vector<int> loc_vec = getVectorOfMappedValues(Window::getAbsoluteDimensionMapFromLocation(cur_loc), access_order);
            //create the buffers
            buffers[loc_vec] = new VHDLInterface::ProcessVariable(getValueName(stream_value)+getVectorAsString(loc_vec)+"_v", getSizeInBits(stream_value));
            bufferToSignal[buffers[loc_vec]] = parent->createSignal<VHDLInterface::Signal>(getValueName(stream_value)+getVectorAsString(loc_vec), getSizeInBits(stream_value));
          }
          window_accesser.getChannelIterator().reset();
        }
        Window::getElementIterator<Window::LocationIterator>(window_accesser.getChannelSpace(), access_order).reset();
      }
      window_accesser.getTimeIterator().reset();
      //create the step-state buffers
      for(Window::LocationIterator TI = step_accesser.getTimeIterator(); !TI.isDone(); ++TI)
      {
        //iterate across the channel window
        for(Window::LocationIterator BI = Window::getElementIterator<Window::LocationIterator>(step_accesser.getChannelSpace(), access_order); !BI.isDone(); ++BI)
        {
          //and iterate across the address windows
          for(Window::LocationIterator CI = step_accesser.getChannelIterator(); !CI.isDone(); ++CI)
          {
            //create the buffer, and the signals
            CountingPointer<Window::Location> cur_loc = BI.getIteratedLocation();
            //if its a relative location (and it should be if its from a window!) we actually want the locus, not the relative corner.
            CountingPointer<Window::RelativeLocation> cp = cur_loc.dyn_cast<Window::RelativeLocation>();
            if( !cp.isNull() )
            {
              cur_loc = cp->getLocus();
            }
            std::vector<int> loc_vec = getVectorOfMappedValues(Window::getAbsoluteDimensionMapFromLocation(cur_loc), access_order);
            //create the buffers
            buffers[loc_vec] = new VHDLInterface::ProcessVariable(getValueName(stream_value)+getVectorAsString(loc_vec)+"_v", getSizeInBits(stream_value));
            bufferToSignal[buffers[loc_vec]] = parent->createSignal<VHDLInterface::Signal>(getValueName(stream_value)+getVectorAsString(loc_vec), getSizeInBits(stream_value));
          }
          step_accesser.getChannelIterator().reset();
        }
        Window::getElementIterator<Window::LocationIterator>(step_accesser.getChannelSpace(), access_order).reset();
      }
      step_accesser.getTimeIterator().reset();
      //create a variable to hold the previous state we were in
      VHDLInterface::State* prev_state = NULL;
      //process the buffer states
      for(Window::LocationIterator TI = window_accesser.getTimeIterator(); !TI.isDone(); ++TI)
      {
        Window::resetRelativeDimension(window_accesser.getChannelSpace()->getTopLeft());
        CountingPointer<Window::Location> cur_loc = window_accesser.getChannelSpace()->getTopLeft();
        std::vector<int> loc_vec = getVectorOfMappedValues(Window::getAbsoluteDimensionMapFromLocation(cur_loc), access_order);
        //create the state we are in
        VHDLInterface::State* cur_state = getStateNamed(state, "S_WINDOW"+getVectorAsString(loc_vec));
        //set up the input data iterator
        std::vector<VHDLInterface::Variable*>::iterator DAT_IN = inputs[index].data_in.begin();
        //iterate across the channel window
        for(Window::LocationIterator BI = Window::getElementIterator<Window::LocationIterator>(window_accesser.getChannelSpace(), access_order); !BI.isDone(); ++BI)
        {
          //and iterate across the address windows
          for(Window::LocationIterator CI = window_accesser.getChannelIterator(); !CI.isDone(); ++CI)
          {
            //create the buffer, and the signals
            cur_loc = BI.getIteratedLocation();
            //if its a relative location (and it should be if its from a window!) we actually want the locus, not the relative corner.
            CountingPointer<Window::RelativeLocation> cp = cur_loc.dyn_cast<Window::RelativeLocation>();
            if( !cp.isNull() )
            {
              cur_loc = cp->getLocus();
            }
            //because we want to shift the data in, start writing the buffer elements at the location that the shift buffer is long
            {
              CountingPointer<Window::Location> rel(new Window::RelativeLocation(cur_loc));
              CountingPointer<Window::Location> desired = step_accesser.getBufferSpace()->getBottomRight();
              rel->setRelative(livHandler->getInnermostLIV(), desired->getRelative(livHandler->getInnermostLIV()));
              cur_loc = rel;
            }
            loc_vec = getVectorOfMappedValues(Window::getAbsoluteDimensionMapFromLocation(cur_loc), access_order);
            //wrap around the data_in vector, if need be
            if( DAT_IN == inputs[index].data_in.end() )
            {
              DAT_IN = inputs[index].data_in.begin();
              //wrapping around forces us into a new state
              cur_state = getStateNamed(state, "S_WINDOW"+getVectorAsString(loc_vec));
            }
            //load the incoming data element
            cs->addStatement(cur_state, new VHDLInterface::AssignmentStatement(buffers[loc_vec], *DAT_IN, p));
            //write a transition from the previous state to this state
            if( prev_state != NULL and prev_state != cur_state )
            {
              cs->addStatement(prev_state, new VHDLInterface::AssignmentStatement(state, cur_state, p));
            }
            prev_state = cur_state;
            //increment the data_in element we are going to use
            ++DAT_IN;
          }
          window_accesser.getChannelIterator().reset();
        }
        assert( DAT_IN == inputs[index].data_in.end() and "Number of data channels not factor of buffer window!" );
        Window::getElementIterator<Window::LocationIterator>(window_accesser.getChannelSpace(), access_order).reset();
      }
      window_accesser.getTimeIterator().reset();
      bool has_window_states = (prev_state != NULL);
      //handle the step states
      for(Window::LocationIterator TI = step_accesser.getTimeIterator(); !TI.isDone(); ++TI)
      {
        Window::resetRelativeDimension(step_accesser.getChannelSpace()->getTopLeft());
        CountingPointer<Window::Location> cur_loc = step_accesser.getChannelSpace()->getTopLeft();
        std::vector<int> loc_vec = getVectorOfMappedValues(Window::getAbsoluteDimensionMapFromLocation(cur_loc), access_order);
        //create the state we are in
        VHDLInterface::State* cur_state = getStateNamed(state, "S_STEP"+getVectorAsString(loc_vec));
        //set up the input data iterator
        std::vector<VHDLInterface::Variable*>::iterator DAT_IN = inputs[index].data_in.begin();
        //iterate across the channel window
        for(Window::LocationIterator BI = Window::getElementIterator<Window::LocationIterator>(step_accesser.getChannelSpace(), access_order); !BI.isDone(); ++BI)
        {
          //and iterate across the address windows
          for(Window::LocationIterator CI = step_accesser.getChannelIterator(); !CI.isDone(); ++CI)
          {
            //create the buffer, and the signals
            cur_loc = BI.getIteratedLocation();
            //if its a relative location (and it should be if its from a window!) we actually want the locus, not the relative corner.
            CountingPointer<Window::RelativeLocation> cp = cur_loc.dyn_cast<Window::RelativeLocation>();
            if( !cp.isNull() )
            {
              cur_loc = cp->getLocus();
            }
            loc_vec = getVectorOfMappedValues(Window::getAbsoluteDimensionMapFromLocation(cur_loc), access_order);
            //wrap around the data_in vector, if need be
            if( DAT_IN == inputs[index].data_in.end() )
            {
              DAT_IN = inputs[index].data_in.begin();
              //wrapping around forces us into a new state
              cur_state = getStateNamed(state, "S_STEP"+getVectorAsString(loc_vec));
            }
            //load the incoming data element with a shift buffer statement
            cs->addStatement(cur_state, getBufferShiftStatement(p, buffers, step_accesser.getBufferSpace(), livHandler->getInnermostLIV(), cur_loc, *DAT_IN, access_order));
            //write a transition from the previous state to this state
            if( prev_state != NULL and prev_state != cur_state )
            {
              cs->addStatement(prev_state, new VHDLInterface::AssignmentStatement(state, cur_state, p));
            }
            prev_state = cur_state;
            //increment the data_in element we are going to use
            ++DAT_IN;
          }
          step_accesser.getChannelIterator().reset();
        }
        assert( DAT_IN == inputs[index].data_in.end() and "Number of data channels not factor of step window!" );
        Window::getElementIterator<Window::LocationIterator>(step_accesser.getChannelSpace(), access_order).reset();
      }
      step_accesser.getTimeIterator().reset();
      //write a transition from the last step state to wherever it happens to go
      {//own namespace
        std::vector<int> first_step_vec = getVectorOfMappedValues(Window::getAbsoluteDimensionMapFromLocation(step_accesser.getBufferSpace()->getTopLeft()), access_order);
        VHDLInterface::State* first_step_state = getStateNamed(state, "S_STEP"+getVectorAsString(first_step_vec));
        std::vector<int> first_window_vec = getVectorOfMappedValues(Window::getAbsoluteDimensionMapFromLocation(window_accesser.getBufferSpace()->getTopLeft()), access_order);
        VHDLInterface::State* first_window_state = first_step_state;
        if( has_window_states )
          first_window_state = getStateNamed(state, "S_WINDOW"+getVectorAsString(first_window_vec));
        cs->addStatement(prev_state, 
                      new VHDLInterface::IfStatement(p, livHandler->shouldIncrement(livHandler->getInnermostLIV()),
                                              new VHDLInterface::AssignmentStatement(state, first_step_state, p),
                                              new VHDLInterface::AssignmentStatement(state, first_window_state, p)
                                              )
                         );
        cs->addStatement(prev_state, livHandler->getIncrementStatement(p));
        //Create the done state
        VHDLInterface::State* s_done = state->addState("S_DONE");
        cs->addStatement(s_done, new VHDLInterface::AssignmentStatement(state, s_done, p));
        //transition from the last state to the done state, assuming no shenangans
        cs->addStatement(prev_state, new VHDLInterface::IfStatement(p, livHandler->getDoneCondition(), new VHDLInterface::AssignmentStatement(state, s_done, p))); 
      }
      //also add a load of all of the buffer variables to the buffer signals
      for(std::map<VHDLInterface::Variable*, VHDLInterface::Variable*>::iterator BSI = bufferToSignal.begin(); BSI != bufferToSignal.end(); ++BSI)
      {
        cs->addStatement(prev_state, new VHDLInterface::AssignmentStatement(BSI->second, BSI->first, p));
      }
      cs->addStatement(prev_state, new VHDLInterface::AssignmentStatement(outputValueValid, VHDLInterface::ConstantInt::get(1), p));
      //push the data onto the fifo
      MicroFifo mfifo(getValueName(stream_value)+"_micro_fifo");
      mfifo.mapAddressWidth(3);
      mfifo.mapDataWidth(buffers.size()*getSizeInBits(stream_value));
      mfifo.mapAlmostFullCount(2);
      mfifo.mapAlmostEmptyCount(0);
      mfifo.mapClk(parent->getStandardPorts().clk);
      mfifo.mapRst(parent->getStandardPorts().rst);
      mfifo.mapValidIn(outputValueValid);
      mfifo.mapFullOut(outputStallIn);
      std::vector<VHDLInterface::Variable*> micro_data_in;
      {//seperate namespace to fill the micro_data_in vector
        std::map<llvm::Value*,int> window = Window::getAbsoluteDimensionMapFromLocation(step_accesser.getBufferSpace()->getBottomRight());
        for(llvm::MultiForVar<int> i(std::vector<int>(access_order.size()),getVectorOfMappedValues(window, /*written_LIV*/access_order)); !i.done(); ++i)
        {
          if( !buffers[i.getRaw()] )
          {
            INTERNAL_ERROR("Buffer:\n" << buffers << "\n" <<
                             "element " << i.getRaw() << " does not exist!\n");
          }
          assert( buffers[i.getRaw()] );
          assert( bufferToSignal[buffers[i.getRaw()]] );
          micro_data_in.push_back(bufferToSignal[buffers[i.getRaw()]]);
        }
      }
      mfifo.mapInputAndOutputVector(micro_data_in, outputs[index].data_out, parent);
      mfifo.mapReadEnableIn(outputs[index].read_enable_in);
      mfifo.mapEmptyOut(outputs[index].empty_out);
      mfifo.generateCode(parent);
    }
  }  
};

InputSmartBuffer::AddressVariables::AddressVariables() : address_rdy_out(NULL), address_stall_in(NULL), clk(NULL)
{
}
InputSmartBuffer::InputSmartBuffer() : FifoInterfaceBlock<llvm::Value*,int>()
{
}
void InputSmartBuffer::setAddressVariables(llvm::Value* stream, VHDLInterface::Variable* rdy, VHDLInterface::Variable* stall, std::vector<std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*> > base_count_pair, VHDLInterface::Variable* clk)
{
  addressVariables[stream].address_rdy_out = rdy;
  addressVariables[stream].address_stall_in = stall;
  addressVariables[stream].address_out = base_count_pair;
  addressVariables[stream].clk = clk;
}
void InputSmartBuffer::generateVHDL(VHDLInterface::Entity* liv, llvm::ROCCCLoopInformation &loopInfo)
{
  assert(this->getOutputIndexes().size() == 1);
  int out_index = this->getOutputIndexes().at(0);
  std::vector<FifoInterfaceBlock<llvm::Value*,llvm::Value*>*> inputBuffers;
  std::vector<llvm::Value*> inBuffs = loopInfo.inputBuffers;
  for(std::vector<llvm::Value*>::iterator II = inBuffs.begin(); II != inBuffs.end(); ++II)
  {
    LoopInductionVariableHandler livHandler, address_livHandler;
    livHandler.setIndexes(loopInfo.indexes);
    livHandler.setChildren(loopInfo.loopChildren);
    livHandler.setEndValues(loopInfo.endValues);
    address_livHandler.setIndexes(loopInfo.indexes);
    address_livHandler.setChildren(loopInfo.loopChildren);
    address_livHandler.setEndValues(loopInfo.endValues);
    InputSmartBufferImpl* isb = new InputSmartBufferImpl(*II);
    inputBuffers.push_back(isb);
    isb->initializeInputInterfacePorts(*II,
                         this->getInputReadEnableOut(*II),
                         this->getInputEmptyIn(*II),
                         this->getInputDataIn(*II)
                                     );
    AddressGenerator* ag = new InputAddressGenerator(*II);
    ag->initializeVHDLInterface(
                        addressVariables[*II].address_rdy_out,
                        addressVariables[*II].address_stall_in,
                        addressVariables[*II].address_out
                                     );
    std::vector<llvm::Value*> indexes = loopInfo.inputBufferLoopIndexes[*II];
    for(std::vector<llvm::Value*>::iterator LIVI = loopInfo.indexes.begin(); LIVI != loopInfo.indexes.end(); ++LIVI)
    {
      VHDLInterface::Variable* new_endValue = NULL;
      if( livHandler.getEndValueVHDLValue(*LIVI) == NULL and not livHandler.isEndValueInfinity(*LIVI) )
      {
        assert( address_livHandler.getEndValueVHDLValue(*LIVI) == NULL and not address_livHandler.isEndValueInfinity(*LIVI) );
        llvm::Value* ev = loopInfo.endValues[*LIVI];
        if( liv->getPort(ev).size() == 0 )
          liv->addPort(getValueName(ev), getSizeInBits(ev), VHDLInterface::Port::INPUT, ev);
        new_endValue = liv->getPort(ev).at(0);
      }
      livHandler.setVHDLInterface(*LIVI, liv->createSignal<VHDLInterface::Signal>(getValueName(*II)+"_"+getValueName(*LIVI),getSizeInBits(*LIVI)), new_endValue);
      address_livHandler.setVHDLInterface(*LIVI, liv->createSignal<VHDLInterface::Signal>(getValueName(*II)+"_address_"+getValueName(*LIVI),getSizeInBits(*LIVI)), new_endValue);
    }
    std::map<llvm::Value*,int> window_dimensions;
    std::vector<std::pair<llvm::Value*,std::vector<int> > > bufferElements = loopInfo.inputBufferIndexes[*II];
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
    std::vector<VHDLInterface::Variable*> data_outs;
    //the data out values are either real data outs, or need to have a temporary
    //  created for them; this will check each buffer element, find the llvm value
    //  that exists at that window point if the element exists, and if it exists,
    //  finds the matching Variable* from the vector of output elements and passes
    //  it along; if the element doesnt exist, ie is a dummy buffer element, then
    //  create a dummy signal to hold it.
    for(llvm::MultiForVar<int> i = getMultiForVarForWindow(window_dimensions, std::vector<llvm::Value*>(indexes.rbegin(),indexes.rend())); !i.done(); ++i)
    {
      std::stringstream ss;
      ss << getValueName(*II) << "_s" << getVectorAsString(i.getRaw());
      llvm::Value* val = NULL;
      for(std::vector<std::pair<llvm::Value*,std::vector<int> > >::iterator IBI = loopInfo.inputBufferIndexes[*II].begin(); IBI != loopInfo.inputBufferIndexes[*II].end(); ++IBI)
      {
        if( std::vector<int>(IBI->second.rbegin(),IBI->second.rend()) == i.getRaw() )
          val = IBI->first;
      }
      bool found = false;
      std::vector<VHDLInterface::Variable*> dataO = this->getOutputDataOut(out_index);
      for(std::vector<VHDLInterface::Variable*>::iterator DI = dataO.begin(); DI != dataO.end(); ++DI)
      {
        if( val != NULL and (*DI)->getLLVMValue() == val )
        {
          data_outs.push_back(*DI);
          found = true;
        }
      }
      assert(found or val == NULL);
      if( !found )
        data_outs.push_back(liv->createSignal<VHDLInterface::Signal>(ss.str(), getSizeInBits(*II)));
    }
    isb->initializeOutputInterfacePorts(*II,
                         this->getOutputReadEnableIn(out_index),
                         liv->createSignal<VHDLInterface::Signal>(getValueName(*II)+"_empty_s", 1),
                         data_outs
                                     );
    std::map<llvm::Value*,int> step_dimensions;
    step_dimensions = window_dimensions;
    if( window_dimensions.find(livHandler.getInnermostLIV()) != window_dimensions.end() and
        window_dimensions[livHandler.getInnermostLIV()] >= loopInfo.inputBufferNumInvalidated[*II] )
    {
      step_dimensions[livHandler.getInnermostLIV()] = loopInfo.inputBufferNumInvalidated[*II];
    }
    BufferSpaceAccesser window_accesser = getWindowBufferSpaceAccessor(window_dimensions, std::vector<llvm::Value*>(indexes.rbegin(),indexes.rend()), writtenIndexesUsed, addressVariables[*II].address_out.size());
    BufferSpaceAccesser step_accesser = getStepBufferSpaceAccessor(window_dimensions, std::vector<llvm::Value*>(indexes.rbegin(),indexes.rend()), writtenIndexesUsed, addressVariables[*II].address_out.size());
    //move the step space over a number of elements equal to the size of the buffer space along the innermost written LIV
    step_accesser.getBufferSpace()->getTopLeft()->setAbsolute(writtenIndexesUsed.back(), 
                                                  step_accesser.getBufferSpace()->getTopLeft()->getAbsolute(writtenIndexesUsed.back()) + window_accesser.getBufferSpace()->getBottomRight()->getAbsolute(writtenIndexesUsed.back()) ); 
    isb->createVHDL(liv, &livHandler, window_accesser, step_accesser, writtenIndexesUsed);
    ag->createVHDL(liv, &address_livHandler, window_accesser, step_accesser, addressVariables[*II].clk);
  }
  {//assign to the empty port
    VHDLInterface::AssignmentStatement* ass = liv->createSynchronousStatement(this->getOutputEmptyOut(out_index));
    for(std::vector<FifoInterfaceBlock<llvm::Value*,llvm::Value*>*>::iterator II = inputBuffers.begin(); II != inputBuffers.end(); ++II)
    {
      ass->addCase(VHDLInterface::ConstantInt::get(1), VHDLInterface::Wrap((*II)->getOutputEmptyOut(*(*II)->getOutputIndexes().begin())) == VHDLInterface::ConstantInt::get(1));
    }
    ass->addCase(VHDLInterface::ConstantInt::get(0));
  }
}
