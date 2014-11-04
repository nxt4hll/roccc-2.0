#include "rocccLibrary/VHDLComponents/AddressGenerator.h"
#include "rocccLibrary/MultiForVar.hpp"
#include <cassert>
#include <sstream>
#include "rocccLibrary/GetValueName.h"
#include "rocccLibrary/InternalWarning.h"

std::string getIntAsString(int a)
{
  std::stringstream ss;
  ss << a;
  return ss.str();
}

//helper function to get a vector's worth of values from a map
template<class T, class V>
std::vector<V> getVectorOfMappedValues(std::map<T,V> mmap, std::vector<T> in)
{
  std::vector<V> ret;
  for(typename std::vector<T>::iterator VI = in.begin(); VI != in.end(); ++VI)
  {
    typename std::map<T,V>::iterator MI = mmap.find(*VI);
    if( MI == mmap.end() )
    {
      INTERNAL_ERROR((*VI)->getName() << " does not exist in map!\n");
    }
    assert(MI != mmap.end() and "Value in input vector does not exist in map!");
    ret.push_back(MI->second);
  }
  return ret;
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

class AddressGenerator::AddressCalculationDatapath {
  //maps a liv to a partial multiplications calculated, one per dimension
  std::map<VHDLInterface::Variable*,VHDLInterface::Variable*> partial_multiply;
  //maps a liv to a map from channel address outs to indexes
  std::map<VHDLInterface::Variable*,std::map<std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*>,VHDLInterface::Variable*> > indexes;
  //maps a liv to a map from channel address outs to offsets
  std::map<VHDLInterface::Variable*,std::map<std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*>,VHDLInterface::Variable*> > offsets;
  //incoming count for each channel
  std::map<std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*>,VHDLInterface::Variable*> counts;
  //incoming ready signal
  VHDLInterface::Variable* input_valid;
  //outgoing valid signal
  VHDLInterface::Variable* output_valid;
  //gets a vector of the livs used
  std::vector<VHDLInterface::Variable*> getLIVsUsed()
  {
    std::vector<VHDLInterface::Variable*> ret;
    //go through the partial multiplies, and add all the LIVs used
    for(std::map<VHDLInterface::Variable*,VHDLInterface::Variable*>::iterator PMI = partial_multiply.begin(); PMI != partial_multiply.end(); ++PMI)
    {
      ret.push_back(PMI->first);
    }
    return ret;
  }
  //gets a vector of the pairs of base/count address out ports used
  std::vector<std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*> > getAddressChannelsUsed()
  {
    std::vector<std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*> > ret;
    //go through the partial multiplies, and add all the LIVs used
    for(std::map<std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*>,VHDLInterface::Variable*>::iterator CI = counts.begin(); CI != counts.end(); ++CI)
    {
      ret.push_back(CI->first);
    }
    return ret;
  }
public:
  AddressCalculationDatapath() : input_valid(NULL), output_valid(NULL) {}
  void setPartialMultiply(VHDLInterface::Variable* liv, VHDLInterface::Variable* mult)
  {
    partial_multiply[liv] = mult;
  }
  VHDLInterface::Variable* getPartialMultiply(VHDLInterface::Variable* liv)
  {
    return partial_multiply[liv];
  }
  void setIndex(VHDLInterface::Variable* liv, std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*> address_out, VHDLInterface::Variable* index)
  {
    indexes[liv][address_out] = index;
  }
  VHDLInterface::Variable* getIndex(VHDLInterface::Variable* liv, std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*> address_out)
  {
    return indexes[liv][address_out];
  }
  void setOffset(VHDLInterface::Variable* liv, std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*> address_out, VHDLInterface::Variable* offset)
  {
    offsets[liv][address_out] = offset;
  }
  VHDLInterface::Variable* getOffset(VHDLInterface::Variable* liv, std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*> address_out)
  {
    return offsets[liv][address_out];
  }
  void setCount(std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*> address_out, VHDLInterface::Variable* count)
  {
    counts[address_out] = count;
  }
  VHDLInterface::Variable* getCount(std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*> address_out)
  {
    return counts[address_out];
  }
  void setInputValid(VHDLInterface::Variable* valid)
  {
    input_valid = valid;
  }
  VHDLInterface::Variable* getInputValid()
  {
    return input_valid;
  }
  void setOutputValid(VHDLInterface::Variable* valid)
  {
    output_valid = valid;
  }
  VHDLInterface::Variable* getOutputValid()
  {
    return output_valid;
  }
  VHDLInterface::Statement* createAddressGenerationDatapath(VHDLInterface::Process* p)
  {
    VHDLInterface::MultiStatement* ret = new VHDLInterface::MultiStatement(p);
    std::vector<std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*> > address_channels = getAddressChannelsUsed();
    std::vector<VHDLInterface::Variable*> livs = getLIVsUsed();
    //dp1: find the temp sums, and copy over the count, partials, and valid
    std::map<VHDLInterface::Variable*,std::map<std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*>,VHDLInterface::Variable*> > temp_sums;
    std::map<std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*>,VHDLInterface::Variable*> dp1_counts;
    std::map<VHDLInterface::Variable*,VHDLInterface::Variable*> dp1_partial_multiply;
    VHDLInterface::Variable* dp1_valid = new VHDLInterface::ProcessVariable("dp1_valid", 1);
    VHDLInterface::MultiStatement* ms_dp1 = new VHDLInterface::MultiStatement(p);
    for(std::vector<std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*> >::iterator ACI = address_channels.begin(); ACI != address_channels.end(); ++ACI)
    {
      for(std::vector<VHDLInterface::Variable*>::iterator LUI = livs.begin(); LUI != livs.end(); ++LUI)
      {
        VHDLInterface::Wrap sum = VHDLInterface::Wrap(indexes[*LUI][*ACI]) + VHDLInterface::Wrap(offsets[*LUI][*ACI]);
        temp_sums[*LUI][*ACI] = new VHDLInterface::ProcessVariable(ACI->first->getName()+"_"+(*LUI)->getName()+"_temp_sum", (*LUI)->getSize());
        ms_dp1->addStatement(new VHDLInterface::AssignmentStatement(temp_sums[*LUI][*ACI], sum, p));
      }
      dp1_counts[*ACI] = new VHDLInterface::ProcessVariable(counts[*ACI]->getName()+"_temp1", counts[*ACI]->getSize());
      ms_dp1->addStatement(new VHDLInterface::AssignmentStatement(dp1_counts[*ACI], counts[*ACI], p));
    }
    for(std::vector<VHDLInterface::Variable*>::iterator LUI = livs.begin(); LUI != livs.end(); ++LUI)
    {
      dp1_partial_multiply[*LUI] = new VHDLInterface::ProcessVariable(partial_multiply[*LUI]->getName()+"_temp1", partial_multiply[*LUI]->getSize());
      ms_dp1->addStatement(new VHDLInterface::AssignmentStatement(dp1_partial_multiply[*LUI], partial_multiply[*LUI], p));
    }
    //find the multiplies, and copy over the count and valid
    std::map<VHDLInterface::Variable*,std::map<std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*>,VHDLInterface::Variable*> > temp_muls;
    std::map<std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*>,VHDLInterface::Variable*> dp2_counts;
    VHDLInterface::Variable* dp2_valid = new VHDLInterface::ProcessVariable("dp2_valid", 1);
    VHDLInterface::MultiStatement* ms_dp2 = new VHDLInterface::MultiStatement(p);
    for(std::vector<std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*> >::iterator ACI = address_channels.begin(); ACI != address_channels.end(); ++ACI)
    {
      for(std::vector<VHDLInterface::Variable*>::iterator LUI = livs.begin(); LUI != livs.end(); ++LUI)
      {
        VHDLInterface::Wrap mul = VHDLInterface::Wrap(temp_sums[*LUI][*ACI]) * VHDLInterface::Wrap(dp1_partial_multiply[*LUI]);
        temp_muls[*LUI][*ACI] = new VHDLInterface::ProcessVariable(ACI->first->getName()+"_"+(*LUI)->getName()+"_temp_mul", (*LUI)->getSize());
        ms_dp2->addStatement(new VHDLInterface::AssignmentStatement(temp_muls[*LUI][*ACI], mul, p));
      }
      dp2_counts[*ACI] = new VHDLInterface::ProcessVariable(counts[*ACI]->getName()+"_temp2", counts[*ACI]->getSize());
      ms_dp2->addStatement(new VHDLInterface::AssignmentStatement(dp2_counts[*ACI], dp1_counts[*ACI], p));
    }
    //sum the multiplies and set the base_out, set the count_out, and set output valid
    VHDLInterface::MultiStatement* ms_dp3 = new VHDLInterface::MultiStatement(p);
    for(std::vector<std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*> >::iterator ACI = address_channels.begin(); ACI != address_channels.end(); ++ACI)
    {
      VHDLInterface::Wrap sum = VHDLInterface::ConstantInt::get(0);
      for(std::vector<VHDLInterface::Variable*>::iterator LUI = livs.begin(); LUI != livs.end(); ++LUI)
      {
        sum = sum + VHDLInterface::Wrap(temp_muls[*LUI][*ACI]);
      }
      ms_dp3->addStatement(new VHDLInterface::AssignmentStatement(ACI->first, sum, p));
      ms_dp3->addStatement(new VHDLInterface::AssignmentStatement(ACI->second, dp2_counts[*ACI], p));
    }
    ret->addStatement(new VHDLInterface::IfStatement(p, VHDLInterface::Wrap(dp2_valid) == VHDLInterface::ConstantInt::get(1), ms_dp3));
    ret->addStatement(new VHDLInterface::IfStatement(p, VHDLInterface::Wrap(dp1_valid) == VHDLInterface::ConstantInt::get(1), ms_dp2));
    ret->addStatement(new VHDLInterface::IfStatement(p, VHDLInterface::Wrap(input_valid) == VHDLInterface::ConstantInt::get(1), ms_dp1));
    ret->addStatement(new VHDLInterface::AssignmentStatement(output_valid, dp2_valid, p));
    ret->addStatement(new VHDLInterface::AssignmentStatement(dp2_valid, dp1_valid, p));
    ret->addStatement(new VHDLInterface::AssignmentStatement(dp1_valid, input_valid, p));
    return ret;
  }
};

AddressGenerator::AddressGenerator(llvm::Value* v) : address_rdy_out(NULL), address_stall_in(NULL), addressCalcImpl(new AddressGenerator::AddressCalculationDatapath()), stream_value(v), state(NULL), p(NULL)
{
  assert(stream_value);
}
void AddressGenerator::initializeVHDLInterface(VHDLInterface::Variable* rdy, VHDLInterface::Variable* stall, std::vector<std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*> > base_count_pair)
{
  address_out = base_count_pair;
  address_rdy_out = rdy;
  address_stall_in = stall;
}

VHDLInterface::State* getStateNamed(VHDLInterface::StateVar* state, std::string name)
{
  for(std::vector<VHDLInterface::State*>::iterator SI = state->getStates().begin(); SI != state->getStates().end(); ++SI)
  {
    if( (*SI)->getName() == name )
      return *SI;
  }
  return state->addState(name);
}

std::string getLocationAsString(CountingPointer<Window::Location> loc, Window::LocationIterator::ACCESS_ORDER_TYPE ao)
{
  std::stringstream ss;
  for(Window::LocationIterator::ACCESS_ORDER_TYPE::iterator AOI = ao.begin(); AOI != ao.end(); ++AOI)
  {
    if( AOI != ao.begin() )
      ss << "_";
    ss << loc->getAbsolute(*AOI);
  }
  return ss.str();
}

VHDLInterface::State* getLastState(VHDLInterface::StateVar* sv)
{
  if( sv->getStates().size() == 0 )
    return NULL;
  return sv->getStates().back();
}

VHDLInterface::CaseStatement* AddressGenerator::createInitializationVHDL(VHDLInterface::Entity* parent, LoopInductionVariableHandler* livHandler, BufferSpaceAccesser window_accesser, BufferSpaceAccesser step_accesser, VHDLInterface::Variable* clk)
{
  p = parent->createProcess<VHDLInterface::MultiStatementProcess>(clk);
  state = parent->createSignal<VHDLInterface::StateVar>(getValueName(stream_value)+"_address_state",0);
  VHDLInterface::CaseStatement* cs = new VHDLInterface::CaseStatement(p, state);
  Window::LocationIterator::ACCESS_ORDER_TYPE access_order = window_accesser.getTimeIterator().getAccessOrder();
  Window::LocationIterator::ACCESS_ORDER_TYPE r_access_order(access_order.rbegin(), access_order.rend());
  //go through the base/count pairs and set the datapath calculation variables
  for(std::vector<std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*> >::iterator ADI = address_out.begin(); ADI != address_out.end(); ++ADI)
  {
    addressCalcImpl->setCount(*ADI, new VHDLInterface::ProcessVariable(ADI->second->getName()+"_partial", ADI->second->getSize()));
  }
  //add the valid signals
  addressCalcImpl->setInputValid(new VHDLInterface::ProcessVariable(getValueName(stream_value)+"_address_calc_valid", 1));
  addressCalcImpl->setOutputValid(address_rdy_out);
  //go through the livs and set the partials
  VHDLInterface::Value* mult = VHDLInterface::ConstantInt::get(1);
  for(Window::LocationIterator::ACCESS_ORDER_TYPE::reverse_iterator AOI = access_order.rbegin(); AOI != access_order.rend(); ++AOI)
  {
    VHDLInterface::State* prev_state = getLastState(state);
    VHDLInterface::Variable* cur_liv = livHandler->getLIVVHDLValue(*AOI);
    VHDLInterface::State* cur_state = state->addState(std::string("S_")+(*AOI)->getName()+"_INITIALIZE");
    //every channel has different index and offset variables
    for(std::vector<std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*> >::iterator ADI = address_out.begin(); ADI != address_out.end(); ++ADI)
    {
      VHDLInterface::Variable* index = new VHDLInterface::ProcessVariable(ADI->first->getName()+"_"+(*AOI)->getName()+"_index", cur_liv->getSize());
      VHDLInterface::Variable* offset = new VHDLInterface::ProcessVariable(ADI->first->getName()+"_"+(*AOI)->getName()+"_offset", cur_liv->getSize());
      addressCalcImpl->setIndex(cur_liv, *ADI, index);
      addressCalcImpl->setOffset(cur_liv, *ADI, offset);
    }
    //every channel shares the partial multiplies
    VHDLInterface::Variable* partial_multiply = new VHDLInterface::ProcessVariable((*AOI)->getName()+"_partial", cur_liv->getSize());
    addressCalcImpl->setPartialMultiply(cur_liv, partial_multiply);
    //assign the calculated multiply to the partial multiply value
    cs->addStatement(cur_state, new VHDLInterface::AssignmentStatement(partial_multiply, mult, p));
    //increase the multiply by the current length of the accessed row size
    //   - get the max of either the window or step accesser's bottom right corner
    int length = window_accesser.getBufferSpace()->getBottomRight()->getAbsolute(*AOI);
    if( length < step_accesser.getBufferSpace()->getBottomRight()->getAbsolute(*AOI) )
      length = step_accesser.getBufferSpace()->getBottomRight()->getAbsolute(*AOI);
    //minus 1 because we only want extra elements past the end
    if( livHandler->isEndValueInfinity(*AOI) )
      mult = VHDLInterface::ConstantInt::get(-1);
    else
      mult = VHDLInterface::Wrap(partial_multiply) * (VHDLInterface::Wrap(livHandler->getEndValueVHDLValue(*AOI)) + VHDLInterface::ConstantInt::get(length-1));
    //if we have a previous state, transition from it to here
    if( prev_state )
      cs->addStatement(prev_state, new VHDLInterface::AssignmentStatement(state, cur_state, p));
  }
  return cs;
}

InputAddressGenerator::InputAddressGenerator(llvm::Value* v) : AddressGenerator(v)
{
}

void InputAddressGenerator::createVHDL(VHDLInterface::Entity* parent, LoopInductionVariableHandler* livHandler, BufferSpaceAccesser window_accesser, BufferSpaceAccesser step_accesser, VHDLInterface::Variable* clk)
{
  //create initialization code
  VHDLInterface::CaseStatement* cs = createInitializationVHDL(parent, livHandler, window_accesser, step_accesser, clk);
  assert(state);
  assert(p);
  assert(cs);
  VHDLInterface::State* last_init_state = getLastState(state);
  assert(last_init_state);
  //assign the steady state, and the stall check
  p->addStatement(new VHDLInterface::AssignmentStatement(address_rdy_out, VHDLInterface::ConstantInt::get(0), p));
  VHDLInterface::MultiStatement* ms = new VHDLInterface::MultiStatement(p);
  ms->addStatement(addressCalcImpl->createAddressGenerationDatapath(p));
  ms->addStatement(new VHDLInterface::AssignmentStatement(addressCalcImpl->getInputValid(), VHDLInterface::ConstantInt::get(0), p));
  ms->addStatement(cs);
  p->addStatement(new VHDLInterface::IfStatement(p, VHDLInterface::Wrap(address_stall_in) == VHDLInterface::ConstantInt::get(0), ms));
  //create the first states for the buffer and step
  VHDLInterface::State* first_window_state = createBufferVHDL(livHandler, window_accesser, "S_WINDOW", cs);
  VHDLInterface::State* last_window_state = getLastState(state);
  VHDLInterface::State* first_step_state = createBufferVHDL(livHandler, step_accesser, "S_STEP", cs);
  VHDLInterface::State* last_step_state = getLastState(state);
  if( first_window_state )
  {
    //transition from the last window state to the first step state
    cs->addStatement(last_window_state, new VHDLInterface::AssignmentStatement(state, first_step_state, p));
  }
  else //there are no buffer states
  {
    first_window_state = first_step_state;
    last_window_state = last_step_state;
  }
  //whatever first state we created, transition from the last init state to that first state
  cs->addStatement(last_init_state, new VHDLInterface::AssignmentStatement(state, first_window_state, p));
  //the increment state is in the last step state
  assert(last_step_state);
  cs->addStatement(last_step_state, 
                new VHDLInterface::IfStatement(p, livHandler->shouldIncrement(livHandler->getInnermostLIV()),
                                        new VHDLInterface::AssignmentStatement(state, first_step_state, p),
                                        new VHDLInterface::AssignmentStatement(state, first_window_state, p)
                                        )
                   );
  cs->addStatement(last_step_state, livHandler->getIncrementStatement(p));
  //Create the done state
  VHDLInterface::State* s_done = state->addState("S_DONE");
  cs->addStatement(s_done, new VHDLInterface::AssignmentStatement(state, s_done, p));
  //transition from the last state to the done state, assuming no shenangans
  cs->addStatement(last_step_state, new VHDLInterface::IfStatement(p, livHandler->getDoneCondition(), new VHDLInterface::AssignmentStatement(state, s_done, p))); 
}

OutputAddressGenerator::OutputAddressGenerator(llvm::Value* v) : AddressGenerator(v)
{
}

// This code is responsible for creating the process that outputs the
//  addresses that correspond to an output smart buffer. 
void 
OutputAddressGenerator::createVHDL(VHDLInterface::Entity* parent, 
				   LoopInductionVariableHandler* livHandler, 
				   BufferSpaceAccesser window_accesser, 
				   BufferSpaceAccesser step_accesser, 
				   VHDLInterface::Variable* clk)
{
  //create initialization code
  VHDLInterface::CaseStatement* cs = 
    createInitializationVHDL(parent, livHandler, 
			     window_accesser, step_accesser, clk);
  assert(state); // address state
  assert(p); // the process
  assert(cs); // the case statement

  VHDLInterface::State* last_init_state = getLastState(state);
  assert(last_init_state);

  //assign the steady state, and the stall check
  p->addStatement(new VHDLInterface::AssignmentStatement(address_rdy_out, VHDLInterface::ConstantInt::get(0), p));

  VHDLInterface::MultiStatement* ms = new VHDLInterface::MultiStatement(p);
  ms->addStatement(addressCalcImpl->createAddressGenerationDatapath(p));
  ms->addStatement(new VHDLInterface::AssignmentStatement(addressCalcImpl->getInputValid(), VHDLInterface::ConstantInt::get(0), p));
  ms->addStatement(cs);
  p->addStatement(new VHDLInterface::IfStatement(p, VHDLInterface::Wrap(address_stall_in) == VHDLInterface::ConstantInt::get(0), ms));

  //create the first states for the step and buffer
  VHDLInterface::State* first_step_state = 
    createBufferVHDL(livHandler, step_accesser, "S_STEP", cs);
  VHDLInterface::State* last_step_state = getLastState(state);
  VHDLInterface::State* first_window_state = 
    createBufferVHDL(livHandler, window_accesser, "S_WINDOW", cs);
  VHDLInterface::State* last_window_state = getLastState(state);
  if( first_window_state )
  {
    //transition from the last step state to the first window state 
    if (livHandler->isMultidimensional()) 
    {
      cs->addStatement(last_step_state, 
		       new VHDLInterface::AssignmentStatement(state, 
							      first_window_state,
							      p));
    }
    else
    {
      cs->addStatement(last_step_state, 
		       new VHDLInterface::IfStatement(p, livHandler->shouldIncrement(livHandler->getInnermostLIV()),
						      new VHDLInterface::AssignmentStatement(state, first_step_state, p),
						      new VHDLInterface::AssignmentStatement(state, first_window_state, p)
						      )
		       );
      cs->addStatement(last_step_state, livHandler->getIncrementStatement(p)) ;
    }
  }
  else //there are no buffer states
  {
    first_window_state = first_step_state;
    last_window_state = last_step_state;
  }
  //whatever first state we created, transition from the last init state to that first state
  cs->addStatement(last_init_state, new VHDLInterface::AssignmentStatement(state, first_step_state, p));
  //the increment state is in the last window state
  assert(last_window_state);
  cs->addStatement(last_window_state, 
                new VHDLInterface::IfStatement(p, livHandler->shouldIncrement(livHandler->getInnermostLIV()),
                                        new VHDLInterface::AssignmentStatement(state, first_step_state, p),
                                        new VHDLInterface::AssignmentStatement(state, first_window_state, p)
                                        )
                   );
  cs->addStatement(last_window_state, livHandler->getIncrementStatement(p));
  //Create the done state
  VHDLInterface::State* s_done = state->addState("S_DONE");
  cs->addStatement(s_done, new VHDLInterface::AssignmentStatement(state, s_done, p));
  //transition from the last state to the done state, assuming no shenangans
  cs->addStatement(last_window_state, new VHDLInterface::IfStatement(p, livHandler->getDoneCondition(), new VHDLInterface::AssignmentStatement(state, s_done, p)));
}

VHDLInterface::State* AddressGenerator::createBufferVHDL(LoopInductionVariableHandler* livHandler, BufferSpaceAccesser accesser, std::string prefix, VHDLInterface::CaseStatement* cs)
{
  Window::LocationIterator::ACCESS_ORDER_TYPE access_order = accesser.getTimeIterator().getAccessOrder();
  Window::LocationIterator::ACCESS_ORDER_TYPE r_access_order(access_order.rbegin(), access_order.rend());
  VHDLInterface::State* last_state = NULL;
  VHDLInterface::State* first_state = NULL;
  //generate an address state for each window state
  for(Window::LocationIterator TI = accesser.getTimeIterator(); !TI.isDone(); ++TI)
  {
    CountingPointer<Window::Location> cur_loc = accesser.getChannelSpace()->getTopLeft();
    std::vector<int> loc_vec = getVectorOfMappedValues(Window::getAbsoluteDimensionMapFromLocation(cur_loc), access_order);
    //create the state we are in
    VHDLInterface::State* cur_state = getStateNamed(state, prefix+getVectorAsString(loc_vec));
    //write valid signal
    cs->addStatement(cur_state, new VHDLInterface::AssignmentStatement(addressCalcImpl->getInputValid(), VHDLInterface::ConstantInt::get(1), p));
    //write addresses to each channel
    std::vector<std::pair<VHDLInterface::Variable*,VHDLInterface::Variable*> >::iterator API = address_out.begin();
    assert(!accesser.getChannelIterator().isDone());
    for(Window::LocationIterator CI = accesser.getChannelIterator(); !CI.isDone(); ++CI, ++API)
    {
      cur_loc = accesser.getChannelSpace()->getTopLeft();
      loc_vec = getVectorOfMappedValues(Window::getAbsoluteDimensionMapFromLocation(cur_loc), access_order);
      //write the partial calculations
      for(Window::LocationIterator::ACCESS_ORDER_TYPE::iterator AOI = access_order.begin(); AOI != access_order.end(); ++AOI)
      {
        VHDLInterface::Variable* liv = livHandler->getLIVVHDLValue(*AOI);
        cs->addStatement(cur_state, new VHDLInterface::AssignmentStatement(addressCalcImpl->getIndex(liv, *API), liv, p));
        cs->addStatement(cur_state, new VHDLInterface::AssignmentStatement(addressCalcImpl->getOffset(liv, *API), VHDLInterface::ConstantInt::get(Window::getAbsoluteDimensionMapFromLocation(cur_loc)[*AOI]), p));
      }
      //write the count variable
      int channel_length = accesser.getChannelSpace()->getBottomRight()->getRelative(access_order.back());
      assert(API != address_out.end() and "Cannot have more address channels than window rows.");
      cs->addStatement(cur_state, new VHDLInterface::AssignmentStatement(addressCalcImpl->getCount(*API), VHDLInterface::ConstantInt::get(channel_length), p));
    }
    accesser.getChannelIterator().reset();
    assert( API == address_out.end() and "Not all address channels used in step row generation!" );
    //transition from the last state to here
    if( last_state )
      cs->addStatement(last_state, new VHDLInterface::AssignmentStatement(state, cur_state, p));
    last_state = cur_state;
    if( !first_state )
      first_state = cur_state;
  }
  accesser.getTimeIterator().reset();
  return first_state;
}

