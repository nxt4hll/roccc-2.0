#ifndef CONTAINER_VECTOR_H
#define CONTAINER_VECTOR_H

#include <suif_vector.h>
#include <assert.h>
#include "container.h"


// *********
// * 
// * value Class ContainerVector<T>
// * 
// *********
// 
//
// This implements the Container<T> interface for
// the suif_suif_vector<> (currently called suif_vector<>)
// class.
//
// It caches the last element retrieved
// so that iterating through elements is
// reasonably fast.
// 

template <class T> class ContainerVector : public Container<T> {
private:
  ContainerVector(const ContainerVector<T> &) {};

public:
  ContainerVector() : _ref_count(1)  {
    _vector = new suif_vector<T>;
    _vector_owned = true;
  }
  ContainerVector(suif_vector<T> *the_vector) : _ref_count(1)  {
    _vector = the_vector;
    _vector_owned = false;
  }
  void reset(suif_vector<T> *the_vector) { 
    if (_vector != NULL && _vector_owned) {
      delete _vector;
    }
    _vector = the_vector; }
  
  ~ContainerVector() {}
  

  // Dispatch to class ConstContainer<T>
  unsigned count() const { 
    validate(); return(_vector->size()); }
  bool is_empty() const { 
    validate(); return(_vector->empty()); }
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
    validate(); return(_vector->front()); }
  T tail() const { 
    validate(); return(_vector->back()); }
  bool is_member(T elem) const {  
    validate();  assert(0); }
  unsigned position(T elem) const { 
    validate(); assert(0); }

  // Dispatch to class Container<T>
  void push(T elem) {
    validate(); invalidate_cache(); 
    _vector->insert(_vector->begin(), elem); }
  T pop() {
    validate(); invalidate_cache(); 
    assert(!is_empty());
    T elem = *(_vector->begin()); 
    _vector->erase(_vector->begin());
    return(elem);
  }
  void append(T elem) {
    validate(); invalidate_cache(); _vector->push_back(elem); }
  T unappend() {
    assert(!is_empty());
    invalidate_cache(); 
    T elem = *(_vector->end()); _vector->pop_back(); return(elem); }

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
    // _inter is the same.
    _vector->insert(_iter, elem);
    invalidate_cache();
  }

  T remove(unsigned n) {
    // This is expensive
    validate(); 
    validate_elem(n); 
    find_and_cache_elem(n);
    T elem = *_iter;
    _vector->erase(_iter);
    invalidate_cache();
    return(elem);
  }
  void clear() {
    validate();
    while (!_vector->empty()) {
      _vector->pop_back();
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
  suif_vector<T> *_vector;

  int _ref_count; 
  bool _vector_owned;

  // This is for CACHING!
  // 
  bool _cache_valid; // Of course, the user could use the
  // vector, invalidate the cache, etc.
  unsigned _cached_index;
  //  vector::iterator _begin; // At index.
  suif_vector<T>::iterator _iter; // At index.
  //  vector::iterator _end; // At index.

  void validate() const { assert(_vector != NULL); }
  void validate_elem(unsigned n) const { 
    assert(n >= 0 && n < count()); }

  void invalidate_cache() {
    _cache_valid = false; }
  void invalidate_cache() const {
    ContainerVector<T> *l = (ContainerVector<T> *)this;
    l->invalidate_cache();
  }
    
  T find_and_cache_elem(unsigned id) const {
    ContainerVector<T> *l = (ContainerVector<T> *)this;
    return(l->find_and_cache_elem(id));
  }
  
  T find_and_cache_elem(unsigned id) {
    // We lie about the const because it has no
    // externally visible effect
    _iter = (_vector->begin()) + id;
    //    _iter = (*_vector)[id];
    _cached_index = id;
    return(*_iter);
  }

};

#endif /* CONTAINER_VECTOR */
