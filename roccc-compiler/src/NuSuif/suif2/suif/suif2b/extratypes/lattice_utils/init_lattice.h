#ifndef INIT_LATTICE_H
#define INIT_LATTICE_H

#include "common/i_integer.h"

/* (c) 2000 David Heine, Stanford SUIF Project
 */

/* Top
 *  |
 * INIT
 *  |
 * Bottom
 */
class InitLattice {
 public:
  enum bval { B_TOP, B_INITIAL, B_BOTTOM };
  typedef enum bval BVal;
 private:
  BVal _key;
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
  InitLattice(); /* top */
  InitLattice(BVal); /* to set top, bottom or initial */
  InitLattice(const InitLattice &);
  InitLattice &operator=(const InitLattice &);
  ~InitLattice();
  String toString() const;
  bool is_top() const;
  bool is_bottom() const;
  bool is_initial() const;
  static InitLattice bottom();
  static InitLattice top();
  static InitLattice initial();
  BVal get_key() const;
  bool operator==(const InitLattice &) const;
  bool operator!=(const InitLattice &) const;
  static InitLattice do_meet(const InitLattice &, const InitLattice &);
  bool do_meet_with_test(const InitLattice &);
};


#endif /* INIT_LATTICE_H */
