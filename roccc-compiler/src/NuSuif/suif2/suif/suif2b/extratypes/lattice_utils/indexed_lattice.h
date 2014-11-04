#ifndef INDEXED_LATTICE_H
#define INDEXED_LATTICE_H

#include <bit_vector/cross_map.h>
#include <common/MString.h>
#include "nary.h"

// This contains a map from variable->lattice_value
// All this does is look up
// It should just become a class parameterized by the
// index.
//
template <class IndexObj, class SubLattice>
class IndexedLattice {

  CrossMap<IndexObj> *_map; // shared with many other lattice values
  NaryL<SubLattice> _values;
public:
  IndexedLattice(CrossMap<IndexObj> *map, SubLattice def);
  void set_value(IndexObj v, SubLattice val);
  SubLattice get_value(IndexObj v) const;
  SubLattice get_default_value() const;
  CrossMap<IndexObj > *get_map() const { return(_map); }

  bool do_meet_with_test(const IndexedLattice &other);
  IndexedLattice *clone() const;

  // To string must be passed functions that convert
#if 0 
  // egcs 1.1.2 bug prevents this
  String to_string( String (*index_to_string) (const IndexObj &),
		    String (*lattice_to_string) (const SubLattice &),
		    String delimiter) const;
#endif

  class iterator {
    const IndexedLattice *_map;
    typename CrossMap<IndexObj>::iterator _iter;
  public:
    iterator(const IndexedLattice *map) :
      _map(map),
      _iter(map->_map)
    {}
    iterator(const iterator &other) :
      _map(other._map),
      _iter(other._iter)
      {}
    iterator &operator=(const iterator &other) {
      _map = other._map;
      _iter = other._iter;
      return(*this);
    }
    IndexObj current() const {
      return (_iter.current());
    }
    SubLattice current_val() const {
      return (_map.get_value(current()));
    }
    bool is_valid() const {
      return (_iter.is_valid());
    }
    void next() { _iter.next(); }
    void reset() { _iter.reset(); }
  };
  friend class iterator;

};

// Here are the definitions

template <class IndexObj, class SubLattice>
IndexedLattice<IndexObj, SubLattice>::
IndexedLattice(CrossMap<IndexObj> *map, SubLattice def) :
  _map(map), _values(def)
{}

template <class IndexObj, class SubLattice>
void IndexedLattice<IndexObj, SubLattice>::
set_value(IndexObj v, SubLattice val) {
  size_t i = _map->retrieve_id(v);
  _values.set_value(i, val);
}

template <class IndexObj, class SubLattice>
SubLattice IndexedLattice<IndexObj, SubLattice>::
get_value(IndexObj v) const {
  if (!_map->is_member(v))
    return(get_default_value());
  size_t i = _map->lookup_id(v);
  return(_values.get_value(i));
}

template <class IndexObj, class SubLattice>
SubLattice IndexedLattice<IndexObj, SubLattice>::
get_default_value() const {
  return(_values.get_default_value());
}

template <class IndexObj, class SubLattice>
bool IndexedLattice<IndexObj, SubLattice>::
do_meet_with_test(const IndexedLattice &other) {
  suif_assert_message(other.get_map() == get_map(),
		      ("Can not yet meet IndexedLattice with different object sets"));
  return(_values.do_meet_with_test(other._values));
}

template <class IndexObj, class SubLattice>
IndexedLattice<IndexObj,SubLattice> *IndexedLattice<IndexObj, SubLattice>::
clone() const {
  IndexedLattice *new_l =
    new IndexedLattice(_map, _values.get_default_value());
  new_l->_values = _values;
  return(new_l);
}

/* This does NOT work in egcs 1.1.2  because of template problems */
#if 0
template <class IndexObj, class SubLattice>
String IndexedLattice<IndexObj, SubLattice>::
to_string(
	  String (*index_to_string)(const IndexObj &),
	  String (*lattice_to_string)(const SubLattice &),
	  String delimiter) const {
  String s;
  // Print the default value first
  s += "default: ";
  s += lattice_to_string(get_default_value());
  s += delimiter;

  for ( IndexedLattice<IndexObj,SubLattice>::iterator iter(this);
	iter.is_valid(); iter.next()) {
    IndexObj *idx = iter.current();
    s += index_to_string(idx);
    s += ": ";
    SubLattice l = get_value(idx);
    s += lattice_to_string(l);
    s += delimiter;
  }
  return(s);
};
#endif 


#endif /* INDEXED_LATTICE_H */
