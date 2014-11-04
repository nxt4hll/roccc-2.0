/* file "tos.h" */


/*
   Copyright (c) 1996, 1997 Stanford University

   All rights reserved.

   This software is provided under the terms described in
   the "suif_copyright.h" include file.
 */

#include <common/suif_copyright.h>


#ifndef STY_TOS_H
#define STY_TOS_H

#include <common/machine_dependent.h>
#include "const_cast.h"
#include "zero.h"

/*
   This is the definition of the tos template and related
   templates, which are templates for classes implementing
   totally-ordered sets, for sty, the first-level main library of
   the SUIF system.
 */


/*
 *  classes
 *    * tos: Fininte Totally Ordered Set (Interface)
 *    * ro_tos: Read-Only TOS
 *    * tos_handle: a handle for a given element
 *    * tos_iter: an iterator (a handle plus a pointer to the ro_tos)
 *    * tos_setter: an lvalue to return for [] operators (see below)
 *    * tos_ref: a reference to a tos
 *    * ro_tos_ref: a reference to an ro_tos
 */

/*
 *  Note: The tos_setter class exists only to make expressions like
 *  a[i] = b[i] + c[i] work without giving the user a reference to an
 *  internal member of the tos.
 */

#include "referenced_item.h"

template <class elem_t> class tos;
template <class elem_t> class ro_tos;

template <class elem_t> class tos_handle : public ri_reference
{
  friend class ro_tos<elem_t>;

  public:
  tos_handle(void)  { }
  tos_handle(const tos_handle<elem_t> &other) : ri_reference(other._data)
  { }
  tos_handle(referenced_item *init_data) : ri_reference(init_data)  { }
  ~tos_handle(void)  { }

  const tos_handle<elem_t> &operator=(const tos_handle<elem_t> &other)
  { set_raw_referenced_item(other._data); return (*this); }
  bool operator==(const tos_handle<elem_t> &other) const
  { return (_data == other._data); }
  bool operator!=(const tos_handle<elem_t> &other) const
  { return (_data != other._data); }
};

template <class elem_t> inline tos_handle<elem_t> zero(tos_handle<elem_t> *)
{ return tos_handle<elem_t>(0); }

template <class elem_t> class tos_setter
{
  private:
    tos<elem_t> *_tos;
    tos_handle<elem_t> _handle;
    size_t _number;

    /* This assignment operator takes a *non-const* reference
     * parameter.  This is critical to stop the implicit generation of
     * an assignment operator taking a const reference to tos_setter
     * parameter, which would make ``A[i] = B[j]'' behave wrong.  In
     * the absence of the const-argument assignment operator, a
     * conversion is forced to grab the element and do the assignment
     * with that element. */
    tos_setter &operator=(tos_setter<elem_t> &)  { assert(false); return(*this); }

  public:
    tos_setter(void) : _tos(0), _number(0)  { }
    tos_setter(const tos_setter &other) : _tos(other._tos),
                                          _handle(other._handle), _number(other._number)  { }
    tos_setter(tos<elem_t> *init_tos, tos_handle<elem_t> init_handle) :
      _tos(init_tos), _handle(init_handle), _number(0)  { }
    tos_setter(tos<elem_t> *init_tos, size_t init_number) : 
      _tos(init_tos),
      _handle(),
      _number(init_number)  { }
    ~tos_setter(void)  { }

    tos_setter &operator=(elem_t rhs)
    {
      if (_handle.is_null())
        _tos->set_elem(_number, rhs);
      else
        _tos->set_elem(_handle, rhs);
      return(*this);
    }
    operator elem_t(void) const
    {
      if (_handle.is_null())
        return _tos->elem(_number);
      else
        return _tos->elem(_handle);
    }
};


template <class elem_t> class tos_ref;
template <class elem_t> class ro_tos_ref;

template <class elem_t> class ro_tos
{
  friend class tos_ref<elem_t>;
  friend class ro_tos_ref<elem_t>;

  private:
  virtual void add_ref(void)  { }
  virtual void remove_ref(void)  { }
  virtual bool delete_me(void)  { return false; }

  protected:
  ro_tos(void)  { }
  virtual ~ro_tos(void)  { }

  tos_handle<elem_t> build_handle(referenced_item *data) const
  { return tos_handle<elem_t>(data); }
  referenced_item *from_handle(tos_handle<elem_t> the_handle) const
  { return the_handle._data; }

  public:
  virtual size_t count(void) const = 0;
  virtual bool is_empty(void) const = 0;
  virtual bool count_is(size_t test_count) const = 0;

  virtual bool is_member(elem_t) const = 0;
  virtual size_t position(elem_t) const = 0;
  virtual elem_t elem_by_num(size_t position) const = 0;
  virtual tos_handle<elem_t> lookup_handle(elem_t) const = 0;
  virtual elem_t elem_by_handle(tos_handle<elem_t>) const = 0;
  virtual tos_handle<elem_t> handle_for_num(size_t position) const = 0;
  virtual tos_handle<elem_t> next_handle(tos_handle<elem_t>) const = 0;
  virtual tos_handle<elem_t> previous_handle(tos_handle<elem_t>) const = 0;
  virtual elem_t head(void) const = 0;
  virtual elem_t tail(void) const = 0;
  virtual tos_handle<elem_t> head_handle(void) const = 0;
  virtual tos_handle<elem_t> tail_handle(void) const = 0;

  elem_t operator[](size_t elem_num) const
  { return elem_by_num(elem_num); }
  elem_t elem(size_t position) const  { return elem_by_num(position); }
  elem_t elem(tos_handle<elem_t> the_handle) const
  { return elem_by_handle(the_handle); }
};

template <class elem_t> class tos : public ro_tos<elem_t>
{
  friend class tos_ref<elem_t>;

  protected:
  tos(void)  { }

  public:
  virtual ~tos(void)  { }

  virtual void append(elem_t) = 0;
  virtual void prepend(elem_t) = 0;
  void push(elem_t the_elem)  { prepend(the_elem); }

  virtual void set_elem_by_num(size_t elem_num, elem_t) = 0;
  virtual void insert(size_t elem_num, elem_t) = 0;
  virtual void remove(size_t elem_num) = 0;

  virtual void set_elem_by_handle(tos_handle<elem_t>, elem_t) = 0;
  virtual void insert_before(tos_handle<elem_t>, elem_t) = 0;
  virtual void insert_after(tos_handle<elem_t>, elem_t) = 0;
  virtual void remove(tos_handle<elem_t>) = 0;

  virtual elem_t pop(void) = 0;
  virtual elem_t tail_pop(void) = 0;
  virtual void remove_elem(elem_t the_elem)
  { remove(lookup_handle(the_elem)); }

  virtual void clear(void) = 0;

  tos_setter<elem_t> operator[](size_t elem_num)
  { return tos_setter<elem_t>(this, this->handle_for_num(elem_num)); }
  void set_elem(size_t elem_num, elem_t new_elem)
  { set_elem_by_num(elem_num, new_elem); }
  void set_elem(tos_handle<elem_t> the_handle, elem_t new_elem)
  { set_elem_by_handle(the_handle, new_elem); }
};

template <class elem_t> class ro_tos_ref : public ro_tos<elem_t>
{
  private:
    ro_tos<elem_t> *_real_ro_tos;

  public:
    ro_tos_ref(void) : _real_ro_tos(0)  { }
    ro_tos_ref(const ro_tos_ref<elem_t> &other) :
      _real_ro_tos(other._real_ro_tos)
    {
      if (_real_ro_tos != 0)
        _real_ro_tos->add_ref();
    }
    ro_tos_ref(ro_tos<elem_t> *init_ro_tos) : _real_ro_tos(init_ro_tos)
    {
      if (_real_ro_tos != 0)
        _real_ro_tos->add_ref();
    }
    ~ro_tos_ref(void)
    {
      if (_real_ro_tos != 0)
      {
        _real_ro_tos->remove_ref();
        if (_real_ro_tos->delete_me())
          delete _real_ro_tos;
      }
    }

    size_t count(void) const
    { assert(_real_ro_tos != 0); return _real_ro_tos->count(); }
    bool is_empty(void) const
    { assert(_real_ro_tos != 0); return _real_ro_tos->is_empty(); }
    bool count_is(size_t test_count) const
    {
      assert(_real_ro_tos != 0);
      return _real_ro_tos->count_is(test_count);
    }

    bool is_member(elem_t the_elem) const
    {
      assert(_real_ro_tos != 0);
      return _real_ro_tos->is_member(the_elem);
    }
    size_t position(elem_t the_elem) const
    {
      assert(_real_ro_tos != 0);
      return _real_ro_tos->position(the_elem);
    }
    elem_t elem_by_num(size_t position) const
    { assert(_real_ro_tos != 0); return _real_ro_tos->elem(position); }
    tos_handle<elem_t> lookup_handle(elem_t the_elem) const
    {
      assert(_real_ro_tos != 0);
      return _real_ro_tos->lookup_handle(the_elem);
    }
    elem_t elem_by_handle(tos_handle<elem_t> the_handle) const
    { assert(_real_ro_tos != 0); return _real_ro_tos->elem(the_handle); }
    tos_handle<elem_t> handle_for_num(size_t position) const
    {
      assert(_real_ro_tos != 0);
      return _real_ro_tos->handle_for_num(position);
    }
    tos_handle<elem_t> next_handle(tos_handle<elem_t> the_handle) const
    {
      assert(_real_ro_tos != 0);
      return _real_ro_tos->next_handle(the_handle);
    }
    tos_handle<elem_t> previous_handle(tos_handle<elem_t> the_handle) const
    {
      assert(_real_ro_tos != 0);
      return _real_ro_tos->previous_handle(the_handle);
    }
    elem_t head(void) const
    { assert(_real_ro_tos != 0); return _real_ro_tos->head(); }
    elem_t tail(void) const
    { assert(_real_ro_tos != 0); return _real_ro_tos->tail(); }
    tos_handle<elem_t> head_handle(void) const
    { assert(_real_ro_tos != 0); return _real_ro_tos->head_handle(); }
    tos_handle<elem_t> tail_handle(void) const
    { assert(_real_ro_tos != 0); return _real_ro_tos->tail_handle(); }

    ro_tos_ref <elem_t> &operator=(const ro_tos_ref<elem_t> &other)
    {
      if (&other == this)
        return(*this);
      if (_real_ro_tos != 0)
      {
        _real_ro_tos->remove_ref();
        if (_real_ro_tos->delete_me())
          delete _real_ro_tos;
      }
      _real_ro_tos = other._real_ro_tos;
      if (_real_ro_tos != 0)
        _real_ro_tos->add_ref();
      return(*this);
    }
};

template <class elem_t> class tos_iter
{
  private:
    tos_handle<elem_t> _handle;
    ro_tos_ref<elem_t> _ro_tos_ref;

  public:
    tos_iter(const ro_tos<elem_t> *init_ro_tos) :
      _handle((init_ro_tos == 0) ? tos_handle<elem_t>() :
          init_ro_tos->head_handle()),
      _ro_tos_ref(CONST_CAST(ro_tos<elem_t> *, init_ro_tos))  { }
    tos_iter(ro_tos_ref<elem_t> init_ro_tos_ref) :
      _handle(init_ro_tos_ref.head_handle()),
      _ro_tos_ref(init_ro_tos_ref)
    { }
    tos_iter(void) : _handle(0), _ro_tos_ref(0)  { }
    ~tos_iter(void)  { }

    elem_t get(void) const  { return _ro_tos_ref.elem(_handle); }
    bool done(void) const
    { return _handle.is_null(); }
    tos_handle<elem_t> get_handle(void) const  { return _handle; }

    void increment(void)
    {
      if (!_handle.is_null())
        _handle = _ro_tos_ref.next_handle(_handle);
    }
    void reset(void)  { _handle = _ro_tos_ref.head_handle(); }
    void reset(const ro_tos<elem_t> *new_ro_tos)
    {
      _ro_tos_ref = CONST_CAST(ro_tos<elem_t> *, new_ro_tos);
      reset();
    }
};

template <class elem_t> class tos_ref : public tos<elem_t>
{
  private:
    tos<elem_t> *_real_tos;

  public:
    tos_ref(void) : _real_tos(0)  { }
    tos_ref(const tos_ref<elem_t> &other) : _real_tos(other._real_tos)
    {
      if (_real_tos != 0)
        _real_tos->add_ref();
    }
    tos_ref(tos<elem_t> *init_tos) : _real_tos(init_tos)
    {
      if (_real_tos != 0)
        _real_tos->add_ref();
    }
    ~tos_ref(void)
    {
      if (_real_tos != 0)
      {
        _real_tos->remove_ref();
        if (_real_tos->delete_me())
          delete _real_tos;
      }
    }

    size_t count(void) const
    { assert(_real_tos != 0); return _real_tos->count(); }
    bool is_empty(void) const
    { assert(_real_tos != 0); return _real_tos->is_empty(); }
    bool count_is(size_t test_count) const
    { assert(_real_tos != 0); return _real_tos->count_is(test_count); }

    bool is_member(elem_t the_elem) const
    { assert(_real_tos != 0); return _real_tos->is_member(the_elem); }
    size_t position(elem_t the_elem) const
    { assert(_real_tos != 0); return _real_tos->position(the_elem); }
    elem_t elem_by_num(size_t position) const
    { assert(_real_tos != 0); return _real_tos->elem(position); }
    tos_handle<elem_t> lookup_handle(elem_t the_elem) const
    { assert(_real_tos != 0); return _real_tos->lookup_handle(the_elem); }
    elem_t elem_by_handle(tos_handle<elem_t> the_handle) const
    { assert(_real_tos != 0); return _real_tos->elem(the_handle); }
    tos_handle<elem_t> handle_for_num(size_t position) const
    {
      assert(_real_tos != 0);
      return _real_tos->handle_for_num(position);
    }
    tos_handle<elem_t> next_handle(tos_handle<elem_t> the_handle) const
    { assert(_real_tos != 0); return _real_tos->next_handle(the_handle); }
    tos_handle<elem_t> previous_handle(tos_handle<elem_t> the_handle) const
    {
      assert(_real_tos != 0);
      return _real_tos->previous_handle(the_handle);
    }
    elem_t head(void) const
    { assert(_real_tos != 0); return _real_tos->head(); }
    elem_t tail(void) const
    { assert(_real_tos != 0); return _real_tos->tail(); }
    tos_handle<elem_t> head_handle(void) const
    { assert(_real_tos != 0); return _real_tos->head_handle(); }
    tos_handle<elem_t> tail_handle(void) const
    { assert(_real_tos != 0); return _real_tos->tail_handle(); }

    void append(elem_t the_elem)
    { assert(_real_tos != 0); _real_tos->append(the_elem); }
    void prepend(elem_t the_elem)
    { assert(_real_tos != 0); _real_tos->prepend(the_elem); }

    void set_elem_by_num(size_t elem_num, elem_t the_elem)
    { assert(_real_tos != 0); _real_tos->set_elem(elem_num, the_elem); }
    void insert(size_t elem_num, elem_t the_elem)
    { assert(_real_tos != 0); _real_tos->insert(elem_num, the_elem); }
    void remove(size_t elem_num)
    { assert(_real_tos != 0); _real_tos->remove(elem_num); }

    void set_elem_by_handle(tos_handle<elem_t> the_handle, elem_t the_elem)
    { assert(_real_tos != 0); _real_tos->set_elem(the_handle, the_elem); }
    void insert_before(tos_handle<elem_t> the_handle, elem_t the_elem)
    {
      assert(_real_tos != 0);
      _real_tos->insert_before(the_handle, the_elem);
    }
    void insert_after(tos_handle<elem_t> the_handle, elem_t the_elem)
    {
      assert(_real_tos != 0);
      _real_tos->insert_after(the_handle, the_elem);
    }
    void remove(tos_handle<elem_t> the_handle)
    { assert(_real_tos != 0); _real_tos->remove(the_handle); }

    elem_t pop(void)
    { assert(_real_tos != 0); return _real_tos->pop(); }
    elem_t tail_pop(void)
    { assert(_real_tos != 0); return _real_tos->tail_pop(); }

    void clear(void)  { assert(_real_tos != 0); _real_tos->clear(); }

    tos_ref &operator=(const tos_ref<elem_t> &other)
    {
      if (&other == this)
        return(*this);
      if (_real_tos != 0)
      {
        _real_tos->remove_ref();
        if (_real_tos->delete_me())
          delete _real_tos;
      }
      _real_tos = other._real_tos;
      if (_real_tos != 0)
        _real_tos->add_ref();
      return(*this);
    }

    operator ro_tos_ref<elem_t>(void) const  { return _real_tos; }
};


#endif /* STY_TOS_H */
