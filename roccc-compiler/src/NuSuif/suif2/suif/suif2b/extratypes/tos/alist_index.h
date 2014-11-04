/* file "alist_index.h" */


/*
       Copyright (c) 1996, 1997 Stanford University

       All rights reserved.

       This software is provided under the terms described in
       the "suif_copyright.h" include file.
*/

#include <common/suif_copyright.h>


#ifndef STY_ALIST_INDEX_H
#define STY_ALIST_INDEX_H

//#ifndef SUPPRESS_PRAGMA_INTERFACE
//#pragma interface
//#endif


/*
      This is the definition of the alist_index template and related
      templates, which are templates for classes implementing mappings
      from arbitrary keys to arbitrary types using a simple linked
      list of association pairs, for sty, the first-level main library
      of the SUIF system.
*/


template <class key_t, class elem_t> class alist_item
  {
public:
    key_t the_key;
    elem_t the_elem;

    alist_item(key_t init_key, elem_t init_elem) : the_key(init_key),
            the_elem(init_elem)  { }
    alist_item(const alist_item &other) : the_key(other.the_key),
            the_elem(other.the_elem)  { }
    ~alist_item(void)  { }

    void operator=(const alist_item &other)
      { the_key = other.the_key; the_elem = other.the_elem; }
    bool operator==(const alist_item &other)
      { return (the_key == other.the_key); }
    bool operator!=(const alist_item &other)
      { return (the_key != other.the_key); }
  };

template <class key_t, class elem_t> inline alist_item<key_t, elem_t>
        zero(alist_item<key_t, elem_t> *)
  {
    return alist_item<key_t, elem_t>(zero((key_t *)0),
                                     zero((elem_t *)0));
  }

template <class key_t, class elem_t> class alist_holder :
        public referenced_item
  {
private:
    size_t _ref_count;

public:
    alist_item<key_t, elem_t> data;
    alist_holder<key_t, elem_t> *next;
    alist_holder<key_t, elem_t> *previous;

    alist_holder(alist_item<key_t, elem_t> init_data,
                 alist_holder<key_t, elem_t> *init_next = 0,
                 alist_holder<key_t, elem_t> *init_previous = 0) :
            _ref_count(1), data(init_data), next(init_next),
            previous(init_previous)  { }
    ~alist_holder(void)  { suif_assert(_ref_count == 0); }

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
  };

template <class key_t, class elem_t> class alist_index :
        public Index<key_t, elem_t>
  {
private:
    dlist_tos_base<alist_item<key_t, elem_t>,
                   alist_holder<key_t, elem_t> > _list;

public:
    alist_index(void)  { }
    ~alist_index(void)  { }

    elem_t lookup(key_t the_key) const;
    bool exists(key_t the_key) const;
    index_handle<key_t, elem_t> enter(key_t the_key, elem_t);
    void remove(key_t the_key);

    index_handle<key_t, elem_t> lookup_handle(key_t the_key) const;
    elem_t elem(index_handle<key_t, elem_t>) const;
    void remove(index_handle<key_t, elem_t>);

    void clear(void);
  };

#ifdef INLINE_ALL_TEMPLATES
#define DOING_TEMPLATE_INLINING
#ifdef STY_H
#include <sty/alist_index.cpp>
#else
#include "alist_index.cpp"
#endif
#undef DOING_TEMPLATE_INLINING
#endif /* INLINE_ALL_TEMPLATES */


#endif /* STY_ALIST_INDEX_H */
