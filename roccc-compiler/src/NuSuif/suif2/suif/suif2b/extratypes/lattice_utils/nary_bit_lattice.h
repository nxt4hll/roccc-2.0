#ifndef NARY_BIT_LATTICE_H
#define NARY_BIT_LATTICE_H

#include "bit_lattice.h"
#include "nary.h"

// These are always inlined so they MUST be visible to 
// instantiate or even declare.
typedef NaryL<UnaryRelation> BinaryRelation;
typedef NaryL<NaryL<UnaryRelation> > TrinaryRelation;

class BinaryRelationBitLattice {
  /* Same lattice as the BitLattice */
  //  SGraphBit _is_bottom;
  //  SGraphBit _value;
  //BitLattice _default_value;
  BinaryRelation _is_bottom;
  BinaryRelation _value;
  BitLattice _default_value;

public:
  BinaryRelationBitLattice(); /* default is top */
  BinaryRelationBitLattice(const BitLattice &default_value); /* default value */
  void set_default_value(const BitLattice &default_value);
  BinaryRelationBitLattice(const BinaryRelationBitLattice &);
  BinaryRelationBitLattice &operator=(const BinaryRelationBitLattice &);
  ~BinaryRelationBitLattice();
  bool operator==(const BinaryRelationBitLattice &) const;
  bool operator!=(const BinaryRelationBitLattice &) const;
  
  BitLattice get_value(size_t domain, size_t range) const;
  void set_value(size_t domain, size_t range, const BitLattice &value);
  //  UnaryRelationBitLattice get_domain_vector(size_t domain) const;
  //  UnaryRelationBitLattice get_range_vector(size_t range) const;
  //  void set_domain_vector(size_t domain, const UnaryRelationBitLattice &value);
  //  void set_range_vector(size_t range, const UnaryRelationBitLattice &value);
  
  //  size_t get_domain_msb() const; /* most significant bit */
  //  size_t get_range_msb() const; /* most significant bit */
  //  size_t get_msb() const; /* max(msb_domain, msb_range) 
  //			   * on relations on the same sets */
  size_t significant_bit_count() const;

  bool do_meet_with_test(const BinaryRelationBitLattice &other);

  static BinaryRelationBitLattice transitive_closure(const BinaryRelationBitLattice &);
  static BinaryRelationBitLattice transpose(const BinaryRelationBitLattice &);
  static BinaryRelationBitLattice do_meet(const BinaryRelationBitLattice &, 
				 const BinaryRelationBitLattice &);
};

class TrinaryRelationBitLattice {
  /* arg0, arg1, arg2 */

  /* Same lattice as the BitLattice */
  TrinaryRelation _is_bottom;
  TrinaryRelation _value;
  BitLattice _default_value;

public:
  TrinaryRelationBitLattice(); /* default is top */
  TrinaryRelationBitLattice(const BitLattice &default_value); /* default value */
  void set_default_value(const BitLattice &default_value);
  TrinaryRelationBitLattice(const TrinaryRelationBitLattice &);
  TrinaryRelationBitLattice &operator=(const TrinaryRelationBitLattice &);
  ~TrinaryRelationBitLattice();
  bool operator==(const TrinaryRelationBitLattice &) const;
  bool operator!=(const TrinaryRelationBitLattice &) const;
  
  BitLattice get_value(size_t arg0, size_t arg1, size_t arg2) const;
  void set_value(size_t arg0, size_t arg1, size_t arg2, 
		 const BitLattice &value);

  //  UnaryRelationBitLattice get_domain_vector(size_t domain) const;
  //  UnaryRelationBitLattice get_range_vector(size_t range) const;
  //  void set_domain_vector(size_t domain, const UnaryRelationBitLattice &value);
  //  void set_range_vector(size_t range, const UnaryRelationBitLattice &value);
  
  //  size_t get_domain_msb() const; /* most significant bit */
  //  size_t get_range_msb() const; /* most significant bit */
  //  size_t get_msb() const; /* max(msb_domain, msb_range) 
  //			   * on relations on the same sets */
  size_t significant_bit_count() const;

  bool do_meet_with_test(const TrinaryRelationBitLattice &other);

  static TrinaryRelationBitLattice transitive_closure(const TrinaryRelationBitLattice &);
  static TrinaryRelationBitLattice do_meet(const TrinaryRelationBitLattice &, 
					   const TrinaryRelationBitLattice &);
};




#endif /* NARY_BIT_LATTICE_H */
