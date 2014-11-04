/* file "adlist_tos.h" */


/*
       Copyright (c) 1997 Stanford University

       All rights reserved.

       This software is provided under the terms described in
       the "suif_copyright.h" include file.
*/

#include <common/suif_copyright.h>


#ifndef STY_ADLIST_TOS_H
#define STY_ADLIST_TOS_H

//#ifndef SUPPRESS_PRAGMA_INTERFACE
//#pragma interface
//#endif

#include "tos.h"
/*
      This is the definition of the adlist_tos template and related
      templates, which are templates for classes implementing
      totally-ordered sets with doubly linked lists plus index arrays,
      for sty, the first-level main library of the SUIF system.
*/


/*
 *  The difference between dlist_tos and adlist_tos is that the later
 *  adds an index array for elements that have not changed their
 *  offset from the start since they were last visited.  In other
 *  words, whenever something is inserted or deleted in the middle of
 *  the list, everything in the index array from that position on is
 *  marked invalid.  The invalid parts of the index are filled in if
 *  and when the list is traversed from the beginning or from a known
 *  offset from the beginning.  So insertion and deletion are constant
 *  time operations and generally as long as the list is used purely
 *  as a linked list it will have time complexity just as a doubly
 *  linked list.  But accesses by number will also be as efficient in
 *  terms of time complexity as if it were an array with the exception
 *  of having to rebuild parts of the index after insert or delete
 *  operations.  That means that if only linked-list-style operations
 *  are performed, it is as efficient as a linked-list, and if only
 *  array-style operations are performed it is as efficient as a
 *  simple array.  The cost is in switching back and forth between
 *  array-style and linked-list-style operations, which costs, in
 *  general, time linear with respect to the number of elements to
 *  switch back to array-style accesses after doing linked-list-style
 *  modifications.
 */

template <class elem_t> class adlist_e_tos : public referenced_item
  {
private:
    size_t _ref_count;

public:
    elem_t data;
    adlist_e_tos<elem_t> *next;
    adlist_e_tos<elem_t> *previous;

    adlist_e_tos(elem_t init_data, adlist_e_tos<elem_t> *init_next = 0,
                adlist_e_tos<elem_t> *init_previous = 0) : _ref_count(1),
            data(init_data), next(init_next), previous(init_previous)  { }
    virtual ~adlist_e_tos(void)  { suif_assert(_ref_count == 0); }

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

template <class elem_t, class holder_t> class adlist_tos_base :
        public tos<elem_t>
  {
private:
    holder_t *_head_e;
    holder_t *_tail_e;
    size_t _index_size;
    size_t _index_space;
    holder_t **_index;
    size_t _cached_size;

    void extend_array(void);

public:
    adlist_tos_base(void);
    adlist_tos_base(elem_t elem0);
    adlist_tos_base(elem_t elem0, elem_t elem1);
    //    adlist_tos_base(elem_t elem0, elem_t elem1, elem_t elem2);
    //    adlist_tos_base(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3);
    //    adlist_tos_base(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3,
    //                    elem_t elem4);
    //    adlist_tos_base(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3,
    //                    elem_t elem4, elem_t elem5);
    //    adlist_tos_base(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3,
    //                    elem_t elem4, elem_t elem5, elem_t elem6);
    //    adlist_tos_base(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3,
    //                    elem_t elem4, elem_t elem5, elem_t elem6, elem_t elem7);
    virtual ~adlist_tos_base(void);

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

template <class elem_t> class adlist_tos :
        public adlist_tos_base<elem_t, adlist_e_tos<elem_t> >
  {
public:
    adlist_tos(void)  { }
    adlist_tos(elem_t elem0) :
            adlist_tos_base<elem_t, adlist_e_tos<elem_t> >(elem0)  { }
    adlist_tos(elem_t elem0, elem_t elem1) :
            adlist_tos_base<elem_t, adlist_e_tos<elem_t> >(elem0, elem1)  { }
    adlist_tos(elem_t elem0, elem_t elem1, elem_t elem2) :
            adlist_tos_base<elem_t, adlist_e_tos<elem_t> >(elem0, elem1, elem2)
      { }
    adlist_tos(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3) :
            adlist_tos_base<elem_t, adlist_e_tos<elem_t> >(elem0, elem1, elem2,
                    elem3)  { }
    adlist_tos(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3,
               elem_t elem4) :
            adlist_tos_base<elem_t, adlist_e_tos<elem_t> >(elem0, elem1, elem2,
                    elem3, elem4)  { }
    adlist_tos(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3,
               elem_t elem4, elem_t elem5) :
            adlist_tos_base<elem_t, adlist_e_tos<elem_t> >(elem0, elem1, elem2,
                    elem3, elem4, elem5)  { }
    adlist_tos(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3,
               elem_t elem4, elem_t elem5, elem_t elem6) :
            adlist_tos_base<elem_t, adlist_e_tos<elem_t> >(elem0, elem1, elem2,
                    elem3, elem4, elem5, elem6)  { }
    adlist_tos(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3,
               elem_t elem4, elem_t elem5, elem_t elem6, elem_t elem7) :
            adlist_tos_base<elem_t, adlist_e_tos<elem_t> >(elem0, elem1, elem2,
                    elem3, elem4, elem5, elem6, elem7)  { }
    virtual ~adlist_tos(void)  { }
  };

#ifdef INLINE_ALL_TEMPLATES
#define DOING_TEMPLATE_INLINING
#ifdef STY_H
#include <sty/adlist_tos.cpp>
#else
#include "adlist_tos.cpp"
#endif
#undef DOING_TEMPLATE_INLINING
#endif /* INLINE_ALL_TEMPLATES */


#endif /* STY_ADLIST_TOS_H */
