#ifndef _WINDOW_UTILS_H__
#define _WINDOW_UTILS_H__

#include "rocccLibrary/CountingPointer.hpp"
#include "rocccLibrary/Window/Location.h"
#include "rocccLibrary/Window/CombineVectors.hpp"

namespace Window {

//Test whether any of the dimensions in lhs and rhs are operand Op
//Op must take two DIMENSION_MEASUREMENT_TYPEs and return a bool
template<class Op>
bool areAnyDimensions(Op op, CountingPointer<Location> lhs, CountingPointer<Location> rhs)
{
  std::vector<Location::DIMENSION_INDEX_TYPE> tlv = combineVectors(lhs->getDimensionIndexes(), rhs->getDimensionIndexes());
  for(std::vector<Location::DIMENSION_INDEX_TYPE>::iterator DI = tlv.begin(); DI != tlv.end(); ++DI)
  {
    if( op(lhs->getAbsolute(*DI), rhs->getAbsolute(*DI)) )
      return true;
  }
  return false;
}
//test whether all of the dimensions in lhs and rhs are operand Op
//Op must take two DIMENSION_MEASUREMENT_TYPEs and return a bool
template<class Op>
bool areAllDimensions(Op op, CountingPointer<Location> lhs, CountingPointer<Location> rhs)
{
  std::vector<Location::DIMENSION_INDEX_TYPE> tlv = combineVectors(lhs->getDimensionIndexes(), rhs->getDimensionIndexes());
  for(std::vector<Location::DIMENSION_INDEX_TYPE>::iterator DI = tlv.begin(); DI != tlv.end(); ++DI)
  {
    if( !op(lhs->getAbsolute(*DI), rhs->getAbsolute(*DI)) )
      return false;
  }
  return true;
}

}

#endif
