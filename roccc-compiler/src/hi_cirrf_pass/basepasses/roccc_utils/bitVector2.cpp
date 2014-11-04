// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.

#include <cassert>
#include "bitVector2.h"

BitVector2::BitVector2(unsigned int size)
{
  for (unsigned int i = 0 ; i < size ; ++i)
  {
    value.push_back(false) ;
  }
  assert(value.size() == size) ;
}

BitVector2::~BitVector2()
{
  ; // Nothing to delete
}

// Copy constructor
BitVector2::BitVector2(BitVector2& x)
{
  value = x.value ;
}

void BitVector2::mark(unsigned int location)
{
  assert(location < value.size()) ;
  value[location] = true ;
}

void BitVector2::unmark(unsigned int location)
{
  assert(location < value.size()) ;
  value[location] = false ;
}

bool BitVector2::isMarked(unsigned int location)
{
  assert(location < value.size()) ;
  return value[location] ;
}

unsigned int BitVector2::size()
{
  return value.size() ;
}

void BitVector2::intersect(BitVector2* x)
{
  assert(value.size() == x->size()) ;

  for (unsigned int i = 0 ; i < value.size() ; ++i)
  {
    value[i] = value[i] & x->isMarked(i) ;
  }
}

void BitVector2::merge(BitVector2* x) // Union
{
  assert(value.size() == x->size()) ;
  
  for(unsigned int i = 0 ; i < value.size() ; ++i)
  {
    value[i] = value[i] | x->isMarked(i) ;
  }
}

void BitVector2::copy(BitVector2* x)
{
  assert(value.size() == x->size()) ;
  
  for (unsigned int i = 0 ; i < value.size() ; ++i)
  {
    value[i] = x->isMarked(i) ;
  }
}

void BitVector2::subtract(BitVector2* x)
{
  assert(value.size() == x->size()) ;
  
  for(unsigned int i = 0 ; i < value.size() ; ++i)
  {
    value[i] = value[i] & ~(x->isMarked(i)) ;
  }
}

String BitVector2::getString()
{
  String output = "" ;

  for (int i = value.size() - 1 ; i >= 0 ; --i)
  {
    output += String((int)(value[i])) ;
  }

  return output ;
}

