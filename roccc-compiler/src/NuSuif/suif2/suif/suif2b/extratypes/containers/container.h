#ifndef CONTAINER_H
#define CONTAINER_H

// *******************************************
// *
// * The following two classes:
// *
// * ConstContainer
// * Container
// *
// * are the abstract interface to a
// * Constant (immutable) Container class
// * and a mutable Container class
// *
// * REFERENCE COUNTING 
// * These classes can be reference counted
// *
// * To add a reference on a list 'l':
// *    l->add_ref()
// * 
// * To remove a reference:
// *    l->remove_ref()
// *    if (l->delete_me()) delete l
// *
// * Of course if you are SURE that there is
// * only one owner, you need not check
// * delete_me() before use.
// *
// *
// * WARNING: WARNING: WARNING:
// * If you instantiate a list on the stack
// * or inline in an object, it doesn't
// * matter that it is reference counted:
// * it will STILL disappear when the
// * scope disappears or when the base object
// * is removed.  reference counting does NOT
// * help this.
// *
// *
// * The file "container_iter.h"
// * includes some useful value
// * classes:
// *
// * There are two value classes that
// * do automatic reference counting:
// * ConstContainerRef and ContainerRef
// *
// * In addition there is a standard forward
// * Iterator that can be used. It is bad
// * form to remove elements while iterating.
// *
// *
// * There is an initial implementation
// * of this interface on top of the 
// * list template class in "container_list.h"
// * Other implementations can be created as
// * needed.
// *
// *******************************************
//
// *********
// * 
// * Class ConstContainer
// * 
// *********
// This is an abstract class
// That exports a minimal interface
// use get_elem() to get an element
// by number.  We expect that most
// classes implmenting this interface will
// have some form of caching so that
//
// for (unsigned i = 0 ; i < l->count(); i++) {
// 
// will be efficient.
//
// The class is free to call assert()
// if get_elem() is called with an invalid
// element number.

template <class T> class ConstContainer {

public:
  virtual unsigned count() const = 0;
  virtual bool is_empty() const = 0;
  // this may be more efficient than count();
  virtual bool count_is(unsigned count) const = 0; 
  virtual T elem(unsigned n) const = 0;
  virtual T head() const = 0;
  virtual T tail() const = 0;
  // Should these really be implemented here?
  // I'm inclined to remove them.
  virtual bool is_member(T elem) const = 0;
  virtual unsigned position(T elem) const = 0;

  virtual ~ConstContainer() {}

  // Interface for reference counting.
public: 
  virtual void add_ref(void) {}
  virtual void remove_ref(void) {};
  virtual bool delete_me(void) const { return(false); }

private:
  ConstContainer(const ConstContainer &) {}
  void operator=(const ConstContainer &) {}
};


// *********
// * 
// * Class Container
// * 
// *********
// This class adds the ability to insert elements.
// change elements, etc
// 
// The class is free to call assert()
// if set_elem() is called with an invalid
// element number.
template <class T> class Container : public ConstContainer<T> {
public:
  virtual void push(T elem) = 0;  // prepend
  virtual T pop() = 0;            // remove first element

  virtual void append(T elem) = 0;
  virtual T unappend() = 0;       // remove the last element

  virtual void set_elem(unsigned n, T elem) = 0; // set the element. 
  virtual void insert(unsigned n, T elem) = 0;
  virtual T remove(unsigned n) = 0;

  virtual void clear() = 0;

  // parts of the interface where "elem"
  // can refer to ANY element that is equivalent
  // These MAY be implemented with position()
  virtual void insert_before_elem(T elem, const T &pos) = 0;
  virtual void insert_after_elem(T elem, const T &pos) = 0;
  virtual void add_unique_elem(T elem) = 0;
  virtual T remove_elem(T elem) = 0;

private:
  Container(const Container &) {}
  void operator=(const Container &) {}
};



  
#endif /* CONTAINER_H */
