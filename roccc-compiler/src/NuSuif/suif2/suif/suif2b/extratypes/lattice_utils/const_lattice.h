#ifndef CONST_LATTICE_H
#define CONST_LATTICE_H

#include "common/i_integer.h"
//#include "nary.h"

/* (c) 2000 David Heine, Stanford SUIF Project
 */

/* IntegerLattice Top | IInteger | Bottom */
class ConstantLattice {
 public:
  enum bval { B_TOP, B_CONST, B_BOTTOM };
  typedef enum bval BVal;
 private:
  BVal _key;
  IInteger _value;
  /* This is the lattice:
     
                  TOP
                 /   \
        -inf .. -1 0 1 ... inf    INITIAL
                 \   /
                 BOTTOM
  */
  /*
   * i_integer also encodes "is_undetermined" and is_infinity
   *   is_constant == false ===> _value == is_undetermined
   * is_infinity is NEVER a valid value for a constant.
   *   it is "bottom"
   */
public:
  ConstantLattice(); /* top */
  ConstantLattice(BVal); /* to set top, bottom or initial */
  ConstantLattice(IInteger value); /* 
				    * is_undetermined => BOTTOM
				    * is_infinity => BOTTOM
				    */
  ConstantLattice(const ConstantLattice &);
  ConstantLattice &operator=(const ConstantLattice &);
  ~ConstantLattice();
  String toString() const;
  bool is_top() const;
  IInteger get_constant() const; /* return is_undetermined for TOP or BOTTOM */
  bool is_bottom() const;
  bool is_const() const;

  static ConstantLattice bottom();
  static ConstantLattice top();

  BVal get_key() const;
  bool operator==(const ConstantLattice &) const;
  bool operator!=(const ConstantLattice &) const;
  static ConstantLattice do_meet(const ConstantLattice &, const ConstantLattice &);
  bool do_meet_with_test(const ConstantLattice &);
};


#endif /* CONST_LATTICE_H */
