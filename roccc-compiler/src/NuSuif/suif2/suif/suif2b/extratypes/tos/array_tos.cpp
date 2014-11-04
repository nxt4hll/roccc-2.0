/* file "array_tos.cc" */


/*
   Copyright (c) 1996, 1997 Stanford University

   All rights reserved.

   This software is provided under the terms described in
   the "suif_copyright.h" include file.
 */

#include <common/suif_copyright.h>
#include "zero.h"

/*
   This is the implementation of the array_tos template and related
   templates, which are templates for classes implementing
   totally-ordered sets with simple arrays, for sty, the
   first-level main library of the SUIF system.
 */


#ifndef DOING_TEMPLATE_INLINING

//#define _MODULE_ "libsty"

//#pragma implementation "array_tos.h"


#include "const_cast.h"
#include "referenced_item.h"
#include "ion/ion.h"
#include <common/i_integer.h>
#include "tos.h"
#include "special_ro_tos.h"
#include "array_tos.h"

#endif /* not DOING_TEMPLATE_INLINING */


#if ((!defined(INLINE_ALL_TEMPLATES)) || defined(DOING_TEMPLATE_INLINING))

#define DEFAULT_INIT_SPACE 20


template <class elem_t> array_tos<elem_t>::array_tos(void) :
  _data(new elem_t[DEFAULT_INIT_SPACE]),
  _size(0),
  _space(DEFAULT_INIT_SPACE)
{
  //    _data = new elem_t[DEFAULT_INIT_SPACE];
  //    _size = 0;
  //    _space = DEFAULT_INIT_SPACE;
}

template <class elem_t> array_tos<elem_t>::array_tos(elem_t elem0)
{
  _data = new elem_t[1];
  _size = 1;
  _space = 1;
  _data[0] = elem0;
}

template <class elem_t> array_tos<elem_t>::array_tos(elem_t elem0,
    elem_t elem1)
{
  _data = new elem_t[2];
  _size = 2;
  _space = 2;
  _data[0] = elem0;
  _data[1] = elem1;
}
/*
   template <class elem_t> array_tos<elem_t>::array_tos(elem_t elem0,
   elem_t elem1, elem_t elem2)
   {
   _data = new elem_t[3];
   _size = 3;
   _space = 3;
   _data[0] = elem0;
   _data[1] = elem1;
   _data[2] = elem2;
   }

   template <class elem_t> array_tos<elem_t>::array_tos(elem_t elem0,
   elem_t elem1, elem_t elem2, elem_t elem3)
   {
   _data = new elem_t[4];
   _size = 4;
   _space = 4;
   _data[0] = elem0;
   _data[1] = elem1;
   _data[2] = elem2;
   _data[3] = elem3;
   }

   template <class elem_t> array_tos<elem_t>::array_tos(elem_t elem0,
   elem_t elem1, elem_t elem2, elem_t elem3, elem_t elem4)
   {
   _data = new elem_t[5];
   _size = 5;
   _space = 5;
   _data[0] = elem0;
   _data[1] = elem1;
   _data[2] = elem2;
   _data[3] = elem3;
   _data[4] = elem4;
   }

   template <class elem_t> array_tos<elem_t>::array_tos(elem_t elem0,
   elem_t elem1, elem_t elem2, elem_t elem3, elem_t elem4, elem_t elem5)
   {
   _data = new elem_t[6];
   _size = 6;
   _space = 6;
   _data[0] = elem0;
   _data[1] = elem1;
   _data[2] = elem2;
   _data[3] = elem3;
   _data[4] = elem4;
   _data[5] = elem5;
   }

   template <class elem_t> array_tos<elem_t>::array_tos(elem_t elem0,
   elem_t elem1, elem_t elem2, elem_t elem3, elem_t elem4, elem_t elem5,
   elem_t elem6)
   {
   _data = new elem_t[7];
   _size = 7;
   _space = 7;
   _data[0] = elem0;
   _data[1] = elem1;
   _data[2] = elem2;
   _data[3] = elem3;
   _data[4] = elem4;
   _data[5] = elem5;
   _data[6] = elem6;
   }

   template <class elem_t> array_tos<elem_t>::array_tos(elem_t elem0,
   elem_t elem1, elem_t elem2, elem_t elem3, elem_t elem4, elem_t elem5,
   elem_t elem6, elem_t elem7)
   {
   _data = new elem_t[8];
_size = 8;
_space = 8;
_data[0] = elem0;
_data[1] = elem1;
_data[2] = elem2;
_data[3] = elem3;
_data[4] = elem4;
_data[5] = elem5;
_data[6] = elem6;
_data[7] = elem7;
}
  */
template <class elem_t> array_tos<elem_t>::~array_tos(void)
{
  delete[] _data;
}

template <class elem_t> void array_tos<elem_t>::more_space(void)
{
  _space = (_space * 2) + 3;
  elem_t *new_data = new elem_t[_space];
  for (size_t elem_num = 0; elem_num < _size; ++elem_num)
    new_data[elem_num] = _data[elem_num];
  delete[] _data;
  _data = new_data;
}

template <class elem_t> size_t array_tos<elem_t>::count(void) const
{
  return _size;
}

template <class elem_t> bool array_tos<elem_t>::is_empty(void) const
{
  return (_size == 0);
}

template <class elem_t> bool array_tos<elem_t>::count_is(
    size_t test_count) const
{
  return (_size == test_count);
}

template <class elem_t> bool array_tos<elem_t>::is_member(elem_t the_elem)
  const
{
  for (size_t elem_num = 0; elem_num < _size; ++elem_num)
  {
    if (_data[elem_num] == the_elem)
      return true;
  }
  return false;
}

template <class elem_t> size_t array_tos<elem_t>::position(elem_t the_elem)
  const
{
  for (size_t elem_num = 0; elem_num < _size; ++elem_num)
  {
    if (_data[elem_num] == the_elem)
      return elem_num;
  }
  return (size_t)-1;
}

template <class elem_t> elem_t array_tos<elem_t>::elem_by_num(
    size_t position) const
{
  if ((position < 0) || (position >= _size))
    return zero((elem_t *)0);
  return _data[position];
}

template <class elem_t> void array_tos<elem_t>::append(elem_t the_elem)
{
  if (_size == _space)
    more_space();
  _data[_size] = the_elem;
  ++_size;
}

template <class elem_t> void array_tos<elem_t>::prepend(elem_t the_elem)
{
  if (_size == _space)
    more_space();
  for (size_t elem_num = _size; elem_num > 0; --elem_num)
    _data[elem_num] = _data[elem_num - 1];
  _data[0] = the_elem;
  ++_size;
  count_tos_internal_handle<elem_t> *follow = this->_handles;
  while (follow != 0)
  {
    ++(follow->index);
    follow = follow->next;
  }
}

template <class elem_t> void array_tos<elem_t>::set_elem_by_num(
    size_t elem_num, elem_t the_elem)
{
  assert(elem_num >= 0);
  if (elem_num >= _size)
  {
    while (_space <= elem_num)
      more_space();
    for (size_t elem_index = _size; elem_index < elem_num; ++elem_index)
      _data[elem_index] = zero((elem_t *)0);
    _data[elem_num] = the_elem;
    _size = elem_num + 1;
  }
  else
  {
    _data[elem_num] = the_elem;
  }
}

template <class elem_t> void array_tos<elem_t>::insert(size_t elem_num,
    elem_t the_elem)
{
  assert(elem_num >= 0);
  if (elem_num >= _size)
  {
    while (_space <= elem_num)
      more_space();
    for (size_t elem_index = _size; elem_index < elem_num; ++elem_index)
      _data[elem_index] = zero((elem_t *)0);
    _data[elem_num] = the_elem;
    _size = elem_num + 1;
    return;
  }
  if (_size == _space)
    more_space();
  for (size_t elem_index = _size; elem_index > elem_num; --elem_index)
    _data[elem_index] = _data[elem_index - 1];
  _data[elem_num] = the_elem;
  ++_size;
  count_tos_internal_handle<elem_t> *follow = this->_handles;
  while (follow != 0)
  {
    if (follow->index >= elem_num)
      ++(follow->index);
    follow = follow->next;
  }
}

template <class elem_t> void array_tos<elem_t>::remove(size_t elem_num)
{
  if ((elem_num < 0) || (elem_num >= _size))
    return;
  --_size;
  for (size_t elem_index = elem_num; elem_index < _size; ++elem_index)
    _data[elem_index] = _data[elem_index + 1];
  _data[_size] = zero((elem_t *)0);
  count_tos_internal_handle<elem_t> *follow = this->_handles;
  while (follow != 0)
  {
    if (follow->index > elem_num)
      --(follow->index);
    follow = follow->next;
  }
  return;
}

template <class elem_t> void array_tos<elem_t>::set_elem_by_handle(
    tos_handle<elem_t> the_handle, elem_t the_elem)
{
  count_tos_internal_handle<elem_t> *elem_ptr =
    (count_tos_internal_handle<elem_t> *)(from_handle(the_handle));
  if (elem_ptr != 0)
    set_elem(elem_ptr->index, the_elem);
}

template <class elem_t> void array_tos<elem_t>::insert_before(
    tos_handle<elem_t> the_handle, elem_t the_elem)
{
  count_tos_internal_handle<elem_t> *elem_ptr =
    (count_tos_internal_handle<elem_t> *)(from_handle(the_handle));
  if (elem_ptr != 0)
    insert(elem_ptr->index, the_elem);
}

template <class elem_t> void array_tos<elem_t>::insert_after(
    tos_handle<elem_t> the_handle, elem_t the_elem)
{
  count_tos_internal_handle<elem_t> *elem_ptr =
    (count_tos_internal_handle<elem_t> *)(from_handle(the_handle));
  if (elem_ptr != 0)
    insert(elem_ptr->index + 1, the_elem);
}

template <class elem_t> void array_tos<elem_t>::remove(
    tos_handle<elem_t> the_handle)
{
  count_tos_internal_handle<elem_t> *elem_ptr =
    (count_tos_internal_handle<elem_t> *)(from_handle(the_handle));
  if (elem_ptr != 0)
    remove(elem_ptr->index);
}

template <class elem_t> elem_t array_tos<elem_t>::pop(void)
{
  if (_size == 0)
    return zero((elem_t *)0);
  --_size;
  elem_t result = _data[0];
  for (size_t elem_num = 0; elem_num < _size; ++elem_num)
    _data[elem_num] = _data[elem_num + 1];
  _data[_size] = zero((elem_t *)0);
  count_tos_internal_handle<elem_t> *follow = this->_handles;
  while (follow != 0)
  {
    if (follow->index > 0)
      --(follow->index);
    follow = follow->next;
  }
  return result;
}

template <class elem_t> elem_t array_tos<elem_t>::tail_pop(void)
{
  if (_size == 0)
    return zero((elem_t *)0);
  --_size;
  elem_t result = _data[_size];
  _data[_size] = zero((elem_t *)0);
  return result;
}

template <class elem_t> void array_tos<elem_t>::clear(void)
{
  for (size_t elem_num = 0; elem_num < _size; ++elem_num)
    _data[elem_num] = zero((elem_t *)0);
  _size = 0;
}

#undef DEFAULT_INIT_SPACE

#endif /* ((!defined(INLINE_ALL_TEMPLATES)) ||
          defined(DOING_TEMPLATE_INLINING)) */
