/* file "slist_tos.cc" */


/*
       Copyright (c) 1996, 1997 Stanford University

       All rights reserved.

       This software is provided under the terms described in
       the "suif_copyright.h" include file.
*/

#include <common/suif_copyright.h>
#include "zero.h"

/*
      This is the implementation of the slist_tos template and related
      templates, which are templates for classes implementing
      totally-ordered sets with simple singly linked lists, for sty,
      the first-level main library of the SUIF system.
*/


#ifndef DOING_TEMPLATE_INLINING

//#define _MODULE_ "libsty"

//#pragma implementation "slist_tos.h"


#include "const_cast.h"
#include "referenced_item.h"
#include "ion/ion.h"
#include <common/i_integer.h>
#include <common/MString.h>
#include "tos.h"
#include "slist_tos.h"

#endif /* not DOING_TEMPLATE_INLINING */


#if ((!defined(INLINE_ALL_TEMPLATES)) || defined(DOING_TEMPLATE_INLINING))

template <class elem_t, class holder_t> slist_tos_base<elem_t, holder_t>::
slist_tos_base(elem_t elem0) :
  _head_e(new holder_t(elem0)),
  _tail_e(0)
  {
    _tail_e = _head_e;
  }

template <class elem_t, class holder_t> slist_tos_base<elem_t, holder_t>::
        slist_tos_base(elem_t elem0, elem_t elem1)
  {
    _tail_e = new holder_t(elem1);
    _head_e = new holder_t(elem0, _tail_e);
  }
/*
template <class elem_t, class holder_t> slist_tos_base<elem_t, holder_t>::
        slist_tos_base(elem_t elem0, elem_t elem1, elem_t elem2)
  {
    _tail_e = new holder_t(elem2);
    holder_t *s1 = new holder_t(elem1, _tail_e);
    _head_e = new holder_t(elem0, s1);
  }

template <class elem_t, class holder_t> slist_tos_base<elem_t, holder_t>::
        slist_tos_base(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3)
  {
    _tail_e = new holder_t(elem3);
    holder_t *s2 = new holder_t(elem2, _tail_e);
    holder_t *s1 = new holder_t(elem1, s2);
    _head_e = new holder_t(elem0, s1);
  }

template <class elem_t, class holder_t> slist_tos_base<elem_t, holder_t>::
        slist_tos_base(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3,
                       elem_t elem4)
  {
    _tail_e = new holder_t(elem4);
    holder_t *s3 = new holder_t(elem3, _tail_e);
    holder_t *s2 = new holder_t(elem2, s3);
    holder_t *s1 = new holder_t(elem1, s2);
    _head_e = new holder_t(elem0, s1);
  }

template <class elem_t, class holder_t> slist_tos_base<elem_t, holder_t>::
        slist_tos_base(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3,
                       elem_t elem4, elem_t elem5)
  {
    _tail_e = new holder_t(elem5);
    holder_t *s4 = new holder_t(elem4, _tail_e);
    holder_t *s3 = new holder_t(elem3, s4);
    holder_t *s2 = new holder_t(elem2, s3);
    holder_t *s1 = new holder_t(elem1, s2);
    _head_e = new holder_t(elem0, s1);
  }

template <class elem_t, class holder_t> slist_tos_base<elem_t, holder_t>::
        slist_tos_base(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3,
                       elem_t elem4, elem_t elem5, elem_t elem6)
  {
    _tail_e = new holder_t(elem6);
    holder_t *s5 = new holder_t(elem5, _tail_e);
    holder_t *s4 = new holder_t(elem4, s5);
    holder_t *s3 = new holder_t(elem3, s4);
    holder_t *s2 = new holder_t(elem2, s3);
    holder_t *s1 = new holder_t(elem1, s2);
    _head_e = new holder_t(elem0, s1);
  }

template <class elem_t, class holder_t> slist_tos_base<elem_t, holder_t>::
        slist_tos_base(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3,
                       elem_t elem4, elem_t elem5, elem_t elem6, elem_t elem7)
  {
    _tail_e = new holder_t(elem7);
    holder_t *s6 = new holder_t(elem6, _tail_e);
    holder_t *s5 = new holder_t(elem5, s6);
    holder_t *s4 = new holder_t(elem4, s5);
    holder_t *s3 = new holder_t(elem3, s4);
    holder_t *s2 = new holder_t(elem2, s3);
    holder_t *s1 = new holder_t(elem1, s2);
    _head_e = new holder_t(elem0, s1);
  }
*/
template <class elem_t, class holder_t> slist_tos_base<elem_t, holder_t>::
        ~slist_tos_base(void)
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
        slist_tos_base<elem_t, holder_t>::count(void) const
  {
    size_t result = 0;
    holder_t *follow = _head_e;
    while (follow != 0)
      {
        ++result;
        follow = follow->next;
      }
    return result;
  }

template <class elem_t, class holder_t> bool
        slist_tos_base<elem_t, holder_t>::is_empty(void) const
  {
    return (_head_e == 0);
  }

template <class elem_t, class holder_t> bool
        slist_tos_base<elem_t, holder_t>::count_is(size_t test_count) const
  {
    size_t total = 0;
    holder_t *follow = _head_e;
    while (follow != 0)
      {
        if (total == test_count)
            return false;
        ++total;
        follow = follow->next;
      }
    return (total == test_count);
  }

template <class elem_t, class holder_t> bool
        slist_tos_base<elem_t, holder_t>::is_member(elem_t the_elem) const
  {
    holder_t *follow = _head_e;
    while (follow != 0)
      {
        if (follow->data == the_elem)
            return true;
        follow = follow->next;
      }
    return false;
  }

template <class elem_t, class holder_t> size_t
        slist_tos_base<elem_t, holder_t>::position(elem_t the_elem) const
  {
    size_t total = 0;
    holder_t *follow = _head_e;
    while (follow != 0)
      {
        if (follow->data == the_elem)
            return total;
        ++total;
        follow = follow->next;
      }
    return (size_t)-1;
  }

template <class elem_t, class holder_t> elem_t
        slist_tos_base<elem_t, holder_t>::elem_by_num(size_t position) const
  {
    size_t total = 0;
    holder_t *follow = _head_e;
    while (follow != 0)
      {
        if (total == position)
            return follow->data;
        ++total;
        follow = follow->next;
      }
    return zero((elem_t *)0);
  }

template <class elem_t, class holder_t> tos_handle<elem_t>
        slist_tos_base<elem_t, holder_t>::lookup_handle(elem_t the_elem) const
  {
    holder_t *follow = _head_e;
    while (follow != 0)
      {
        if (follow->data == the_elem)
            return this->build_handle(follow);
        follow = follow->next;
      }
    return this->build_handle(0);
  }

template <class elem_t, class holder_t> elem_t
        slist_tos_base<elem_t, holder_t>::elem_by_handle(
        tos_handle<elem_t> the_handle) const
  {
    holder_t *elem_ptr = (holder_t *)(from_handle(the_handle));
    if (elem_ptr == 0)
        return zero((elem_t *)0);
    else
        return elem_ptr->data;
  }

template <class elem_t, class holder_t> tos_handle<elem_t>
        slist_tos_base<elem_t, holder_t>::handle_for_num(size_t position)
        const
  {
    size_t total = 0;
    holder_t *follow = _head_e;
    while (follow != 0)
      {
        if (total == position)
            return this->build_handle(follow);
        ++total;
        follow = follow->next;
      }
    return this->build_handle(0);
  }

template <class elem_t, class holder_t> tos_handle<elem_t>
        slist_tos_base<elem_t, holder_t>::next_handle(
                tos_handle<elem_t> the_handle) const
  {
    holder_t *elem_ptr = (holder_t *)(from_handle(the_handle));
    if (elem_ptr == 0)
        return this->build_handle(0);
    else
        return this->build_handle(elem_ptr->next);
  }

template <class elem_t, class holder_t> tos_handle<elem_t>
        slist_tos_base<elem_t, holder_t>::previous_handle(
                tos_handle<elem_t> the_handle) const
  {
    holder_t *elem_ptr = (holder_t *)(from_handle(the_handle));
    if (elem_ptr == 0)
        return this->build_handle(0);
    holder_t *follow = _head_e;
    while (follow != 0)
      {
        if (follow->next == elem_ptr)
            return this->build_handle(follow);
        follow = follow->next;
      }
    return this->build_handle(0);
  }

template <class elem_t, class holder_t> elem_t
        slist_tos_base<elem_t, holder_t>::head(void) const
  {
    if (_head_e == 0)
        return zero((elem_t *)0);
    else
        return _head_e->data;
  }

template <class elem_t, class holder_t> elem_t
        slist_tos_base<elem_t, holder_t>::tail(void) const
  {
    if (_tail_e == 0)
        return zero((elem_t *)0);
    else
        return _tail_e->data;
  }

template <class elem_t, class holder_t> tos_handle<elem_t>
        slist_tos_base<elem_t, holder_t>::head_handle(void) const
  {
    if (_head_e == 0)
        return this->build_handle(0);
    else
        return this->build_handle(_head_e);
  }

template <class elem_t, class holder_t> tos_handle<elem_t>
        slist_tos_base<elem_t, holder_t>::tail_handle(void) const
  {
    if (_tail_e == 0)
        return this->build_handle(0);
    else
        return this->build_handle(_tail_e);
  }

template <class elem_t, class holder_t> void
        slist_tos_base<elem_t, holder_t>::append(elem_t the_elem)
  {
    if (_tail_e == 0)
      {
        _head_e = new holder_t(the_elem);
        _tail_e = _head_e;
      }
    else
      {
        _tail_e->next = new holder_t(the_elem, 0);
        _tail_e = _tail_e->next;
      }
  }

template <class elem_t, class holder_t> void
        slist_tos_base<elem_t, holder_t>::prepend(elem_t the_elem)
  {
    if (_head_e == 0)
      {
        _head_e = new holder_t(the_elem);
        _tail_e = _head_e;
      }
    else
      {
        _head_e = new holder_t(the_elem, _head_e);
      }
  }

template <class elem_t, class holder_t> void
        slist_tos_base<elem_t, holder_t>::set_elem_by_num(size_t elem_num,
                                                          elem_t the_elem)
  {
    assert(elem_num >= 0);
    size_t total = 0;
    holder_t *follow = _head_e;
    while (follow != 0)
      {
        if (total == elem_num)
          {
            follow->data = the_elem;
            return;
          }
        ++total;
        follow = follow->next;
      }
    while (total < elem_num)
      {
        append(zero((elem_t *)0));
        ++total;
      }
    append(the_elem);
  }

template <class elem_t, class holder_t> void
        slist_tos_base<elem_t, holder_t>::insert(size_t elem_num,
                                                 elem_t the_elem)
  {
    assert(elem_num >= 0);
    if (elem_num == 0)
      {
        prepend(the_elem);
        return;
      }
    size_t total = 0;
    holder_t *follow = _head_e;
    while (follow != 0)
      {
        if (total + 1 == elem_num)
          {
            follow->next = new holder_t(the_elem, follow->next);
            if (_tail_e == follow)
                _tail_e = follow->next;
            return;
          }
        ++total;
        follow = follow->next;
      }
    while (total < elem_num)
      {
        append(zero((elem_t *)0));
        ++total;
      }
    append(the_elem);
  }

template <class elem_t, class holder_t> void
        slist_tos_base<elem_t, holder_t>::remove(size_t elem_num)
  {
    if (elem_num == 0)
      {
        pop();
        return;
      }
    size_t total = 0;
    holder_t *follow = _head_e;
    while (follow != 0)
      {
        if (total + 1 == elem_num)
          {
            holder_t *old_e = follow->next;
            if (old_e == 0)
                return;
            follow->next = old_e->next;
            if (_tail_e == old_e)
                _tail_e = follow;
            old_e->remove_ref();
            if (old_e->delete_me())
                delete old_e;
            return;
          }
        ++total;
        follow = follow->next;
      }
  }

template <class elem_t, class holder_t> void
        slist_tos_base<elem_t, holder_t>::set_elem_by_handle(
                tos_handle<elem_t> the_handle, elem_t the_elem)
  {
    holder_t *elem_ptr = (holder_t *)(from_handle(the_handle));
    if (elem_ptr == 0)
        return;
    elem_ptr->data = the_elem;
  }

template <class elem_t, class holder_t> void
        slist_tos_base<elem_t, holder_t>::insert_before(
                tos_handle<elem_t> the_handle, elem_t the_elem)
  {
    holder_t *elem_ptr = (holder_t *)(from_handle(the_handle));
    if (elem_ptr == 0)
        return;
    if (_head_e == elem_ptr)
      {
        _head_e = new holder_t(the_elem, elem_ptr);
        return;
      }
    holder_t *follow = _head_e;
    while (follow != 0)
      {
        if (follow->next == elem_ptr)
          {
            follow->next = new holder_t(the_elem, follow->next);
            return;
          }
        follow = follow->next;
      }
    assert(false);
  }

template <class elem_t, class holder_t> void
        slist_tos_base<elem_t, holder_t>::insert_after(
                tos_handle<elem_t> the_handle, elem_t the_elem)
  {
    holder_t *elem_ptr = (holder_t *)(from_handle(the_handle));
    if (elem_ptr == 0)
        return;
    elem_ptr->next = new holder_t(the_elem, elem_ptr->next);
    if (_tail_e == elem_ptr)
        _tail_e = elem_ptr->next;
  }

template <class elem_t, class holder_t> void
        slist_tos_base<elem_t, holder_t>::remove(tos_handle<elem_t> the_handle)
  {
    holder_t *elem_ptr = (holder_t *)(from_handle(the_handle));
    if (elem_ptr == 0)
        return;
    if (_head_e == elem_ptr)
      {
        _head_e = _head_e->next;
        if (_tail_e == elem_ptr)
            _tail_e = 0;
        elem_ptr->remove_ref();
        if (elem_ptr->delete_me())
            delete elem_ptr;
        return;
      }
    holder_t *follow = _head_e;
    while (follow != 0)
      {
        if (follow->next == elem_ptr)
          {
            follow->next = elem_ptr->next;
            if (_tail_e == elem_ptr)
                _tail_e = follow;
            elem_ptr->remove_ref();
            if (elem_ptr->delete_me())
                delete elem_ptr;
            return;
          }
        follow = follow->next;
      }
    assert(false);
  }

template <class elem_t, class holder_t> elem_t
        slist_tos_base<elem_t, holder_t>::pop(void)
  {
    if (_head_e == 0)
        return zero((elem_t *)0);
    holder_t *old_e = _head_e;
    _head_e = old_e->next;
    if (_head_e == 0)
        _tail_e = 0;
    elem_t result = old_e->data;
    old_e->remove_ref();
    if (old_e->delete_me())
        delete old_e;
    return result;
  }

template <class elem_t, class holder_t> elem_t
        slist_tos_base<elem_t, holder_t>::tail_pop(void)
  {
    if (_head_e == 0)
        return zero((elem_t *)0);
    elem_t result = _tail_e->data;
    if (_head_e == _tail_e)
      {
        _head_e->remove_ref();
        if (_head_e->delete_me())
            delete _head_e;
        _head_e = 0;
        _tail_e = 0;
        return result;
      }
    holder_t *follow = _head_e;
    while (follow != 0)
      {
        if (follow->next == _tail_e)
          {
            follow->next = 0;
            _tail_e->remove_ref();
            if (_tail_e->delete_me())
                delete _tail_e;
            _tail_e = follow;
            return result;
          }
        follow = follow->next;
      }
    assert(false);
    return result;
  }

template <class elem_t, class holder_t> void
        slist_tos_base<elem_t, holder_t>::clear(void)
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
  }

#endif /* ((!defined(INLINE_ALL_TEMPLATES)) ||
           defined(DOING_TEMPLATE_INLINING)) */
