/* file "cdlist_tos.cc" */


/*
   Copyright (c) 1996, 1997 Stanford University

   All rights reserved.

   This software is provided under the terms described in
   the "suif_copyright.h" include file.
 */

#include <common/suif_copyright.h>


/*
   This is the implementation of the cdlist_tos template and
   related templates, which are templates for classes implementing
   totally-ordered sets with doubly linked lists plus cache
   pointers, for sty, the first-level main library of the SUIF
   system.
 */


#ifndef DOING_TEMPLATE_INLINING

//#define _MODULE_ "libsty"

//#pragma implementation "cdlist_tos.h"


//#include <machine_dependent.h>
#include "const_cast.h"
//#include "anyt.h"
//#include "zero.h"
#include "referenced_item.h"
#include "ion/ion.h"
#include <common/i_integer.h>
//#include "problems.h"
//#include "i_rational.h"
#include <common/MString.h>
#include "tos.h"
#include "cdlist_tos.h"

#endif /* not DOING_TEMPLATE_INLINING */


#if ((!defined(INLINE_ALL_TEMPLATES)) || defined(DOING_TEMPLATE_INLINING))

template <class elem_t, class holder_t> cdlist_tos_base<elem_t, holder_t>::
cdlist_tos_base(void)
{
  _head_e = 0;
  _tail_e = 0;
  _cached_e = 0;
  _cached_index = 0;
  _cached_size = 0;
}

template <class elem_t, class holder_t> cdlist_tos_base<elem_t, holder_t>::
cdlist_tos_base(elem_t elem0)
{
  _head_e = new holder_t(elem0);
  _tail_e = _head_e;
  _cached_e = _head_e;
  _cached_index = 0;
  _cached_size = 1;
}

template <class elem_t, class holder_t> cdlist_tos_base<elem_t, holder_t>::
cdlist_tos_base(elem_t elem0, elem_t elem1)
{
  _tail_e = new holder_t(elem1);
  _head_e = new holder_t(elem0, _tail_e);
  _tail_e->previous = _head_e;
  _cached_e = _head_e;
  _cached_index = 0;
  _cached_size = 2;
}

/*
   template <class elem_t, class holder_t> cdlist_tos_base<elem_t, holder_t>::
   cdlist_tos_base(elem_t elem0, elem_t elem1, elem_t elem2)
   {
   _tail_e = new holder_t(elem2);
   holder_t *s1 = new holder_t(elem1, _tail_e);
   _tail_e->previous = s1;
   _head_e = new holder_t(elem0, s1);
   s1->previous = _head_e;
   _cached_e = _head_e;
   _cached_index = 0;
   _cached_size = 3;
   }

   template <class elem_t, class holder_t> cdlist_tos_base<elem_t, holder_t>::
   cdlist_tos_base(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3)
   {
   _tail_e = new holder_t(elem3);
   holder_t *s2 = new holder_t(elem2, _tail_e);
   _tail_e->previous = s2;
   holder_t *s1 = new holder_t(elem1, s2);
   s2->previous = s1;
   _head_e = new holder_t(elem0, s1);
   s1->previous = _head_e;
   _cached_e = _head_e;
   _cached_index = 0;
   _cached_size = 4;
   }

   template <class elem_t, class holder_t> cdlist_tos_base<elem_t, holder_t>::
   cdlist_tos_base(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3,
   elem_t elem4)
   {
   _tail_e = new holder_t(elem4);
   holder_t *s3 = new holder_t(elem3, _tail_e);
   _tail_e->previous = s3;
   holder_t *s2 = new holder_t(elem2, s3);
   s3->previous = s2;
   holder_t *s1 = new holder_t(elem1, s2);
   s2->previous = s1;
   _head_e = new holder_t(elem0, s1);
   s1->previous = _head_e;
   _cached_e = _head_e;
   _cached_index = 0;
   _cached_size = 5;
   }

   template <class elem_t, class holder_t> cdlist_tos_base<elem_t, holder_t>::
   cdlist_tos_base(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3,
   elem_t elem4, elem_t elem5)
   {
   _tail_e = new holder_t(elem5);
   holder_t *s4 = new holder_t(elem4, _tail_e);
   _tail_e->previous = s4;
   holder_t *s3 = new holder_t(elem3, s4);
   s4->previous = s3;
   holder_t *s2 = new holder_t(elem2, s3);
   s3->previous = s2;
   holder_t *s1 = new holder_t(elem1, s2);
   s2->previous = s1;
   _head_e = new holder_t(elem0, s1);
   s1->previous = _head_e;
   _cached_e = _head_e;
   _cached_index = 0;
   _cached_size = 6;
   }

   template <class elem_t, class holder_t> cdlist_tos_base<elem_t, holder_t>::
   cdlist_tos_base(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3,
   elem_t elem4, elem_t elem5, elem_t elem6)
   {
   _tail_e = new holder_t(elem6);
holder_t *s5 = new holder_t(elem5, _tail_e);
_tail_e->previous = s5;
holder_t *s4 = new holder_t(elem4, s5);
s5->previous = s4;
holder_t *s3 = new holder_t(elem3, s4);
s4->previous = s3;
holder_t *s2 = new holder_t(elem2, s3);
s3->previous = s2;
holder_t *s1 = new holder_t(elem1, s2);
s2->previous = s1;
_head_e = new holder_t(elem0, s1);
s1->previous = _head_e;
_cached_e = _head_e;
_cached_index = 0;
_cached_size = 7;
}

template <class elem_t, class holder_t> cdlist_tos_base<elem_t, holder_t>::
cdlist_tos_base(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3,
    elem_t elem4, elem_t elem5, elem_t elem6, elem_t elem7)
{
  _tail_e = new holder_t(elem7);
  holder_t *s6 = new holder_t(elem6, _tail_e);
  _tail_e->previous = s6;
  holder_t *s5 = new holder_t(elem5, s6);
  s6->previous = s5;
  holder_t *s4 = new holder_t(elem4, s5);
  s5->previous = s4;
  holder_t *s3 = new holder_t(elem3, s4);
  s4->previous = s3;
  holder_t *s2 = new holder_t(elem2, s3);
  s3->previous = s2;
  holder_t *s1 = new holder_t(elem1, s2);
  s2->previous = s1;
  _head_e = new holder_t(elem0, s1);
  s1->previous = _head_e;
  _cached_e = _head_e;
  _cached_index = 0;
  _cached_size = 8;
}
*/

template <class elem_t, class holder_t> cdlist_tos_base<elem_t, holder_t>::
~cdlist_tos_base(void)
{
  while (_head_e != 0)
  {
    holder_t *current = _head_e;
    _head_e = _head_e->next;
    current->remove_ref();
    if (current->delete_me())
      delete current;
  }
  _tail_e = 0;
}

template <class elem_t, class holder_t> size_t
cdlist_tos_base<elem_t, holder_t>::count(void) const
{
  return _cached_size;
}

template <class elem_t, class holder_t> bool
cdlist_tos_base<elem_t, holder_t>::is_empty(void) const
{
  return (_head_e == 0);
}

template <class elem_t, class holder_t> bool
cdlist_tos_base<elem_t, holder_t>::count_is(size_t test_count) const
{
  return (_cached_size == test_count);
}

template <class elem_t, class holder_t> bool
cdlist_tos_base<elem_t, holder_t>::is_member(elem_t the_elem) const
{
  size_t total = 0;
  holder_t *follow = _head_e;
  while (follow != 0)
  {
    if (follow->data == the_elem)
    {
      typedef cdlist_tos_base<elem_t, holder_t> *this_type;
      CONST_CAST(this_type, this)->_cached_e = follow;
      CONST_CAST(this_type, this)->_cached_index = total;
      return true;
    }
    ++total;
    follow = follow->next;
  }
  return false;
}

template <class elem_t, class holder_t> size_t
cdlist_tos_base<elem_t, holder_t>::position(elem_t the_elem) const
{
  size_t total = 0;
  holder_t *follow = _head_e;
  while (follow != 0)
  {
    if (follow->data == the_elem)
    {
      typedef cdlist_tos_base<elem_t, holder_t> *this_type;
      CONST_CAST(this_type, this)->_cached_e = follow;
      CONST_CAST(this_type, this)->_cached_index = total;
      return total;
    }
    ++total;
    follow = follow->next;
  }
  return (size_t)-1;
}

  template <class elem_t, class holder_t> elem_t
cdlist_tos_base<elem_t, holder_t>::elem_by_num(size_t position)
  const
{
  size_t total;
  holder_t *follow;
  if (position < 0)
  {
    return zero((elem_t *)0);
  }
  else if (position >= _cached_size)
  {
    return zero((elem_t *)0);
  }
  else if (position <= _cached_index / 2)
  {
    /* Start at front. */
    total = 0;
    follow = _head_e;
  }
  else if (position <= (_cached_index + _cached_size) / 2)
  {
    /* Start at cache. */
    total = _cached_index;
    follow = _cached_e;
  }
  else
  {
    /* Start at end. */
    total = _cached_size - 1;
    follow = _tail_e;
  }
  while (total < position)
  {
    ++total;
    follow = follow->next;
  }
  while (total > position)
  {
    --total;
    follow = follow->previous;
  }
  typedef cdlist_tos_base<elem_t, holder_t> *this_type;
  CONST_CAST(this_type, this)->_cached_index = total;
  CONST_CAST(this_type, this)->_cached_e = follow;
  return follow->data;
}

template <class elem_t, class holder_t> tos_handle<elem_t>
cdlist_tos_base<elem_t, holder_t>::lookup_handle(elem_t the_elem) const
{
  size_t total = 0;
  holder_t *follow = _head_e;
  while (follow != 0)
  {
    if (follow->data == the_elem)
    {
      typedef cdlist_tos_base<elem_t, holder_t> *this_type;
      CONST_CAST(this_type, this)->_cached_e = follow;
      CONST_CAST(this_type, this)->_cached_index = total;
      return this->build_handle(follow);
    }
    ++total;
    follow = follow->next;
  }
  return this->build_handle(0);
}

template <class elem_t, class holder_t> elem_t
cdlist_tos_base<elem_t, holder_t>::elem_by_handle(
    tos_handle<elem_t> the_handle) const
{
  holder_t *elem_ptr =
    (holder_t *)(from_handle(the_handle));
  if (elem_ptr == 0)
    return zero((elem_t *)0);
  else
    return elem_ptr->data;
}

  template <class elem_t, class holder_t> tos_handle<elem_t>
cdlist_tos_base<elem_t, holder_t>::handle_for_num(size_t position)
  const
{
  size_t total;
  holder_t *follow;
  if (position < 0)
  {
    return this->build_handle(0);
  }
  else if (position >= _cached_size)
  {
    return this->build_handle(0);
  }
  else if (position <= _cached_index / 2)
  {
    /* Start at front. */
    total = 0;
    follow = _head_e;
  }
  else if (position <= (_cached_index + _cached_size) / 2)
  {
    /* Start at cache. */
    total = _cached_index;
    follow = _cached_e;
  }
  else
  {
    /* Start at end. */
    total = _cached_size - 1;
    follow = _tail_e;
  }
  while (total < position)
  {
    ++total;
    follow = follow->next;
  }
  while (total > position)
  {
    --total;
    follow = follow->previous;
  }
  typedef cdlist_tos_base<elem_t, holder_t> *this_type;
  CONST_CAST(this_type, this)->_cached_index = total;
  CONST_CAST(this_type, this)->_cached_e = follow;
  return this->build_handle(follow);
}

template <class elem_t, class holder_t> tos_handle<elem_t>
cdlist_tos_base<elem_t, holder_t>::next_handle(
    tos_handle<elem_t> the_handle) const
{
  holder_t *elem_ptr = (holder_t *)(from_handle(the_handle));
  if (elem_ptr == 0)
    return this->build_handle(0);
  else
    return this->build_handle(elem_ptr->next);
}

template <class elem_t, class holder_t> tos_handle<elem_t>
cdlist_tos_base<elem_t, holder_t>::previous_handle(
    tos_handle<elem_t> the_handle) const
{
  holder_t *elem_ptr = (holder_t *)(from_handle(the_handle));
  if (elem_ptr == 0)
    return this->build_handle(0);
  else
    return this->build_handle(elem_ptr->previous);
}

template <class elem_t, class holder_t> elem_t
cdlist_tos_base<elem_t, holder_t>::head(void) const
{
  if (_head_e == 0)
    return zero((elem_t *)0);
  else
    return _head_e->data;
}

template <class elem_t, class holder_t> elem_t
cdlist_tos_base<elem_t, holder_t>::tail(void) const
{
  if (_tail_e == 0)
    return zero((elem_t *)0);
  else
    return _tail_e->data;
}

template <class elem_t, class holder_t> tos_handle<elem_t>
cdlist_tos_base<elem_t, holder_t>::head_handle(void) const
{
  if (_head_e == 0)
    return this->build_handle(0);
  else
    return this->build_handle(_head_e);
}

template <class elem_t, class holder_t> tos_handle<elem_t>
cdlist_tos_base<elem_t, holder_t>::tail_handle(void) const
{
  if (_tail_e == 0)
    return this->build_handle(0);
  else
    return this->build_handle(_tail_e);
}

  template <class elem_t, class holder_t> void
cdlist_tos_base<elem_t, holder_t>::append(elem_t the_elem)
{
  if (_tail_e == 0)
  {
    _head_e = new holder_t(the_elem);
    _tail_e = _head_e;
  }
  else
  {
    _tail_e->next = new holder_t(the_elem, 0, _tail_e);
    _tail_e = _tail_e->next;
  }
  _cached_e = _tail_e;
  _cached_index = _cached_size;
  ++_cached_size;
}

  template <class elem_t, class holder_t> void
cdlist_tos_base<elem_t, holder_t>::prepend(elem_t the_elem)
{
  if (_head_e == 0)
  {
    _head_e = new holder_t(the_elem);
    _tail_e = _head_e;
  }
  else
  {
    _head_e->previous = new holder_t(the_elem, _head_e);
    _head_e = _head_e->previous;
  }
  _cached_e = _head_e;
  _cached_index = 0;
  ++_cached_size;
}

  template <class elem_t, class holder_t> void
cdlist_tos_base<elem_t, holder_t>::set_elem_by_num(size_t elem_num,
    elem_t the_elem)
{
  assert(elem_num >= 0);
  size_t total;
  holder_t *follow;
  if (elem_num >= _cached_size)
  {
    while (_cached_size < elem_num)
      append(zero((elem_t *)0));
    append(the_elem);
    return;
  }
  else if (elem_num <= _cached_index / 2)
  {
    /* Start at front. */
    total = 0;
    follow = _head_e;
  }
  else if (elem_num <= (_cached_index + _cached_size) / 2)
  {
    /* Start at cache. */
    total = _cached_index;
    follow = _cached_e;
  }
  else
  {
    /* Start at end. */
    total = _cached_size - 1;
    follow = _tail_e;
  }
  while (total < elem_num)
  {
    ++total;
    follow = follow->next;
  }
  while (total > elem_num)
  {
    --total;
    follow = follow->previous;
  }
  _cached_index = total;
  _cached_e = follow;
  follow->data = the_elem;
}

  template <class elem_t, class holder_t> void
cdlist_tos_base<elem_t, holder_t>::insert(size_t elem_num,
    elem_t the_elem)
{
  assert(elem_num >= 0);
  size_t total;
  holder_t *follow;
  if (elem_num == 0)
  {
    prepend(the_elem);
    return;
  }
  if (elem_num >= _cached_size)
  {
    while (_cached_size < elem_num)
      append(zero((elem_t *)0));
    append(the_elem);
    return;
  }
  else if (elem_num - 1 <= _cached_index / 2)
  {
    /* Start at front. */
    total = 0;
    follow = _head_e;
  }
  else if (elem_num - 1 <= (_cached_index + _cached_size) / 2)
  {
    /* Start at cache. */
    total = _cached_index;
    follow = _cached_e;
  }
  else
  {
    /* Start at end. */
    total = _cached_size - 1;
    follow = _tail_e;
  }
  while (total < elem_num - 1)
  {
    ++total;
    follow = follow->next;
  }
  while (total > elem_num - 1)
  {
    --total;
    follow = follow->previous;
  }
  holder_t *new_e = new holder_t(the_elem, follow->next, follow);
  follow->next = new_e;
  if (_tail_e == follow)
    _tail_e = new_e;
  else
    new_e->next->previous = new_e;
  _cached_index = elem_num;
  _cached_e = new_e;
  ++_cached_size;
}

  template <class elem_t, class holder_t> void
cdlist_tos_base<elem_t, holder_t>::remove(size_t elem_num)
{
  size_t total;
  holder_t *follow;
  if (elem_num < 0)
  {
    return;
  }
  else if (elem_num >= _cached_size)
  {
    return;
  }
  else if (elem_num <= _cached_index / 2)
  {
    /* Start at front. */
    total = 0;
    follow = _head_e;
  }
  else if (elem_num <= (_cached_index + _cached_size) / 2)
  {
    /* Start at cache. */
    total = _cached_index;
    follow = _cached_e;
  }
  else
  {
    /* Start at end. */
    total = _cached_size - 1;
    follow = _tail_e;
  }
  while (total < elem_num)
  {
    ++total;
    follow = follow->next;
  }
  while (total > elem_num)
  {
    --total;
    follow = follow->previous;
  }
  holder_t *old_e = follow;
  if (_head_e == old_e)
  {
    _head_e = old_e->next;
    _cached_index = 0;
    _cached_e = _head_e;
  }
  else
  {
    old_e->previous->next = old_e->next;
    _cached_index = total - 1;
    _cached_e = old_e->previous;
  }
  if (_tail_e == old_e)
    _tail_e = old_e->previous;
  else
    old_e->next->previous = old_e->previous;
  old_e->remove_ref();
  if (old_e->delete_me())
    delete old_e;
  --_cached_size;
}

template <class elem_t, class holder_t> void
cdlist_tos_base<elem_t, holder_t>::set_elem_by_handle(
    tos_handle<elem_t> the_handle, elem_t the_elem)
{
  holder_t *elem_ptr = (holder_t *)(from_handle(the_handle));
  if (elem_ptr == 0)
    return;
  elem_ptr->data = the_elem;
}

template <class elem_t, class holder_t> void
cdlist_tos_base<elem_t, holder_t>::insert_before(
    tos_handle<elem_t> the_handle, elem_t the_elem)
{
  holder_t *elem_ptr = (holder_t *)(from_handle(the_handle));
  if (elem_ptr == 0)
    return;
  holder_t *new_e = new holder_t(the_elem, elem_ptr, elem_ptr->previous);
  elem_ptr->previous = new_e;
  if (_head_e == elem_ptr)
    _head_e = new_e;
  else
    new_e->previous->next = new_e;
  _cached_e = _head_e;
  _cached_index = 0;
  ++_cached_size;
}

template <class elem_t, class holder_t> void
cdlist_tos_base<elem_t, holder_t>::insert_after(
    tos_handle<elem_t> the_handle, elem_t the_elem)
{
  holder_t *elem_ptr = (holder_t *)(from_handle(the_handle));
  if (elem_ptr == 0)
    return;
  holder_t *new_e = new holder_t(the_elem, elem_ptr->next);
  elem_ptr->next = new_e;
  if (_tail_e == elem_ptr)
    _tail_e = new_e;
  else
    new_e->next->previous = new_e;
  _cached_e = _head_e;
  _cached_index = 0;
  ++_cached_size;
}

template <class elem_t, class holder_t> void
cdlist_tos_base<elem_t, holder_t>::remove(
    tos_handle<elem_t> the_handle)
{
  holder_t *elem_ptr = (holder_t *)(from_handle(the_handle));
  if (elem_ptr == 0)
    return;
  if (_head_e == elem_ptr)
    _head_e = elem_ptr->next;
  else
    elem_ptr->previous->next = elem_ptr->next;
  if (_tail_e == elem_ptr)
    _tail_e = elem_ptr->previous;
  else
    elem_ptr->next->previous = elem_ptr->previous;
  elem_ptr->remove_ref();
  if (elem_ptr->delete_me())
    delete elem_ptr;
  _cached_e = _head_e;
  _cached_index = 0;
  --_cached_size;
}

  template <class elem_t, class holder_t> elem_t
cdlist_tos_base<elem_t, holder_t>::pop(void)
{
  if (_head_e == 0)
    return zero((elem_t *)0);
  holder_t *old_e = _head_e;
  _head_e = old_e->next;
  if (_tail_e == old_e)
    _tail_e = 0;
  else
    old_e->next->previous = 0;
  if (_cached_e == old_e)
    _cached_e = old_e->next;
  else
    --_cached_index;
  --_cached_size;
  elem_t result = old_e->data;
  old_e->remove_ref();
  if (old_e->delete_me())
    delete old_e;
  return result;
}

  template <class elem_t, class holder_t> elem_t
cdlist_tos_base<elem_t, holder_t>::tail_pop(void)
{
  if (_head_e == 0)
    return zero((elem_t *)0);
  holder_t *old_e = _tail_e;
  if (_head_e == _tail_e)
    _head_e = 0;
  else
    old_e->previous->next = 0;
  _tail_e = old_e->previous;
  if (_cached_e == old_e)
  {
    _cached_e = old_e->previous;
    --_cached_index;
  }
  --_cached_size;
  elem_t result = old_e->data;
  old_e->remove_ref();
  if (old_e->delete_me())
    delete old_e;
  return result;
}

  template <class elem_t, class holder_t> void
cdlist_tos_base<elem_t, holder_t>::clear(void)
{
  while (_head_e != 0)
  {
    holder_t *current = _head_e;
    _head_e = _head_e->next;
    current->remove_ref();
    if (current->delete_me())
      delete current;
  }
  _head_e = 0;
  _tail_e = 0;
  _cached_e = 0;
  _cached_index = 0;
  _cached_size = 0;
}

#endif /* ((!defined(INLINE_ALL_TEMPLATES)) ||
          defined(DOING_TEMPLATE_INLINING)) */
