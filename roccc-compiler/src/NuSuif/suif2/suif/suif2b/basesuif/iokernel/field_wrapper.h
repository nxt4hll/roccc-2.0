#ifndef FIELD_WRAPPER
#define FIELD_WRAPPER
#include "iokernel_forwarders.h"
#include "object_wrapper.h"

// The following object is Immutable.

class FieldWrapper {
public:
  FieldWrapper(const ObjectWrapper &obj,
	       const LString &field_name);
  FieldWrapper(const FieldWrapper &other);
  FieldWrapper &operator=(const FieldWrapper &other);

  ObjectWrapper get_object() const;
  LString get_field_name() const;

private:
  ObjectWrapper _obj;
  LString _field_name;
};

#endif /* FIELD_WRAPPER */
