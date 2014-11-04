#include "bit_lattice.h"

BitLattice::BitLattice() : 
  _is_bottom(0), _value(0) 
{} /* top */

BitLattice::BitLattice(BVal val) : 
  _is_bottom(val == B_BOTTOM || val == B_FALSE),
  _value(val == B_BOTTOM || val == B_TRUE)
{}

BitLattice::BitLattice(const BitLattice &val) :
  _is_bottom(val._is_bottom),
  _value(val._value)
{}

BitLattice &BitLattice::operator=(const BitLattice &val) {
  _is_bottom = val._is_bottom;
  _value = val._value;
  return(*this);
}
BitLattice::BitLattice(bool value) : 
  _is_bottom(!value),
  _value(value)
{}
BitLattice::BitLattice(bool is_bottom, bool value) : 
  _is_bottom(is_bottom),
  _value(value)
{}

BitLattice::~BitLattice() {}

String BitLattice::toString() const { 
  if (is_top()) return "^";
  if (is_bottom()) return "v";
  if (is_true()) return "1";
  assert(is_false());
  return "0";
}

bool BitLattice::is_definite() const { 
  return(_is_bottom != _value);
}

bool BitLattice::is_top() const { 
  return (!is_definite() && !_is_bottom); 
}

bool BitLattice::is_true() const { 
  return (is_definite() && _value);
}
bool BitLattice::is_false() const { 
  return (is_definite() && !_value);
}

bool BitLattice::is_bottom() const { 
  return (!is_definite() && _is_bottom);
} 

BitLattice::BVal 
BitLattice::get_value() const { 
  if (!_is_bottom && !_value) return(B_TOP);
  if (_is_bottom && _value) return(B_BOTTOM);
  if (!_is_bottom && _value) return(B_TRUE);
  assert(_is_bottom && !_value);
  return(B_FALSE);
}
bool BitLattice::get_bottom_bit() const { return _is_bottom; }
bool BitLattice::get_value_bit() const { return _value; }


bool BitLattice::operator==(const BitLattice &other) const {
  return(other._is_bottom == _is_bottom
	 && other._value == _value);
}
bool BitLattice::operator!=(const BitLattice &other) const {
  return(!(*this == other));
}

/* Eventually, these relations should be turned into 
 * logic equations that can be used for vectors as
 * well as bits
 */

/* meet in place. return true if changed */
bool BitLattice::do_meet_with_test(const BitLattice &other) {
  BitLattice result = do_meet(*this, other);
  if (result == (*this))
    return(false);
  *this = result;
  return(true);
}

/*
 * OR
 *          Top   1   0   Bot
 * Top      Top   1   Top Top
 * 1        1     1   1   1
 * 0        Top   1   0   Bot
 * Bot      Top   1   Bot Bot
 */

BitLattice BitLattice::do_or(const BitLattice &val1, 
			     const BitLattice &val2) {
  if (val1.is_true())
    return(val1);
  if (val2.is_true())
    return(val2);
  if (val1.is_top())
    return(val1);
  if (val2.is_top())
    return(val2);
  if (val1.is_false() &&
      val2.is_false())
    return(val1);

  return(BitLattice(B_BOTTOM));
}

/*
 * AND
 *          Top   1   0   Bot
 * Top      Top   Top 0   Top
 * 1        Top   1   0   Bot
 * 0        0     0   0   0
 * Bot      Top   Bot 0   Bot
 */
BitLattice BitLattice::do_and(const BitLattice &val1, 
			      const BitLattice &val2) {
  if (val1.is_false())
    return(val1);
  if (val2.is_false())
    return(val2);
  if (val1.is_top())
    return(val1);
  if (val2.is_top())
    return(val2);
  if (val1.is_true() && val2.is_true())
    return(val1);
  return(BitLattice(B_BOTTOM));
}


/*
 * Meet
 *          Top   1   0   Bot
 * Top      Top   1   0   Bot
 * 1        1     1   Bot Bot
 * 0        0     Bot 0   Bot
 * Bot      Bot   Bot Bot Bot
 */

/* meet in place. return true if changed */
BitLattice BitLattice::do_meet(const BitLattice &val1, 
			       const BitLattice &val2) {
  return(BitLattice(val1.get_bottom_bit() || val2.get_bottom_bit(),
		    val1.get_value_bit() || val2.get_value_bit()));
}


bool BitLattice::
do_and_with_test(const BitLattice &other) {
  BitLattice result = do_and(*this, other);
  if (result == (*this))
    return(false);
  *this = result;
  return(true);
}
bool BitLattice::
do_or_with_test(const BitLattice &other) {
  BitLattice result = do_or(*this, other);
  if (result == (*this))
    return(false);
  *this = result;
  return(true);
}

/*
 * Not
 *          
 * Top      Top
 * 1        0  
 * 0        1  
 * Bot      Bot
 */

bool BitLattice::
do_not_with_test() {
  if (!is_definite())
    return(false);
  _value = !_value;
  _is_bottom = !_is_bottom;
  return(true);
}

BitLattice BitLattice::
do_not(const BitLattice &val) {
  if (!val.is_definite())
    return(val);
  /* switched */
  return(BitLattice(val._value, val._is_bottom));
}




/* 
 * The default constructor builds a bit vector
 * with all values "top"
 * Initial implementation does not reference count
 * These should be reference counted and
 * 
 */

UnaryRelationBitLattice::UnaryRelationBitLattice() : /* default value is TOP */
  _is_bottom(),
  _value(),
  _default_value()
{}

UnaryRelationBitLattice::UnaryRelationBitLattice(const UnaryRelationBitLattice &other) :
  _is_bottom(other._is_bottom),
  _value(other._value),
  _default_value(other._default_value)
{}

UnaryRelationBitLattice::
UnaryRelationBitLattice(const BitLattice &default_value) :
  _is_bottom(),
  _value(),
  _default_value(default_value)
{
  if (default_value.get_bottom_bit()) {
    _is_bottom = _is_bottom.do_negate(_is_bottom);
  }
  if (default_value.get_value_bit()) {
    _is_bottom = _is_bottom.do_negate(_is_bottom);
  }
}

UnaryRelationBitLattice &UnaryRelationBitLattice::operator=(const UnaryRelationBitLattice &other) 
{
  _is_bottom = other._is_bottom;
  _value = other._value;
  _default_value = other._default_value;
  return(*this);
}
UnaryRelationBitLattice::~UnaryRelationBitLattice() 
{}

bool UnaryRelationBitLattice::
operator==(const UnaryRelationBitLattice &other) const {
  if (_is_bottom != other._is_bottom) return(false);
  if (_value != other._value) return(false);
  if (! (_default_value == other._default_value)) return(false);
  return(true);
}

bool UnaryRelationBitLattice::
operator!=(const UnaryRelationBitLattice &other) const {
  return(!(*this == other));
}

BitLattice UnaryRelationBitLattice::get_value(size_t i) const {
  return(BitLattice(_is_bottom.get_value(i), _value.get_value(i)));
}
void UnaryRelationBitLattice::set_value(size_t i, const BitLattice &value) {
  _is_bottom.set_value(i, value.get_bottom_bit());
  _value.set_value(i, value.get_value_bit());
}

void UnaryRelationBitLattice::set_default_value(const BitLattice &value) {
  assert(_is_bottom.is_empty() && _value.is_empty());
  if (value.get_bottom_bit() != _default_value.get_bottom_bit())
    _value = _value.do_negate(_value);

  if (value.get_value_bit() != _default_value.get_value_bit())
    _is_bottom = _is_bottom.do_negate(_is_bottom);
  
  _default_value = value;
}
size_t UnaryRelationBitLattice::significant_bit_count() const {
  size_t vm = _value.significant_bit_count();
  size_t bm = _is_bottom.significant_bit_count();
  if (bm > vm)
    vm = bm;
  return(vm);
}

UnaryRelationBitLattice UnaryRelationBitLattice::
map_unary(unary_bit_transform fn,
	  const UnaryRelationBitLattice &val) {

  UnaryRelationBitLattice new_val(fn(val._default_value));
  size_t nsb = val.significant_bit_count();
  for (size_t i = 0; i < nsb;i++) {
    new_val.set_value(i, fn(val.get_value(i)));
  }
  return(new_val);
}

UnaryRelationBitLattice UnaryRelationBitLattice::
map_binary(binary_bit_transform fn,
	  const UnaryRelationBitLattice &val1,
	  const UnaryRelationBitLattice &val2) {
  UnaryRelationBitLattice new_val(fn(val1._default_value,
				     val2._default_value));
  size_t nsb = val1.significant_bit_count();
  size_t nsb2 = val2.significant_bit_count();
  if (nsb2 > nsb)
    nsb = nsb2;

  for (size_t i = 0; i < nsb;i++) {
    new_val.set_value(i, fn(val1.get_value(i),
			    val2.get_value(i)));
  }
  return(new_val);
}

bool UnaryRelationBitLattice::
map_unary_with_test(unary_bit_transform fn) {
  bool changed = false;
  BitLattice new_default = fn(_default_value);
  if (_default_value != new_default) {
    _default_value = new_default;
    changed = true;
  }
  size_t nsb = significant_bit_count();
  for (size_t i = 0; i < nsb;i++) {
    BitLattice old_value = get_value(i);
    BitLattice new_value = fn(get_value(i));
    if (new_value != old_value) {
      set_value(i, new_value);
      changed = true;
    }
  }
  return(changed);
}
bool UnaryRelationBitLattice::
map_binary_with_test(binary_bit_transform fn, 
		     const UnaryRelationBitLattice &val) {
  bool changed = false;
  BitLattice new_default = fn(_default_value, val._default_value);
  if (_default_value != new_default) {
    _default_value = new_default;
    changed = true;
  }

  size_t nsb = significant_bit_count();
  for (size_t i = 0; i < nsb;i++) {
    BitLattice old_value = get_value(i);
    BitLattice old_value2 = val.get_value(i);
    BitLattice new_value = fn(old_value, old_value2);
    if (new_value != old_value) {
      set_value(i, new_value);
      changed = true;
    }
  }
  return(changed);
}


bool UnaryRelationBitLattice::do_meet_with_test(const UnaryRelationBitLattice &other) {
  return(map_binary_with_test(&BitLattice::do_meet,
			      other));
}
  
UnaryRelationBitLattice UnaryRelationBitLattice::
do_meet(const UnaryRelationBitLattice &rel1,
     const UnaryRelationBitLattice &rel2) {
  return(map_binary(&BitLattice::do_meet,
		    rel1, rel2));
}

bool UnaryRelationBitLattice::
do_or_with_test(const UnaryRelationBitLattice &other) {
  return(map_binary_with_test(&BitLattice::do_or,
			      other));
}
  
UnaryRelationBitLattice UnaryRelationBitLattice::
do_or(const UnaryRelationBitLattice &rel1,
     const UnaryRelationBitLattice &rel2) {
  return(map_binary(&BitLattice::do_or,
		    rel1, rel2));
}

bool UnaryRelationBitLattice::
do_and_with_test(const UnaryRelationBitLattice &other) {
  return(map_binary_with_test(&BitLattice::do_and,
			      other));
}
  
UnaryRelationBitLattice UnaryRelationBitLattice::
do_and(const UnaryRelationBitLattice &rel1,
       const UnaryRelationBitLattice &rel2) {
  return(map_binary(&BitLattice::do_and,
		    rel1, rel2));
}

bool UnaryRelationBitLattice::
do_not_with_test() {
  return(map_unary_with_test(&BitLattice::do_not));
}
  
UnaryRelationBitLattice UnaryRelationBitLattice::
do_not(const UnaryRelationBitLattice &rel) {
  return(map_unary(&BitLattice::do_not, rel));
}


