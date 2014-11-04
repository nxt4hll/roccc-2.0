#ifndef BIT_LATTICE_H
#define BIT_LATTICE_H

//#include "nary.h"

/* (c) 2000 David Heine, Stanford SUIF Project
 */

#include <common/MString.h>
#include "unary.h"

/* This is a value class representing a single bit */
class BitLattice {
  bool _is_bottom; /*  */
  bool _value;  /* if is_bottom==value,  1==bottom, 0== top
		   if is_bottom!=value,  0=false, 1=true
		*/
  /* This is the lattice:
     
            top  (00)
           /   \
   (01) true   false (10)
           \   /
          bottom (11)
  */
public:
  enum bval { B_TOP, B_TRUE, B_FALSE, B_BOTTOM };
  typedef enum bval BVal;
public:
  BitLattice(); /* top */
  BitLattice(BVal);
  BitLattice(const BitLattice &);
  BitLattice &operator=(const BitLattice &);
  BitLattice(bool value); /* true or false */
  BitLattice(bool is_bottom, bool value);

  ~BitLattice();
  String toString() const;
  bool is_top() const;
  bool is_true() const;
  bool is_false() const;
  bool is_bottom() const;

  bool is_definite() const; /* true or false? */

  bool get_bottom_bit() const;
  bool get_value_bit() const;

  BVal get_value() const;
  bool operator==(const BitLattice &) const;
  bool operator!=(const BitLattice &) const;
  
  /* meet in place. return true if changed */
  bool do_and_with_test(const BitLattice &other);
  bool do_or_with_test(const BitLattice &other);
  bool do_not_with_test();
  bool do_meet_with_test(const BitLattice &other);
  
  
  /* Meet into another BitLattice */
  static BitLattice do_meet(const BitLattice &, const BitLattice &);
  static BitLattice do_and(const BitLattice &, const BitLattice &);
  static BitLattice do_or(const BitLattice &, const BitLattice &);
  static BitLattice do_not(const BitLattice &);
};

typedef BitLattice (*unary_bit_transform) (const BitLattice &);
typedef BitLattice (*binary_bit_transform) (const BitLattice &, const BitLattice &);

/* 
 * The default constructor builds a bit vector
 * with all values "top"
 * Initial implementation does not reference count
 * These should be reference counted and
 * 
 */

class UnaryRelationBitLattice {
  /* Same lattice as the BitLattice */
  UnaryRelation _is_bottom;
  UnaryRelation _value;
  BitLattice _default_value;
public:
  UnaryRelationBitLattice(); /* default is top */
  UnaryRelationBitLattice(const UnaryRelationBitLattice &other);
  UnaryRelationBitLattice(const BitLattice &default_value); /* default value */
  UnaryRelationBitLattice &operator=(const UnaryRelationBitLattice &other);
  ~UnaryRelationBitLattice();

  static UnaryRelationBitLattice map_unary(unary_bit_transform fn,
					   const UnaryRelationBitLattice &val);
  static UnaryRelationBitLattice map_binary(binary_bit_transform, 
		  const UnaryRelationBitLattice &val1,
		  const UnaryRelationBitLattice &val2);
  bool map_unary_with_test(unary_bit_transform fn);
  bool map_binary_with_test(binary_bit_transform fn, 
			    const UnaryRelationBitLattice &val);

  
  bool operator==(const UnaryRelationBitLattice &) const;
  bool operator!=(const UnaryRelationBitLattice &) const;
  BitLattice get_value(size_t i) const;
  void set_value(size_t i, const BitLattice &value);
  void set_default_value(const BitLattice &value);
  size_t significant_bit_count() const; /* most significant bit */

  bool do_or_with_test(const UnaryRelationBitLattice &other);
  bool do_and_with_test(const UnaryRelationBitLattice &other);
  bool do_not_with_test();
  bool do_meet_with_test(const UnaryRelationBitLattice &other);
  
  static UnaryRelationBitLattice do_or(const UnaryRelationBitLattice &,
					 const UnaryRelationBitLattice &);
  static UnaryRelationBitLattice do_and(const UnaryRelationBitLattice &,
					 const UnaryRelationBitLattice &);
  static UnaryRelationBitLattice do_not(const UnaryRelationBitLattice &);
  static UnaryRelationBitLattice do_meet(const UnaryRelationBitLattice &,
					 const UnaryRelationBitLattice &);

};


//typedef NaryL<UnaryRelation> BinaryRelation;
//typedef NaryL<NaryL<UnaryRelation> > TrinaryRelation;





#endif /* BIT_LATTICE_H */
