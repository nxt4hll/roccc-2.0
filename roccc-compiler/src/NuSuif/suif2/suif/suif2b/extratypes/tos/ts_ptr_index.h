/* file "ts_ptr_index.h" */


/*
       Copyright (c) 1996, 1997 Stanford University

       All rights reserved.

       This software is provided under the terms described in
       the "suif_copyright.h" include file.
*/

#include <common/suif_copyright.h>


#ifndef STY_TS_PTR_INDEX_H
#define STY_TS_PTR_INDEX_H

//#ifndef SUPPRESS_PRAGMA_INTERFACE
//#pragma interface
//#endif


/*
      This is the definition of the ts_ptr_index template and related
      templates, which are templates for classes implementing mappings
      from pointers to arbitrary types to arbitrary types using the
      tree_string_index algorithm on an ASCII form of the pointer, for
      sty, the first-level main library of the SUIF system.
*/
#include "tree_string_index.h"

template <class key_base_t, class elem_t> class ts_ptr_index :
        public Index<key_base_t *, elem_t>
  {
private:
    tree_string_index<elem_t> _tsi;

public:
    ts_ptr_index(void) : _tsi('0', '9')  { }
    ~ts_ptr_index(void)  { }

    elem_t lookup(key_base_t *the_key) const;
    bool exists(key_base_t *the_key) const;
    index_handle<key_base_t *, elem_t> enter(key_base_t *the_key, elem_t);
    void remove(key_base_t *the_key);

    index_handle<key_base_t *, elem_t> lookup_handle(key_base_t *the_key)
            const;
    elem_t elem(index_handle<key_base_t *, elem_t>) const;
    void remove(index_handle<key_base_t *, elem_t>);

    void clear(void);
  };

#ifdef INLINE_ALL_TEMPLATES
#define DOING_TEMPLATE_INLINING
#ifdef STY_H
#include <sty/ts_ptr_index.cpp>
#else
#include "ts_ptr_index.cpp"
#endif
#undef DOING_TEMPLATE_INLINING
#endif /* INLINE_ALL_TEMPLATES */


#endif /* STY_TS_PTR_INDEX_H */
