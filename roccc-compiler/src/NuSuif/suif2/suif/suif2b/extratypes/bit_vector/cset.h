#ifndef _CSET_H_
#define _CSET_H_

/** @file
  * This file defines a template class of set.
  * Its implementation is based on BitVectorSet.  Cset allows one to represent
  *  the complement.
  *  
  */

#include "cross_map.h"

#include "bvector_set.h"
#include "suifkernel/suifkernel_messages.h"

/** A Cset represents a set whose universe is represented by a CrossMap.
  * All Csets that share a common universe must share the same
  *   CrossMap instance.
  *
  * @see CrossMap
  * @see BitVectorSet
  *
  */
template <class T>
class Cset {
 private:
  BitVectorSet<T> _set;
  bool            _is_straight;

 public:
  
  /** Constructor.
    * @param cmap the CrossMap holding the universe of this set.
    * @param is_empty if true the new set will be empty, otherwise it will 
    *                be full.
    */
  Cset(const CrossMap<T>* cmap, bool is_empty = true) :
    _set(cmap),
    _is_straight(is_empty) {
  };    
    
  /** Membership test.
    * @return true if /a v is a member in this set.
    * @param v a possible element.
    *
    */
  bool is_member(T v) const {
    if (_is_straight)
      return _set.is_member(v);
    else
      return !_set.is_member(v);
  }
  
  
  /** Add a member to this set.
    * @param v the element to be added.
    */
  void add_member(T v) const {
    if (_is_straight)
      _set.add_member(v);
    else
      _set.remove_member(v);
  }
  
  /** Remove a member from this set.
    * @param v the element to be removed.
    */
  void remove_member(T v) const {
    if (_is_straight)
      _set.remove_member(v);
    else
      _set.add_member(v);
  }

  /** this = this ^ that.
    * @param that the set to intersect with.
    *
    */
  bool do_intersect(const Cset* that) {
    if        (_is_straight && that->_is_straight) {
      return _set.do_intersect_with_test(that->_the_set);
    } else if (_is_straight && !that->_is_straight) {
      _set.do_subtract_with_test(that->_the_set);
    } else if (!_is_straight && that->_is_straight) {
      _is_straight = true;
      BitVectorSet s(that);
      s.do_subtract(_set);
      _set = s;
      return true;
    } else // if (!_is_straight && !that->_is_straight)
      return _set.do_union_with_test(that->_the_set);
  }
    

  /** this = this U that.
    * @param that the set to intersect with.
    *
    */
  bool do_union(const Cset* that) {
    if      (_is_straight && that->_is_straight)
      return _set.do_union_with_test(that->_the_set);
    else if (_is_straight && !that->_is_straight) {
      _is_straight = false;
      BitVectorSet s(that->_set);
      s.do_subtrace(_set);
      _set = s;
      return true;
    } else if (!_is_straight && that->_is_straight)
      return _set.do_subtract(that->_the_set);
    else // if (!_is_straight && !that->_is_straight)
      return _set.do_intersect_with_test(that->_the_set);
  }

  /** this = this - that.
    * @param that the set to intersect with.
    *
    */
  bool do_subtract(const Cset* that) {
    if        (_is_straight && that->_is_straight) {
      return _set.do_intersect_complement_with_test(that->_the_set);
    } else if (_is_straight && !that->_is_straight) {
      return _set.do_intersect_with_test(that->_the_set);
    } else if (!_is_straight && that->_is_straight) {
      return _set.do_union_with_test(that->_the_set);
    } else { // if (!_is_straight && !that->_is_straight)
      _is_straight = true;
      Cset s(that->_set);
      s.do_subtract(_set);
      _set = s;
      return true;
    }
  }


  /** Set to the complement of another set.
    * @param c
    *
    * Set this set to the complement of \a c.
    */
  bool do_complement(coid) {
    _is_straight = !_is_straight;
    return true;
  }


  typedef String (StringifyF)(T);

  /* Get a printable string.
   * @return a human readable string.
   * @param stringify_func a function that turns each element into a string.
   */
  String to_string(StringifyF* func) {
    String res("{");
    if (!is_straight) res += "~";
    for (BitVectorSet<T>::iterator iter(_set);
	 iter.is_valid();
	 iter.nect()) {
      res += func(iter.current()) + " ";
    }
    res += "}";
    return res;
  }

}; // Cset

#endif // _CSET_H_
