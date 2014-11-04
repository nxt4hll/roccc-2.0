#include "rocccLibrary/Window/Location.h"

namespace Window {

Location::Location() : loc()
{
}
Location::Location(const CountingPointer<Location::MAP_TYPE>& _loc) : loc(_loc)
{
}
Location::Location(const Location& other) : loc(other.loc)
{
}
Location::DIMENSION_MEASUREMENT_TYPE Location::getRelative(const Location::DIMENSION_INDEX_TYPE& index)
{
  return (*loc)[index];
}
void Location::setRelative(const Location::DIMENSION_INDEX_TYPE& index, Location::DIMENSION_MEASUREMENT_TYPE m)
{
  (*loc)[index] = m;
}
Location::DIMENSION_MEASUREMENT_TYPE Location::getAbsolute(const Location::DIMENSION_INDEX_TYPE& index)
{
  return getRelative(index);
}
void Location::setAbsolute(const Location::DIMENSION_INDEX_TYPE& index, Location::DIMENSION_MEASUREMENT_TYPE m)
{
  Location::DIMENSION_MEASUREMENT_TYPE cur_abs = this->getAbsolute(index);
  Location::DIMENSION_MEASUREMENT_TYPE shift_amount = m - cur_abs;
  Location::DIMENSION_MEASUREMENT_TYPE cur_rel = this->getRelative(index);
  this->setRelative(index, cur_rel + shift_amount);
  assert(this->getAbsolute(index) == m);
}
std::vector<Location::DIMENSION_INDEX_TYPE> Location::getDimensionIndexes()
{
  std::vector<Location::DIMENSION_INDEX_TYPE> ret;
  for(Location::MAP_TYPE::iterator MI = loc->begin(); MI != loc->end(); ++MI)
    ret.push_back(MI->first);
   return ret;
}

void setLocationDimensionsToLocation(CountingPointer<Location> change, CountingPointer<Location> desired)
{
  std::vector<Location::DIMENSION_INDEX_TYPE> indexes = desired->getDimensionIndexes();
  for(std::vector<Location::DIMENSION_INDEX_TYPE>::iterator II = indexes.begin(); II != indexes.end(); ++II)
  {
    change->setAbsolute(*II, desired->getAbsolute(*II));
  }
}
CountingPointer<Location> getCopyOfLocation(CountingPointer<Location> loc)
{
  CountingPointer<Location> ret;
  setLocationDimensionsToLocation(ret, loc);
  return ret;
}
CountingPointer<Location> setDimensionsToMappedValues(CountingPointer<Location> loc, const Location::MAP_TYPE& mt)
{
  for(Location::MAP_TYPE::const_iterator MI = mt.begin(); MI != mt.end(); ++MI)
    loc->setRelative(MI->first, MI->second);
  return loc;
}
CountingPointer<Location> getLocationFromDimensionMap(const Location::MAP_TYPE& mt)
{
  CountingPointer<Location> ret;
  return setDimensionsToMappedValues(ret, mt);
}
Location::MAP_TYPE getAbsoluteDimensionMapFromLocation(CountingPointer<Location> loc)
{
  Location::MAP_TYPE ret;
  std::vector<Location::DIMENSION_INDEX_TYPE> indexes = loc->getDimensionIndexes();
  for(std::vector<Location::DIMENSION_INDEX_TYPE>::iterator II = indexes.begin(); II != indexes.end(); ++II)
  {
    ret[*II] = loc->getAbsolute(*II);
  }
  return ret;
}
Location::MAP_TYPE getRelativeDimensionMapFromLocation(CountingPointer<Location> loc)
{
  Location::MAP_TYPE ret;
  std::vector<Location::DIMENSION_INDEX_TYPE> indexes = loc->getDimensionIndexes();
  for(std::vector<Location::DIMENSION_INDEX_TYPE>::iterator II = indexes.begin(); II != indexes.end(); ++II)
  {
    ret[*II] = loc->getRelative(*II);
  }
  return ret;
}
void resetRelativeDimension(CountingPointer<Location> loc)
{
  std::vector<Location::DIMENSION_INDEX_TYPE> indexes = loc->getDimensionIndexes();
  for(std::vector<Location::DIMENSION_INDEX_TYPE>::iterator II = indexes.begin(); II != indexes.end(); ++II)
  {
    loc->setRelative(*II, 0);
  }
}

}
