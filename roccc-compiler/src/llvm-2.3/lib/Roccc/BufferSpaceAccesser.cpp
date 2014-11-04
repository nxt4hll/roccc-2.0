#include "rocccLibrary/Window/BufferSpaceAccesser.h"
#include "rocccLibrary/Window/RelativeLocation.h"
#include "rocccLibrary/InductionVariableInfo.h"

#include "rocccLibrary/InternalWarning.h"
#include <sstream>
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

BufferSpaceAccesser::BufferSpaceAccesser(Window::Location::MAP_TYPE buffer, Window::Location::MAP_TYPE address, Window::Location::MAP_TYPE channel, Window::LocationIterator::ACCESS_ORDER_TYPE ao) : buffer_space(NULL), address_space(NULL), channel_space(NULL), access_order(ao)
{

#if 0
  INTERNAL_SERIOUS_WARNING("BufferSpace: " << getVectorAsString(getVectorOfMappedValues(buffer, access_order)) << "\n"
                           "AddressSpace: " << getVectorAsString(getVectorOfMappedValues(address, access_order)) << "\n"
                           "ChannelSpace: " << getVectorAsString(getVectorOfMappedValues(channel, access_order)) << "\n");
#endif

  CountingPointer<Window::Location> window_tl(new Window::Location());
  CountingPointer<Window::Location> window_br(new Window::RigidRelativeLocation(window_tl, buffer));
  buffer_space = CountingPointer<Window::Window>(new Window::Window(window_tl, window_br));
  
  CountingPointer<Window::Location> address_tl(new Window::RelativeLocation(window_tl));
  CountingPointer<Window::Location> address_br(new Window::RigidRelativeLocation(address_tl, address));
  address_space = CountingPointer<Window::Window>(new Window::Window(address_tl, address_br));
  
  CountingPointer<Window::Location> channel_tl(new Window::RelativeLocation(address_tl));
  CountingPointer<Window::Location> channel_br(new Window::RigidRelativeLocation(channel_tl, channel));
  channel_space = CountingPointer<Window::Window>(new Window::Window(channel_tl, channel_br));
}
Window::LocationIterator BufferSpaceAccesser::getTimeIterator()
{
  //reset the address space to the start of the buffer space
  Window::resetRelativeDimension(address_space->getTopLeft());
  Window::resetRelativeDimension(channel_space->getTopLeft());
  Window::SteppedLocationIterator ret = Window::getWindowIterator<Window::SteppedLocationIterator>(address_space, buffer_space, access_order);
  ret.setStepAmount(Window::getAreaOfWindow(address_space));
  return ret;
}
Window::LocationIterator BufferSpaceAccesser::getChannelIterator()
{
  //reset the channel space to the start of the address space
  Window::resetRelativeDimension(channel_space->getTopLeft());
  Window::SteppedLocationIterator ret = Window::getWindowIterator<Window::SteppedLocationIterator>(channel_space, address_space, access_order);
  ret.setStepAmount(Window::getAreaOfWindow(channel_space));
  return ret;
}
CountingPointer<Window::Window> BufferSpaceAccesser::getBufferSpace()
{
  return buffer_space;
}
CountingPointer<Window::Window> BufferSpaceAccesser::getAddressSpace()
{
  return address_space;
}
CountingPointer<Window::Window> BufferSpaceAccesser::getChannelSpace()
{
  return channel_space;
}

BufferSpaceAccesser getWindowBufferSpaceAccessor(Window::Location::MAP_TYPE window_dimensions, std::vector<llvm::Value*> accessed_LIV, std::vector<llvm::Value*> written_LIV, int num_address_channels)
{
  int shift_amount = getIVStepSize(written_LIV.back());
  //the buffer space is the window_dimensions, but minus the step size of the innermost LIV
  Window::Location::MAP_TYPE buffer_space = window_dimensions;
  //if we use the last written LIV at all, update the shift size, otherwise we need to maximanize it
  if( window_dimensions.find(written_LIV.back()) != window_dimensions.end() )
  {
    if( window_dimensions[written_LIV.back()] < shift_amount )
      shift_amount = window_dimensions[written_LIV.back()];
    //buffering needs to be along the innermost LIV, from the WRITTEN LIV standpoint
    buffer_space[written_LIV.back()] -= shift_amount;
  }
  else
  {
    if( window_dimensions[accessed_LIV.back()] < shift_amount )
      shift_amount = window_dimensions[accessed_LIV.back()];
    buffer_space[accessed_LIV.back()] -= shift_amount;
  }
  //the channel space is one row along the innermost accessed LIV
  Window::Location::MAP_TYPE channel_space = buffer_space;
  for(Window::Location::MAP_TYPE::iterator CSI = channel_space.begin(); CSI != channel_space.end(); ++CSI)
  {
   //everything except the innermost LIV gets set to only 1 element
    if( CSI->first != accessed_LIV.back() and CSI->second > 1 )
      CSI->second = 1;
  }
  //create windows for the buffer and channel spaces
  CountingPointer<Window::Location> buffer_tl;
  CountingPointer<Window::Location> buffer_br(new Window::RigidRelativeLocation(buffer_tl, buffer_space));
  CountingPointer<Window::Window> buffer_window(new Window::Window(buffer_tl, buffer_br));
  CountingPointer<Window::Location> channel_tl(new Window::RelativeLocation(buffer_tl));
  CountingPointer<Window::Location> channel_br(new Window::RigidRelativeLocation(channel_tl, channel_space));
  CountingPointer<Window::Window> channel_window(new Window::Window(channel_tl, channel_br));
  //create an iterator for the channel space over the buffer space
  setLocationDimensionsToLocation(channel_window->getTopLeft(), buffer_window->getTopLeft());
  Window::SteppedLocationIterator channel_iterator = Window::getWindowIterator<Window::SteppedLocationIterator>(channel_window, buffer_window, accessed_LIV);
  channel_iterator.setStepAmount(Window::getRelativeDimensionMapFromLocation(channel_window->getBottomRight()));
  //the address space is the channel space incremented num_address_channels times
  //if we are already done (ie, were empty to begin with) dont bother checking anything about the address channels
  if( !channel_iterator.isDone() )
  {
    for(int c = 1; c < num_address_channels; ++c)
      ++channel_iterator;
    //FIXME add in actual checking for too many address channels
    assert( !channel_iterator.isDone() and "Too many address channels!" );
  }
  //now the iterator's location should be bottom location for the address space
  Window::Location::MAP_TYPE address_space = getAbsoluteDimensionMapFromLocation(channel_iterator.getIteratedLocation());
  return BufferSpaceAccesser(buffer_space, address_space, channel_space, accessed_LIV);
}
BufferSpaceAccesser getStepBufferSpaceAccessor(Window::Location::MAP_TYPE window_dimensions, std::vector<llvm::Value*> accessed_LIV, std::vector<llvm::Value*> written_LIV, int num_address_channels)
{ 
  int shift_amount = getIVStepSize(written_LIV.back());
  //the buffer space is the window_dimensions, but minus the step size of the innermost LIV
  Window::Location::MAP_TYPE buffer_space = window_dimensions;
  //if we use the last written LIV at all, update the shift size, otherwise we need to maximanize it
  if( window_dimensions.find(written_LIV.back()) != window_dimensions.end() )
  {
    if( window_dimensions[written_LIV.back()] < shift_amount )
      shift_amount = window_dimensions[written_LIV.back()];
    //buffering needs to be along the innermost LIV, from the WRITTEN LIV standpoint
    buffer_space[written_LIV.back()] = shift_amount;
  }
  else
  {
    if( window_dimensions[accessed_LIV.back()] < shift_amount )
      shift_amount = window_dimensions[accessed_LIV.back()];
    buffer_space[accessed_LIV.back()] = shift_amount;
  }
  //the channel space is one row along the innermost accessed LIV
  Window::Location::MAP_TYPE channel_space = buffer_space;
  for(Window::Location::MAP_TYPE::iterator CSI = channel_space.begin(); CSI != channel_space.end(); ++CSI)
  {
   //everything except the innermost LIV gets set to only 1 element
    if( CSI->first != accessed_LIV.back() and CSI->second > 1 )
      CSI->second = 1;
  }
  //create windows for the buffer and channel spaces
  CountingPointer<Window::Location> buffer_tl;
  CountingPointer<Window::Location> buffer_br(new Window::RigidRelativeLocation(buffer_tl, buffer_space));
  CountingPointer<Window::Window> buffer_window(new Window::Window(buffer_tl, buffer_br));
  CountingPointer<Window::Location> channel_tl(new Window::RelativeLocation(buffer_tl));
  CountingPointer<Window::Location> channel_br(new Window::RigidRelativeLocation(channel_tl, channel_space));
  CountingPointer<Window::Window> channel_window(new Window::Window(channel_tl, channel_br));
  //create an iterator for the channel space over the buffer space
  setLocationDimensionsToLocation(channel_window->getTopLeft(), buffer_window->getTopLeft());
  Window::SteppedLocationIterator channel_iterator = Window::getWindowIterator<Window::SteppedLocationIterator>(channel_window, buffer_window, accessed_LIV);
  channel_iterator.setStepAmount(Window::getRelativeDimensionMapFromLocation(channel_window->getBottomRight()));
  //the address space is the channel space incremented num_address_channels times
  for(int c = 1; c < num_address_channels; ++c)
    ++channel_iterator;
  //FIXME add in actual checking for too many address channels
  assert( !channel_iterator.isDone() and "Too many address channels!" );
  //now the iterator's location should be bottom location for the address space
  Window::Location::MAP_TYPE address_space = getAbsoluteDimensionMapFromLocation(channel_iterator.getIteratedLocation());
  return BufferSpaceAccesser(buffer_space, address_space, channel_space, accessed_LIV);
}
