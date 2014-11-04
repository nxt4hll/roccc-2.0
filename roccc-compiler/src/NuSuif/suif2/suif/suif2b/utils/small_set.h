#ifndef _SMALL_SET_H_
#define _SMALL_SET_H_

/**	@file
 *	A set template for small sets.
 */


#include <assert.h>

/**	\class small_set small_set.h utils/small_set.h
 *
 */
template <class T>
class small_set {
 public:
  typedef T SetType;
  typedef T value_type;
  typedef T* iterator;
  typedef const T* const_iterator;
  typedef bool (EqualChecker)(const T&, const T&);
  
  static bool is_equal(const T& x1, const T& x2) {
    return x1 == x2;
  };

 protected:
  T *      _buff;
  iterator _start;
  iterator _finish;
  iterator _end_of_storage;
  EqualChecker * const _equal_checker;

  void allocate_set(unsigned n) {
    if (n == 0) n++;
    _buff = new T[n];
    _start = _buff;
    _finish = _buff;
    _end_of_storage = _buff + n;
  }	

  void set_fill(iterator& start, unsigned n, const T& val) {
    iterator tmp = start;
    while (tmp < _finish) {
      *(tmp) = val;
      tmp++;		
    }				
  }	

  /* copy from start up to, but not including, end...to dest */
  iterator copy(const_iterator start, const_iterator end, iterator dest) {
    for (const_iterator tmp = start; tmp < end; tmp++, dest++) {
      *dest = *tmp;
    }
    return dest;
  }
  
 void remove_at(iterator pos) {
   suif_assert_message(_start <= pos  &&  pos < _finish,
		       ("Out-of-range error."));
   copy(pos+1, _finish, pos);
   --_finish;
 }

 public:
  small_set(EqualChecker* f = &small_set<T>::is_equal) :
    _buff(0),
    _start(0),
    _finish(0),
    _end_of_storage(0),
    _equal_checker(f) {

    allocate_set(1);
    _start = _buff;
    _finish = _buff;
  }
  
  /**	Create a set
    *	@param	Size to create
    *	@param  Default value for elements 
    */
  small_set(unsigned n,
	    const T& val = T(),
	    EqualChecker* f = &small_set<T>::is_equal) :
    _buff(0), _start(0), _finish(0), _end_of_storage(0), _equal_checker(f) {
    iterator tmp;
    allocate_set(n);
    _start = _buff;
    tmp = _start;
    _finish = tmp + n;	
    set_fill(_start, n, val);
  }
  
  small_set(const small_set<T>& vec) :
    _buff(0), _start(0), _finish(0), _end_of_storage(0),
    _equal_checker(vec._equal_checker) {

    const_iterator it = vec.begin(), end = vec.end();
    unsigned n = vec.size();
    delete [] _buff;
    allocate_set(n);
    copy(it, end, _buff);
    _start = _buff;
    _finish = _buff + n;
  }
  
  ~small_set() {
    delete [] _buff;
  }

  small_set<T>* clone() const {
    return new small_set<T>(*this); 
  } 
  
  small_set<T>& operator=(const small_set<T>& vec) {
    const_iterator it = vec.begin(), end = vec.end();
    unsigned n = vec.size();
    delete [] _buff;
    allocate_set(n);
    copy(it, end, _buff);
    _start = _buff;
    _finish = _buff + n;
    return *this;
  }

  iterator begin() { return _start; }
  iterator end() { return _finish; }
  
  const_iterator begin() const { return _start; }
  const_iterator end() const { return _finish; }

  /**	return number of elements */
  inline unsigned size() const { return _finish - _start; }

  /**	Is the set empty */
  inline bool is_empty() const { return size() == 0; }	
  
  void remove(const T& x) {
    for (iterator it = begin(); it < end();) {
      if (_equal_checker(*it, x))
	remove_at(it);
      else 
	it++;
    }
  }

  void add(const T& x) {
    if (is_member(x)) return;
    if (_finish != _end_of_storage) {
      *_finish = x;
      _finish++;
    } else {
      int oldsize = size();
      int newsize = 2 * oldsize + 1;
      T *pOld = _buff;
      allocate_set(newsize);
      copy(pOld, pOld+oldsize, _buff);
      _finish = _buff + oldsize;
      *_finish = x;
      _finish++;
      delete [] pOld;
    }
  }
  
  bool is_member(const T& x) const {
    for (const_iterator it = begin(); it != end(); it++) {
      if (_equal_checker((*it), x)) return true;
    }
    return false;
  }
  
  /**	Empty the set */
  void clear(void) {
    _finish = _buff;
  }
  
  void do_union(const small_set<T>* that) {
    for (const_iterator it = that->begin(); it != that->end(); it++) {
      add((*it));
    }
  }
  
  void do_intersect(const small_set<T>* that) {
    small_set<T> reject;
    for (const_iterator it = begin(); it < end(); it++) {
      if (!that->is_member((*it))) reject.add(*it);
    }
    for (iterator it = reject.begin(); it < reject.end(); it++) {
      remove(*it);
    }
  }

  
  void do_subtract(const small_set<T>* that) {
    small_set<T> reject;
    for (const_iterator it = begin(); it < end(); it++) {
      if (that->is_member(*it)) reject.add(*it);
    }
    for (iterator it = reject.begin(); it < reject.end(); it++) {
      remove(*it);
    }
  }
  
}; // small_set
	    
	    
#endif _SMALL_SET_H_
