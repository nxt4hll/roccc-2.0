/* file "referenced_item.h" */


/*
       Copyright (c) 1996, 1997 Stanford University

       All rights reserved.

       This software is provided under the terms described in
       the "suif_copyright.h" include file.
*/

#include <common/suif_copyright.h>

#ifndef STY_REFERENCED_ITEM_H
#define STY_REFERENCED_ITEM_H

#include "suifkernel/suifkernel_forwarders.h"

/*
      This is the definition of the referenced_item class, which is
      for internal use by sty, the first-level main library of the
      SUIF system.
*/


/*
      The idea of this class is that for various interfaces in sty, we
      want to pass back to the user a reference to something internal
      (reference not meaning necessarily a C++ reference, but the more
      general sense of the word reference).  Essentially, a pointer is
      being returned that the user can then later pass to other
      methods of the interface.  But some implementations of a
      particular interface may pass pointers to structures they keep
      anyway, while other implementations may build the internal
      structures on-the-fly when a reference to them is requested by
      the user.  An example of this is a handle for an element in a
      tos, which in a linked-list implementation is a pointer to an
      item in the list while in an array implementation it's an
      auxiliary structure that is only constructed when such a handle
      is requested by the user.  We want to be able to delete the
      specially constructed structure when the user is done with the
      reference to it.  We handle this by passing a special handle
      class to the user where the handle contains only one field, the
      pointer.  The only differences between the pointer itself and
      the handle are that the handle will tell the pointer when the
      handle is copied or de-allocated and if necessary deallocate it
      when the last reference is de-allocated.  We implement this by
      having all such internal structures inherit from this pure
      virtual class, referenced_item.

      Having a class wrapped around the pointer given to a user also
      gives the added benefit of making it somewhat harder to
      circumvent the separation of interface and implementation.
      Since pointer casts happen all over the place and are entirely
      necessary in C++ to move up the class heirarchy, it's tempting
      to simply cast a pointer a user is given to get access to the
      internals of what the user knows it points to.  The wrapper
      class makes it hard enough to discourage such usage.

      Note that it's important to have a single class for all things
      that the user is going to get a handle to so that
      behind-the-scenes one class can use the handle it gets from a
      component class to give to the user as a totally different kind
      of handle.  The handle classes should have different types for
      better type checking.  That means that some classes will
      actually have to be made dependent on the internals of the
      handle for another class (though only to the extent of knowing
      that the handle is just a wrapper for a referenced_item
      pointer).  This is a small reduction in modularity, but it is
      necessary to get efficient implementations the way these
      interfaces are structured.
 */
//#include "sty_basics.h"

class referenced_item
  {
private:
    //    virtual void virtual_function_table_hack(void);

protected:
    referenced_item(void)  { }

public:
    virtual ~referenced_item(void)  { }

    virtual void add_ref(void) = 0;
    virtual void remove_ref(void) = 0;
    virtual bool delete_me(void) = 0;
  };

class ri_reference
  {
private:
    //    virtual void virtual_function_table_hack(void);

protected:
    referenced_item *_data;

    void old_ref(referenced_item *old_data)
      {
        if (old_data != 0)
          {
            old_data->remove_ref();
            if (old_data->delete_me())
                delete old_data;
          }
      }

    ri_reference(const ri_reference &) : _data(0) {};
    ri_reference &operator=(const ri_reference &) { suif_assert(0); return *this; }

    ri_reference(void) : _data(0) {}
    ri_reference(referenced_item *init_data) : _data(init_data)
      {
        if (init_data != 0)
            init_data->add_ref();
      }
    virtual ~ri_reference(void)  { old_ref(_data); }

public:
    /* no public constructors or destructors should exist here */

    bool is_null(void) const  { return _data == 0; }

    /* DANGER!  The following allows access to the raw referenced_item
     * pointer.  Avoid using this if at all possible.  If it is used,
     * the user is responsible for seeing that add_ref() and
     * remove_ref() are called when appropriate on the resulting
     * pointer. */
    referenced_item *raw_referenced_item(void) const  { return _data; }
    void set_raw_referenced_item(referenced_item *new_data)
      {
        if (new_data != 0)
            new_data->add_ref();
        old_ref(_data);
        _data = new_data;
      }
  };


#endif /* STY_REFERENCED_ITEM_H */
