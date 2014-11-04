#ifndef LIST_WRAPPER
#define LIST_WRAPPER
#include "iokernel_forwarders.h"
#include "object_wrapper.h"

// The following object is Immutable.

class ListWrapper {
public:
  // check is_aggregate() first.
  ListWrapper(const ObjectWrapper &obj);

  ListWrapper(Address address, const ListMetaClass *mc);

  ListWrapper(const ListWrapper &);
  ListWrapper &operator=(const ListWrapper &);

  Address get_address() const;
  const ListMetaClass *get_meta_class() const;


  // This does NOT allow for iteration over all of the
  // fields for a union.  just the one the tag refers to
  size_t get_length() const;
  ObjectWrapper get_element(size_t offset) const;
  sf_owned Iterator *get_iterator() const;

  ObjectWrapper get_object() const;

  bool is_null() const;
  static bool is_list(const ObjectWrapper &obj);

private:
  const ListMetaClass *_meta_class;
  Address _address;  // This is USUALLY a GenericList *
};

#endif /* LIST_WRAPPER */
