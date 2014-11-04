#ifndef _WINDOW_H__
#define _WINDOW_H__

#include "rocccLibrary/Window/Location.h"
#include "rocccLibrary/CountingPointer.hpp"
#include <vector>

namespace Window {

class Window {
  //these names are a bit of a misnomer, as the "top left" of a 4 dimensional
  //  rectangular solid is a 2 dimensional plane; but they help conceptualizing
  CountingPointer<Location> top_left, bottom_right;
public:
  //the top left should be less than or equal to the bottom right in every dimension
  bool isValid();
  Window(CountingPointer<Location> tl, CountingPointer<Location> br);
  CountingPointer<Location> getTopLeft();
  CountingPointer<Location> getBottomRight();
  std::vector<Location::DIMENSION_INDEX_TYPE> getDimensionIndexes();
};

//two windows intersect if they share any points
bool windowsIntersect(CountingPointer<Window> lhs, CountingPointer<Window> rhs);
//a window 'inner' is inside of another window 'outer' if there is no point in 'inner' that isnt in 'outer'
bool windowIsInsideWindow(CountingPointer<Window> inner, CountingPointer<Window> outer);
//get the area of a window
Location::MAP_TYPE getAreaOfWindow(CountingPointer<Window> wind);

}

#endif
