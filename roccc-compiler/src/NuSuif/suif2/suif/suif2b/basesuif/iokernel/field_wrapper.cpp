#include "common/system_specific.h"
#include "field_wrapper.h"

// This is just an object that knows it's "parent" object and field
FieldWrapper::FieldWrapper(const ObjectWrapper &obj,
			   const LString &field_name) :
  _obj(obj), _field_name(field_name)
{}
FieldWrapper::FieldWrapper(const FieldWrapper &other) :
  _obj(other.get_object()), 
  _field_name(other.get_field_name())
{
}

FieldWrapper &FieldWrapper::operator=(const FieldWrapper &other) {
  _obj = other.get_object(); 
  _field_name = other.get_field_name();
  return(*this);
}

LString FieldWrapper::get_field_name() const {
  return(_field_name);
}

ObjectWrapper FieldWrapper::get_object() const {
  return(_obj);
}

