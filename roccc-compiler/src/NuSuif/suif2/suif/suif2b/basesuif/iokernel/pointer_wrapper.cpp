#include "common/system_specific.h"
#include "pointer_wrapper.h"
#include "object_wrapper.h"
#include "cast.h"
#include "pointer_meta_class.h"

PointerWrapper::PointerWrapper(Address address, 
			       const PointerMetaClass *meta_class) :
  _meta_class(meta_class),
  _address(address)
{
}

PointerWrapper::PointerWrapper(const ObjectWrapper &obj) :
  _meta_class(NULL), _address(0)
{
  kernel_assert_message(is_pointer(obj), 
			("Attempt to pointer object with non-pointer"));
  _meta_class = to<PointerMetaClass>(obj.get_meta_class());
  _address = obj.get_address();
}
PointerWrapper::PointerWrapper(const PointerWrapper &other) :
  _meta_class(other.get_meta_class()), 
  _address(other.get_address())
{}

PointerWrapper &PointerWrapper::operator=(const PointerWrapper &other) {
  _meta_class = other.get_meta_class();
  _address = other.get_address();
  return(*this);
}

ObjectWrapper PointerWrapper::dereference() const {
  kernel_assert_message(!is_null(), ("NULL dereference in PointerWrapper"));
  Address address = *(Address*) get_address();
  MetaClass *mc = get_meta_class()->get_base_type();
  ObjectWrapper newobj(address, mc);
  return(newobj.update_meta_class());

  //  if (address && is_kind_of_object_meta_class(mc)) {
  //    Object *obj = (Object*)address;
  //    return(ObjectWrapper(obj));
  //  }
  //  return(ObjectWrapper(address, mc));
}

ObjectWrapper PointerWrapper::get_object() const {
  return(ObjectWrapper(get_address(), get_meta_class()));
}

bool PointerWrapper::is_null() const {
  return(get_address() == NULL);
}

bool PointerWrapper::is_pointer(const ObjectWrapper &obj) {
  const MetaClass *mc = obj.get_meta_class();
  return(is_kind_of<PointerMetaClass>(mc));
}

const PointerMetaClass *PointerWrapper::get_meta_class() const {
  return(_meta_class);
}

Address PointerWrapper::get_address() const {
  return (_address);
}

