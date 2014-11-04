#ifndef _COUNTING_POINTER_H__
#define _COUNTING_POINTER_H__

#include <cassert> //for assert()
#include <cstddef> //for NULL

template<class T>
class CountingPointer {
  T* t;
  int* count;
public:
  //if this constructor is used, T must be defualt constructable
  CountingPointer() : t(new T()), count(new int)
  {
    (*count) = 1;
  }
  //if this constructor is used, CountingPointer owns the pointer
  explicit CountingPointer(T* _t) : t(_t), count(new int)
  {
    (*count) = 1;
  }
  //copy constructor
  CountingPointer(const CountingPointer& other) : t(other.t), count(other.count)
  {
    assert( count );
    ++(*count);
  }
  CountingPointer& operator =(const CountingPointer& other)
  {
    t = other.t;
    count = other.count;
    assert( count );
    ++(*count);
    return *this;
  }
  virtual ~CountingPointer()
  {
    assert( count );
    --(*count);
    if( (*count) < 1 )
    {
      delete count;
      if( t )
        delete t;
    }
  }
  //code that calls this function is responsible for deleting the pointer - RISKY
  T* takeOwnership()
  {
    assert( count );
    ++(*count);
    return t;
  }
  //explicit getter/setter
  T get() const
  {
    assert( t );
    return *t;
  }
  void set(T _t)
  {
    assert( t );
    *t = _t;
  }
  //less explicit dereference operator
  T& operator *()
  {
    assert( t );
    return *t;
  }
  //and access operator
  T* operator ->()
  {
    assert( t );
    return t;
  }
  //any CountingPointer specialization can access any other CountingPointer specialization
  template< class DYN >
  friend class CountingPointer;
  template< class DYN >
  //specifically for this type of issue; dynamic casting
  CountingPointer<DYN> dyn_cast()
  {
    CountingPointer<DYN> ret(NULL);
    ret.t = dynamic_cast<DYN*>(t);
    if( !ret.t )
      return ret;
    if( ret.count )
      delete ret.count;
    ret.count = count;
    assert( ret.count );
    ++(*ret.count);
    return ret;
  }
  bool isNull()
  {
    return (t == NULL);
  }
  //comparison operator ==
  template< class DYN >
  bool operator == (const CountingPointer<DYN>& other) const
  {
    return (t == other.t);
  }
  //comparison operator !=
  template< class DYN >
  bool operator != (const CountingPointer<DYN>& other) const
  {
    return (t != other.t);
  }
};

#endif
