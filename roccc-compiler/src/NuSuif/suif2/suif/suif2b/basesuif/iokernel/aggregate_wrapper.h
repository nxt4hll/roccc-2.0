#ifndef AGGREGATE_WRAPPER
#define AGGREGATE_WRAPPER
#include "iokernel_forwarders.h"
#include "object_wrapper.h"
#include "field_wrapper.h"
#include "field_description.h"

// The following object is Immutable.

class AggregateWrapper {
public:
  // check is_aggregate() first.
  AggregateWrapper(const ObjectWrapper &obj);

  AggregateWrapper(Address address, const AggregateMetaClass *mc);

  AggregateWrapper(const AggregateWrapper &);
  AggregateWrapper &operator=(const AggregateWrapper &);

  Address get_address() const;
  const AggregateMetaClass *get_meta_class() const;

  FieldDescription *get_field_description(const LString &field_name) const;

  // This does NOT allow for iteration over all of the
  // fields for a union.  just the one the tag refers to
  //  size_t get_field_count() const;
  size_t get_field_count() const;
  //  FieldWrapper get_field(size_t id) const;
  FieldDescription *get_field_description(size_t id) const;

  // These expect to get a field from this metaclass and not its
  // parents.  They will assert if used incorrectly.
  size_t get_proper_field_count() const;
  //  FieldWrapper get_proper_field(size_t id) const;
  FieldDescription *get_proper_field_description(size_t id) const;

  bool has_field(const LString &field_name) const;
  FieldWrapper get_field(const LString &field_name) const;

  ObjectWrapper get_object() const;

  bool is_null() const;
  static bool is_aggregate(const ObjectWrapper &obj);

private:
  const AggregateMetaClass *_meta_class;
  Address _address;
};

#endif /* AGGREGATE_WRAPPER */
