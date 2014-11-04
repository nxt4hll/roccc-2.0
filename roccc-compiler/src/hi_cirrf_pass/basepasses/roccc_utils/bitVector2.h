// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
/*

  This file describes a bit vector purely as a bit vector and not associated
   with a bit vector map.  This allows us to perform things correctly in
   the ROCCCC 2.0 environment.

  In order to be placed in an annotation, this must be derived from SuifObject.
    That means we're going to have to declare a couple of virtual functions.

*/

#ifndef __BIT_VECTOR_TWO_DOT_H__
#define __BIT_VECTOR_TWO_DOT_H__

#include "common/MString.h"
#include "suifkernel/suif_object.h"

#include <vector>


class BitVector2 : public SuifObject
{
 private:
  std::vector<bool> value ;
  
 public:
  BitVector2(unsigned int i) ;
  ~BitVector2() ;

  BitVector2(BitVector2& x) ;

  void mark(unsigned int location) ;
  void unmark(unsigned int location) ;
  bool isMarked(unsigned int location) ;

  void intersect(BitVector2* x) ;
  void merge(BitVector2* x) ; // Union
  void copy(BitVector2* x) ;
  void subtract(BitVector2* x) ;

  String getString() ;

  unsigned int size() ;

} ;

#endif 
