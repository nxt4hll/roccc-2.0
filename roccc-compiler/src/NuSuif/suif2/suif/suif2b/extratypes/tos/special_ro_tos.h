/* file "special_ro_tos.h" */


/*
   Copyright (c) 1997 Stanford University

   All rights reserved.

   This software is provided under the terms described in
   the "suif_copyright.h" include file.
 */

#include <common/suif_copyright.h>


#ifndef STY_SPECIAL_RO_TOS_H
#define STY_SPECIAL_RO_TOS_H

//#ifndef SUPPRESS_PRAGMA_INTERFACE
//#pragma interface
//#endif


/*
   This is the declaration of several special ro_tos class
   templates for sty, the first-level main library of the SUIF
   system.
 */

#include "referenced_item.h"
#include "tos.h"

template <class elem_t> class count_tos_internal_handle :
  public referenced_item
{
  private:
    size_t _ref_count;

  public:
    size_t index;
    count_tos_internal_handle<elem_t> *next;
    count_tos_internal_handle<elem_t> **previous;

    count_tos_internal_handle(size_t init_index,
        count_tos_internal_handle<elem_t> *init_next,
        count_tos_internal_handle<elem_t> **init_previous) : _ref_count(0),
                                                             index(init_index), next(init_next), previous(init_previous)  { }
    ~count_tos_internal_handle(void)  { suif_assert(_ref_count == 0); }

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
      if (_ref_count == 0)
      {
        *previous = next;
        if (next != 0)
          next->previous = previous;
      }
    }
    bool delete_me(void)  { return (_ref_count == 0); }
};

template <class elem_t> class count_based_ro_tos : public ro_tos<elem_t>
{
  protected:
    count_tos_internal_handle<elem_t> *_handles;

    count_based_ro_tos(void) : _handles(0)  { }

  public:
    virtual ~count_based_ro_tos(void)
    {
      count_tos_internal_handle<elem_t> *follow = _handles;
      while (follow != 0)
      {
        count_tos_internal_handle<elem_t> *current = follow;
        follow = follow->next;
        delete current;
      }
    }

    tos_handle<elem_t> lookup_handle(elem_t the_elem) const
    {
      size_t the_count = this->count();
      for (size_t elem_num = 0; elem_num < the_count; ++elem_num)
      {
        if (this->elem_by_num(elem_num) == the_elem)
          return handle_for_num(elem_num);
      }
      return this->build_handle(0);
    }
    elem_t elem_by_handle(tos_handle<elem_t> the_handle) const
    {
      count_tos_internal_handle<elem_t> *elem_ptr =
        (count_tos_internal_handle<elem_t> *)(from_handle(the_handle));
      if (elem_ptr == 0)
        return zero((elem_t *)0);
      if (elem_ptr->index >= this->count())
        return zero((elem_t *)0);
      return elem_by_num(elem_ptr->index);
    }
    tos_handle<elem_t> handle_for_num(size_t position) const
    {
      if ((position < 0) || (position >= this->count()))
        return this->build_handle(0);
      typedef count_based_ro_tos<elem_t> *this_type;
      count_based_ro_tos<elem_t> *mod_this = CONST_CAST(this_type, this);
      mod_this->_handles =
        new count_tos_internal_handle<elem_t>(position, _handles,
            &(mod_this->_handles));
      if (_handles->next != 0)
        _handles->next->previous = &(_handles->next);
      return this->build_handle(_handles);
    }
    tos_handle<elem_t> next_handle(tos_handle<elem_t> the_handle) const
    {
      count_tos_internal_handle<elem_t> *elem_ptr =
        (count_tos_internal_handle<elem_t> *)(from_handle(the_handle));
      if (elem_ptr == 0)
        return this->build_handle(0);
      if (elem_ptr->index + 1 >= this->count())
        return this->build_handle(0);
      return handle_for_num(elem_ptr->index + 1);
    }
    tos_handle<elem_t> previous_handle(tos_handle<elem_t> the_handle) const
    {
      count_tos_internal_handle<elem_t> *elem_ptr =
        (count_tos_internal_handle<elem_t> *)(from_handle(the_handle));
      if (elem_ptr == 0)
        return this->build_handle(0);
      if (elem_ptr->index == 0)
        return this->build_handle(0);
      return handle_for_num(elem_ptr->index - 1);
    }
    elem_t head(void) const  { return this->elem_by_num((size_t)0); }
    elem_t tail(void) const
    {
      if (this->count() == 0)
        return zero((elem_t *)0);
      else
        return elem_by_num(this->count() - 1);
    }
    tos_handle<elem_t> head_handle(void) const  { return handle_for_num(0); }
    tos_handle<elem_t> tail_handle(void) const
    {
      if (this->count() == 0)
        return this->build_handle(0);
      else
        return handle_for_num(this->count() - 1);
    }
};

/*
 *  NOTE: The count_based_tos and count_based_ro_tos classes define
 *  exactly the same methods in the same way!  Originally, only
 *  count_based_ro_tos existed and it was used both by classes that
 *  are not tos sub-classes (singleton_ro_tos and
 *  array_and_count_ro_tos) and by array_tos, which is a tos
 *  sub-class.  This meant that array_tos had to use multiple
 *  inheritance from both tos and count_based_ro_tos.  Worse, both tos
 *  and count_based_ro_tos had to be declared to have ``virtual''
 *  inheritance from ro_tos or else the count_based_ro_tos definitions
 *  of methods wouldn't override the same functions when seeing the
 *  array_tos as a ``tos'' (so the virtual function table had empty
 *  entries).  With all this virtual inheritance, everything worked,
 *  but later I was told by Urs that this kind of virtual multiple
 *  inheritance led to huge blowups in the sizes of these objects.
 *  Since tos objects are used so often, it wasn't worth all that
 *  extra overhead just to avoid duplicating some code.  So
 *  count_based_tos duplicates count_based_ro_tos and we have no more
 *  multiple inheritance.  -- CSW 7/30/1997
 */

template <class elem_t> class count_based_tos : public tos<elem_t>
{
  protected:
    count_tos_internal_handle<elem_t> *_handles;

    count_based_tos(void) : _handles(0)  { }
  private:
    count_based_tos(const count_based_tos &) : _handles(0)  { assert(0); }
    count_based_tos &operator=(const count_based_tos &) { assert(0); return(*this); }

  public:
    virtual ~count_based_tos(void)
    {
      count_tos_internal_handle<elem_t> *follow = _handles;
      while (follow != 0)
      {
        count_tos_internal_handle<elem_t> *current = follow;
        follow = follow->next;
        delete current;
      }
    }

    tos_handle<elem_t> lookup_handle(elem_t the_elem) const
    {
      size_t the_count = this->count();
      for (size_t elem_num = 0; elem_num < the_count; ++elem_num)
      {
        if (this->elem_by_num(elem_num) == the_elem)
          return handle_for_num(elem_num);
      }
      return this->build_handle(0);
    }
    elem_t elem_by_handle(tos_handle<elem_t> the_handle) const
    {
      count_tos_internal_handle<elem_t> *elem_ptr =
        (count_tos_internal_handle<elem_t> *)(from_handle(the_handle));
      if (elem_ptr == 0)
        return zero((elem_t *)0);
      if (elem_ptr->index >= this->count())
        return zero((elem_t *)0);
      return elem_by_num(elem_ptr->index);
    }
    tos_handle<elem_t> handle_for_num(size_t position) const
    {
      if ((position < 0) || (position >= this->count()))
        return this->build_handle(0);
      typedef count_based_tos<elem_t> *this_type;
      count_based_tos<elem_t> *mod_this = CONST_CAST(this_type, this);
      mod_this->_handles =
        new count_tos_internal_handle<elem_t>(position, _handles,
            &(mod_this->_handles));
      if (_handles->next != 0)
        _handles->next->previous = &(_handles->next);
      return this->build_handle(_handles);
    }
    tos_handle<elem_t> next_handle(tos_handle<elem_t> the_handle) const
    {
      count_tos_internal_handle<elem_t> *elem_ptr =
        (count_tos_internal_handle<elem_t> *)(from_handle(the_handle));
      if (elem_ptr == 0)
        return this->build_handle(0);
      if (elem_ptr->index + 1 >= this->count())
        return this->build_handle(0);
      return handle_for_num(elem_ptr->index + 1);
    }
    tos_handle<elem_t> previous_handle(tos_handle<elem_t> the_handle) const
    {
      count_tos_internal_handle<elem_t> *elem_ptr =
        (count_tos_internal_handle<elem_t> *)(from_handle(the_handle));
      if (elem_ptr == 0)
        return this->build_handle(0);
      if (elem_ptr->index == 0)
        return this->build_handle(0);
      return handle_for_num(elem_ptr->index - 1);
    }
    elem_t head(void) const  { return this->elem_by_num((size_t)0); }
    elem_t tail(void) const
    {
      if (this->count() == 0)
        return zero((elem_t *)0);
      else
        return elem_by_num(this->count() - 1);
    }
    tos_handle<elem_t> head_handle(void) const  { return handle_for_num(0); }
    tos_handle<elem_t> tail_handle(void) const
    {
      if (this->count() == 0)
        return this->build_handle(0);
      else
        return handle_for_num(this->count() - 1);
    }
};

template <class elem_t> class empty_ro_tos : public ro_tos<elem_t>
{
  public:
    empty_ro_tos(void)  { }
    virtual ~empty_ro_tos(void)  { }

    virtual size_t count(void) const  { return 0; }
    virtual bool is_empty(void) const  { return true; }
    virtual bool count_is(size_t test_count) const
    { return (test_count == 0); }

    virtual bool is_member(elem_t) const  { return false; }
    virtual size_t position(elem_t) const  { return (size_t)-1; }
    virtual elem_t elem_by_num(size_t /* position */) const
    { return zero((elem_t *)0); }
    virtual tos_handle<elem_t> lookup_handle(elem_t) const
    { return this->build_handle(0); }
    virtual elem_t elem_by_handle(tos_handle<elem_t>) const
    { return zero((elem_t *)0); }
    virtual tos_handle<elem_t> handle_for_num(size_t /* position */) const
    { return this->build_handle(0); }
    virtual tos_handle<elem_t> next_handle(tos_handle<elem_t>) const
    { return this->build_handle(0); }
    virtual tos_handle<elem_t> previous_handle(tos_handle<elem_t>) const
    { return this->build_handle(0); }
    virtual elem_t head(void) const  { return zero((elem_t *)0); }
    virtual elem_t tail(void) const  { return zero((elem_t *)0); }
    virtual tos_handle<elem_t> head_handle(void) const
    { return this->build_handle(0); }
    virtual tos_handle<elem_t> tail_handle(void) const
    { return this->build_handle(0); }
};

template <class elem_t> class singleton_ro_tos :
  public count_based_ro_tos<elem_t>
{
  private:
    elem_t _element;

  public:
    singleton_ro_tos(elem_t init_element) : _element(init_element)  { }
    virtual ~singleton_ro_tos(void)  { }

    virtual size_t count(void) const  { return 1; }
    virtual bool is_empty(void) const  { return false; }
    virtual bool count_is(size_t test_count) const
    { return (test_count == 1); }

    virtual bool is_member(elem_t the_elem) const
    { return (the_elem == _element); }
    virtual size_t position(elem_t the_elem) const
    {
      if (the_elem == _element)
        return 0;
      else
        return (size_t)-1;
    }
    virtual elem_t elem_by_num(size_t position) const
    {
      if (position == 0)
        return _element;
      else
        return zero((elem_t *)0);
    }
};

template <class elem_t> class array_and_count_ro_tos :
  public count_based_ro_tos<elem_t>
{
  private:
    elem_t *_array;
    size_t _count;

  public:
    array_and_count_ro_tos(elem_t *init_array, size_t init_count) :
      _array(init_array), _count(init_count)
    { }
    virtual ~array_and_count_ro_tos(void)  { }

    virtual size_t count(void) const  { return _count; }
    virtual bool is_empty(void) const  { return (_count == 0); }
    virtual bool count_is(size_t test_count) const
    { return (test_count == _count); }

    virtual bool is_member(elem_t the_elem) const
    {
      for (size_t elem_num = 0; elem_num < _count; ++elem_num)
      {
        if (_array[elem_num] == the_elem)
          return true;
      }
      return false;
    }
    virtual size_t position(elem_t the_elem) const
    {
      for (size_t elem_num = 0; elem_num < _count; ++elem_num)
      {
        if (_array[elem_num] == the_elem)
          return elem_num;
      }
      return (size_t)-1;
    }
    virtual elem_t elem_by_num(size_t position) const
    {
      if ((position < 0) || (position >= _count))
        return zero((elem_t *)0);
      return _array[position];
    }
};


#endif /* STY_SPECIAL_RO_TOS_H */
