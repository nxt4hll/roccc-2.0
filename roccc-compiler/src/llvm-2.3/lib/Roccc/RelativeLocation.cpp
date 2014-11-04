#include "rocccLibrary/Window/RelativeLocation.h"
#include "rocccLibrary/Window/CombineVectors.hpp"

namespace Window {

RelativeLocation::RelativeLocation(const CountingPointer<Location>& loc) : Location(), locus(loc)
{
}
RelativeLocation::RelativeLocation(const CountingPointer<Location>& loc, const Location::MAP_TYPE& dist) : Location(), locus(loc)
{
  for(Location::MAP_TYPE::const_iterator DI = dist.begin(); DI != dist.end(); ++DI)
  {
    this->setRelative(DI->first, DI->second);
  }
}
Location::DIMENSION_MEASUREMENT_TYPE RelativeLocation::getAbsolute(const Location::DIMENSION_INDEX_TYPE& index)
{
  return locus->getAbsolute(index) + this->getRelative(index);
}
std::vector<Location::DIMENSION_INDEX_TYPE> RelativeLocation::getDimensionIndexes()
{
  return combineVectors( this->Location::getDimensionIndexes(), locus->getDimensionIndexes() );
}
CountingPointer<Location> RelativeLocation::getLocus()
{
  return locus;
}

RigidRelativeLocation::RigidRelativeLocation(const CountingPointer<Location>& loc) : RelativeLocation(loc)
{
}
RigidRelativeLocation::RigidRelativeLocation(const CountingPointer<Location>& loc, const Location::MAP_TYPE& dist) : RelativeLocation(loc, dist)
{
}
void RigidRelativeLocation::setAbsolute(const DIMENSION_INDEX_TYPE& index, DIMENSION_MEASUREMENT_TYPE m)
{
  DIMENSION_MEASUREMENT_TYPE cur_abs = this->getAbsolute(index);
  DIMENSION_MEASUREMENT_TYPE shift_amount = m - cur_abs;
  locus->setAbsolute(index, locus->getAbsolute(index) + shift_amount);
  assert(this->getAbsolute(index) == m);
}

}

