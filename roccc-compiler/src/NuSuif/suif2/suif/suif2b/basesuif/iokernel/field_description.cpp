#include "field_description.h"
#include "object_wrapper.h"
#include "aggregate_wrapper.h"
#include "meta_class.h"
#include <stdio.h>

FieldDescription::FieldDescription(size_t off, const MetaClass *mc, 
				   LString memberName) :
  offset(off), metaClass(mc), memberName(memberName) 
{}
size_t FieldDescription::get_offset() const 
{
  return(offset); 
}
const LString &FieldDescription::get_member_name() const { 
  return(memberName); 
}
const MetaClass *FieldDescription::get_meta_class() const {
  return(metaClass); 
}

  // Mutators. This should RARELY be needed.
void FieldDescription::set_offset(size_t off) { 
  offset = off; 
}

  // utility functionality
ObjectWrapper FieldDescription::build_object(Address base_address) const {
  return(ObjectWrapper(get_address(base_address), 
		       metaClass));
}

ObjectWrapper FieldDescription::build_object(const AggregateWrapper &base_obj) const {
  return(build_object(base_obj.get_address()));
}
ObjectWrapper FieldDescription::build_object(Object *base_obj) const {
  return(build_object(ObjectWrapper(base_obj)));
}


Address FieldDescription::get_address(Address base_address) const {
  return(((Byte*)base_address) + offset);
}
void
FieldDescription::print_debug() const 
{
  printf("%s", get_debug_text().c_str());
}

String
FieldDescription::get_debug_text() const 
{
  return(String(get_member_name()) + " " 
	 + get_meta_class()->get_instance_name() + " " +
	 String(get_offset()) + 
	 "[" + String(get_meta_class()->get_size_of_instance()) + "]");
}


