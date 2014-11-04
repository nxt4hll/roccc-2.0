#ifndef _LOCATION_H__
#define _LOCATION_H__

#include <map>
#include <vector>
#include "rocccLibrary/CountingPointer.hpp"
#include "llvm/Value.h"

namespace Window {

class Location {
public:
  typedef llvm::Value* DIMENSION_INDEX_TYPE;
  //DIMENSION_MEASUREMENT_TYPE must implement:
  //  DIMENSION_MEASUREMENT_TYPE operator + (DIMENSION_MEASUREMENT_TYPE,DIMENSION_MEASUREMENT_TYPE)
  //  DIMENSION_MEASUREMENT_TYPE operator - (DIMENSION_MEASUREMENT_TYPE,DIMENSION_MEASUREMENT_TYPE)
  //                        bool operator < (DIMENSION_MEASUREMENT_TYPE,DIMENSION_MEASUREMENT_TYPE)
  //                        bool operator == (DIMENSION_MEASUREMENT_TYPE,DIMENSION_MEASUREMENT_TYPE)
  //  DIMENSION_MEASUREMENT_TYPE operator ++(DIMENSION_MEASUREMENT_TYPE)
  typedef int DIMENSION_MEASUREMENT_TYPE;
  typedef std::map<DIMENSION_INDEX_TYPE,DIMENSION_MEASUREMENT_TYPE> MAP_TYPE;
private:
  CountingPointer<MAP_TYPE> loc;
public:
  Location();
  explicit Location(const CountingPointer<MAP_TYPE>& _loc);
  explicit Location(const Location& other);
  //getter/setters to access the location in a given dimension
  virtual DIMENSION_MEASUREMENT_TYPE getRelative(const DIMENSION_INDEX_TYPE& index);
  virtual void setRelative(const DIMENSION_INDEX_TYPE& index, DIMENSION_MEASUREMENT_TYPE m);
  virtual DIMENSION_MEASUREMENT_TYPE getAbsolute(const DIMENSION_INDEX_TYPE& index);
  virtual void setAbsolute(const DIMENSION_INDEX_TYPE& index, DIMENSION_MEASUREMENT_TYPE m);
  //provides an unordered list of the dimensions that the Location exists in
  virtual std::vector<DIMENSION_INDEX_TYPE> getDimensionIndexes();
};

//sets all the dimensions of change to the dimensions of desired
void setLocationDimensionsToLocation(CountingPointer<Location> change, CountingPointer<Location> desired);
//creates a copy of a location, as opposed to sharing a location
CountingPointer<Location> getCopyOfLocation(CountingPointer<Location> loc);
//given a map, sets the dimensions of a location to the dimensions in that map
CountingPointer<Location> setDimensionsToMappedValues(CountingPointer<Location> loc, const Location::MAP_TYPE& mt);
//given a map, creates a location with the dimensions found in the map
CountingPointer<Location> getLocationFromDimensionMap(const Location::MAP_TYPE& mt);
//convert a location to a dimension map, using the absolute position of that location
Location::MAP_TYPE getAbsoluteDimensionMapFromLocation(CountingPointer<Location> loc);
//convert a location to a dimension map, using the relative position of that location
Location::MAP_TYPE getRelativeDimensionMapFromLocation(CountingPointer<Location> loc);
//reset a location's relative dimensions all to 0
void resetRelativeDimension(CountingPointer<Location> loc);
}

#endif

