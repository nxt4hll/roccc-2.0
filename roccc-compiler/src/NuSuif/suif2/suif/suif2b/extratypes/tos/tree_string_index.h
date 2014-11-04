/* file "tree_string_index.h" */


/*
       Copyright (c) 1996, 1997 Stanford University

       All rights reserved.

       This software is provided under the terms described in
       the "suif_copyright.h" include file.
*/

#include <common/suif_copyright.h>


#ifndef STY_TREE_STRING_INDEX_H
#define STY_TREE_STRING_INDEX_H

//#ifndef SUPPRESS_PRAGMA_INTERFACE
//#pragma interface
//#endif


/*
      This is the definition of the tree_string_index template and
      related templates, which are templates for classes implementing
      mappings from character strings to arbitrary types using a tree
      representation for fast lookups, for sty, the first-level main
      library of the SUIF system.
*/

#include "index.h"
#include "zero.h"

template <class elem_t> class tsi_node;
template <class elem_t> class tsi_entry;

template <class elem_t> class tree_string_index : public
        Index<const char *, elem_t>
  {
    friend class tsi_node<elem_t>;

private:
    tsi_node<elem_t> *_root_node;
    size_t _list_size;
    char _expected_lower;
    char _expected_upper;

    tsi_node<elem_t> *follow_prefix_match(const char *string,
                                          size_t *prefix_length) const;
    void internal_development_dump(char **buffer, size_t *buf_size,
                                   size_t position,
                                   tsi_node<elem_t> *base_node, ion *ip,
                                   void (*node_func)(elem_t data, ion *ip),
                                   bool show_internals);
    void get_development_stats(tsi_node<elem_t> *base_node,
                               size_t *entry_count,
                               size_t *substring_node_count,
                               size_t *char_list_node_count,
                               size_t *char_table_node_count,
                               size_t *leaf_node_count,
                               size_t *substring_char_count,
                               size_t *char_list_place_count,
                               size_t *char_list_used_count,
                               size_t *char_table_place_count,
                               size_t *char_table_used_count);
    tsi_entry<elem_t> *lookup_entry(const char *the_key) const;

public:
    tree_string_index(char expected_lower = ' ', char expected_upper = '~',
                      size_t list_size = 4);
    ~tree_string_index(void);

    elem_t lookup(const char *the_key) const;
    elem_t lookup_prefix(const char *the_key, const char **suffix) const;
    bool exists(const char *the_key) const;
    index_handle<const char *, elem_t> enter(const char *the_key, elem_t);
    void remove(const char *the_key);

    index_handle<const char *, elem_t> lookup_handle(const char *the_key)
            const;
    elem_t elem(index_handle<const char *, elem_t>) const;
    void remove(index_handle<const char *, elem_t>);

    void clear(void);

    void development_dump(ion *ip = stderr_ion,
                          void (*node_func)(elem_t data, ion *ip) = 0);
    void development_internals_dump(ion *ip = stderr_ion);
    void development_stats_dump(ion *ip = stderr_ion);
  };


#ifndef ANTIQUE_CPP
/*
 *  OPTIMIZATION:  To limit duplication of generated code, we put in
 *  an automatic optimization that reuses the ``void *'' instantiation
 *  of tree_string index plus appropriate pointer casts for all
 *  tree_string_index instantiations of pointer types.
 */

class void_holder
  {
public:
    void *v;

    void_holder(void *init_v) : v(init_v)  { }
    void_holder(const void_holder &other) : v(other.v)  { }
    ~void_holder(void)  { }

    const void_holder &operator=(const void_holder &other)  {
      v = other.v; return(*this); }
    bool operator==(const void_holder other)  { return (v == other.v); }
    bool operator!=(const void_holder other)  { return (v != other.v); }

    operator void *(void) const  { return v; }
    template <class elem_t> operator elem_t *(void) const
      { return (elem_t *)v; }
  };

inline void_holder zero(void_holder *)  { return 0; }

extern void (*indirection_function)(void *, ion *);
extern void indirect_print(void_holder data, ion *ip);

template <class elem_t> class tree_string_index<elem_t *> :
        public Index<const char *, elem_t *>
  {
private:
    tree_string_index<void_holder> _wrapped;

public:
    tree_string_index(char expected_lower = ' ', char expected_upper = '~',
                      size_t list_size = 4) :
            _wrapped(expected_lower, expected_upper, list_size)
      { }
    ~tree_string_index(void)  { }

    elem_t *lookup(const char *the_key) const
      { return (elem_t *)(_wrapped.lookup(the_key)); }
    elem_t *lookup_prefix(const char *the_key, const char **suffix) const
      { return (elem_t *)(_wrapped.lookup_prefix(the_key, suffix)); }
    bool exists(const char *the_key) const
      { return _wrapped.exists(the_key); }
    index_handle<const char *, elem_t *> enter(const char *the_key,
                                               elem_t *the_elem)
      {
        return build_handle(_wrapped.enter(the_key, (void *)the_elem).
                raw_referenced_item());
      }
    void remove(const char *the_key)  { _wrapped.remove(the_key); }

    index_handle<const char *, elem_t *> lookup_handle(const char *the_key)
            const
      {
        return build_handle(_wrapped.lookup_handle(the_key).
                raw_referenced_item());
      }
    elem_t *elem(index_handle<const char *, elem_t *> the_handle) const
      {
        index_handle<const char *, void_holder> void_handle;
        void_handle.set_raw_referenced_item(the_handle.raw_referenced_item());
        return (elem_t *)(_wrapped.elem(void_handle));
      }
    void remove(index_handle<const char *, elem_t *> the_handle)
      {
        index_handle<const char *, void_holder> void_handle;
        void_handle.set_raw_referenced_item(the_handle.raw_referenced_item());
        _wrapped.remove(void_handle);
      }

    void clear(void)  { _wrapped.clear(); }

    void development_dump(ion *ip = stderr_ion,
                          void (*node_func)(elem_t *data, ion *ip) = 0)
      {
        void (*orig_func)(void *, ion *) = indirection_function;
        indirection_function = (void (*)(void *, ion *))node_func;
        _wrapped.development_dump(ip, indirect_print);
        indirection_function = orig_func;
      }
    void development_internals_dump(ion *ip = stderr_ion)
      { _wrapped.development_internals_dump(ip); }
    void development_stats_dump(ion *ip = stderr_ion)
      { _wrapped.development_stats_dump(ip); }
  };
#endif /* not ANTIQUE_CPP */


#ifdef INLINE_ALL_TEMPLATES
#define DOING_TEMPLATE_INLINING
#ifdef STY_H
#include <sty/tree_string_index.cpp>
#else
#include "tree_string_index.cpp"
#endif
#undef DOING_TEMPLATE_INLINING
#endif /* INLINE_ALL_TEMPLATES */


#endif /* STY_TREE_STRING_INDEX_H */
