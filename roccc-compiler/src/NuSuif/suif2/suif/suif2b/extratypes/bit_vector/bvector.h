#ifndef _bit_vector_bvector_h_
#define _bit_vector_bvector_h_

/** @file
  * This file contains the Bvector class.
  */

#include "bit_vector.h"

/** A Bvector is an ordered sequence of boolean values.
  *
  * The length of this sequence is finite but extendable.
  *
  * A Bvector is a convenient representation of a set, where the true/false
  * value represents membership/non-membership of the corresponding elements.
  * Some of the operations are designed to implement the set operations.
  *
  * While BitVector has infinite (undetermined) length, Bvector has a
  * well-defined length.
  */
class Bvector {
 private:
  size_t     _length;
  BitVector _bits;
  BitVector _mask;  /* of 1's in the first _length bits */

  bool is_zero_bits(const BitVector&) const;

 public:
  /** Constructor.
    * @param length initial length of this bit vector.
    * @param is_full if ture, all bits in this vector will be turn on.
    */
  Bvector(size_t length, bool is_full);

  /** Copy constructor.
    */
  Bvector(const Bvector&);

  /** Check if a bit is on or off.
    * @param n index (0 based) of the bit.
    * @return true if the \a n th bit is on, otherwise false.
    */
  bool get_bit(size_t n) const;

  /** Set a bit in this vector.
    * @param n index (0 based) of the bit to be set.
    * @param v the value to set to the \a n th bit.
    */
  void set_bit(size_t n, bool v);

  /** Get the current length of this vector.
    * @return number of bits in this vector.
    */
  size_t get_length(void) const;

  /** Expand or contract the length of this vector.
    * @param len the new length in number of bits.
    * @param def new value set to the new bits.
    */
  void   set_length(size_t len, bool def = false);

  /** Test for equality of two vectors.
    * Two bit vectors are equal if they have the same bit patterns.
    * @param that the other bit vector.
    * @return true iff this vector is equal to \a that.
    */
  bool is_equal(const Bvector* that) const;

  /** Test if this set is a (proper or improper) subset of the other.
    * @param super the other set.
    * @return true if this set is a subset of \a super.
    */
  bool is_subset(const Bvector* super) const;

  /** Test if this set is a proper subset of the other.
    * @param super the other set.
    * @return true if this set is a proper subset of \a super.
    */
  bool is_proper_subset(const Bvector* super) const;

  /** Test if all bits are off (false).
    * @return true if all bits in this vector are off.
    */
  bool is_all_zero(void) const;

  /** Test if all bits are on (true).
    * @return true iff all bits in this vector are on.
    */
  bool is_all_one(void) const;

  /** Bitwise AND.
    * Bitwise AND of \a b1 and \a b2 and set the result to this vector.
    */
  void set_and(const Bvector*, const Bvector*);

  /** Bitwise OR.
    * Bitwise OR or \a b1 and \a b2 and set the result to this vector.
    */
  void set_or(const Bvector* b1, const Bvector* b2);

  /** Bitwise NOT.
    * Bitwise NOT of \a b and set the result to this vector.
    */
  void set_not(const Bvector* b);

  /** Set minus.
    * Set this vector to the result of (\a b1  - \a b2).
    */
  void set_substract(const Bvector* b1, const Bvector* b2);

  /** Set all bits in this vector.
    * @param v the value to set each bit with.
    */
  void set_all(bool v);

  /** Get a readable string representation of this bit vector.
    * @return a string.
    */
  String to_string(void);
};
			 
#endif  // _bit_vector__bvector_h_
