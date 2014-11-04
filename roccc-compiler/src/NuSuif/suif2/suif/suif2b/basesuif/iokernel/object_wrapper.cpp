#include "common/system_specific.h"
#include "object_wrapper.h"
#include "object.h"
#include "meta_class.h"
// #include <iostream.h>
#include <iostream> //jul modif
#include "cast.h"
using namespace std;//jul modif

ObjectWrapper::ObjectWrapper() :
  _address(0),
  _meta_class(0)
{}
  
ObjectWrapper::ObjectWrapper(Address address, const MetaClass *metaclass):
  _address(address), _meta_class(metaclass)
{
  if (_meta_class)
    kernel_assert(is_kind_of<MetaClass>(_meta_class));
}

ObjectWrapper::ObjectWrapper(Object *obj) :
  _address((Address)obj), _meta_class(obj ? obj->get_meta_class() : NULL) 
{
  kernel_assert_message(obj != NULL, 
			("Null Object Passed to ObjectWrapper without a metaclass"));
}

ObjectWrapper::ObjectWrapper(const Object * const obj) :
  _address((Address)obj), _meta_class(obj->get_meta_class())
{}
  
ObjectWrapper::ObjectWrapper(const ObjectWrapper &other) :
  _address(other._address), _meta_class(other._meta_class)
{}

ObjectWrapper &ObjectWrapper::operator=(const ObjectWrapper &other) {
  _address = other._address;
  _meta_class = other._meta_class;
  return(*this);
}
  
bool ObjectWrapper::operator==(const ObjectWrapper &other) {
  if (_address != other._address) return(false);
  if (_meta_class != other._meta_class) return(false);
  return(true);
}

Address ObjectWrapper::get_address() const { 
  return _address; 
}
const MetaClass *ObjectWrapper::get_meta_class() const { 
  return _meta_class; 
}
Object *ObjectWrapper::get_object() const {
  const MetaClass *mc = _meta_class;
  while (mc != 0) {
    if (mc->get_instance_name() == Object::get_class_name()) {
      return((Object *)_address);
    }
    mc = mc->get_link_meta_class();
  }
  return(0);
}
    
void ObjectWrapper::print_debug() const {
  LString name;
  if (_meta_class != NULL) {
    name = _meta_class->get_instance_name();
  }
  // wouldn't it be nice to have a printer...
  cerr << name.c_str() << " @ " << _address << "\n";
}

//Walker::ApplyStatus ObjectWrapper::walk(Walker &walk) const {
//  get_meta_class()->walk(get_address(), w);
//}

// These are "const" even though we write things into the address
// or delete things from the address
ObjectWrapper ObjectWrapper::update_meta_class() const {
  const MetaClass *mc = get_meta_class()->get_meta_class(get_address());
  return(ObjectWrapper(get_address(), mc));
}


/*
void ObjectWrapper::construct() const {
  kernel_assert(_meta_class != NULL)
  get_meta_class()->construct_object(get_address());
}
*/
void ObjectWrapper::initialize(InputStream *inputStream) const {
  kernel_assert(_meta_class != NULL)
  get_meta_class()->initialize(*this, inputStream);
}
void ObjectWrapper::destruct(bool called_from_destructor) const {
  kernel_assert(_meta_class != NULL)
  get_meta_class()->destruct(*this, called_from_destructor);
}

bool ObjectWrapper::is_null() const {
  return(get_address() == 0);
}

/*
ObjectWrapper PointerWrapper::deref() const {
  iokernel_assert(!is_null());
  Address a = *(Address*)_address;
  // Why do we need to expose the a...
  // the pointer metaclass is usually the same one for all subclasses
  // of Object. so we use this extra level of indirection
  // to get at the REAL meta class.
  MetaClass *mc = _meta_class->get_base_type()->get_meta_class( a );
  return(ObjectWrapper(a, mc));
}

void PointerWrapper::store(ObjectWrapper obj) const;

ObjectWrapper AggregateWrapper::get_field(const FieldDescription &d) {
  return ObjectWrapper(_address + d->offset,
		       d->metaClass);
}
*/
