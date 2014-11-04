#include "const_lattice.h"

ConstantLattice::ConstantLattice() :
  _key(B_TOP),
  _value()
{}
ConstantLattice::ConstantLattice(BVal key) :
  _key(key),
  _value()
{
  assert(key != B_CONST);
}
ConstantLattice::ConstantLattice(IInteger value) :
  _key(B_CONST),
  _value(value)
{
  assert(!value.is_undetermined());
  assert(!value.is_signless_infinity());
}
ConstantLattice::ConstantLattice(const ConstantLattice &other) :
  _key(other._key),
  _value(other._value)
{  
}

ConstantLattice &ConstantLattice::operator=(const ConstantLattice &other) {
  _key = other._key;
  _value = other._value;
  return(*this);
}

ConstantLattice::~ConstantLattice() {}

bool ConstantLattice::is_top() const {
  return(_key == B_TOP);
}
IInteger ConstantLattice::get_constant() const {
  return(_value);
}

String ConstantLattice::toString() const { 
  switch(_key) {
  case B_TOP:
    return("^");
  case B_CONST:
    return(_value.to_String());
  case B_BOTTOM:
    return("v");
  default:
    break;
  }
  assert(0);
  return("v");
}

bool ConstantLattice::is_bottom() const {
  return(_key == B_BOTTOM);
}

bool ConstantLattice::is_const() const {
  return(_key == B_CONST);
}

ConstantLattice ConstantLattice::bottom() {
  return(ConstantLattice(B_BOTTOM));
}
ConstantLattice ConstantLattice::top() {
  return(ConstantLattice(B_BOTTOM));
}



ConstantLattice::BVal ConstantLattice::get_key() const {
  return(_key);
}
bool ConstantLattice::operator==(const ConstantLattice &other) const {
  return(_key == other._key && 
	 (_value == other._value
	  || (_value.is_undetermined() && other._value.is_undetermined())));
}

bool ConstantLattice::operator!=(const ConstantLattice &other) const {
  return(!(*this == other));
}

ConstantLattice ConstantLattice::do_meet(const ConstantLattice &val1,
					 const ConstantLattice &val2) {
  if (val1.is_top()) return(val2);
  if (val2.is_top()) return(val1);
  if (val1.is_bottom()) return(val1);
  if (val2.is_bottom()) return(val2);
  if (val1.get_key() != val2.get_key())
    return(ConstantLattice(B_BOTTOM));
  if (val1.get_key() != B_CONST)
    return(val1);

  if (val1.get_constant() != val2.get_constant())
    return(ConstantLattice(B_BOTTOM));

  return(val1);
}

bool ConstantLattice::
do_meet_with_test(const ConstantLattice &val1) {
  ConstantLattice val = do_meet(*this, val1);
  if (val == *this)
    return(false);
  *this = val;
  return(true);
}
