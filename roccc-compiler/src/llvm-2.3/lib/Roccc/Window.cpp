#include "rocccLibrary/Window/Window.h"
#include "rocccLibrary/Window/CombineVectors.hpp"
#include "rocccLibrary/Window/WindowUtils.h"

namespace Window {

bool Window::isValid()
{
  return areAllDimensions(std::less_equal<Location::DIMENSION_MEASUREMENT_TYPE>(), top_left, bottom_right);
}
Window::Window(CountingPointer<Location> tl, CountingPointer<Location> br) : top_left(tl), bottom_right(br)
{
  assert( isValid() );
}
CountingPointer<Location> Window::getTopLeft()
{
  return top_left;
}
CountingPointer<Location> Window::getBottomRight()
{
  return bottom_right;
}
std::vector<Location::DIMENSION_INDEX_TYPE> Window::getDimensionIndexes()
{
  return combineVectors(top_left->getDimensionIndexes(), bottom_right->getDimensionIndexes());
}

//T must implement:
//  bool operator <= (T,T)
template<class T>
bool linesIntersect(T a_start, T a_end, T b_start, T b_end)
{
  if( b_start <= a_start and a_start <= b_end )
    return true;
  if( a_start <= b_start and b_start <= a_end )
    return true;
  return false;
}

bool windowsIntersect(CountingPointer<Window> lhs, CountingPointer<Window> rhs)
{
  //have to check every time we do anything with these damn windows
  assert( lhs->isValid() );
  assert( rhs->isValid() );
  //do all of our dimensions intersect?
  std::vector<Location::DIMENSION_INDEX_TYPE> tlv = combineVectors(lhs->getDimensionIndexes(), rhs->getDimensionIndexes());
  for(std::vector<Location::DIMENSION_INDEX_TYPE>::iterator DI = tlv.begin(); DI != tlv.end(); ++DI)
  {
    if( !linesIntersect(lhs->getTopLeft()->getAbsolute(*DI), lhs->getBottomRight()->getAbsolute(*DI), rhs->getTopLeft()->getAbsolute(*DI), rhs->getBottomRight()->getAbsolute(*DI)) )
      return false;
  }
  return true;
}

bool windowIsInsideWindow(CountingPointer<Window> inner, CountingPointer<Window> outer)
{
  //have to check every time we do anything with these damn windows
  assert( inner->isValid() );
  assert( outer->isValid() );
  //do all of our dimensions intersect?
  std::vector<Location::DIMENSION_INDEX_TYPE> tlv = combineVectors(inner->getDimensionIndexes(), outer->getDimensionIndexes());
  for(std::vector<Location::DIMENSION_INDEX_TYPE>::iterator DI = tlv.begin(); DI != tlv.end(); ++DI)
  {
    //does the inner window's top left point fall within the outer window? (hint: it better!)
    if( !linesIntersect(inner->getTopLeft()->getAbsolute(*DI),
                        inner->getTopLeft()->getAbsolute(*DI),
                        outer->getTopLeft()->getAbsolute(*DI),
                        outer->getBottomRight()->getAbsolute(*DI)) )
      return false;
    //does the inner window's bottom right point fall within the outer window? (hint: it better!)
    if( !linesIntersect(inner->getBottomRight()->getAbsolute(*DI),
                        inner->getBottomRight()->getAbsolute(*DI),
                        outer->getTopLeft()->getAbsolute(*DI),
                        outer->getBottomRight()->getAbsolute(*DI)) )
      return false;
  }
  return true;
}

Location::MAP_TYPE getAreaOfWindow(CountingPointer<Window> wind)
{
  Location::MAP_TYPE ret;
  std::vector<Location::DIMENSION_INDEX_TYPE> tlv = combineVectors(wind->getTopLeft()->getDimensionIndexes(), wind->getTopLeft()->getDimensionIndexes());
  for(std::vector<Location::DIMENSION_INDEX_TYPE>::iterator DI = tlv.begin(); DI != tlv.end(); ++DI)
  {
    ret[*DI] = wind->getBottomRight()->getAbsolute(*DI) - wind->getTopLeft()->getAbsolute(*DI);
  }
  return ret;
}

}
