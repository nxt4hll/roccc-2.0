#include "common/system_specific.h"
#include "aggregate_wrapper.h"
#include "cast.h"
#include "union_meta_class.h"
#include "field_description.h"

AggregateWrapper::AggregateWrapper(Address address,
				   const AggregateMetaClass *mc) :
  _meta_class(mc), _address(address)
{
}
AggregateWrapper::AggregateWrapper(const ObjectWrapper &obj) :
  _meta_class(NULL), _address(0)
{
  kernel_assert_message(is_aggregate(obj), 
			("Non-aggregate object passed to constructor"));
  _meta_class = to<AggregateMetaClass>(obj.get_meta_class());
  _address = obj.get_address();
}

AggregateWrapper::AggregateWrapper(const AggregateWrapper &other) :
  _meta_class(other.get_meta_class()), _address(other.get_address())
{}
  
AggregateWrapper &AggregateWrapper::operator=(const AggregateWrapper &other) {
  _meta_class = other.get_meta_class();
  _address = other.get_address();
  return(*this);
}

Address AggregateWrapper::get_address() const {
  return(_address);
}
const AggregateMetaClass *AggregateWrapper::get_meta_class() const {
  return(_meta_class);
}


size_t AggregateWrapper::get_field_count() const {
  if (is_null()) return(0);
  return(_meta_class->get_field_count(get_address()));
}


#if 0
FieldWrapper AggregateWrapper::get_field(size_t id) const {
  kernel_assert_message(id < get_field_count(), ("invalid field #%d",id));
  FieldDescription *fd = 
    _meta_class->get_field_description(get_address(), id);
  return(FieldWrapper(fd->build_object(get_object()),
		      fd->get_member_name()));
}
#endif

FieldDescription *AggregateWrapper::get_field_description(size_t id) const {
  if (id >= get_field_count()) return NULL;
  FieldDescription *fd = 
    _meta_class->get_field_description(get_address(), id);
  return(fd);
}


// This does NOT allow for iteration over all of the
// fields for a union.  just the one the tag refers to
size_t AggregateWrapper::get_proper_field_count() const {
  if (is_null()) return(0);
  return(_meta_class->get_proper_field_count(get_address()));
}

#if 0
FieldWrapper AggregateWrapper::get_proper_field(size_t id) const {
  kernel_assert_message(id < get_proper_field_count(), 
			("invalid proper field #%d",id));
  return(get_field(id));
}
#endif

FieldDescription *AggregateWrapper::get_proper_field_description(size_t id) const {
  kernel_assert_message(id < get_proper_field_count(), 
			("invalid proper field #%d",id));
  return(get_field_description(id));
}

FieldDescription *AggregateWrapper::get_field_description(const LString &field) const {
  FieldDescription *fd = _meta_class->get_field_description(get_address(),
							    field);
  return(fd);
}

bool AggregateWrapper::has_field(const LString &name) const {
  return(get_field_description(name) != NULL);
}
  

FieldWrapper AggregateWrapper::get_field(const LString &name) const {
  FieldDescription *fd = _meta_class->get_field_description(get_address(),
							    name);
  kernel_assert_message(fd != NULL, ("no field named %s", name.c_str()));
  
  return(FieldWrapper(fd->build_object(get_object()),
		      fd->get_member_name()));
}
  
ObjectWrapper AggregateWrapper::get_object() const {
  return(ObjectWrapper(get_address(), get_meta_class()));
}

bool AggregateWrapper::is_null() const {
  return(get_address() == NULL);
}

bool AggregateWrapper::is_aggregate(const ObjectWrapper &obj) {
  const MetaClass *mc = obj.get_meta_class();
  return(is_kind_of<AggregateMetaClass>(mc));
}
