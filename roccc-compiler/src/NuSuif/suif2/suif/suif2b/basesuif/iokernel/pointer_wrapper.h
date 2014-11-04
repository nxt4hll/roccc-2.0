#ifndef POINTER_WRAPPER_H
#define POINTER_WRAPPER_H

#include "iokernel_forwarders.h"

class PointerWrapper {
public:
  PointerWrapper(const ObjectWrapper &obj);
  PointerWrapper(Address address, const PointerMetaClass *meta_class);
  PointerWrapper(const PointerWrapper &other);

  PointerWrapper &operator=(const PointerWrapper &other);

  ObjectWrapper get_object() const;
  ObjectWrapper dereference() const;

  bool is_null() const;

  const PointerMetaClass *get_meta_class() const;
  Address get_address() const;

  static bool is_pointer(const ObjectWrapper &obj);

private:
  const PointerMetaClass *_meta_class;
  Address _address;
};

#endif /* POINTER_WRAPPER_H */
