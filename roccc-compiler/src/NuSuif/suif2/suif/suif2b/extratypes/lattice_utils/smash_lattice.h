#ifndef SMASH_LATTICE_H
#define SMASH_LATTICE_H

/* (c) 2000 David Heine, Stanford SUIF Project
 */

#include <common/MString.h>

/* This is the SMASH product of two lattices
 * each lattice MUST have a Class::B_TOP and Class::B_BOTTOM
 */
template <class Lattice1, class Lattice2>
class SmashLattice {
 public:
  enum bval { B_TOP, B_MIDDLE, B_BOTTOM };
  typedef enum bval BVal;
 private:
  BVal _key;
  Lattice1 _value1;
  Lattice2 _value2;
  /* This is a product lattice where the top and bottoms
   * correspond
   *
   *
   *  i.e SmashLattice(IntegerLattice, InitLattice)
   *
   *     
                  TOP
                 /   \
        -inf .. -1 0 1 ... inf 
                 \   /
                 BOTTOM


                  TOP
                   |
                 INITIAL
                   |
                 BOTTOM

                  TOP
                 /   \
              CONST  INITIAL
	         \   /
              CONST,INITIAL
	           |
	         BOTTOM
  */
public:
  SmashLattice(); // This is ALWAYS top.
  SmashLattice(BVal); // This is only valid for TOP or BOTTOM
  SmashLattice(Lattice1 value1, Lattice2 value2); // This specifies others
  SmashLattice(const SmashLattice &other);
  SmashLattice &operator=(const SmashLattice &);
  ~SmashLattice();
  String toString() const;

  static SmashLattice top();
  static SmashLattice bottom();

  bool is_top() const;
  bool is_bottom() const;
  Lattice1 get_value1() const;
  Lattice2 get_value2() const;
  BVal get_key() const;
  bool operator==(const SmashLattice &) const;
  bool operator!=(const SmashLattice &) const;
  // We may need to parameterize the meet/widen operation
  // later.
  static SmashLattice do_meet(const SmashLattice &, const SmashLattice &);
  bool do_meet_with_test(const SmashLattice &);
  static SmashLattice do_widen(const SmashLattice &, const SmashLattice &);
  bool do_widen_with_test(const SmashLattice &);
};

/*
 * Smash Lattice Implementations
 */

template <class Lattice1, class Lattice2>
SmashLattice<Lattice1, Lattice2>::SmashLattice() :
  _key(B_TOP),
  _value1(),
  _value2()
{
}

template <class Lattice1, class Lattice2>
SmashLattice<Lattice1, Lattice2>::SmashLattice(BVal bval) :
  _key(bval),
  _value1(),
  _value2()
{
  suif_assert(bval == B_BOTTOM
	      || bval == B_TOP);
  if (bval == B_BOTTOM)
    _value1 = Lattice1(Lattice1::B_BOTTOM);
}

template <class Lattice1, class Lattice2>
SmashLattice<Lattice1, Lattice2>::SmashLattice(Lattice1 value1, Lattice2 value2) :
  _key(B_MIDDLE),
  _value1(value1),
  _value2(value2)
{
  if (_value1.is_top() && _value2.is_top())
    _key = B_TOP;
  if (_value1.is_bottom() && _value2.is_bottom())
    _key = B_BOTTOM;
}

template <class Lattice1, class Lattice2>
SmashLattice<Lattice1, Lattice2>::SmashLattice(const SmashLattice &other) :
  _key(other._key),
  _value1(other._value1),
  _value2(other._value2)
{
}

template <class Lattice1, class Lattice2>
SmashLattice<Lattice1, Lattice2> &SmashLattice<Lattice1, Lattice2>::operator=(const SmashLattice &other) {
  _key = other._key;
  _value1 = other._value1;
  _value2 = other._value2;
  return(*this);
}

template <class Lattice1, class Lattice2>
SmashLattice<Lattice1, Lattice2>::~SmashLattice() {}

template <class Lattice1, class Lattice2>
String SmashLattice<Lattice1, Lattice2>::toString() const {
  if (is_top()) return ("TOP");
  if (is_bottom()) return ("BOTTOM");
  String s = String("{") + _value1.toString() + ", " +
    _value2.toString() + "}";
  return(s);
}

template <class Lattice1, class Lattice2>
bool SmashLattice<Lattice1, Lattice2>::is_top() const {
  return(_key == B_TOP);
}
template <class Lattice1, class Lattice2>
bool SmashLattice<Lattice1, Lattice2>::is_bottom() const {
  return(_key == B_BOTTOM);
}

template <class Lattice1, class Lattice2>
Lattice1 SmashLattice<Lattice1, Lattice2>::get_value1() const 
{
  return(_value1);
}

template <class Lattice1, class Lattice2>
Lattice2 SmashLattice<Lattice1, Lattice2>::get_value2() const {
  return(_value2);
}

template <class Lattice1, class Lattice2>
typename SmashLattice<Lattice1, Lattice2>::BVal SmashLattice<Lattice1, Lattice2>::get_key() const {
  return(_key);
}

template <class Lattice1, class Lattice2>
SmashLattice<Lattice1, Lattice2> SmashLattice<Lattice1, Lattice2>::bottom() {
  return(SmashLattice(B_BOTTOM));
}
template <class Lattice1, class Lattice2>
SmashLattice<Lattice1, Lattice2> SmashLattice<Lattice1, Lattice2>::top() {
  return(SmashLattice(B_TOP));
}


template <class Lattice1, class Lattice2>
bool SmashLattice<Lattice1, Lattice2>::operator==(const SmashLattice &other) const {
  return(_value1 == other._value1 &&
	 _value2 == other._value2);
}
template <class Lattice1, class Lattice2>
bool SmashLattice<Lattice1, Lattice2>::operator!=(const SmashLattice &other) const {
  return(! ((*this) == other));
}

template <class Lattice1, class Lattice2>
SmashLattice<Lattice1, Lattice2> SmashLattice<Lattice1, Lattice2>::
do_meet(const SmashLattice &val1, 
	const SmashLattice &val2) {
  return(SmashLattice(Lattice1::do_meet(val1._value1,
					val2._value1),
		      Lattice2::do_meet(val1._value2,
					val2._value2)));
}
template <class Lattice1, class Lattice2>
SmashLattice<Lattice1, Lattice2> SmashLattice<Lattice1, Lattice2>::
do_widen(const SmashLattice &val1, 
	 const SmashLattice &val2) {
  return(SmashLattice(Lattice1::do_widen(val1._value1,
					 val2._value1),
		      Lattice2::do_widen(val1._value2,
					 val2._value2)));
}

template <class Lattice1, class Lattice2>
bool SmashLattice<Lattice1, Lattice2>::do_meet_with_test(const SmashLattice &other) {
  SmashLattice result = do_meet(*this, other);
  if (result == *this) return(false);
  *this = result;
  return(true);
}

template <class Lattice1, class Lattice2>
bool SmashLattice<Lattice1, Lattice2>::do_widen_with_test(const SmashLattice &other) {
  SmashLattice result = do_widen(*this, other);
  if (result == *this) return(false);
  *this = result;
  return(true);
}



#endif /* SMASH_LATTICE_H */
