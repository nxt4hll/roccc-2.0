#ifndef CONTAINER_LIST_H
#define CONTAINER_LIST_H

#include <suif_list.h>
#include <assert.h>
#include "container.h"


// *********
// * 
// * value Class ContainerList<T>
// * 
// *********
// 
//
// This implements the Container<T> interface for
// the suif_list<> (currently called list<>)
// class.
//
// It caches the last element retrieved
// so that iterating through elements is
// reasonably fast.
// 

template <class T> class ContainerList : public Container<T> {
private:
  ContainerList(const ContainerList<T> &) {};

public:
  ContainerList() : _ref_count(1)  {
    _list = new list<T>;
    _list_owned = true;
  }
  ContainerList(list<T> *the_list) : _ref_count(1)  {
    _list = the_list;
    _list_owned = false;
  }
  reset(list<T> *the_list) { 
    if (_list != NULL && _list_owned) {
      delete _list;
    }
    _list = the_list; }
  
  ~ContainerList() {}
  

  // Dispatch to class ConstContainer<T>
  unsigned count() const { 
    validate(); return(_list->length()); }
  bool is_empty() const { 
    validate(); return(_list->empty()); }
  bool count_is(unsigned cnt) const { 
    validate(); return(count() == cnt); }
  T elem(unsigned n) const { 
    validate();
    validate_elem(n);
    if (n == 0) { return(head()); } // No caching
    if (n == count()) { return(tail()); } // No caching
    return(find_and_cache_elem(n));
  }
  T head() const { 
    validate(); return(_list->front()); }
  T tail() const { 
    validate(); return(_list->back()); }
  bool is_member(T elem) const {  
    validate();  assert(0); }
  unsigned position(T elem) const { 
    validate(); assert(0); }

  // Dispatch to class Container<T>
  void push(T elem) {
    validate(); invalidate_cache(); _list->push_front(elem); }
  T pop() {
    validate(); invalidate_cache(); 
    assert(!is_empty());
    T elem = *(_list->begin()); _list->pop_front(); return(elem); }
  void append(T elem) {
    validate(); invalidate_cache(); _list->push_back(elem); }
  T unappend() {
    assert(!is_empty());
    invalidate_cache(); 
    T elem = *(_list->end()); _list->pop_back(); return(elem); }

  // Don't need to invalidate the cache!!
  void set_elem(unsigned n, T elem) {
    validate(); validate_elem(n);
    // Set the iterator
    find_and_cache_elem(n);
    (*_iter) = elem;
  }

  void insert(unsigned n, T elem) {
    find_and_cache_elem(n);
    // update the cache while we're at it..
    _iter = _list->insert(_iter, elem);
  }

  T remove(unsigned n) {
    // This is expensive
    validate(); 
    validate_elem(n); 
    find_and_cache_elem(n);
    T elem = *_iter;
    _list->erase(_iter);
    invalidate_cache();
    return(elem);
  }
  void clear() {
    validate();
    while (!_list->empty()) {
      _list->pop_front();
    }
    invalidate_cache();
  }
protected:
  void add_ref() {
    _ref_count++;
  }
  void remove_ref() {
    assert(_ref_count > 0);
    _ref_count--;
  }
  bool delete_me() {
    return(_ref_count == 0);
  }

private:
  list<T> *_list;

  int _ref_count; 
  bool _list_owned;

  // This is for CACHING!
  // 
  bool _cache_valid; // Of course, the user could use the
  // list, invalidate the cache, etc.
  unsigned _cached_index;
  //  list::iterator _begin; // At index.
  list<T>::iterator _iter; // At index.
  //  list::iterator _end; // At index.

  void validate() const { assert(_list != NULL); }
  void validate_elem(unsigned n) const { 
    assert(n >= 0 && n < count()); }

  void invalidate_cache() {
    _cache_valid = false; }
  void invalidate_cache() const {
    ContainerList<T> *l = (ContainerList<T> *)this;
    l->invalidate_cache();
  }
    
  T find_and_cache_elem(unsigned id) const {
    ContainerList<T> *l = (ContainerList<T> *)this;
    return(l->find_and_cache_elem(id));
  }
  
  T find_and_cache_elem(unsigned id) {
    // We lie about the const because it has no
    // externally visible effect
    if (!_cache_valid) {
      //      _begin = _list->begin();
      //      _end = _list->end();
      _iter = _list->begin();
      _cached_index = 0;
    }
      
    // @@@ If we're closer
    // to the end or beggining,
    // we should really iterate from there.
    // Current implementation always goes from
    // the cached value.
    if (id > _cached_index) {
      while (id != _cached_index) {
	_iter++;
	_cached_index++;
      }
    } else if (id < _cached_index) {
      while (id != _cached_index) {
	_iter--;
	_cached_index--;
      }
    }
    assert(_cached_index == id);
    return(*_iter);
    // Iter is set.
  }


};

#endif /* CONTAINER_LIST */
