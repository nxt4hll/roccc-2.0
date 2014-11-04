/* file "hash_index.cc" */


/*
       Copyright (c) 1996, 1997 Stanford University

       All rights reserved.

       This software is provided under the terms described in
       the "suif_copyright.h" include file.
*/

#include <common/suif_copyright.h>


/*
      This is the implementation of the hash_index template and
      related templates, which are templates for classes implementing
      mappings from arbitrary keys to arbitrary types using hash
      tables with user-supplied hashing functions, for sty, the
      first-level main library of the SUIF system.
*/


#ifndef DOING_TEMPLATE_INLINING

//#define _MODULE_ "libsty"

//#pragma implementation "hash_index.h"


#include <common/machine_dependent.h>
#include "const_cast.h"
#include "referenced_item.h"
#include "ion/ion.h"
#include <common/i_integer.h>
#include "tos.h"
#include "index.h"
#include "hash_index.h"

#endif /* not DOING_TEMPLATE_INLINING */


#if ((!defined(INLINE_ALL_TEMPLATES)) || defined(DOING_TEMPLATE_INLINING))

template <class key_t, class elem_t> class hash_bin_type :
        public referenced_item
  {
private:
    size_t _ref_count;

public:
    key_t the_key;
    elem_t the_elem;
    hash_bin_type<key_t, elem_t> *next;

    hash_bin_type(void) : _ref_count(1), the_key(zero((key_t *)0)),
            next(0)
      { }
    hash_bin_type(key_t init_key, elem_t init_elem) : _ref_count(1),
	  the_key(init_key), the_elem(init_elem), next(0)  { }
    ~hash_bin_type(void)
      { suif_assert((next == 0) && (_ref_count == 0)); }

    void add_ref(void)
      {
        if (_ref_count < SIZE_T_MAX)
            ++_ref_count;
      }
    void remove_ref(void)
      {
        assert(_ref_count > 0);
        if (_ref_count < SIZE_T_MAX)
            --_ref_count;
      }
    bool delete_me(void)  { return (_ref_count == 0); }

    static void kill(hash_bin_type<key_t, elem_t> *to_kill)
      {
        if (to_kill->next != 0)
          {
            kill(to_kill->next);
            to_kill->next = 0;
          }
        to_kill->remove_ref();
        if (to_kill->delete_me())
            delete to_kill;
      }
  };


template <class key_t, class elem_t> hash_index<key_t, elem_t>::hash_index(
        size_t (*hash_function)(key_t the_key, size_t table_size),
        size_t table_size)
  {
    assert(table_size > 0);
    _hash_function = hash_function;
    _table_size = table_size;
    _table = new hash_bin_type<key_t, elem_t> *[table_size];
    for (size_t bin_num = 0; bin_num < table_size; ++bin_num)
        _table[bin_num] = 0;
  }

template <class key_t, class elem_t> hash_index<key_t, elem_t>::
        ~hash_index(void)
  {
    for (size_t bin_num = 0; bin_num < _table_size; ++bin_num)
      {
        if (_table[bin_num] != 0)
            hash_bin_type<key_t, elem_t>::kill(_table[bin_num]);
      }
    delete[] _table;
  }

template <class key_t, class elem_t> elem_t hash_index<key_t, elem_t>::lookup(
        key_t the_key) const
  {
    size_t bin_num = (*_hash_function)(the_key, _table_size);
    assert((bin_num >= 0) && (bin_num < _table_size));
    hash_bin_type<key_t, elem_t> *follow = _table[bin_num];
    while (follow != 0)
      {
        if (follow->the_key == the_key)
            return follow->the_elem;
        follow = follow->next;
      }
    return zero((elem_t *)0);
  }

template <class key_t, class elem_t> bool hash_index<key_t, elem_t>::exists(
        key_t the_key) const
  {
    size_t bin_num = (*_hash_function)(the_key, _table_size);
    assert((bin_num >= 0) && (bin_num < _table_size));
    hash_bin_type<key_t, elem_t> *follow = _table[bin_num];
    while (follow != 0)
      {
        if (follow->the_key == the_key)
            return true;
        follow = follow->next;
      }
    return false;
  }

template <class key_t, class elem_t> index_handle<key_t, elem_t>
        hash_index<key_t, elem_t>::enter(key_t the_key, elem_t the_elem)
  {
    size_t bin_num = (*_hash_function)(the_key, _table_size);
    assert((bin_num >= 0) && (bin_num < _table_size));
    hash_bin_type<key_t, elem_t> *new_bin =
            new hash_bin_type<key_t, elem_t>(the_key, the_elem);
    hash_bin_type<key_t, elem_t> *follow = _table[bin_num];
    if (follow == 0)
      {
        _table[bin_num] = new_bin;
      }
    else
      {
        while (follow->next != 0)
          {
            assert(follow->the_key != the_key);
            follow = follow->next;
          }
        follow->next = new_bin;
      }
    return this->build_handle(new_bin);
  }

template <class key_t, class elem_t> void hash_index<key_t, elem_t>::remove(
        key_t the_key)
  {
    size_t bin_num = (*_hash_function)(the_key, _table_size);
    assert((bin_num >= 0) && (bin_num < _table_size));
    hash_bin_type<key_t, elem_t> *follow = _table[bin_num];
    if (follow == 0)
        return;
    if (follow->the_key == the_key)
      {
        _table[bin_num] = follow->next;
        follow->next = 0;
        hash_bin_type<key_t, elem_t>::kill(follow);
        return;
      }
    while (follow->next != 0)
      {
        if (follow->next->the_key == the_key)
          {
            hash_bin_type<key_t, elem_t> *old_bin = follow->next;
            follow->next = old_bin->next;
            old_bin->next = 0;
            hash_bin_type<key_t, elem_t>::kill(old_bin);
            return;
          }
        follow = follow->next;
      }
  }

template <class key_t, class elem_t> index_handle<key_t, elem_t>
        hash_index<key_t, elem_t>::lookup_handle(key_t the_key) const
  {
    size_t bin_num = (*_hash_function)(the_key, _table_size);
    assert((bin_num >= 0) && (bin_num < _table_size));
    hash_bin_type<key_t, elem_t> *follow = _table[bin_num];
    while (follow != 0)
      {
        if (follow->the_key == the_key)
            return this->build_handle(follow);
        follow = follow->next;
      }
    return this->build_handle(0);
  }

template <class key_t, class elem_t> elem_t hash_index<key_t, elem_t>::elem(
        index_handle<key_t, elem_t> the_handle) const
  {
    hash_bin_type<key_t, elem_t> *this_bin =
            (hash_bin_type<key_t, elem_t> *)(from_handle(the_handle));
    assert(this_bin != 0);
    return this_bin->the_elem;
  }

template <class key_t, class elem_t> void hash_index<key_t, elem_t>::remove(
        index_handle<key_t, elem_t> the_handle)
  {
    hash_bin_type<key_t, elem_t> *this_bin =
            (hash_bin_type<key_t, elem_t> *)(from_handle(the_handle));
    assert(this_bin != 0);
    if (this_bin->next != 0)
      {
        hash_bin_type<key_t, elem_t> *old_bin = this_bin->next;
        this_bin->the_key = old_bin->the_key;
        this_bin->the_elem = old_bin->the_elem;
        this_bin->next = old_bin->next;
        old_bin->next = 0;
        hash_bin_type<key_t, elem_t>::kill(old_bin);
      }
    else
      {
        size_t bin_num = (*_hash_function)(this_bin->the_key, _table_size);
        hash_bin_type<key_t, elem_t> *first_bin = _table[bin_num];
        assert(first_bin != 0);
        if (first_bin == this_bin)
          {
            _table[bin_num] = 0;
          }
        else
          {
            this_bin->the_key = first_bin->the_key;
            this_bin->the_elem = first_bin->the_elem;
            _table[bin_num] = first_bin->next;
            first_bin->next = 0;
          }
        hash_bin_type<key_t, elem_t>::kill(first_bin);
      }
  }

template <class key_t, class elem_t> void hash_index<key_t, elem_t>::
        clear(void)
  {
    for (size_t bin_num = 0; bin_num < _table_size; ++bin_num)
      {
	if (_table[bin_num] != 0)
	  {
	    hash_bin_type<key_t, elem_t>::kill(_table[bin_num]);
	    _table[bin_num] = 0;
	  }
      }
  }

#endif /* ((!defined(INLINE_ALL_TEMPLATES)) ||
           defined(DOING_TEMPLATE_INLINING)) */
