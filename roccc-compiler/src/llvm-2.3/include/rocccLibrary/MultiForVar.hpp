/*
 *  MultiForVar.h
 *
 */

#ifndef _MULTI_FOR_VAR_HPP__
#define _MULTI_FOR_VAR_HPP__

#include <vector>

namespace llvm {
  
  template<class T>
  class MultiForVar {
    std::vector<T> _begin, _end, _vars;
    inline unsigned dim(){return _vars.size();}
  public:
    MultiForVar(std::vector<T> begin, std::vector<T> end) : _begin(begin), _end(end), _vars(_begin)
    {
      assert(begin.size() == end.size() and "Starting and ending vector<T> not the same size!" );
    }
    MultiForVar& operator ++()
    {
      if( done() )
        return *this;
      ++_vars[dim()-1];
      for(unsigned d = dim()-1; d > 0; --d)
      {
        if( _vars[d] >= _end[d] )
        {
          _vars[d] = _begin[d];
          ++_vars[d-1];
        }
      }
      return *this;
    }
    MultiForVar& operator +=(unsigned step)
    {
      for(unsigned i = 0; i < step; ++i)
        this->operator++();
      return *this;
    }
    bool done()
    {
      //check if any of the start values are set to the end values; leads to immediate done
      for(unsigned c = 0; c < _begin.size(); ++c)
        if( _begin[c] == _end[c] )
          return true;
      if( _vars.size() > 0 and _end.size() > 0 )
        return (_vars[0] >= _end[0]);
      return true;
    }
    T operator[](int d)
    {
      return _vars.at(d);
    }
    int numDimensions(){
      return dim();
    }
    std::vector<T>& getRaw(){
      return _vars;
    }
    std::vector<T> getBegin(){return _begin;}
    std::vector<T> getEnd(){return _end;}
  };

  template<class T>
  class ArrayTraverser {
    std::vector<T> _begin, _size, _vars;
    inline int dim(){return _vars.size();}
  public:
    ArrayTraverser(std::vector<T> begin, std::vector<T> size) : _begin(begin), _size(size), _vars(_begin)
    {
      assert(begin.size() == size.size() and "Starting and ending vector<T> not the same size!" );
    }
    ArrayTraverser& operator ++()
    {
      ++_vars[dim()-1];
      for(int d = dim()-1; d > 0; --d)
      {
        if( _vars[d] >= _size[d] )
        {
          _vars[d] = 0;
          ++_vars[d-1];
        }
      }
      return *this;
    }
    bool done()
    {
      return (!operator < ( _size ));
    }
    T operator[](int d)
    {
      return _vars.at(d);
    }
    int numDimensions(){
      return dim();
    }
    std::vector<T>& getRaw(){
      return _vars;
    }
    bool operator < (std::vector<T> rhs)
    {
      assert( _vars.size() == rhs.size() && "Comparing vectors of unequal size!" );
      for(int d = 0; d < dim(); ++d)
      {
        if( _vars[d] < rhs[d] )
          return true;
        if( _vars[d] > rhs[d] )
          return false;
      }
      return false;
    }
  };
  
  
}
#endif
