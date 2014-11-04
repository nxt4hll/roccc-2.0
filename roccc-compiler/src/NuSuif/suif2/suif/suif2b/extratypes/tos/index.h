/* file "index.h" */


/*
       Copyright (c) 1996, 1997 Stanford University

       All rights reserved.

       This software is provided under the terms described in
       the "suif_copyright.h" include file.
*/

#include <common/suif_copyright.h>


#ifndef STY_INDEX_H
#define STY_INDEX_H

//#ifndef SUPPRESS_PRAGMA_INTERFACE
//#pragma interface
//#endif


/*
      This is the definition of the index template and related
      templates, which are templates for classes implementing mappings
      from arbitrary keys to arbitrary types, for sty, the first-level
      main library of the SUIF system.
*/

#include "referenced_item.h"

template <class key_t, class elem_t> class Index;

template <class key_t, class elem_t> class index_handle : public ri_reference
  {
    friend class Index<key_t, elem_t>;

private:
    index_handle(referenced_item *init_data) : ri_reference(init_data)  { }

public:
    index_handle(void)  { }
    index_handle(const index_handle<key_t, elem_t> &other) :
            ri_reference(other._data)  { }
    ~index_handle(void)  { }

    const index_handle<key_t, elem_t> &operator=(const index_handle<key_t, elem_t> &other)
      { set_raw_referenced_item(other._data); return(*this); }
    bool operator==(const index_handle<key_t, elem_t> &other) const
      { return (_data == other._data); }
    bool operator!=(const index_handle<key_t, elem_t> &other) const
      { return (_data != other._data); }
  };

template <class key_t, class elem_t> class index_ref;

template <class key_t, class elem_t> class Index
  {
    friend class index_ref<key_t, elem_t>;

private:
    virtual void add_ref(void)  { }
    virtual void remove_ref(void)  { }
    virtual bool delete_me(void)  { return false; }

protected:
    Index(void)  { }
public:
    virtual ~Index(void)  { }
protected:

    index_handle<key_t, elem_t> build_handle(referenced_item *data) const
      { return index_handle<key_t, elem_t>(data); }
    referenced_item *from_handle(index_handle<key_t, elem_t> the_handle) const
      { return the_handle._data; }

public:
    virtual elem_t lookup(key_t) const = 0;
    virtual bool exists(key_t) const = 0;
    virtual index_handle<key_t, elem_t> enter(key_t, elem_t) = 0;
    virtual void remove(key_t) = 0;

    virtual index_handle<key_t, elem_t> lookup_handle(key_t) const = 0;
    virtual elem_t elem(index_handle<key_t, elem_t>) const = 0;
    virtual void remove(index_handle<key_t, elem_t>) = 0;

    virtual void clear(void) = 0;
  };

template <class key_t, class elem_t> class index_ref :
        public Index<key_t, elem_t>
  {
private:
    Index<key_t, elem_t> *_real_index;

public:
    index_ref(void) : _real_index(0)  { }
    index_ref(const index_ref &other) : _real_index(other._real_index)
      {
        if (_real_index != 0)
            _real_index->add_ref();
      }
    virtual ~index_ref(void)
      {
        if (_real_index != 0)
          {
            _real_index->remove_ref();
            if (_real_index->delete_me())
                delete _real_index;
          }
      }

    elem_t lookup(key_t the_key) const
      { return _real_index->lookup(the_key); }
    bool exists(key_t the_key) const
      { return _real_index->exists(the_key); }
    index_handle<key_t, elem_t> enter(key_t the_key, elem_t the_elem)
      { return _real_index->enter(the_key, the_elem); }
    void remove(key_t the_key)  { _real_index->remove(the_key); }

    index_handle<key_t, elem_t> lookup_handle(key_t the_key) const
      { return _real_index->lookup_handle(the_key); }
    elem_t elem(index_handle<key_t, elem_t> handle) const
      { return _real_index->elem(handle); }
    void remove(index_handle<key_t, elem_t> handle)
      { _real_index->remove(handle); }

    void clear(void)  { _real_index->clear(); }

    const index_ref<key_t, elem_t> &operator=(const index_ref<key_t, elem_t> &other)
      {
        if (&other == this)
            return(*this);
        if (_real_index != 0)
          {
            _real_index->remove_ref();
            if (_real_index->delete_me())
                delete _real_index;
          }
        _real_index = other._real_index;
        if (_real_index != 0)
            _real_index->add_ref();
	return(*this);
      }
  };


#endif /* STY_INDEX_H */
