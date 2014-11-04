#include "common/system_specific.h"
#include "object.h"
#include "meta_class.h"
#include "aggregate_meta_class.h"
#include "field_description.h"
#include "aggregate_wrapper.h"


// #ifdef PGI_BUILD
// #include <new>
// #else
// #include <new.h>
// #endif
#include <new>// jul modif

bool is_kind_of_object_meta_class(const MetaClass *meta) {
  while (meta && 
	 (meta->get_instance_name() != Object::get_class_name()))
    {
      meta = meta->get_link_meta_class();
    } 
  return(meta != NULL);
}

const MetaClass* Object::get_meta_class() const {
  return _meta_class;
}


const AggregateMetaClass* Object::get_aggregate_meta_class() const {
  return _meta_class;
}

const LString& Object::getClassName() const {
  return _meta_class->get_instance_name();
}


bool Object::isA( const LString& className ) const {
  return className == Object::getClassName();
}

bool Object::isKindOf( const LString& className ) const {
  for (  const MetaClass* m = get_meta_class(); 
	 m ; m = m->get_link_meta_class() ) {
    if ( className == m->get_instance_name() ) return true;
  }
  return false;
}




void Object::constructor_function( Address instance ) {
     new (instance) Object;
}


Object::Object() :
  _meta_class( 0 ) {
}


Object::~Object() {
  if ( _meta_class ) _meta_class->destruct( this, true );
}


void Object::set_meta_class( const MetaClass* meta_class ) {
  //  kernel_assert(is_kind_of<AggregateMetaClass>(meta_class));
  //  _meta_class = to<AggregateMetaClass>(meta_class);
  _meta_class = (const AggregateMetaClass *)meta_class;
}


static const LString object_class_name("Object");

const LString &Object::get_class_name() {
  return object_class_name;
}

#if 0
Address Object::get_member( const LString& name,
                            Address& return_address,
                            const MetaClass*&  return_meta_class ) const {
  kernel_assert_message( _meta_class, ("Empty MetaClass") );

  FieldDescription* field_description = 
    _meta_class->get_field_description( (void*)address,  name );
  if ( field_description ) {
    //    return_address = ((const Object *)this) + field_description->offset;
    AggregateWrapper agg_obj((Address)this, _meta_class);
    ObjectWrapper obj = field_description->build_object(agg_obj);
    obj = obj.update_meta_class();
    return_address = obj.get_address();
    return_meta_class = obj.get_meta_class();
    //return_address = ((Byte*)this) + field_description->offset;
    //    return_meta_class = field_description->
    //                           metaClass->get_meta_class( return_address );
  } else {
    return_address = 0;
    return_meta_class = 0;
  }
  return return_address;
}
#endif

FieldWrapper Object::get_member( const LString& name ) const {
  kernel_assert_message( _meta_class, ("Empty MetaClass") );

  AggregateWrapper obj((void*)this, _meta_class);
  return(obj.get_field(name));
}


#ifdef AG
Object& Object::operator=(const Object&) {
  kernel_assert(false);
  return(*this);
}
Object::Object(const Object&) :
  _meta_class(0)
{
  kernel_assert(false);
}
#endif









