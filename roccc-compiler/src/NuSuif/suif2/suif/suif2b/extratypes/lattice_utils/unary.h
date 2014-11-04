#ifndef LATTICE_UTILS_UNARY_H
#define LATTICE_UTILS_UNARY_H

#include <bit_vector/bit_vector.h>
class UnaryRelation {
  BitVector _value;
 public:
  UnaryRelation(); /* default is 0 */
  UnaryRelation(const UnaryRelation &other);
  UnaryRelation(bool default_value);
  static UnaryRelation do_negate(const UnaryRelation &val);
  UnaryRelation &operator =(const UnaryRelation &other);
  bool operator ==(const UnaryRelation &other) const;
  bool operator !=(const UnaryRelation &other) const;


  bool get_value(size_t i) const;
  void set_value(size_t i, bool value);

  bool is_empty() const;
  size_t size() const; /* for this dimension */
  void set_default_value(bool default_value);
  size_t significant_bit_count() const;
  bool do_meet_with_test(const UnaryRelation &other);
  static UnaryRelation do_meet(const UnaryRelation &val1, const UnaryRelation &val2);
};

#endif //LATTICE_UTILS_UNARY_H
