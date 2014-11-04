#include "common/system_specific.h"
#include "stl_meta_class.h"
#include "iokernel_forwarders.h"
#include "object_wrapper.h"

// #ifdef PGI_BUILD
// #include <new>
// #else
// #include <new.h>
// #endif
#include <new>


MetaClass* TypeLessSTLDescriptor::get_element_meta_class() const {
    return _element_meta_class;
}


size_t TypeLessSTLDescriptor::get_size_of_instance() const {
     return _size;
}


ConstructorFunction TypeLessSTLDescriptor::get_constructor_function() const {
  return _constructor_function;
}


DestructorFunction TypeLessSTLDescriptor::get_destructor_function() const {
  return _destructor_function;
}



const LString& STLMetaClass::get_class_name() {
  // a ListMetaClass is not persistent. It will therefore return the class name of
  // its persistent base class
  return ListMetaClass::get_class_name();
}

STLMetaClass::STLMetaClass( LString metaClassName ) :
  ListMetaClass( metaClassName ),
  _stl_descriptor(0)
{
};

STLMetaClass::~STLMetaClass() {
  delete _stl_descriptor;
}




void STLMetaClass::set_descriptor( TypeLessSTLDescriptor* stl_descriptor ) {
    _stl_descriptor = stl_descriptor;
    _element_meta_class = stl_descriptor->get_element_meta_class();
    set_size( stl_descriptor->get_size_of_instance() );
}


Iterator* STLMetaClass::get_iterator( ConstAddress instance,
				      Iterator::Contents contents ) const {
  Iterator* return_value = 0;
  if ( instance && ( contents == Iterator::Owned ) ) {
    return_value = _stl_descriptor->get_iterator( instance );

  }
  return return_value;
}

void STLMetaClass::constructor_function( Address place ) {
  new (place) STLMetaClass();
}


void STLMetaClass::copy_over( Address target, GenericList* source ) const {
  int instanceSize = _element_meta_class->get_size_of_instance();
  int i = source->count;
  Address space = source->space;
  Iterator* iterator = get_iterator( target );
  while (i--) {
   iterator->add( (Byte*)space );
   space = ((Byte*)space) + instanceSize;
  }
  delete iterator;
  delete (char*)(source->space);
  //  delete source;
}


ConstructorFunction STLMetaClass::get_constructor_function() const {
  return _stl_descriptor->get_constructor_function();
}


void STLMetaClass::set_constructor_function( ConstructorFunction constructorFunction ) {
  kernel_assert( false );
}



void STLMetaClass::destruct( const ObjectWrapper &obj,
			     bool called_from_destructor ) const {
  kernel_assert(obj.get_meta_class() == this);
  // an stl object is never destroyed from the destructor
  kernel_assert( !called_from_destructor );

  _stl_descriptor->get_destructor_function()( obj );
}

