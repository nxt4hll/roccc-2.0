/* file "alist_index.cc" */


/*
       Copyright (c) 1996, 1997 Stanford University

       All rights reserved.

       This software is provided under the terms described in
       the "suif_copyright.h" include file.
*/

#include <common/suif_copyright.h>


/*
      This is the implementation of the alist_index template and
      related templates, which are templates for classes implementing
      mappings from arbitrary keys to arbitrary types using a simple
      linked list of association pairs, for sty, the first-level main
      library of the SUIF system.
*/


#ifndef DOING_TEMPLATE_INLINING

//#define _MODULE_ "libsty"

//#pragma implementation "alist_index.h"


#include <common/machine_dependent.h>
#include "const_cast.h"
#include "referenced_item.h"
#include "ion/ion.h"
#include <common/i_integer.h>
#include "tos.h"
#include "dlist_tos.h"
#include "index.h"
#include "alist_index.h"

#endif /* not DOING_TEMPLATE_INLINING */


#if ((!defined(INLINE_ALL_TEMPLATES)) || defined(DOING_TEMPLATE_INLINING))

template <class key_t, class elem_t> elem_t alist_index<key_t, elem_t>::
        lookup(key_t the_key) const
  {
    tos_handle<alist_item<key_t, elem_t> > follow = _list.handle_for_num(0);
    while (!follow.is_null())
      {
        alist_item<key_t, elem_t> this_item = _list.elem(follow);
        if (this_item.the_key == the_key)
            return this_item.the_elem;
        follow = _list.next_handle(follow);
      }
    return zero((elem_t *)0);
  }

template <class key_t, class elem_t> bool alist_index<key_t, elem_t>::
        exists(key_t the_key) const
  {
    tos_handle<alist_item<key_t, elem_t> > follow = _list.handle_for_num(0);
    while (!follow.is_null())
      {
        alist_item<key_t, elem_t> this_item = _list.elem(follow);
        if (this_item.the_key == the_key)
            return true;
        follow = _list.next_handle(follow);
      }
    return false;
  }

template <class key_t, class elem_t> index_handle<key_t, elem_t>
        alist_index<key_t, elem_t>::enter(key_t the_key, elem_t the_elem)
  {
    assert(!exists(the_key));
    _list.prepend(alist_item<key_t, elem_t>(the_key, the_elem));
    return this->build_handle(_list.handle_for_num(0).raw_referenced_item());
  }

template <class key_t, class elem_t> void alist_index<key_t, elem_t>::
        remove(key_t the_key)
  {
    tos_handle<alist_item<key_t, elem_t> > follow = _list.handle_for_num(0);
    while (!follow.is_null())
      {
        alist_item<key_t, elem_t> this_item = _list.elem(follow);
        if (this_item.the_key == the_key)
          {
            _list.remove(follow);
            return;
          }
        follow = _list.next_handle(follow);
      }
  }

template <class key_t, class elem_t> index_handle<key_t, elem_t>
        alist_index<key_t, elem_t>::lookup_handle(key_t the_key) const
  {
    tos_handle<alist_item<key_t, elem_t> > follow = _list.handle_for_num(0);
    while (!follow.is_null())
      {
        alist_item<key_t, elem_t> this_item = _list.elem(follow);
        if (this_item.the_key == the_key)
            return this->build_handle(follow.raw_referenced_item());
        follow = _list.next_handle(follow);
      }
    return this->build_handle(0);
  }

template <class key_t, class elem_t> elem_t alist_index<key_t, elem_t>::
        elem(index_handle<key_t, elem_t> the_handle) const
  {
    alist_holder<key_t, elem_t> *holder =
            (alist_holder<key_t, elem_t> *)(from_handle(the_handle));
    assert(holder != 0);
    return holder->data.the_elem;
  }

template <class key_t, class elem_t> void alist_index<key_t, elem_t>::
        remove(index_handle<key_t, elem_t> the_handle)
  {
    alist_holder<key_t, elem_t> *holder =
            (alist_holder<key_t, elem_t> *)(from_handle(the_handle));
    assert(holder != 0);
    tos_handle<alist_item<key_t, elem_t> > list_handle = holder;
    _list.remove(list_handle);
  }

template <class key_t, class elem_t> void alist_index<key_t, elem_t>::
        clear(void)
  {
    _list.clear();
  }

#endif /* ((!defined(INLINE_ALL_TEMPLATES)) ||
           defined(DOING_TEMPLATE_INLINING)) */
