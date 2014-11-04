#ifndef IOKERNEL__FIELD_DESCRIPTION_H
#define IOKERNEL__FIELD_DESCRIPTION_H

#include "iokernel_forwarders.h"

typedef suif_vector<FieldDescription*> FieldDescriptionList;

class FieldDescription {
  friend class ObjectFactory;
  size_t offset;
  const MetaClass* metaClass;
  LString memberName;
  ObjectWrapper build_object(Address base_address) const;

public:
  //  FieldDescription() : offset(0), metaClass(0), memberName() {}
  FieldDescription(size_t off, const MetaClass *mc, LString memberName);
  const LString &get_member_name() const;
  const MetaClass *get_meta_class() const;
  size_t get_offset() const;

  // Mutators. This should RARELY be needed.
  void set_offset(size_t off);


  // utility functionality
  ObjectWrapper build_object(const AggregateWrapper &base_obj) const;
  ObjectWrapper build_object(Object *base_obj) const;

  // get_address should go away too.  build the object and get the address.
  Address get_address(Address base_address) const;
  void print_debug() const;
  String get_debug_text() const;

  // the following two declarations do not have a definition
  FieldDescription(const FieldDescription &other);
  FieldDescription &operator=(const FieldDescription &other);
};

#endif
