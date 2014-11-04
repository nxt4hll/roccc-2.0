#ifndef BITVECTOR_SET_H
#define BITVECTOR_SET_H

#include "cross_map.h"
#include "bit_vector.h"


template <class T>
class BitVectorSet {

  CrossMap<T>* const _map;
  BitVector          _bits;
  
public:
  BitVectorSet(CrossMap<T> *map) : _map(map),  _bits() {}

  BitVectorSet(const BitVectorSet &other) : 
    _map(other._map),
    _bits(other._bits) 
    {}

  CrossMap<T>* get_universe(void) const { return _map; };

  void operator=(const BitVectorSet &other) {
    kernel_assert(other._map == _map);
    _bits = other._bits;
  }

  bool is_member(T v) const {
    if (_map->is_member(v)) {
      size_t val = _map->lookup_id(v);
      return(_bits.get_bit(val));
    }
    return(false);
  }
  void add_member(T v) {
    size_t val = _map->retrieve_id(v);
    _bits.set_bit(val, true);
  }

  void remove_member(T v) {
    size_t val = _map->retrieve_id(v);
    _bits.set_bit(val, false);
  }


  bool operator==(const BitVectorSet &other) const {
    kernel_assert(other._map == _map);
    return(other._bits == _bits);
  }
  bool operator!=(const BitVectorSet &other) const {
    kernel_assert(other._map == _map);
    return(!is_equal_to(other));
  }
  
  bool is_proper_subset(const BitVectorSet &other) const {
    kernel_assert(other._map == _map);
    return(_bits.is_less_than(other));
  }
  bool is_proper_superset(const BitVectorSet &other) const {
    kernel_assert(other._map == _map);
    return(_bits.is_greater_than(other));
  }
  bool is_subset(const BitVectorSet &other) const {
    kernel_assert(other._map == _map);
    return(_bits.is_less_than_or_equal_to(other._bits));
  }
  bool is_superset(const BitVectorSet &other) const {
    kernel_assert(other._map == _map);
    return(_bits.is_greater_than_or_equal_to(other._bits));
  }
  //BitVectorSet invert(void) const {
    // this changes the default
  //    }

    /* The following two operations are identical to operator&=() and
     * operator|=() respectively except that they also return a
     * bool value which is true if and only if this bit vector was
     * changed by the operation. */
  
  BitVectorSet *clone() const {
    return(new BitVectorSet(*this));
  }

  void do_intersect(const BitVectorSet &other) {
    kernel_assert(other._map == _map);
    _bits &= other._bits;
  }

  void do_union(const BitVectorSet &other) {
    kernel_assert(other._map == _map);
    _bits |= other._bits;
  }

  void do_complement(void) {
    BitVector ones;
    ones.set_to_ones();
    _bits ^= ones;
  }

  bool do_intersect_with_test(const BitVectorSet &other) {
    kernel_assert(other._map == _map);
    return(_bits.do_and_with_test(other._bits));
  }
  bool do_union_with_test(const BitVectorSet &other) {
    kernel_assert(other._map == _map);
    return(_bits.do_or_with_test(other._bits));
  }
  bool do_subtract_with_test(const BitVectorSet &other) {
    kernel_assert(other._map == _map);
    return(_bits.do_subtract_with_test(other._bits));
  }
    
  size_t count() const {
    return(_bits.count());
  }

  
  class iterator {
    const BitVectorSet* const _set;
    CrossMap<T>::iterator     _iter;
    T                         _next;
  public:
    iterator(const BitVectorSet *set) :
      _set(set),
      _iter(set->_map),
      _next(_iter.current())
    {
      while (_iter.is_valid()) {
	_next = _iter.current();
	if (_set->is_member(_next)) return;
	_iter.next();
      }
    }

    T current() const {
      return(_next);
    }

    bool is_valid() const {
      return(_iter.is_valid());
    }

    void next() {
      while (_iter.is_valid()) {
	_iter.next();
	_next = _iter.current();
	if (_set->is_member(_next)) return;
      }
    }

    void reset() { _iter.reset(); }
  };

  friend class BitVectorSet::iterator;

};

#endif /* BITVECTOR_SET_H */
