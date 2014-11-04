#include "nary_bit_lattice.h"

BinaryRelationBitLattice::BinaryRelationBitLattice() :
  _is_bottom(),
  _value(),
  _default_value(BitLattice())
{
  
}

BinaryRelationBitLattice::
BinaryRelationBitLattice(const BitLattice &default_value) :
  _is_bottom(),
  _value(),
  _default_value(default_value)
{
}

void BinaryRelationBitLattice::
set_default_value(const BitLattice &default_value) {
  assert(significant_bit_count() == 0);
  if (_default_value.get_bottom_bit() != default_value.get_bottom_bit())
    _is_bottom = _is_bottom.do_negate(_is_bottom);
  if (_default_value.get_value_bit() != default_value.get_value_bit())
    _value = _value.do_negate(_value);
  _default_value = default_value;
}

BinaryRelationBitLattice::
BinaryRelationBitLattice(const BinaryRelationBitLattice &other) :
  _is_bottom(other._is_bottom),
  _value(other._value),
  _default_value(other._default_value)
{
}

BinaryRelationBitLattice &BinaryRelationBitLattice::operator=(const BinaryRelationBitLattice &other)
{
  _is_bottom = other._is_bottom;
  _value = other._value;
  _default_value = other._default_value;
  return(*this);
}

BinaryRelationBitLattice::~BinaryRelationBitLattice()
{}

bool BinaryRelationBitLattice::
operator==(const BinaryRelationBitLattice &other) const {
  if (!(other._default_value == _default_value)) return false;
  if (!(other._is_bottom == _is_bottom)) return false;
  if (!(other._value == _value)) return false;
  return(true);
}

bool BinaryRelationBitLattice::
operator!=(const BinaryRelationBitLattice &other) const {
  return(!(*this == other));
}

BitLattice BinaryRelationBitLattice::
get_value(size_t domain, size_t range) const {
  return(BitLattice(_is_bottom.get_value(domain).get_value(range),
		    _value.get_value(domain).get_value(range)));
}

void BinaryRelationBitLattice::
set_value(size_t domain, size_t range, const BitLattice &value) {
  if (domain < _is_bottom.size() ||
      value.get_bottom_bit() != _default_value.get_bottom_bit()) {
    UnaryRelation &r1 = _is_bottom[domain];
    r1.set_value(range, value.get_bottom_bit());
  }
  if (domain < _value.size() ||
      value.get_value_bit() != _default_value.get_value_bit()) {
    UnaryRelation &r1 = _value[domain];
    r1.set_value(range, value.get_value_bit());
  }
}

size_t BinaryRelationBitLattice::
significant_bit_count() const {
  size_t m1 = _is_bottom.significant_bit_count();
  size_t m2 = _value.significant_bit_count();
  if (m2 > m1) return m2;
  return(m1);
}

bool BinaryRelationBitLattice::do_meet_with_test(const BinaryRelationBitLattice &other) {
  bool changed = false;
  if (_is_bottom.do_meet_with_test(other._is_bottom)) changed = true;
  if (_value.do_meet_with_test(other._value)) changed = true;
  if (_default_value.do_meet_with_test(other._default_value)) changed = true;
  return(changed);
}

#if 0
BinaryRelationBitLattice BinaryRelationBitLattice::
transitive_closure(const BinaryRelationBitLattice &other);

BinaryRelationBitLattice BinaryRelationBitLattice::
transpose(const BinaryRelationBitLattice &other);

#endif

BinaryRelationBitLattice BinaryRelationBitLattice::
do_meet(const BinaryRelationBitLattice &val1, 
     const BinaryRelationBitLattice &val2)
{
  BinaryRelationBitLattice val(val1);
  val.do_meet_with_test(val2);
  return(val);
}

TrinaryRelationBitLattice::TrinaryRelationBitLattice() :
  _is_bottom(), _value(), _default_value()
{}

TrinaryRelationBitLattice::
TrinaryRelationBitLattice(const BitLattice &default_value) :
  _is_bottom(),
  _value(),
  _default_value(default_value)
{
  if (default_value.get_bottom_bit())
    _is_bottom = _is_bottom.do_negate(_is_bottom);
  if (default_value.get_value_bit())
    _value = _value.do_negate(_value);
}
  
void TrinaryRelationBitLattice::
set_default_value(const BitLattice &default_value) {
  assert(significant_bit_count() == 0);
  if (_default_value.get_bottom_bit() != default_value.get_bottom_bit())
    _is_bottom = _is_bottom.do_negate(_is_bottom);
  if (_default_value.get_value_bit() != default_value.get_value_bit())
    _value = _value.do_negate(_value);
  _default_value = default_value;
}
TrinaryRelationBitLattice::
TrinaryRelationBitLattice(const TrinaryRelationBitLattice &other) :
  _is_bottom(other._is_bottom),
  _value(other._value),
  _default_value(other._default_value)
{
}
TrinaryRelationBitLattice &
TrinaryRelationBitLattice::operator=(const TrinaryRelationBitLattice &other) {
  _is_bottom = other._is_bottom;
  _value = other._value;
  _default_value = other._default_value;
  return(*this);
}
TrinaryRelationBitLattice::~TrinaryRelationBitLattice() {}

bool TrinaryRelationBitLattice::
operator==(const TrinaryRelationBitLattice &other) const {
  if (significant_bit_count() != other.significant_bit_count()) return false;
  if (!(_is_bottom == other._is_bottom)) return false;
  if (!(_value == other._value)) return false;
  if (!(_default_value == other._default_value)) return false;
  return(true);
}

bool TrinaryRelationBitLattice::
operator!=(const TrinaryRelationBitLattice &other) const {
  return(!(*this == other));
}
  
BitLattice 
TrinaryRelationBitLattice::get_value(size_t arg0, size_t arg1, 
				     size_t arg2) const {
  return(BitLattice(_is_bottom.get_value(arg0).get_value(arg1).get_value(arg2),
		    _value.get_value(arg0).get_value(arg1).get_value(arg2)));
}
void TrinaryRelationBitLattice::
set_value(size_t arg0, size_t arg1, size_t arg2, 
	  const BitLattice &value) {

  if (arg0 < _is_bottom.size() ||
      value.get_bottom_bit() != _default_value.get_bottom_bit()) {
    BinaryRelation &r0 = _is_bottom[arg0];
    if (arg1 < r0.size() ||
	value.get_bottom_bit() != _default_value.get_bottom_bit()) {
      UnaryRelation &r1 = r0[arg1];
      r1.set_value(arg2, value.get_bottom_bit());
    }
  }
  if (arg0 < _value.size() ||
      value.get_value_bit() != _default_value.get_value_bit()) {
    BinaryRelation &r0 = _value[arg0];
    if (arg1 < r0.size() ||
	value.get_value_bit() != _default_value.get_value_bit()) {
      UnaryRelation &r1 = r0[arg1];
      r1.set_value(arg2, value.get_value_bit());
    }
  }
}

size_t TrinaryRelationBitLattice::significant_bit_count() const 
{
  size_t m1 = _is_bottom.significant_bit_count();
  size_t m2 = _value.significant_bit_count();
  if (m2 > m1)
    return(m2);
  return(m1);
}

bool TrinaryRelationBitLattice::
do_meet_with_test(const TrinaryRelationBitLattice &other) {
  bool changed = false;
  if (_is_bottom.do_meet_with_test(other._is_bottom)) changed = true;
  if (_value.do_meet_with_test(other._value)) changed = true;
  if (_default_value.do_meet_with_test(other._default_value)) changed = true;
  return(changed);
}

#if 0
TrinaryRelationBitLattice TrinaryRelationBitLattice::
transitive_closure(const TrinaryRelationBitLattice &);
#endif

TrinaryRelationBitLattice TrinaryRelationBitLattice::
do_meet(const TrinaryRelationBitLattice &val1, 
	const TrinaryRelationBitLattice &val2) {
  TrinaryRelationBitLattice val(val1);
  val.do_meet_with_test(val2);
  return(val);
}


