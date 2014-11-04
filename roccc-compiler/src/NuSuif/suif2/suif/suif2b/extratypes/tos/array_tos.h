/* file "array_tos.h" */


/*
   Copyright (c) 1996, 1997 Stanford University

   All rights reserved.

   This software is provided under the terms described in
   the "suif_copyright.h" include file.
 */

#include <common/suif_copyright.h>


#ifndef STY_ARRAY_TOS_H
#define STY_ARRAY_TOS_H

#include "special_ro_tos.h"

//#ifndef SUPPRESS_PRAGMA_INTERFACE
//#pragma interface
//#endif


/*
   This is the definition of the array_tos template and related
   templates, which are templates for classes implementing
   totally-ordered sets with simple arrays, for sty, the
   first-level main library of the SUIF system.
 */


template <class elem_t> class array_tos : protected /*public*/ count_based_tos<elem_t>
{
  private:
    elem_t *_data;
    size_t _size;
    size_t _space;

    void more_space(void);
    array_tos(const array_tos &) {assert(0); }
    array_tos &operator=(const array_tos &) {assert(0); return(*this);}
  public:
    array_tos(void);
    array_tos(elem_t elem0);
    array_tos(elem_t elem0, elem_t elem1);
    /*
       array_tos(elem_t elem0, elem_t elem1, elem_t elem2);
       array_tos(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3);
       array_tos(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3,
       elem_t elem4);
       array_tos(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3,
       elem_t elem4, elem_t elem5);
       array_tos(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3,
       elem_t elem4, elem_t elem5, elem_t elem6);
       array_tos(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3,
       elem_t elem4, elem_t elem5, elem_t elem6, elem_t elem7);
     */
    virtual ~array_tos(void);

    virtual size_t count(void) const;
    virtual bool is_empty(void) const;
    virtual bool count_is(size_t test_count) const;

    virtual bool is_member(elem_t) const;
    virtual size_t position(elem_t) const;
    virtual elem_t elem_by_num(size_t position) const;

    virtual void append(elem_t);
    virtual void prepend(elem_t);

    virtual void set_elem_by_num(size_t elem_num, elem_t);
    virtual void insert(size_t elem_num, elem_t);
    virtual void remove(size_t elem_num);

    virtual void set_elem_by_handle(tos_handle<elem_t>, elem_t);
    virtual void insert_before(tos_handle<elem_t>, elem_t);
    virtual void insert_after(tos_handle<elem_t>, elem_t);
    virtual void remove(tos_handle<elem_t>);

    virtual elem_t pop(void);
    virtual elem_t tail_pop(void);

    virtual void clear(void);

    virtual tos_setter<elem_t> operator[](size_t elem_num)
    { return tos_setter<elem_t>(this, elem_num); }

    /* array_tos-specific methods */
    virtual elem_t *storage(void)  { return _data; }
    virtual void force_space(size_t new_space_requirement)
    {
      while (_space < new_space_requirement)
        more_space();
    }
};

#ifdef INLINE_ALL_TEMPLATES
#define DOING_TEMPLATE_INLINING
#ifdef STY_H
#include <sty/array_tos.cpp>
#else
#include "array_tos.cpp"
#endif
#undef DOING_TEMPLATE_INLINING
#endif /* INLINE_ALL_TEMPLATES */


#endif /* STY_ARRAY_TOS_H */
