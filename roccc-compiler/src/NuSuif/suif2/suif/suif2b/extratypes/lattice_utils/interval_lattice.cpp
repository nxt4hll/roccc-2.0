#include "interval_lattice.h"
#include <suifkernel/suifkernel_messages.h>
  
IntervalLattice::~IntervalLattice() {}
IntervalLattice::IntervalLattice() :
  _key(B_TOP),
  _start_val(), // These must be reset
  _end_val() // these must be reset to use
{
}

IntervalLattice::IntervalLattice(BVal bval) :
  _key(bval),
  _start_val(),
  _end_val()
{
  suif_assert(bval != B_INTERVAL);
  if (bval == B_BOTTOM) {
    _start_val = IInteger::i_negative_inf();
    _end_val =  IInteger::i_positive_inf();
  }
}

IntervalLattice::IntervalLattice(const IntervalLattice &other) :
  _key(other._key),
  _start_val(other._start_val),
  _end_val(other._end_val)
{
  
}

IntervalLattice &IntervalLattice::operator=(const IntervalLattice &other) {
  _key = other._key;
  _start_val = other._start_val;
  _end_val = other._end_val;
  return(*this);
}

IntervalLattice::IntervalLattice(IInteger start_val,
			 IInteger end_val) :
  _key(B_INTERVAL),
  _start_val(start_val),
  _end_val(end_val)
{
  normalize();
}

void IntervalLattice::normalize() {
  if (is_interval()) {
    if (_start_val.is_undetermined() ||
	_end_val.is_undetermined() ||
	_start_val.is_signless_infinity() ||
	_end_val.is_signless_infinity()) 
      {
	_start_val = IInteger::i_negative_inf();
	_end_val =  IInteger::i_positive_inf();
      }
    if (_start_val == IInteger::i_positive_inf() &&
	_end_val == IInteger::i_positive_inf()) {
      _key = B_BOTTOM;
    }
  }
  return;
}

IntervalLattice IntervalLattice::top() {
  return(IntervalLattice(B_TOP));
}

IntervalLattice IntervalLattice::bottom() {
  return(IntervalLattice(B_BOTTOM));
}


bool IntervalLattice::operator==(const IntervalLattice &other) const {
  if (_key != other._key) return false;
  if (_start_val != other._start_val) return false;
  if (_end_val != other._end_val) return false;
  return(true);
}

String IntervalLattice::toString() const {
  if (is_bottom()) return ("<-inf...+inf>");
  //  if (get_is_exact()) return (
  //			      String("[") +
  //			      _start_val.to_String()
  //			      + "..." +
  //			      _end_val.to_String()
  //			      + "]");
  return (
	  String("<") +
	  _start_val.to_String()
	  + "..." +
	  _end_val.to_String()
	  + ">");
}  

bool IntervalLattice::operator!=(const IntervalLattice &other) const {
  return(! (*this == other));
}

bool IntervalLattice::is_bottom() const {
  return(_key == B_BOTTOM);
}
bool IntervalLattice::is_top() const {
  return(_key == B_TOP);
}
bool IntervalLattice::is_interval() const {
  return(_key == B_INTERVAL);
}

IInteger IntervalLattice::get_lower_bound() const {
  return(_start_val);
};

IInteger IntervalLattice::get_upper_bound() const {
  return(_end_val);
};




IntervalLattice IntervalLattice::do_meet(const IntervalLattice &val1,
				 const IntervalLattice &val2) {
  if (val1.is_top()) return(val2);
  if (val2.is_top()) return(val1);
  if (val1.is_bottom()) return(val1);
  if (val2.is_bottom()) return(val2);

  IInteger start_val = val1.get_lower_bound();
  IInteger end_val = val1.get_upper_bound();

  IInteger start2_val = val2.get_lower_bound();
  IInteger end2_val = val2.get_upper_bound();

  if (start2_val < start_val) {
    start_val = start2_val;
  }
  if (end2_val > end_val) {
    end_val = end2_val;
  }
  return(IntervalLattice(start_val, end_val));
}

// WARNING. ORDER COUNTS WITH WIDEN.
IntervalLattice IntervalLattice::do_widen(const IntervalLattice &val1,
				 const IntervalLattice &val2) {
  if (val1.is_top()) return(val2);
  if (val2.is_top()) return(val1);
  if (val1.is_bottom()) return(val1);
  if (val2.is_bottom()) return(val2);
  
  
  IInteger start1_val = val1.get_lower_bound();
  IInteger end1_val = val1.get_upper_bound();

  IInteger start2_val = val1.get_lower_bound();
  IInteger end2_val = val1.get_upper_bound();

  IInteger start_val = start1_val;
  IInteger end_val = end1_val;
  if (start2_val < start_val) {
    start_val = IInteger::i_negative_inf();
  }
  if (end2_val > end_val) {
    end_val = IInteger::i_positive_inf();
  }
  return(IntervalLattice(start_val, end_val));
}


bool IntervalLattice::
do_meet_with_test(const IntervalLattice &val1) {
  IntervalLattice val = do_meet(*this, val1);
  if (val == *this)
    return(false);
  *this = val;
  return(true);
}

bool IntervalLattice::
do_widen_with_test(const IntervalLattice &val1) {
  IntervalLattice val = do_widen(*this, val1);
  if (val == *this)
    return(false);
  *this = val;
  return(true);
}
