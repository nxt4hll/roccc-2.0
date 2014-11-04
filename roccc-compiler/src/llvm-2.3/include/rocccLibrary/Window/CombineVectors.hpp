#ifndef _COMBINE_VECTORS_HPP__
#define _COMBINE_VECTORS_HPP__

#include <vector>
#include <algorithm>

//helper function to create a vector that is the join of two vectors, handling uniques
template<class T>
std::vector<T> combineVectors(const std::vector<T>& lhs, const std::vector<T>& rhs)
{
  std::vector<T> tmp;
  tmp.insert(tmp.end(), lhs.begin(), lhs.end());
  tmp.insert(tmp.end(), rhs.begin(), rhs.end());
  std::sort(tmp.begin(), tmp.end());
  typename std::vector<T>::iterator new_end = std::unique(tmp.begin(), tmp.end());
  std::vector<T> ret;
  ret.insert(ret.end(), tmp.begin(), new_end);
  return ret;
}

#endif
