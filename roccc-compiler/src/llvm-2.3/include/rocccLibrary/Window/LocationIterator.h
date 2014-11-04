#ifndef _LOCATION_ITERATOR_H__
#define _LOCATION_ITERATOR_H__

#include "rocccLibrary/CountingPointer.hpp"
#include "rocccLibrary/Window/Location.h"
#include "rocccLibrary/Window/Window.h"
#include <vector>


#include "rocccLibrary/Window/RelativeLocation.h"

namespace Window {

class LocationIteratorImpl;

class LocationIterator {
public:
  typedef std::vector<Location::DIMENSION_INDEX_TYPE> ACCESS_ORDER_TYPE;
protected:
  LocationIteratorImpl* impl;
public:
  //the Location 'iter' will traverse around the Window 'window' in access_order order;
  //  iter MUST be a RelativeLocation relative to window's top left!
  LocationIterator(CountingPointer<Window> wind, CountingPointer<Location> it, ACCESS_ORDER_TYPE ao=ACCESS_ORDER_TYPE());
  LocationIterator(CountingPointer<Window> wind, ACCESS_ORDER_TYPE ao=ACCESS_ORDER_TYPE());
  CountingPointer<Location> getIteratedLocation();
  void setIteratedLocation(CountingPointer<Location> loc);
  ACCESS_ORDER_TYPE getAccessOrder();
  void setAccessOrder(ACCESS_ORDER_TYPE ao);
  void operator ++();
  bool isDone();
  LocationIterator getCopy();
  //moves the iterated location to the top left corner of the window
  void reset();
};

class SteppedLocationIterator : public LocationIterator {
public:
  SteppedLocationIterator(CountingPointer<Window> wind, CountingPointer<Location> it, ACCESS_ORDER_TYPE ao=ACCESS_ORDER_TYPE());
  SteppedLocationIterator(CountingPointer<Window> wind, ACCESS_ORDER_TYPE ao=ACCESS_ORDER_TYPE());
  void setStepAmount(Location::MAP_TYPE mt);
  Location::MAP_TYPE getStepAmount();
};

//Creates a LocationIterator that is tied to a window and moves the window inside of another window.
//  the iterator itself is tied to inner's bottom right location.
//When the inner resets along a dimension, it resets not to the edge of outer, but to
//  the edge that it started at. If the user wishes for the inner to follow outer,
//  it is up to the user to set inner's top left to outer's top left before calling.
template<class ITERATOR>
ITERATOR getWindowIterator(CountingPointer<Window> inner, CountingPointer<Window> outer, LocationIterator::ACCESS_ORDER_TYPE ao=LocationIterator::ACCESS_ORDER_TYPE())
{
  assert( windowIsInsideWindow(inner, outer) );
  CountingPointer<Location> inner_bottom_right_copy = getCopyOfLocation(inner->getBottomRight());
  CountingPointer<Window> sub_outer(new Window(inner_bottom_right_copy, outer->getBottomRight()));
  ITERATOR ret(sub_outer, inner->getBottomRight(), ao);
  return ret;
}

template<class ITERATOR>
ITERATOR getElementIterator(CountingPointer<Window> outer, LocationIterator::ACCESS_ORDER_TYPE ao=LocationIterator::ACCESS_ORDER_TYPE())
{
  CountingPointer<Location> inner_tl(new RelativeLocation(outer->getTopLeft()));
  CountingPointer<Location> inner_br(new RigidRelativeLocation(inner_tl));
  for(LocationIterator::ACCESS_ORDER_TYPE::iterator AOI = ao.begin(); AOI != ao.end(); ++AOI)
  {
    if( outer->getBottomRight()->getRelative(*AOI) > 0 )
      inner_br->setRelative(*AOI, 1); //1 long in every dimension
  }
  CountingPointer<Window> inner(new Window(inner_tl, inner_br));
  return getWindowIterator<ITERATOR>(inner, outer, ao);
}

}

#endif
