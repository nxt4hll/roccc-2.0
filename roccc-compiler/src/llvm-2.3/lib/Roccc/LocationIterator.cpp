#include "rocccLibrary/Window/LocationIterator.h"

namespace Window {

class LocationIteratorImpl {
public:
  CountingPointer<Window> window;
  CountingPointer<Location> iter;
  //access order - end element is incremented first, with first element being incremented last
  LocationIterator::ACCESS_ORDER_TYPE access_order;
public:
  LocationIteratorImpl(CountingPointer<Window> wind, CountingPointer<Location> it, LocationIterator::ACCESS_ORDER_TYPE ao) : window(wind), iter(it), access_order(ao)
  {
  }
  virtual void operator ++()
  {
    if( isDone() )
      return;
    for(LocationIterator::ACCESS_ORDER_TYPE::reverse_iterator AOI = access_order.rbegin(); AOI != access_order.rend(); ++AOI)
    {
      Location::DIMENSION_MEASUREMENT_TYPE val = iter->getAbsolute(*AOI);
      ++val;
      LocationIterator::ACCESS_ORDER_TYPE::reverse_iterator next = AOI;
      ++next;
       //if we still havent reached the bottom right of the window, then increment the iterator and return
      // OR
      //if we are at the end dimension and cant go any higher, increment no matter what
      if( val <= window->getBottomRight()->getAbsolute(*AOI) or
          next == access_order.rend() )
       {
        iter->setAbsolute(*AOI, val);
        return;
      }
      else //otherwise, reset that dimension and walk up to the next dimension
      {
        iter->setAbsolute(*AOI, window->getTopLeft()->getAbsolute(*AOI));
      }
    }
  }
  virtual bool isDone()
  {
    //only the topmost element actually matters. only check that.
    LocationIterator::ACCESS_ORDER_TYPE::iterator AOI = access_order.begin();
    return (iter->getAbsolute(*AOI) > window->getBottomRight()->getAbsolute(*AOI));
  }
  virtual LocationIteratorImpl* clone()
  {
    return new LocationIteratorImpl(window, getCopyOfLocation(iter), access_order);
  }
};

LocationIterator::LocationIterator(CountingPointer<Window> wind, CountingPointer<Location> it, LocationIterator::ACCESS_ORDER_TYPE ao) : impl(new LocationIteratorImpl(wind, it, ao))
{
}
LocationIterator::LocationIterator(CountingPointer<Window> wind, LocationIterator::ACCESS_ORDER_TYPE ao) : impl(new LocationIteratorImpl(wind, getCopyOfLocation(wind->getTopLeft()), ao))
{
}
CountingPointer<Location> LocationIterator::getIteratedLocation()
{
  return impl->iter;
}
void LocationIterator::setIteratedLocation(CountingPointer<Location> loc)
{
  impl->iter = loc;
}
LocationIterator::ACCESS_ORDER_TYPE LocationIterator::getAccessOrder()
{
  return impl->access_order;
}
void LocationIterator::setAccessOrder(LocationIterator::ACCESS_ORDER_TYPE ao)
{
  impl->access_order = ao;
}
void LocationIterator::operator ++()
{
  ++(*impl);
}
bool LocationIterator::isDone()
{
  return impl->isDone();
}
LocationIterator LocationIterator::getCopy()
{
  LocationIterator ret = *this;
  ret.impl = impl->clone();
  return ret;
}
void LocationIterator::reset()
{
  setLocationDimensionsToLocation(impl->iter, impl->window->getTopLeft());
}

class SteppedLocationIteratorImpl : public LocationIteratorImpl {
public:
  Location::MAP_TYPE step_amount;
public:
  SteppedLocationIteratorImpl(CountingPointer<Window> wind, CountingPointer<Location> it, LocationIterator::ACCESS_ORDER_TYPE ao) : LocationIteratorImpl(wind, it, ao)
  {
  }
  virtual bool isDone()
  {
    for(Location::MAP_TYPE::iterator SAI = step_amount.begin(); SAI != step_amount.end(); ++SAI)
    {
      if( SAI->second == 0 ) //if we dont move along a dimensions that we have space along, then we are done already!
        return true;
    }
    return LocationIteratorImpl::isDone();
  }
  virtual void operator ++()
  {
    if( isDone() )
      return;
    for(LocationIterator::ACCESS_ORDER_TYPE::reverse_iterator AOI = access_order.rbegin(); AOI != access_order.rend(); ++AOI)
    {
      Location::DIMENSION_MEASUREMENT_TYPE val = iter->getAbsolute(*AOI);
      assert( step_amount.find(*AOI) != step_amount.end() );
      val += step_amount[*AOI];
      LocationIterator::ACCESS_ORDER_TYPE::reverse_iterator next = AOI;
       ++next;
      //if we still havent reached the bottom right of the window, then increment the iterator and return
      // OR
      //if we are at the end dimension and cant go any higher, increment no matter what
      if( val <= window->getBottomRight()->getAbsolute(*AOI) or
           next == access_order.rend() )
      {
        iter->setAbsolute(*AOI, val);
        return;
      }
      else //otherwise, reset that dimension and walk up to the next dimension
      {
        iter->setAbsolute(*AOI, window->getTopLeft()->getAbsolute(*AOI));
      }
    }
  }
  virtual LocationIteratorImpl* clone()
  {
    SteppedLocationIteratorImpl* ret = new SteppedLocationIteratorImpl(window, getCopyOfLocation(iter), access_order);
    ret->step_amount = step_amount;
    return ret;
  }
};

SteppedLocationIterator::SteppedLocationIterator(CountingPointer<Window> wind, CountingPointer<Location> it, ACCESS_ORDER_TYPE ao) : LocationIterator(wind, it, ao)
{
  delete this->impl;
  this->impl = new SteppedLocationIteratorImpl(wind, it, ao);
}
SteppedLocationIterator::SteppedLocationIterator(CountingPointer<Window> wind, ACCESS_ORDER_TYPE ao) : LocationIterator(wind, ao)
{
  delete this->impl;
  this->impl = new SteppedLocationIteratorImpl(wind, getCopyOfLocation(wind->getTopLeft()), ao);
}
void SteppedLocationIterator::setStepAmount(Location::MAP_TYPE mt)
{
  SteppedLocationIteratorImpl* s_impl = dynamic_cast<SteppedLocationIteratorImpl*>(impl);
  assert(s_impl and "Implementation changed!");
  s_impl->step_amount = mt;
}
Location::MAP_TYPE SteppedLocationIterator::getStepAmount()
{
  SteppedLocationIteratorImpl* s_impl = dynamic_cast<SteppedLocationIteratorImpl*>(impl);
  assert(s_impl and "Implementation changed!");
  return s_impl->step_amount;
}

}
