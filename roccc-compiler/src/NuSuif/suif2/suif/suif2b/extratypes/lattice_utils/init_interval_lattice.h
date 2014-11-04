#ifndef INIT_INTERVAL_LATTICE_H
#define INIT_INTERVAL_LATTICE_H

#include "interval_lattice.h"

/* 
 * InitIntervalVal
 *  This is an interval lattice that handles initial values
 *   It does NOT have finite descending but has a widen
 *   operation that will force convergence. 
 * The init value is the SMASH product of
 *   INIT + IntervalLattice, IntervalLattice
 *
 *   with TOP and bottom
 *
 * This is useful in region-based analysis
 */
class InitIntervalVal {
  enum bval { B_TOP, B_CONST, B_INITIAL, B_BOTTOM };
  typedef enum bval BVal;

  bool _is_top;
  bool _is_bottom;
  enum 

  IntervalLattice _init_lattice;
  IntervalLattice _const_lattice;
public:
  IntervalVal(bool is_exact,
	      IInteger start_offset,
	      IInteger end_offset);
  IntervalVal(); /* bottom offset (i.e. any offset, inexact) */
  IntervalVal(const IntervalVal &other);
  IntervalVal &operator=(const IntervalVal &other);
  bool operator==(const IntervalVal &other) const;
  bool operator!=(const IntervalVal &other) const;

  ~IntervalVal();
  String toString() const;
  //  ObjectContext *get_object_context() const;

  bool is_top() const;
  bool get_is_exact() const;
  // Computed
  bool is_bottom() const;
  IInteger get_start_offset() const;
  IInteger get_end_offset() const;
  

  static IntervalVal do_meet(const IntervalVal &v1, 
			     const IntervalVal &v2);
  static IntervalVal do_widen(const IntervalVal &v1, 
			     const IntervalVal &v2);
  bool do_meet_with_test(const IntervalVal &other);
  // Widen will force upper bounds to Inf or
  // lower bounds to -inf
  bool do_widen_with_test(const IntervalVal &other);
};



#endif /* INTERVAL_LATTICE_H */
