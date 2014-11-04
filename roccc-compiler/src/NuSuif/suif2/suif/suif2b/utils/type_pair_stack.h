
#ifndef _SUIFLINK_TYPE_PAIR_STACK_H_
#define _SUIFLINK_TYPE_PAIR_STACK_H_

/*
 * @file
 * This file contains TypePairStack class.
 */


#include "basicnodes/basic.h"

/** A TypePairStack holds a list of \<Type*, Type*\>.
 *
 * This class was originally designed to stop the
 * infinite recursion when processing two recursive type objects.
 * It was implemented to use only stack memory.
 *
 * Example:
 * \code
 * void compare_type(Type* t1, Type* t2, TypePairStack* stk) {
 *   if (stk.is_in(t1, t2)) {
 *     // compare_type(t1, t2,...) has been called before
 *   } else {
 *     // recursively call on sub-components of t1 and t2.
 *     TypePairStack newStk(t1, t2, stk);
 *     compare_type(t1->subtype, t2->subtype, &newStk);
 *   }
 * \endcode
 */
class TypePairStack {
 public:

  /** Constructor.
    * @param t1
    * @param t2
    * @param oldstack
    *
    * The new stack has \<\a t1, \a t2\> and all the pairs in \a oldstack.
    */
  TypePairStack(sf_refed Type * t1, sf_refed Type *t2, sf_refed TypePairStack *oldstack);
  TypePairStack(const TypePairStack &other);
  TypePairStack &operator=(const TypePairStack &other);

  /** Check if a type pair is in this stack.
    * @param t1.
    * @param t2.
    * @return true of \<\a t1, \a t2 \> is in this stack.
    */
  bool is_in(Type *t1, Type *t2);
  
 private:
  sf_refed Type *_type1;
  sf_refed Type *_type2;
  sf_refed TypePairStack *_more;
};

#endif // _SUIFLINK_TYPE_PAIR_STACK_H_
