#include "sgraph_bit_iter.h"
#include "bit_vector/bit_vector.h"
#include <string.h>
#include <stdlib.h>

SGraphBitIter::SGraphBitIter() : 
  _bits(NULL), _owned(false), _iter(0) {}
SGraphBitIter::SGraphBitIter(const SGraphBitIter &other) :
  _bits(NULL), _owned(false), _iter(0) {
  reset(other._bits, false);
  }
SGraphBitIter::SGraphBitIter(const BitVector *bits, bool owned) :
  _bits(NULL), _owned(false), _iter(0) {
  reset(bits, owned); 
}
SGraphBitIter &SGraphBitIter::operator=(const SGraphBitIter &other) {
  reset(other._bits, false);
  return(*this);
}
void SGraphBitIter::reset(const BitVector *bits, bool owned) {
   delete _iter;

   if (_owned && _bits != NULL) {
	 delete _bits;
   }

  _bits = bits;
  _owned = owned;
  _iter = new BitVectorIter(_bits);
}

SGraphNodeIter *SGraphBitIter::clone() const {
  if (_owned) return new SGraphBitIter(new BitVector(*_bits), true);
  return(new SGraphBitIter(_bits, false));
}

void SGraphBitIter::reset() {
  _iter->first();
}

bool SGraphBitIter::done() const { 
  if (!_iter) return(true);
  return _iter->done(); 
}
size_t SGraphBitIter::get() const { 
  assert(!done());
  return(_iter->get());
}
void SGraphBitIter::increment() {
  if (done()) return;
  _iter->next();
}
