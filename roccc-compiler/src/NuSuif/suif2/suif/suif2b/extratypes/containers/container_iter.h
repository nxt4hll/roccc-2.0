#ifndef CONTAINER_ITER_H
#define CONTAINER_ITER_H

#include <assert.h>
#include <container.h>

// *******************************************
// *
// * The following two classes:
// *
// * ConstContainerRef
// * ContainerRef
// *
// * create value classes that do correct reference
// * counting of the container classes that they
// * use.  They only add new constructors
// * and assignment operators.
// *
// *******************************************
//
// *********
// * 
// * value Class ConstContainerRef
// * 
// *********
//
// Value class for encapsulating a ConstContainer
// Does reference counting on the ConstContainer
// 
template <class T> class ConstContainerRef : public ConstContainer<T> {

public:
  // This is the big deal
  void operator=(const ConstContainerRef<T> &other) {
    if (&other == this) return;
    if (_real_c != NULL) { real_remove_ref(); }
    _real_c = other.real_c; { real_add_ref(); }
  }

  // Initialize from a VOID, another ConstContainerRef or a
  // ConstContainer

  ConstContainerRef(void) : 
    _real_c(NULL) {}
  ConstContainerRef(const ConstContainerRef<T> &other) :
    _real_c(other._real_c) { real_add_ref(); }
  ConstContainerRef(const ConstContainer<T> *c) :
    _real_c((ConstContainer<T> *)c) { real_add_ref(); }
  
  ~ConstContainerRef() { real_remove_ref(); }

  // The rest of the class just encapsulates
  // The container class and provides reference
  // counting.
  // See the "class ConstContainer" for documentation
  unsigned count() const { 
    validate(); return(_real_c->count()); }
  bool is_empty() const { 
    validate(); return(_real_c->is_empty()); }
  bool count_is(unsigned count) const { 
    validate(); return(_real_c->count_is(count)); }
  T elem(unsigned n) const { 
    validate(); return(_real_c->elem(n)); }
  T head() const { 
    validate(); return(_real_c->head()); }
  T tail() const { 
    validate(); return(_real_c->tail()); }

  bool is_member(T elem) const { 
    validate(); return(_real_c->is_member(elem)); }
  unsigned position(T elem) const { 
    validate(); return(_real_c->position(elem)); }
  
private:
  ConstContainer<T> *_real_c;

  void real_add_ref() { 
    if (_real_c != NULL)
      _real_c->add_ref();
  }
  void real_remove_ref() { 
    if (_real_c != NULL) {
      _real_c->remove_ref();
      if (_real_c->delete_me()) { delete _real_c; }
      _real_c = NULL;
    }
  }
  void validate() const {  assert(_real_c != NULL); }
};

// *********
// * 
// * value Class ContainerRef
// * 
// *********
//
// Value class for a Container
// Does reference counting on the 
// Container
template <class T> class ContainerRef : public Container<T> {

public:
  // This is the big deal
  void operator=(const ContainerRef<T> &other) {
    if (&other == this) return;
    if (_real_c != NULL) { real_remove_ref(); }
    _real_c = other.real_c; { real_add_ref(); }
  }

  // These initializers are unreadable in this format.
  // Initialize from a VOID, another ConstContainerRef or a
  // ConstContainer

  ContainerRef(void) : 
    _real_c(NULL) {}
  ContainerRef(const ConstContainerRef<T> &other) :
    _real_c(other._real_c) { real_add_ref(); }
  ContainerRef(ConstContainer<T> *init_const_container) :
    _real_c(init_const_container) { real_add_ref(); }
  
  ~ContainerRef() { real_remove_ref(); }

  // The rest of the class just dispatches
  // methods to the Container
  // See the "class Container" for documentation

  // These are documented in class ConstContainer<T>
  unsigned count() const { 
    validate(); return(_real_c->count()); }
  bool is_empty() const { 
    validate(); return(_real_c->is_empty()); }
  bool count_is(unsigned count) const { 
    validate(); return(_real_c->count_is(count)); }
  T elem(unsigned n) const { 
    validate(); return(_real_c->elem(n)); }
  T head() const { 
    validate(); return(_real_c->head()); }
  T tail() const { 
    validate(); return(_real_c->tail()); }
  bool is_member(T elem) const { 
    validate(); return(_real_c->is_member()); }
  unsigned position(T elem) const { 
    validate(); return(_real_c->position()); }

  // These are documented in class Container<T>
  void push(T elem) {
    validate(); _real_c->push(elem); }
  T pop() {
    validate(); return(_real_c->pop()); }
  void append(T elem) {
    validate(); _real_c->append(elem); }
  T unappend() {
    validate(); return(_real_c->unappend()); }
  void set_elem(unsigned n, T elem) {
    validate(); _real_c->set_elem(elem); }
  void insert(unsigned n, T elem) {
    validate(); _real_c->insert(n, elem); }
  T remove(unsigned n) {
    validate(); return(_real_c->remove(n)); }

  void clear() {
    validate(); _real_c->clear(); }

  
private:
  ConstContainer<T> *_real_c;

  // add_ref and remove_ref add a remove references
  // to the REAL container class, not this class.
  void real_add_ref() { 
    if (_real_c != NULL)
      _real_c->add_ref();
  }
  void real_remove_ref() { 
    if (_real_c != NULL) {
      _real_c->remove_ref();
      if (_real_c->delete_me()) { delete _real_c; }
      _real_c = NULL;
    }
  }
  void validate() const {  assert(_real_c != NULL); }

};





// *********
// * 
// * value Class ConstContainerIter
// * 
// *********
//
// This just implements a value class iterator that
// can be used on a ConstContainer<T> c like:
// 
// for (ConstContainerIter<T> iter(c);
//      !iter.done(); iter.increment()) {
//     T val = iter.get();
// }
//
// Of course you can always iterate over the a ConstContainer
// with an integer iteration as in the example in the
// "container.h" code.
//
// for (unsigned i = 0; c->count_is(i); i++) {
//     T val = c->elem(i);
// }
//
// OR:
//
// for (unsigned i = 0; i < c->count(); i++) {
//     T val = c->elem(i);
// }
//
template <class T> class ConstContainerIter {
public:
  ConstContainerIter(const ConstContainer<T> *c) : _c(c) { reset(); }

  ~ConstContainerIter() {}

  void reset() {
    _handle = 0;    _val = 0;
    update();
  }
  void update() {
    _done = _c.count_is(_handle);
    if (!_done) { _val = _c.elem(_handle); }
  }
  void reset(const ConstContainer<T> *c) {
    _c = ConstContainerRef(c);
    reset();
  }
  T get() const { return(_val); }
  void increment() { 
    assert(!done()); 
    _handle++;
    update();
  }
  bool done() const { return _done; }
private:

  ConstContainerRef<T> _c;  // inlined reference counted value class.
  bool _done;       // cache the done condition
  T _val;           // cache the last value
  unsigned _handle; // cache the last index

};

#endif /* CONTAINER_ITER_H */
