#ifndef INTERVAL_LATTICE_H
#define INTERVAL_LATTICE_H

#include <common/i_integer.h>

/* 
 * IntervalLattice
 *  This is an interval lattice.   It does NOT have finite descending
 * chains
 *  It is expected to be used by value.
 */
class IntervalLattice {
 public:
  enum bval { B_TOP, B_INTERVAL, B_BOTTOM };
  typedef enum bval BVal;
 private:
  BVal _key;

  IInteger _start_val; /* for bottom, this will be neg_inf() */
  IInteger _end_val; /* for top, this will be pos_inf() */
public:
  IntervalLattice(); /* TOP */
  IntervalLattice(BVal); /* For Top or Bottom */
  // If start offset is the same as the end offset
  // This is an integer
  IntervalLattice(IInteger start_val,
		  IInteger end_val);
  IntervalLattice(const IntervalLattice &other);
  IntervalLattice &operator=(const IntervalLattice &other);
  bool operator==(const IntervalLattice &other) const;
  bool operator!=(const IntervalLattice &other) const;

  void normalize();  // If the user passes a signless_inf or undetermined
  // value, then it is BOTTOM.

  ~IntervalLattice();
  String toString() const;
  //  ObjectContext *get_object_context() const;

  bool is_top() const;
  bool is_bottom() const;
  bool is_interval() const;

  static IntervalLattice top();
  static IntervalLattice bottom();

  IInteger get_lower_bound() const;
  IInteger get_upper_bound() const;
  

  static IntervalLattice do_meet(const IntervalLattice &v1, 
			     const IntervalLattice &v2);
  static IntervalLattice do_widen(const IntervalLattice &v1, 
			     const IntervalLattice &v2);
  bool do_meet_with_test(const IntervalLattice &other);
  // Widen will force upper bounds to Inf or
  // lower bounds to -inf
  bool do_widen_with_test(const IntervalLattice &other);
};



#endif /* INTERVAL_LATTICE_H */
