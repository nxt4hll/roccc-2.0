#include "init_lattice.h"

InitLattice::InitLattice() :
  _key(B_TOP)
{}
InitLattice::InitLattice(BVal key) :
  _key(key)
{}
InitLattice::InitLattice(const InitLattice &other) :
  _key(other._key)
{  
}

InitLattice &InitLattice::operator=(const InitLattice &other) {
  _key = other._key;
  return(*this);
}

InitLattice InitLattice::bottom() {
  return(InitLattice(B_BOTTOM));
}
InitLattice InitLattice::top() {
  return(InitLattice(B_TOP));
}
InitLattice InitLattice::initial() {
  return(InitLattice(B_INITIAL));
}


InitLattice::~InitLattice() {}

bool InitLattice::is_top() const {
  return(_key == B_TOP);
}
String InitLattice::toString() const { 
  switch(_key) {
  case B_TOP:
    return("TOP");
  case B_INITIAL:
    return("INIT");
  case B_BOTTOM:
    return("BOT");
  default:
    break;
  }
  assert(0);
  return("BOT");
}

bool InitLattice::is_bottom() const {
  return(_key == B_BOTTOM);
}
bool InitLattice::is_initial() const {
  return(_key == B_INITIAL);
}

InitLattice::BVal InitLattice::get_key() const {
  return(_key);
}

bool InitLattice::operator==(const InitLattice &other) const {
  return(_key == other._key);
}

bool InitLattice::operator!=(const InitLattice &other) const {
  return(!(*this == other));
}

InitLattice InitLattice::do_meet(const InitLattice &val1,
					 const InitLattice &val2) {
  if (val1.is_top()) return(val2);
  if (val2.is_top()) return(val1);
  if (val1.is_bottom()) return(val1);
  if (val2.is_bottom()) return(val2);
  if (val1.get_key() != val2.get_key())
    return(InitLattice(B_BOTTOM));
  return(val1);
}

bool InitLattice::
do_meet_with_test(const InitLattice &val1) {
  InitLattice val = do_meet(*this, val1);
  if (val == *this)
    return(false);
  *this = val;
  return(true);
}
