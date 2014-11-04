#ifndef _BUFFER_SPACE_ACCESSER_H__
#define _BUFFER_SPACE_ACCESSER_H__

#include "rocccLibrary/CountingPointer.hpp"
#include "rocccLibrary/Window/Window.h"
#include "rocccLibrary/Window/LocationIterator.h"

class BufferSpaceAccesser {
  CountingPointer<Window::Window> buffer_space;
  CountingPointer<Window::Window> address_space;
  CountingPointer<Window::Window> channel_space;
  Window::LocationIterator::ACCESS_ORDER_TYPE access_order;
public:
  //the easy constructor. takes the elements already processed.
  BufferSpaceAccesser(Window::Location::MAP_TYPE buffer, Window::Location::MAP_TYPE address, Window::Location::MAP_TYPE channel, Window::LocationIterator::ACCESS_ORDER_TYPE ao);
  //When you increment this iterator, you are signaling you
  //  are working with the next chunk in time.
  Window::LocationIterator getTimeIterator();
  //When you increment this iterator, you are signaling you
  //  are working with the elements in the current chunk in time.
  Window::LocationIterator getChannelIterator();
  //get the entire window space that is being accessed
  CountingPointer<Window::Window> getBufferSpace();
  //the address space is a slice into the buffer space; the address space
  //  contains those locations that will be genereted that clock cycle
  CountingPointer<Window::Window> getAddressSpace();
  //the channel space is a slice into the address space; the channel space
  //  contains those locations that will be accessed by that channel in that clock cycle
  CountingPointer<Window::Window> getChannelSpace();
};

BufferSpaceAccesser getWindowBufferSpaceAccessor(Window::Location::MAP_TYPE window_dimensions, std::vector<llvm::Value*> accessed_LIV, std::vector<llvm::Value*> written_LIV, int num_address_channels);
BufferSpaceAccesser getStepBufferSpaceAccessor(Window::Location::MAP_TYPE window_dimensions, std::vector<llvm::Value*> accessed_LIV, std::vector<llvm::Value*> written_LIV, int num_address_channels);

#endif

