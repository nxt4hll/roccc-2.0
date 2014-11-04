#ifndef _RELATIVE_LOCATION_H__
#define _RELATIVE_LOCATION_H__

#include "rocccLibrary/Window/Location.h"
#include "rocccLibrary/CountingPointer.hpp"
#include <vector>

namespace Window {

//This moves relative to the locus, but if it is moved, it doesnt move the locus
class RelativeLocation : public Location {
protected:
  CountingPointer<Location> locus;
public:
  explicit RelativeLocation(const CountingPointer<Location>& loc);
  RelativeLocation(const CountingPointer<Location>& loc, const Location::MAP_TYPE& dist);
  virtual Location::DIMENSION_MEASUREMENT_TYPE getAbsolute(const Location::DIMENSION_INDEX_TYPE& index);
  virtual std::vector<Location::DIMENSION_INDEX_TYPE> getDimensionIndexes();
  CountingPointer<Location> getLocus();
};

//this moves relative to the locus, and when it is moved, it moves the locus in order
//  to preserve the current relative dimensions
class RigidRelativeLocation : public RelativeLocation {
public:
  explicit RigidRelativeLocation(const CountingPointer<Location>& loc);
  RigidRelativeLocation(const CountingPointer<Location>& loc, const Location::MAP_TYPE& dist);
  virtual void setAbsolute(const DIMENSION_INDEX_TYPE& index, DIMENSION_MEASUREMENT_TYPE m);
};

}

#endif
