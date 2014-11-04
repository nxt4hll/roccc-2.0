/* file "slist_tos.h" */


/*
       Copyright (c) 1996, 1997 Stanford University

       All rights reserved.

       This software is provided under the terms described in
       the "suif_copyright.h" include file.
*/

#include <common/suif_copyright.h>


#ifndef STY_SLIST_TOS_H
#define STY_SLIST_TOS_H

//#ifndef SUPPRESS_PRAGMA_INTERFACE
//#pragma interface
//#endif

#include <common/machine_dependent.h>
#include "tos.h"

/*
      This is the definition of the slist_tos template and related
      templates, which are templates for classes implementing
      totally-ordered sets with simple singly linked lists, for sty,
      the first-level main library of the SUIF system.
*/


template <class elem_t> class slist_e_tos : public referenced_item
  {
private:
    size_t _ref_count;

public:
    elem_t data;
    slist_e_tos<elem_t> *next;

    slist_e_tos(elem_t init_data, slist_e_tos<elem_t> *init_next = 0) :
            _ref_count(1), data(init_data), next(init_next)  { }
    ~slist_e_tos(void)  { suif_assert(_ref_count == 0); }

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

template <class elem_t, class holder_t> class slist_tos_base :
        public tos<elem_t>
  {
private:
    holder_t *_head_e;
    holder_t *_tail_e;

    slist_tos_base(const slist_tos_base &) : _head_e(0), _tail_e(0) { 
      assert(false);
    }
    slist_tos_base &operator=(const slist_tos_base &) {
      assert(false); return(*this);
    }
public:
    slist_tos_base(void) : _head_e(0), _tail_e(0) { }
    slist_tos_base(elem_t elem0);
    slist_tos_base(elem_t elem0, elem_t elem1);
    //    slist_tos_base(elem_t elem0, elem_t elem1, elem_t elem2);
    //    slist_tos_base(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3);
    //    slist_tos_base(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3,
    //                   elem_t elem4);
    //    slist_tos_base(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3,
    //                   elem_t elem4, elem_t elem5);
    //    slist_tos_base(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3,
    //                   elem_t elem4, elem_t elem5, elem_t elem6);
    //    slist_tos_base(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3,
    //                   elem_t elem4, elem_t elem5, elem_t elem6, elem_t elem7);
    virtual ~slist_tos_base(void);

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


template <class elem_t> class slist_tos :
        public slist_tos_base<elem_t, slist_e_tos<elem_t> >
  {
private:
    slist_tos(const slist_tos &) {assert(0);}
    slist_tos &operator=(const slist_tos &) {assert(0); return(*this);}
public:
    slist_tos(void)  { }
    slist_tos(elem_t elem0) :
            slist_tos_base<elem_t, slist_e_tos<elem_t> >(elem0)  { }
    slist_tos(elem_t elem0, elem_t elem1) :
            slist_tos_base<elem_t, slist_e_tos<elem_t> >(elem0, elem1)  { }
    /*
    slist_tos(elem_t elem0, elem_t elem1, elem_t elem2) :
            slist_tos_base<elem_t, slist_e_tos<elem_t> >(elem0, elem1, elem2)
      { }
    slist_tos(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3) :
            slist_tos_base<elem_t, slist_e_tos<elem_t> >(elem0, elem1, elem2,
                    elem3)  { }
    slist_tos(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3,
              elem_t elem4) :
            slist_tos_base<elem_t, slist_e_tos<elem_t> >(elem0, elem1, elem2,
                    elem3, elem4)  { }
    slist_tos(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3,
              elem_t elem4, elem_t elem5) :
            slist_tos_base<elem_t, slist_e_tos<elem_t> >(elem0, elem1, elem2,
                    elem3, elem4, elem5)  { }
    slist_tos(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3,
              elem_t elem4, elem_t elem5, elem_t elem6) :
            slist_tos_base<elem_t, slist_e_tos<elem_t> >(elem0, elem1, elem2,
                    elem3, elem4, elem5, elem6)  { }
    slist_tos(elem_t elem0, elem_t elem1, elem_t elem2, elem_t elem3,
              elem_t elem4, elem_t elem5, elem_t elem6, elem_t elem7) :
            slist_tos_base<elem_t, slist_e_tos<elem_t> >(elem0, elem1, elem2,
                    elem3, elem4, elem5, elem6, elem7)  { }
    */
    virtual ~slist_tos(void)  { }
  };

#ifdef INLINE_ALL_TEMPLATES
#define DOING_TEMPLATE_INLINING
#ifdef STY_H
#include <sty/slist_tos.cpp>
#else
#include "slist_tos.cpp"
#endif
#undef DOING_TEMPLATE_INLINING
#endif /* INLINE_ALL_TEMPLATES */


#endif /* STY_SLIST_TOS_H */
