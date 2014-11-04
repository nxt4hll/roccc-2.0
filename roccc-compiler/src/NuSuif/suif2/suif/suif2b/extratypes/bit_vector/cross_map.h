#ifndef CROSS_MAP_H
#define CROSS_MAP_H

#include "common/machine_dependent.h"
#include "common/common_forwarders.h"
#include "common/suif_hash_map.h"
#include "common/suif_vector.h"


// map the node class to an integer that
// is allocated when we find a new one
// We can get the integer from the node
// and the node from the integer.

template <class node_t> 
class CrossMap {
  typedef suif_hash_map<node_t, size_t> map_t;
  typedef suif_vector<node_t> vect_t;

  map_t *_map;
  vect_t *_vect;
 public:
  CrossMap() { 
    _map = new suif_hash_map<node_t, size_t>();
    _vect = new vect_t();
  }
  ~CrossMap() { delete _map; delete _vect; }
  CrossMap &operator=(const CrossMap &other) {
    delete _map;
    delete _vect;
    _map = new map_t(other._map);
    _vect = new vect_t(other._vect);
    return(*this);
  }

  CrossMap(const CrossMap &other) :
    _map(new map_t(other._map)),
    _vect(new vect_t(other._vect))
    {}

  CrossMap *clone() const {
    return(new CrossMap(*this));
  }
  
  bool is_member(node_t node) const { 
    typename map_t::iterator iter = _map->find(node);
    return(!(iter == _map->end()));
  }

  size_t lookup_id(node_t node) const { 
    typename map_t::iterator iter = _map->find(node);
    assert(!(iter == _map->end()));
    return((*iter).second);
  }
 
 size_t retrieve_id(node_t node) { 
    typename map_t::iterator iter = _map->find(node);
    if (iter == _map->end()) {
      size_t id = _vect->size();
      _vect->push_back(node);
      _map->enter_value(node, id);
      return(id);
    }
    return((*iter).second);
  }

  node_t get_node(size_t id) const {
    return((*_vect)[id]);
  }

  size_t size() const { return(_vect->size()); }

  // for (CrossMap<a>::iterator iter(the_cross_map);
  //      iter.is_valid(); iter.next()) {
  //   a idx = iter.current();
  // 
  // }

  class iterator {
    suif_vector<node_t>*          _vect;
    typename suif_vector<node_t>::iterator _iter;
  public:
    iterator(const CrossMap *map) :
      _vect(map->_vect),
      _iter(map->_vect->begin())
      {}
    iterator(const iterator &other) :
      _vect(other._vect),
      _iter(other._iter)
      {}
    iterator &operator=(const iterator &other) {
      _vect = other._vect;
      _iter = other._iter;
    }
    node_t current() const {
      return (*_iter);
    }
    bool is_valid() const {
      return (_iter != _vect->end());
    }
    void next() { _iter++; }
    void reset() { _iter = _vect->begin(); }
  };

  friend class iterator;
};
  
  
#endif /* CROSS_MAP_H */
