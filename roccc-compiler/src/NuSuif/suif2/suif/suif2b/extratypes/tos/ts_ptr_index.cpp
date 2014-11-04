/* file "ts_ptr_index.cc" */


/*
       Copyright (c) 1996, 1997 Stanford University

       All rights reserved.

       This software is provided under the terms described in
       the "suif_copyright.h" include file.
*/

#include <common/suif_copyright.h>


/*
      This is the implementation of the ts_ptr_index template and
      related templates, which are templates for classes implementing
      mappings from pointers to arbitrary types to arbitrary types
      using the tree_string_index algorithm on an ASCII form of the
      pointer, for sty, the first-level main library of the SUIF
      system.
*/


#ifndef DOING_TEMPLATE_INLINING

#include <common/machine_dependent.h>
#include "referenced_item.h"
#include "ion/ion.h"
#include <common/i_integer.h>
#include "index.h"
#include "tree_string_index.h"
#include "ts_ptr_index.h"

#endif /* not DOING_TEMPLATE_INLINING */


#if ((!defined(INLINE_ALL_TEMPLATES)) || defined(DOING_TEMPLATE_INLINING))

template <class key_base_t, class elem_t> elem_t
        ts_ptr_index<key_base_t, elem_t>::lookup(key_base_t *the_key) const
  {
    char buffer[100];

    sprintf(buffer, "%p", the_key);
    return _tsi.lookup(buffer);
  }

template <class key_base_t, class elem_t> bool
        ts_ptr_index<key_base_t, elem_t>::exists(key_base_t *the_key) const
  {
    char buffer[100];

    sprintf(buffer, "%p", the_key);
    return _tsi.exists(buffer);
  }

template <class key_base_t, class elem_t> index_handle<key_base_t *, elem_t>
        ts_ptr_index<key_base_t, elem_t>::enter(key_base_t *the_key,
                elem_t the_elem)
  {
    char buffer[100];

    sprintf(buffer, "%p", the_key);
    index_handle<const char *, elem_t> tsi_handle =
            _tsi.enter(buffer, the_elem);
    return build_handle(tsi_handle.raw_referenced_item());
  }

template <class key_base_t, class elem_t> void
        ts_ptr_index<key_base_t, elem_t>::remove(key_base_t *the_key)
  {
    char buffer[100];

    sprintf(buffer, "%p", the_key);
    _tsi.remove(buffer);
  }

template <class key_base_t, class elem_t> index_handle<key_base_t *, elem_t>
        ts_ptr_index<key_base_t, elem_t>::lookup_handle(key_base_t *the_key)
        const
  {
    char buffer[100];

    sprintf(buffer, "%p", the_key);
    index_handle<const char *, elem_t> tsi_handle = _tsi.lookup_handle(buffer);
    return build_handle(tsi_handle.raw_referenced_item());
  }

template <class key_base_t, class elem_t> elem_t
        ts_ptr_index<key_base_t, elem_t>::elem(
                index_handle<key_base_t *, elem_t> the_handle) const
  {
    index_handle<const char *, elem_t> tsi_handle;
    tsi_handle.set_raw_referenced_item(from_handle(the_handle));
    return _tsi.elem(tsi_handle);
  }

template <class key_base_t, class elem_t> void
        ts_ptr_index<key_base_t, elem_t>::remove(
                index_handle<key_base_t *, elem_t> the_handle)
  {
    index_handle<const char *, elem_t> tsi_handle;
    tsi_handle.set_raw_referenced_item(from_handle(the_handle));
    _tsi.remove(tsi_handle);
  }

template <class key_base_t, class elem_t> void
        ts_ptr_index<key_base_t, elem_t>::clear(void)
  {
    _tsi.clear();
  }

#endif /* ((!defined(INLINE_ALL_TEMPLATES)) ||
           defined(DOING_TEMPLATE_INLINING)) */
