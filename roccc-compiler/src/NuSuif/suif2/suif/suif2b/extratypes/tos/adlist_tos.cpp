/* file "adlist_tos.cc" */


/*
   Copyright (c) 1997 Stanford University

   All rights reserved.

   This software is provided under the terms described in
   the "suif_copyright.h" include file.
 */

#include <common/suif_copyright.h>


/*
   This is the implementation of the adlist_tos template and
   related templates, which are templates for classes implementing
   totally-ordered sets with doubly linked lists plus index arrays,
   for sty, the first-level main library of the SUIF system.
 */


#ifndef DOING_TEMPLATE_INLINING

#include "const_cast.h"
#include "referenced_item.h"
#include "ion/ion.h"
#include <common/i_integer.h>
#include <common/MString.h>
#include "tos.h"
#include "adlist_tos.h"

#endif /* not DOING_TEMPLATE_INLINING */


#if ((!defined(INLINE_ALL_TEMPLATES)) || defined(DOING_TEMPLATE_INLINING))

#define INIT_INDEX_SPACE 5


template <class elem_t, class holder_t> adlist_tos_base<elem_t, holder_t>::
adlist_tos_base(void)
{
  _head_e = 0;
  _tail_e = 0;
  _index_size = 0;
  _index_space = INIT_INDEX_SPACE;
  _index = new holder_t *[INIT_INDEX_SPACE];
  _cached_size = 0;
}

template <class elem_t, class holder_t> adlist_tos_base<elem_t, holder_t>::
adlist_tos_base(elem_t elem0)
{
  _head_e = new holder_t(elem0);
  _tail_e = _head_e;
  _index_size = 1;
  _index_space = 1;
  _index = new holder_t *[1];
  _index[0] = _head_e;
  _cached_size = 1;
}

template <class elem_t, class holder_t> adlist_tos_base<elem_t, holder_t>::
adlist_tos_base(elem_t elem0, elem_t elem1)
{
  _tail_e = new holder_t(elem1);
  _head_e = new holder_t(elem0, _tail_e);
  _tail_e->previous = _head_e;
  _index_size = 2;
  _index_space = 2;
  _index = new holder_t *[2];
  _index[0] = _head_e;
  _index[1] = _tail_e;
  _cached_size = 2;
}
/*
   template <class elem_t, class holder_t> adlist_tos_base<elem_t, holder_t>::
   adlist_tos_base(elem_t elem0, elem_t elem1, elem_t elem2)
   {
   _tail_e = new holder_t(elem2);
   holder_t *s1 = new holder_t(elem1, _tail_e);
   _tail_e->previous = s1;
   _head_e = new holder_t(elem0, s1);
   s1->previous = _head_e;
   _index_size = 3;
   _index_space = 3;
   _index = new holder_t *[3];
   _index[0] = _head_e;
   _index[1] = s1;
   _index[2] = _tail_e;
   _cached_size = 3;
   }

   template <class elem_t, class holder_t> adlist_tos_base<elem_t, holder_t>::
   adlist_tos_base(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3)
   {
   _tail_e = new holder_t(elem3);
   holder_t *s2 = new holder_t(elem2, _tail_e);
   _tail_e->previous = s2;
   holder_t *s1 = new holder_t(elem1, s2);
   s2->previous = s1;
   _head_e = new holder_t(elem0, s1);
   s1->previous = _head_e;
   _index_size = 4;
   _index_space = 4;
   _index = new holder_t *[4];
   _index[0] = _head_e;
   _index[1] = s1;
   _index[2] = s2;
   _index[3] = _tail_e;
   _cached_size = 4;
   }

   template <class elem_t, class holder_t> adlist_tos_base<elem_t, holder_t>::
   adlist_tos_base(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3,
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
   _index_size = 5;
   _index_space = 5;
   _index = new holder_t *[5];
   _index[0] = _head_e;
   _index[1] = s1;
   _index[2] = s2;
   _index[3] = s3;
   _index[4] = _tail_e;
   _cached_size = 5;
   }

   template <class elem_t, class holder_t> adlist_tos_base<elem_t, holder_t>::
   adlist_tos_base(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3,
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
_index_size = 6;
_index_space = 6;
_index = new holder_t *[6];
_index[0] = _head_e;
_index[1] = s1;
_index[2] = s2;
_index[3] = s3;
_index[4] = s4;
_index[5] = _tail_e;
_cached_size = 6;
}

template <class elem_t, class holder_t> adlist_tos_base<elem_t, holder_t>::
adlist_tos_base(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3,
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
  _index_size = 7;
  _index_space = 7;
  _index = new holder_t *[7];
  _index[0] = _head_e;
  _index[1] = s1;
  _index[2] = s2;
  _index[3] = s3;
  _index[4] = s4;
  _index[5] = s5;
  _index[6] = _tail_e;
  _cached_size = 7;
}

template <class elem_t, class holder_t> adlist_tos_base<elem_t, holder_t>::
adlist_tos_base(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3,
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
  _index_size = 8;
  _index_space = 8;
  _index = new holder_t *[8];
  _index[0] = _head_e;
  _index[1] = s1;
  _index[2] = s2;
  _index[3] = s3;
  _index[4] = s4;
  _index[5] = s5;
  _index[6] = s6;
  _index[7] = _tail_e;
  _cached_size = 8;
}
*/
template <class elem_t, class holder_t> adlist_tos_base<elem_t, holder_t>::
~adlist_tos_base(void)
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
  delete[] _index;
}

  template <class elem_t, class holder_t> void
adlist_tos_base<elem_t, holder_t>::extend_array(void)
{
  if (_cached_size == _index_space)
  {
    _index_space = (_index_space * 2) + 3;
    holder_t **new_index = new holder_t *[_index_space];
    for (size_t index_num = 0; index_num < _index_size; ++index_num)
      new_index[index_num] = _index[index_num];
    delete[] _index;
    _index = new_index;
  }
}

template <class elem_t, class holder_t> size_t
adlist_tos_base<elem_t, holder_t>::count(void) const
{
  return _cached_size;
}

template <class elem_t, class holder_t> bool
adlist_tos_base<elem_t, holder_t>::is_empty(void) const
{
  return (_head_e == 0);
}

template <class elem_t, class holder_t> bool
adlist_tos_base<elem_t, holder_t>::count_is(size_t test_count) const
{
  return (_cached_size == test_count);
}

template <class elem_t, class holder_t> bool
adlist_tos_base<elem_t, holder_t>::is_member(elem_t the_elem) const
{
  size_t total = 0;
  holder_t *follow = _head_e;
  while (follow != 0)
  {
    if (total >= _index_size)
    {
      _index[total] = follow;
      typedef adlist_tos_base<elem_t, holder_t> *this_type;
      ++(CONST_CAST(this_type, this)->_index_size);
    }
    if (follow->data == the_elem)
      return true;
    ++total;
    follow = follow->next;
  }
  return false;
}

template <class elem_t, class holder_t> size_t
adlist_tos_base<elem_t, holder_t>::position(elem_t the_elem) const
{
  size_t total = 0;
  holder_t *follow = _head_e;
  while (follow != 0)
  {
    if (total >= _index_size)
    {
      _index[total] = follow;
      typedef adlist_tos_base<elem_t, holder_t> *this_type;
      ++(CONST_CAST(this_type, this)->_index_size);
    }
    if (follow->data == the_elem)
      return total;
    ++total;
    follow = follow->next;
  }
  return (size_t)-1;
}

  template <class elem_t, class holder_t> elem_t
adlist_tos_base<elem_t, holder_t>::elem_by_num(size_t position)
  const
{
  if (position < 0)
    return zero((elem_t *)0);
  else if (position >= _cached_size)
    return zero((elem_t *)0);
  if (position < _index_size)
    return _index[position]->data;
  if (_index_size == 0)
  {
    _index[0] = _head_e;
    typedef adlist_tos_base<elem_t, holder_t> *this_type;
    ++(CONST_CAST(this_type, this)->_index_size);
  }
  size_t total = _index_size - 1;
  holder_t *follow = _index[total];
  while (total < position)
  {
    ++total;
    follow = follow->next;
    _index[total] = follow;
  }
  return follow->data;
}

template <class elem_t, class holder_t> tos_handle<elem_t>
adlist_tos_base<elem_t, holder_t>::lookup_handle(elem_t the_elem) const
{
  size_t total = 0;
  holder_t *follow = _head_e;
  while (follow != 0)
  {
    if (total >= _index_size)
    {
      _index[total] = follow;
      typedef adlist_tos_base<elem_t, holder_t> *this_type;
      ++(CONST_CAST(this_type, this)->_index_size);
    }
    if (follow->data == the_elem)
      return this->build_handle(follow);
    ++total;
    follow = follow->next;
  }
  return this->build_handle(0);
}

template <class elem_t, class holder_t> elem_t
adlist_tos_base<elem_t, holder_t>::elem_by_handle(
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
adlist_tos_base<elem_t, holder_t>::handle_for_num(size_t position)
  const
{
  if (position < 0)
    return this->build_handle(0);
  else if (position >= _cached_size)
    return this->build_handle(0);
  if (position < _index_size)
    return this->build_handle(_index[position]);
  if (_index_size == 0)
  {
    _index[0] = _head_e;
    typedef adlist_tos_base<elem_t, holder_t> *this_type;
    ++(CONST_CAST(this_type, this)->_index_size);
  }
  size_t total = _index_size - 1;
  holder_t *follow = _index[total];
  while (total < position)
  {
    ++total;
    follow = follow->next;
    _index[total] = follow;
  }
  return this->build_handle(follow);
}

template <class elem_t, class holder_t> tos_handle<elem_t>
adlist_tos_base<elem_t, holder_t>::next_handle(
    tos_handle<elem_t> the_handle) const
{
  holder_t *elem_ptr = (holder_t *)(from_handle(the_handle));
  if (elem_ptr == 0)
    return this->build_handle(0);
  else
    return this->build_handle(elem_ptr->next);
}

template <class elem_t, class holder_t> tos_handle<elem_t>
adlist_tos_base<elem_t, holder_t>::previous_handle(
    tos_handle<elem_t> the_handle) const
{
  holder_t *elem_ptr = (holder_t *)(from_handle(the_handle));
  if (elem_ptr == 0)
    return this->build_handle(0);
  else
    return this->build_handle(elem_ptr->previous);
}

template <class elem_t, class holder_t> elem_t
adlist_tos_base<elem_t, holder_t>::head(void) const
{
  if (_head_e == 0)
    return zero((elem_t *)0);
  else
    return _head_e->data;
}

template <class elem_t, class holder_t> elem_t
adlist_tos_base<elem_t, holder_t>::tail(void) const
{
  if (_tail_e == 0)
    return zero((elem_t *)0);
  else
    return _tail_e->data;
}

template <class elem_t, class holder_t> tos_handle<elem_t>
adlist_tos_base<elem_t, holder_t>::head_handle(void) const
{
  if (_head_e == 0)
    return this->build_handle(0);
  else
    return this->build_handle(_head_e);
}

template <class elem_t, class holder_t> tos_handle<elem_t>
adlist_tos_base<elem_t, holder_t>::tail_handle(void) const
{
  if (_tail_e == 0)
    return this->build_handle(0);
  else
    return this->build_handle(_tail_e);
}

  template <class elem_t, class holder_t> void
adlist_tos_base<elem_t, holder_t>::append(elem_t the_elem)
{
  extend_array();
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
  if (_index_size == _cached_size)
  {
    ++_index_size;
    _index[_cached_size] = _tail_e;
  }
  ++_cached_size;
}

  template <class elem_t, class holder_t> void
adlist_tos_base<elem_t, holder_t>::prepend(elem_t the_elem)
{
  extend_array();
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
  _index_size = 1;
  _index[0] = _head_e;
  ++_cached_size;
}

  template <class elem_t, class holder_t> void
adlist_tos_base<elem_t, holder_t>::set_elem_by_num(size_t elem_num,
    elem_t the_elem)
{
  assert(elem_num >= 0);
  if (elem_num >= _cached_size)
  {
    while (_cached_size < elem_num)
      append(zero((elem_t *)0));
    append(the_elem);
    return;
  }
  if (elem_num < _index_size)
  {
    _index[elem_num]->data = the_elem;
    return;
  }
  if (_index_size == 0)
  {
    _index[0] = _head_e;
    typedef adlist_tos_base<elem_t, holder_t> *this_type;
    ++(CONST_CAST(this_type, this)->_index_size);
  }
  size_t total = _index_size - 1;
  holder_t *follow = _index[total];
  while (total < elem_num)
  {
    ++total;
    follow = follow->next;
    _index[total] = follow;
  }
  follow->data = the_elem;
}

  template <class elem_t, class holder_t> void
adlist_tos_base<elem_t, holder_t>::insert(size_t elem_num,
    elem_t the_elem)
{
  assert(elem_num >= 0);
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
  extend_array();
  holder_t *follow;
  if (elem_num - 1 < _index_size)
  {
    follow = _index[elem_num - 1];
  }
  else
  {
    if (_index_size == 0)
    {
      _index[0] = _head_e;
      typedef adlist_tos_base<elem_t, holder_t> *this_type;
      ++(CONST_CAST(this_type, this)->_index_size);
    }
    size_t total = _index_size - 1;
    follow = _index[total];
    while (total < elem_num - 1)
    {
      ++total;
      follow = follow->next;
      _index[total] = follow;
    }
  }
  holder_t *new_e = new holder_t(the_elem, follow->next, follow);
  follow->next = new_e;
  if (_tail_e == follow)
    _tail_e = new_e;
  else
    new_e->next->previous = new_e;
  _index[elem_num] = new_e;
  _index_size = elem_num + 1;
  ++_cached_size;
}

  template <class elem_t, class holder_t> void
adlist_tos_base<elem_t, holder_t>::remove(size_t elem_num)
{
  if (elem_num < 0)
    return;
  else if (elem_num >= _cached_size)
    return;
  holder_t *follow;
  if (elem_num < _index_size)
  {
    follow = _index[elem_num];
  }
  else
  {
    if (_index_size == 0)
    {
      _index[0] = _head_e;
      typedef adlist_tos_base<elem_t, holder_t> *this_type;
      ++(CONST_CAST(this_type, this)->_index_size);
    }
    size_t total = _index_size - 1;
    follow = _index[total];
    while (total < elem_num)
    {
      ++total;
      follow = follow->next;
      _index[total] = follow;
    }
  }
  holder_t *old_e = follow;
  if (_head_e == old_e)
    _head_e = old_e->next;
  else
    old_e->previous->next = old_e->next;
  if (_tail_e == old_e)
    _tail_e = old_e->previous;
  else
    old_e->next->previous = old_e->previous;
  old_e->remove_ref();
  if (old_e->delete_me())
    delete old_e;
  _index_size = elem_num;
  --_cached_size;
}

template <class elem_t, class holder_t> void
adlist_tos_base<elem_t, holder_t>::set_elem_by_handle(
    tos_handle<elem_t> the_handle, elem_t the_elem)
{
  holder_t *elem_ptr = (holder_t *)(from_handle(the_handle));
  if (elem_ptr == 0)
    return;
  elem_ptr->data = the_elem;
}

template <class elem_t, class holder_t> void
adlist_tos_base<elem_t, holder_t>::insert_before(
    tos_handle<elem_t> the_handle, elem_t the_elem)
{
  holder_t *elem_ptr = (holder_t *)(from_handle(the_handle));
  if (elem_ptr == 0)
    return;
  extend_array();
  holder_t *new_e = new holder_t(the_elem, elem_ptr, elem_ptr->previous);
  elem_ptr->previous = new_e;
  if (_head_e == elem_ptr)
    _head_e = new_e;
  else
    new_e->previous->next = new_e;
  _index_size = 0;
  ++_cached_size;
}

template <class elem_t, class holder_t> void
adlist_tos_base<elem_t, holder_t>::insert_after(
    tos_handle<elem_t> the_handle, elem_t the_elem)
{
  holder_t *elem_ptr = (holder_t *)(from_handle(the_handle));
  if (elem_ptr == 0)
    return;
  extend_array();
  holder_t *new_e = new holder_t(the_elem, elem_ptr->next);
  elem_ptr->next = new_e;
  if (_tail_e == elem_ptr)
    _tail_e = new_e;
  else
    new_e->next->previous = new_e;
  _index_size = 0;
  ++_cached_size;
}

template <class elem_t, class holder_t> void
adlist_tos_base<elem_t, holder_t>::remove(
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
  _index_size = 0;
  --_cached_size;
}

  template <class elem_t, class holder_t> elem_t
adlist_tos_base<elem_t, holder_t>::pop(void)
{
  if (_head_e == 0)
    return zero((elem_t *)0);
  holder_t *old_e = _head_e;
  _head_e = old_e->next;
  if (_tail_e == old_e)
    _tail_e = 0;
  else
    old_e->next->previous = 0;
  --_cached_size;
  elem_t result = old_e->data;
  old_e->remove_ref();
  if (old_e->delete_me())
    delete old_e;
  _index_size = 0;
  return result;
}

  template <class elem_t, class holder_t> elem_t
adlist_tos_base<elem_t, holder_t>::tail_pop(void)
{
  if (_head_e == 0)
    return zero((elem_t *)0);
  holder_t *old_e = _tail_e;
  if (_head_e == _tail_e)
    _head_e = 0;
  else
    old_e->previous->next = 0;
  _tail_e = old_e->previous;
  if (_index_size == _cached_size)
    --_index_size;
  --_cached_size;
  elem_t result = old_e->data;
  old_e->remove_ref();
  if (old_e->delete_me())
    delete old_e;
  return result;
}

  template <class elem_t, class holder_t> void
adlist_tos_base<elem_t, holder_t>::clear(void)
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
  _cached_size = 0;
  _index_size = 0;
}

#undef INIT_INDEX_SPACE

#endif /* ((!defined(INLINE_ALL_TEMPLATES)) ||
          defined(DOING_TEMPLATE_INLINING)) */
