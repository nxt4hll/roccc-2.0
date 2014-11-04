/* file "hash_index.h" */


/*
       Copyright (c) 1996, 1997 Stanford University

       All rights reserved.

       This software is provided under the terms described in
       the "suif_copyright.h" include file.
*/

#include <common/suif_copyright.h>


#ifndef STY_HASH_INDEX_H
#define STY_HASH_INDEX_H

//#ifndef SUPPRESS_PRAGMA_INTERFACE
//#pragma interface
//#endif


/*
      This is the definition of the hash_index template and related
      templates, which are templates for classes implementing mappings
      from arbitrary keys to arbitrary types using hash tables with
      user-supplied hashing functions, for sty, the first-level main
      library of the SUIF system.
*/


template <class key_t, class elem_t> class hash_bin_type;

template <class key_t, class elem_t> class hash_index :
        public Index<key_t, elem_t>
  {
    typedef size_t hash_function_type(key_t the_key, size_t table_size);

private:
    hash_function_type *_hash_function;
    size_t _table_size;
    hash_bin_type<key_t, elem_t> **_table;

public:
    hash_index(hash_function_type *hash_function, size_t table_size);
    ~hash_index(void);

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
#include <sty/hash_index.cpp>
#else
#include "hash_index.cpp"
#endif
#undef DOING_TEMPLATE_INLINING
#endif /* INLINE_ALL_TEMPLATES */


#endif /* STY_HASH_INDEX_H */
