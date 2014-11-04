#include "unary.h"

UnaryRelation::UnaryRelation() :
  _value()
{}
UnaryRelation::UnaryRelation(const UnaryRelation &other) :
  _value(other._value)
{}
UnaryRelation::UnaryRelation(bool default_value) : 
  _value()
{
  if (default_value) _value.set_to_ones();
}
UnaryRelation UnaryRelation::do_negate(const UnaryRelation &val) {
  assert(val.is_empty());
  UnaryRelation r(!val._value.get_infinity_bit());
  return(r);
}
UnaryRelation &UnaryRelation::operator =(const UnaryRelation &other) {
  _value = other._value;
  return(*this);
}
bool UnaryRelation::operator ==(const UnaryRelation &other) const {
  return(_value == other._value);
}
bool UnaryRelation::operator !=(const UnaryRelation &other) const {
  return(_value != other._value);
}

bool UnaryRelation::get_value(size_t i) const {
  return(_value.get_bit(i));
}
void UnaryRelation::set_value(size_t i, bool value) {
  _value.set_bit(i, value);
}

bool UnaryRelation::is_empty() const {
  return(_value.num_significant_bits() == 0);
}
size_t UnaryRelation::size() const {
  return(_value.num_significant_bits());
}
void UnaryRelation::set_default_value(bool default_value) {
  assert(is_empty());
  if (default_value != _value.get_infinity_bit())
    _value = ~_value;
}
size_t UnaryRelation::significant_bit_count() const {
  return(size());
}
bool UnaryRelation::do_meet_with_test(const UnaryRelation &other) {
  BitVector value = this->_value | other._value;
  if (value == _value) return false;
  _value = value;
  return(true);
}
UnaryRelation UnaryRelation::
do_meet(const UnaryRelation &val1, const UnaryRelation &val2) {
  UnaryRelation r(val1);
  r.do_meet_with_test(val2);
  return(r);
}

