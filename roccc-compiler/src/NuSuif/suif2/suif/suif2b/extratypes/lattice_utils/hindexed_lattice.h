#ifndef HINDEXED_LATTICE_H
#define HINDEXED_LATTICE_H

// This is a lattice indexed ONLY by a hash function
// on the IndexObj

#include <common/suif_hash_map.h>
#include "lattice_utils_forwarders.h"

// This contains a map from variable->lattice_value
// All this does is look up
// It should just become a class parameterized by the
// index.
//
template <class IndexObj, class SubLattice>
class HIndexedLattice {

  typedef suif_hash_map<IndexObj, SubLattice> VMap;
  typedef typename VMap::const_iterator ConstVIter;
  typedef typename VMap::iterator VIter;
  VMap *_values;
  SubLattice _default_value;
public:
  HIndexedLattice(SubLattice def);
  HIndexedLattice(const HIndexedLattice &other);
  HIndexedLattice &operator=(const HIndexedLattice &other);
  ~HIndexedLattice();
  void set_value(IndexObj v, SubLattice val);
  SubLattice get_value(IndexObj v) const;
  SubLattice get_default_value() const;

  bool do_meet_with_test(const HIndexedLattice &other);
  HIndexedLattice *clone() const;

  // pass in a function pointer to a function that
  // works on one
  bool do_apply_fn_with_test(const HIndexedLattice &other,
			     bool(*fn) (SubLattice &, const SubLattice &));

  // To string must be passed functions that convert
#if 0 
  // egcs 1.1.2 bug prevents this
  String to_string( String (*index_to_string) (const IndexObj &),
		    String (*lattice_to_string) (const SubLattice &),
		    String delimiter) const;
#endif

  class iterator {
    suif_hash_map<IndexObj,SubLattice> *_map;
    typename suif_hash_map<IndexObj,SubLattice>::iterator _iter;
    typename suif_hash_map<IndexObj,SubLattice>::iterator _iter_end;
  public:
    iterator(const HIndexedLattice *map) :
      _map(map->_values),
      _iter(map->_values->begin()),
      _iter_end(map->_values->end())
    {}
    iterator(const iterator &other) :
      _map(other._map),
      _iter(other._iter),
      _iter_end(other._iter_end)
      {}
    iterator &operator=(const iterator &other) {
      _map = other._map;
      _iter = other._iter;
      _iter_end = other._iter_end;
      return(*this);
    }
    IndexObj current() const {
      return ((*_iter).first);
    }
    SubLattice current_val() const {
      return ((*_iter).second);
    }
    bool is_valid() const {
      return (_iter != _iter_end);
    }
    void next() { _iter++; }
    void reset() { 
      _iter = _map->_values->begin();
      _iter_end = _map->_values->end();
    }
  };
  friend class iterator;

};

// Here are the definitions

template <class IndexObj, class SubLattice>
HIndexedLattice<IndexObj, SubLattice>::
HIndexedLattice(SubLattice def) :
  _values( new suif_hash_map<IndexObj, SubLattice>()),
  _default_value(def)
{}

template <class IndexObj, class SubLattice>
HIndexedLattice<IndexObj, SubLattice>::
HIndexedLattice(const HIndexedLattice<IndexObj, SubLattice> &other) :
  _values( new suif_hash_map<IndexObj, SubLattice>()),
  _default_value(other._default_value)
{
  for (VIter iter =
	 other._values->begin(); iter != other._values->end(); iter++) {
    _values->enter_value((*iter).first, (*iter).second);
  }
}

template <class IndexObj, class SubLattice>
HIndexedLattice<IndexObj, SubLattice> &
HIndexedLattice<IndexObj, SubLattice>::
operator=(const HIndexedLattice<IndexObj, SubLattice> &other) {
  _values->clear();
  for (VIter iter =
	 other._values->begin(); iter != other._values->end(); iter++) {
    _values->enter_value((*iter).first, (*iter).second);
  }
  _default_value = other._default_value;
  return(*this);
}



template <class IndexObj, class SubLattice>
HIndexedLattice<IndexObj, SubLattice>::
~HIndexedLattice() {
  delete _values;
}

template <class IndexObj, class SubLattice>
void HIndexedLattice<IndexObj, SubLattice>::
set_value(IndexObj v, SubLattice val) {
  _values->enter_value(v, val);
}

template <class IndexObj, class SubLattice>
SubLattice HIndexedLattice<IndexObj, SubLattice>::
get_value(IndexObj v) const {
  typename suif_hash_map<IndexObj, SubLattice>::const_iterator iter =
    _values->find(v);
  if (iter == _values->end())
    return(get_default_value());
  return((*iter).second);
}

template <class IndexObj, class SubLattice>
SubLattice HIndexedLattice<IndexObj, SubLattice>::
get_default_value() const {
  return(_default_value);
}

template <class IndexObj, class SubLattice>
bool HIndexedLattice<IndexObj, SubLattice>::
do_meet_with_test(const HIndexedLattice &other) {
  // This is just do_apply_fn with test but the FN is
  // meet.
  
  bool changed = false;
  // Make a list of all of the ones that are not default
  suif_hash_map<IndexObj, bool> v;
  for (iterator iter(this); iter.is_valid(); iter.next()) {
    v.enter_value(iter.current(), true);
  }
  for (iterator iter(&other); iter.is_valid(); iter.next()) {
    v.enter_value(iter.current(), true);
  }

  // Now walk over that list and apply for each
  for (typename suif_hash_map<IndexObj, bool>::iterator it =
	 v.begin(); it != v.end(); it++) {
    IndexObj obj = (*it).first;
    SubLattice v1 = get_value(obj);
    SubLattice v2 = other.get_value(obj);
    // This is the function
    if (v1.do_meet_with_test(v2)) {
      changed = true;
      set_value(obj, v1);
    }
  }
  return(changed);
}

template <class IndexObj, class SubLattice>
HIndexedLattice<IndexObj,SubLattice> *HIndexedLattice<IndexObj, SubLattice>::
clone() const {
  HIndexedLattice *new_l =
    new HIndexedLattice(*this);
  return(new_l);
}

template <class IndexObj, class SubLattice>
bool HIndexedLattice<IndexObj, SubLattice>::
do_apply_fn_with_test(const HIndexedLattice &other,
		      bool(*fn) (SubLattice &, const SubLattice &)) {
  bool changed = false;
  // Make a list of all of the ones that are not default
  suif_hash_map<IndexObj, bool> v;
  for (iterator iter(this); iter.is_valid(); iter.next()) {
    v.enter_value(iter.current(), true);
  }
  for (iterator iter(&other); iter.is_valid(); iter.next()) {
    v.enter_value(iter.current(), true);
  }

  // Now walk over that list and apply for each
  for (typename suif_hash_map<IndexObj, bool>::iterator it =
	 v.begin(); it != v.end(); it++) {
    IndexObj obj = (*it).first;
    SubLattice v1 = get_value(obj);
    SubLattice v2 = other.get_value(obj);
    if ((*fn)(v1, v2)) {
      changed = true;
      set_value(obj, v1);
    }
  }
  return(changed);
}

/* This does NOT work in egcs 1.1.2  because of template problems */
#if 0
template <class IndexObj, class SubLattice>
String HIndexedLattice<IndexObj, SubLattice>::
to_string(
	  String (*index_to_string)(const IndexObj &),
	  String (*lattice_to_string)(const SubLattice &),
	  String delimiter) const {
  String s;
  // Print the default value first
  s += "default: ";
  s += lattice_to_string(get_default_value());
  s += delimiter;

  for ( terator iter(this);
	iter.is_valid(); iter.next()) {
    IndexObj idx = iter.current();
    s += index_to_string(idx);
    s += ": ";
    SubLattice l = get_value(idx);
    s += lattice_to_string(l);
    s += delimiter;
  }
  return(s);
};
#endif 


#endif /* HINDEXED_LATTICE_H */
