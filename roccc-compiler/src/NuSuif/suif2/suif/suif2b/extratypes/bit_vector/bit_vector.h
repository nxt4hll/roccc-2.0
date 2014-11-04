/* file "bit_vector.h" */


/*
       Copyright (c) 1997 Stanford University

       All rights reserved.

       This software is provided under the terms described in
       the "suif_copyright.h" include file.
*/

//#include <suif_copyright.h>


#ifndef STY_BIT_VECTOR_H
#define STY_BIT_VECTOR_H

//#ifndef SUPPRESS_PRAGMA_INTERFACE
//#pragma interface
//#endif


/*
      This is the definition of the bit_vector class for sty, the
      first-level main library of the SUIF system.
*/
#include <stddef.h>
#include <common/MString.h>
#include <common/i_integer.h>
#include <ion/ion.h>

class SuifEnv;
class BitVectorBlock;

extern "C" void init_bit_vector(SuifEnv *suif_env);

// There is a unique one-to-one mapping from 
// a bitvector to an IInteger.
// When the infinity_bit is 0, the integer is the
//   binary digit where the 0th bit is the least significant.
// When the infinity_bit is 1, the IInteger is negative.
//    The interpretation is value of the 2s complement-form
//    bit pattern
// 

class BitVector
  {
    friend void init_bit_vector(SuifEnv *suif_env);

private:
    static BitVectorBlock *_zero_block;
    static BitVectorBlock *_ones_block;

    BitVectorBlock *_block;
    size_t _count; // cache the count MUTABLE
    bool _has_count; // count cached - MUTABLE

    static void do_initialization(void);

    void add_reference(BitVectorBlock *the_block);
    void remove_reference(BitVectorBlock *the_block);
    void make_changable(void);

    static unsigned char convert_from_hex(char hex_char);

public:
    typedef unsigned long ChunkT;

    BitVector(void);
    BitVector(const BitVector &other);
    ~BitVector(void);

    bool get_bit(size_t bit_num) const;
    bool get_infinity_bit(void) const; // i.e. sign bit
    size_t num_significant_bits(void) const; // number of highest bit
    size_t get_chunk_count() const;
    ChunkT get_chunk(size_t chunk_num) const;

    void set_bit(size_t bit_num, bool new_value);
    void set_chunk(size_t chunk_num, ChunkT new_chunk);
    void set_to_zero(void);
    void set_to_ones(void);

    void operator=(const BitVector &other);

    String to_string(void) const;
    void from_string(String new_value);

    IInteger to_i_integer(void) const;
    void from_i_integer(IInteger new_value);

    size_t written_length(void) const;
    void write(char *location) const;

    void read(const char *location);

    void print(FILE *fp = stdout) const;
    void print(ion *the_ion) const;

    bool is_equal_to(const BitVector &other) const;
    bool is_not_equal_to(const BitVector &other) const
      { return !is_equal_to(other); }
    bool is_less_than(const BitVector &other) const;
    bool is_greater_than(const BitVector &other) const
      { return other.is_less_than(*this); }
    bool is_less_than_or_equal_to(const BitVector &other) const
      { return !is_greater_than(other); }
    bool is_greater_than_or_equal_to(const BitVector &other) const
      { return !is_less_than(other); }

    BitVector invert(void) const;

    bool operator==(const BitVector &other) const
      { return is_equal_to(other); }
    bool operator!=(const BitVector &other) const
      { return !(*this == other); }
    bool operator<(const BitVector &other) const
      { return is_less_than(other); }
    bool operator>(const BitVector &other) const
      { return (other < *this); }
    bool operator<=(const BitVector &other) const
      { return !(*this > other); }
    bool operator>=(const BitVector &other) const
      { return !(*this < other); }

    BitVector operator^(const BitVector &other) const;
    BitVector operator&(const BitVector &other) const;
    BitVector operator|(const BitVector &other) const;
    BitVector operator~(void) const;
    BitVector operator<<(size_t shift_amount) const;
    BitVector operator>>(size_t shift_amount) const;

    BitVector operator-(const BitVector &other) const;

    void subtract(const BitVector &other);

    bool operator!(void) const;

    void operator^=(const BitVector &other);
    void operator&=(const BitVector &other);
    void operator|=(const BitVector &other);
    void operator>>=(size_t shift_amount);
    void operator<<=(size_t shift_amount);

    /* The following two operations are identical to operator&=() and
     * operator|=() respectively except that they also return a
     * bool value which is true if and only if this bit vector was
     * changed by the operation. */
    bool do_and_with_test(const BitVector &other);
    bool do_or_with_test(const BitVector &other);
    bool do_subtract_with_test(const BitVector &other);

    size_t count() const;
  };

// when the sign bit is 0, iterate over the 1s
// when the sign bit is 1, iterate over the 0s
// 
class BitVectorIter {
public:
  BitVectorIter(const BitVector *bv);
  BitVectorIter(const BitVectorIter &other);
  BitVectorIter &operator=(const BitVectorIter &other);
  // support the old-style increment(), done()
  // and the new-style is_valid(); next()
  bool is_valid() const;
  void next();
  size_t current() const;
  void first(); // reset

  void increment(); // next()
  bool done() const; // !is_valid()
  size_t get() const; // current()
  void reset(); // first()

private:
  bool _done;
  //  unsigned _current_chunk;
  //  unsigned _current_byte;
  size_t _current_bit; // this is the current bit number.
  unsigned char _remaining_byte; // the value remaining for the byte
  // the current bit is in.  The current bit has been removed from this.
  // also it is the negation when the ones bit of the bitvector is 1.

  //  size_t _current;
  const BitVector *_bv;
  //  bool _sign_bit;
};
  

#endif /* STY_BIT_VECTOR_H */
