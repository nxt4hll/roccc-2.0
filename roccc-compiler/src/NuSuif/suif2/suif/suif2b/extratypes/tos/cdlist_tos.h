/* file "cdlist_tos.h" */


/*
   Copyright (c) 1996, 1997 Stanford University

   All rights reserved.

   This software is provided under the terms described in
   the "suif_copyright.h" include file.
 */

#include <common/suif_copyright.h>


#ifndef STY_CDLIST_TOS_H
#define STY_CDLIST_TOS_H

//#ifndef SUPPRESS_PRAGMA_INTERFACE
//#pragma interface
//#endif


/*
   This is the definition of the cdlist_tos template and related
   templates, which are templates for classes implementing
   totally-ordered sets with doubly linked lists plus cache
   pointers, for sty, the first-level main library of the SUIF
   system.
 */


/*
 *  The difference between dlist_tos and cdlist_tos is that the later
 *  adds a pointer to the wrapper for the most recently accessed
 *  element plus a cache of that element's index and a record of the
 *  total number of elements in the list.  That makes functions that
 *  access elements by number much more efficient when the access
 *  pattern has locality (such as when iterating sequentially through
 *  all the integers), and makes returning the total number of
 *  elements more efficient.  It makes these sequential accesses by
 *  number and calculation of the total number of elements O(1)
 *  complexity operations instead of linear in the number of elements,
 *  at the cost of a constant factor to most functions that access
 *  elements for the additional overhead, plus a constant amount of
 *  space per list.
 */

template <class elem_t> class cdlist_e_tos : public referenced_item
{
  private:
    size_t _ref_count;

  public:
    elem_t data;
    cdlist_e_tos<elem_t> *next;
    cdlist_e_tos<elem_t> *previous;

    cdlist_e_tos(elem_t init_data, cdlist_e_tos<elem_t> *init_next = 0,
        cdlist_e_tos<elem_t> *init_previous = 0) : _ref_count(1),
                                                   data(init_data), next(init_next), previous(init_previous)  { }
    virtual ~cdlist_e_tos(void)  { suif_assert(_ref_count == 0); }

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

template <class elem_t, class holder_t> class cdlist_tos_base :
  public tos<elem_t>
{
  private:
    holder_t *_head_e;
    holder_t *_tail_e;
    holder_t *_cached_e;
    size_t _cached_index;
    size_t _cached_size;

  public:
    cdlist_tos_base(void);
    cdlist_tos_base(elem_t elem0);
    cdlist_tos_base(elem_t elem0, elem_t elem1);
    /*
       cdlist_tos_base(elem_t elem0, elem_t elem1, elem_t elem2);
       cdlist_tos_base(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3);
       cdlist_tos_base(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3,
       elem_t elem4);
       cdlist_tos_base(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3,
       elem_t elem4, elem_t elem5);
       cdlist_tos_base(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3,
       elem_t elem4, elem_t elem5, elem_t elem6);
       cdlist_tos_base(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3,
       elem_t elem4, elem_t elem5, elem_t elem6, elem_t elem7);
     */
    virtual ~cdlist_tos_base(void);

    virtual size_t count(void) const;
    virtual bool is_empty(void) const;
    virtual bool count_is(size_t test_count) const;

    virtual bool is_member(elem_t) const;
    virtual size_t position(elem_t) const;
    virtual elem_t elem_by_num(size_t position) const;
    virtual tos_handle<elem_t> lookup_handle(elem_t) const;
    virtual elem_t elem_by_handle(tos_handle<elem_t>) const;
    virtual tos_handle<elem_t> handle_for_num(size_t position) const;
    virtual tos_handle<elem_t> next_handle(tos_handle<elem_t>) const;
    virtual tos_handle<elem_t> previous_handle(tos_handle<elem_t>) const;
    virtual elem_t head(void) const;
    virtual elem_t tail(void) const;
    virtual tos_handle<elem_t> head_handle(void) const;
    virtual tos_handle<elem_t> tail_handle(void) const;

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
    { return tos_setter<elem_t>(this, handle_for_num(elem_num)); }
};

template <class elem_t> class cdlist_tos :
  public cdlist_tos_base<elem_t, cdlist_e_tos<elem_t> >
{
  public:
    cdlist_tos(void)  { }
    cdlist_tos(elem_t elem0) :
      cdlist_tos_base<elem_t, cdlist_e_tos<elem_t> >(elem0)  { }
    cdlist_tos(elem_t elem0, elem_t elem1) :
      cdlist_tos_base<elem_t, cdlist_e_tos<elem_t> >(elem0, elem1)  { }
    cdlist_tos(elem_t elem0, elem_t elem1, elem_t elem2) :
      cdlist_tos_base<elem_t, cdlist_e_tos<elem_t> >(elem0, elem1, elem2)
    { }
    cdlist_tos(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3) :
      cdlist_tos_base<elem_t, cdlist_e_tos<elem_t> >(elem0, elem1, elem2,
          elem3)  { }
    cdlist_tos(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3,
        elem_t elem4) :
      cdlist_tos_base<elem_t, cdlist_e_tos<elem_t> >(elem0, elem1, elem2,
          elem3, elem4)  { }
    cdlist_tos(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3,
        elem_t elem4, elem_t elem5) :
      cdlist_tos_base<elem_t, cdlist_e_tos<elem_t> >(elem0, elem1, elem2,
          elem3, elem4, elem5)  { }
    cdlist_tos(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3,
        elem_t elem4, elem_t elem5, elem_t elem6) :
      cdlist_tos_base<elem_t, cdlist_e_tos<elem_t> >(elem0, elem1, elem2,
          elem3, elem4, elem5, elem6)  { }
    cdlist_tos(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3,
        elem_t elem4, elem_t elem5, elem_t elem6, elem_t elem7) :
      cdlist_tos_base<elem_t, cdlist_e_tos<elem_t> >(elem0, elem1, elem2,
          elem3, elem4, elem5, elem6, elem7)  { }
    virtual ~cdlist_tos(void)  { }
};

#ifdef INLINE_ALL_TEMPLATES
#define DOING_TEMPLATE_INLINING
#ifdef STY_H
#include <sty/cdlist_tos.cpp>
#else
#include "cdlist_tos.cpp"
#endif
#undef DOING_TEMPLATE_INLINING
#endif /* INLINE_ALL_TEMPLATES */


#endif /* STY_CDLIST_TOS_H */
