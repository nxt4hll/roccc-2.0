
#include "bvector.h"

Bvector::Bvector(size_t len, bool is_full) :
  _length(len),
  _bits(),
  _mask()
{
  for (size_t i = 0; i < len; i++) {
    _mask.set_bit(i, true);
  }
  if (is_full) _bits.set_to_ones();
}

Bvector::Bvector(const Bvector& that) :
  _length(that._length),
  _bits(that._bits),
  _mask(that._mask)
{}


// return true if the first _length th bits in bv are all zeros.
//
bool Bvector::is_zero_bits(const BitVector& bv) const
{
  static const BitVector empty;
  return (bv & _mask).is_equal_to(empty);
}


bool Bvector::get_bit(size_t i) const
{
  return _bits.get_bit(i);
}

void Bvector::set_bit(size_t i, bool b)
{
  _bits.set_bit(i, b);
}

size_t Bvector::get_length(void) const
{
  return _length;
}

void Bvector::set_length(size_t len, bool def)
{
  if (len == _length)
    return;
  else if (len < _length) {
    for (size_t i = len ; i < _length; i++)
      _mask.set_bit(i, false);
    _length = len;
  } else {  // _length < len
    for (size_t i = _length; i < len; i++) {
      _mask.set_bit(i, true);
      _bits.set_bit(i, def);
    }
    _length = len;
  }
}

bool Bvector::is_equal(const Bvector* that) const
{
  return is_zero_bits(this->_bits ^ that->_bits);
}

bool Bvector::is_subset(const Bvector* that) const
{
  return is_zero_bits(this->_bits - that->_bits);
}

bool Bvector::is_proper_subset(const Bvector* super) const
{
  return (is_subset(super) && !is_equal(super));
}

bool Bvector::is_all_zero(void) const
{
  return is_zero_bits(_bits);
}

bool Bvector::is_all_one(void) const
{
  return is_zero_bits(_bits ^ _mask);
}

void Bvector::set_and(const Bvector* a, const Bvector* b)
{
  _bits = (a->_bits & b->_bits);
}

void Bvector::set_or(const Bvector* a, const Bvector* b)
{
  _bits = (a->_bits | b->_bits);
}

void Bvector::set_not(const Bvector* a)
{
  _bits = (a->_bits).invert();
}

void Bvector::set_substract(const Bvector* a, const Bvector* b)
{
  _bits = (a->_bits & ~(b->_bits));
}

void Bvector::set_all(bool is_one)
{
  if (is_one)
    _bits.set_to_ones();
  else
    _bits.set_to_zero();
}

String Bvector::to_string(void)
{
  String res("[");
  for (size_t i = 0; i < _length; i++) {
    res += (get_bit(i) ? '1' : '0');
  }
  res += "]";
  return res;
}

	     
