#include "common/system_specific.h"
#include "meta_class.h"
#include "clone_stream.h"
#include "walker.h"
#include "meta_class_iter.h"

// #ifdef PGI_BUILD
// #include <new>
// #else
// #include <iostream.h>
// #include <new.h>
// #endif
#include <new>
#include <iostream>//jul modif
using namespace std;//jul modif

static const LString meta_class_class_name("MetaClass");

const LString& MetaClass::get_class_name() {return meta_class_class_name;}



MetaClass::MetaClass( LString meta_class_name ) :
  _pre_init( 0 ),
  _post_init( 0 ),
  _meta_class_name( meta_class_name ),
  _size( 0 ),
  _alignment(1),
  _meta_class_id( 0 ),
  _owning_factory( 0 ),
  _constructor_function( 0 ),
  _has_constructed_object(false)
{
}


MetaClass::~MetaClass() {
  _meta_class = 0;
}


const LString& MetaClass::get_instance_name() const {
  return _meta_class_name;
}


size_t MetaClass::get_size_of_instance() const {
  return _size;
}

size_t MetaClass::get_alignment_of_instance() const {
  return _alignment;
}

void MetaClass::read ( const ObjectWrapper &obj,
		       InputStream* inputStream ) const {
  kernel_assert_message( false, ( "Called abstract method MetaClass::read" ) );
}


void MetaClass::write( const ObjectWrapper &obj,
		       OutputStream* outputStream ) const {
  kernel_assert_message( false, ( "Called abstract method MetaClass::write" ) );
}


const MetaClass* MetaClass::get_meta_class( Address address ) const {
  return (MetaClass *)this;
}


void MetaClass::set_constructor_function( ConstructorFunction constructor_function ) {
  _constructor_function = constructor_function;
}


ConstructorFunction MetaClass::get_constructor_function() const {
  return _constructor_function;
}

void MetaClass::construct_object( Address address ) const {
  ((MetaClass*)this)->_has_constructed_object = true;
  ConstructorFunction func = get_constructor_function();
  if ( func ) {
     func( address );
  }
}

bool MetaClass::has_constructed_object() const {
  return _has_constructed_object;
}

String MetaClass::get_debug_text() const {
  // At this level all we know is the name and the size
  // and whether it is generic.
  String result = 
    String("MetaClass: ") + get_instance_name() 
    + String(" [") + String(get_size()) +"]\n";
  return(result);
}

void MetaClass::print_debug() const {
  String str = get_debug_text();
  cout << str;
}

void MetaClass::set_size( size_t size ) {
  _size = size;
}

size_t MetaClass::get_size() const {
    return _size;
    }

void MetaClass::set_alignment( size_t alignment ) {
  _alignment = alignment;
}



void MetaClass::adjust_field_offsets() {
}


void MetaClass::set_meta_class_id( MetaClassId id ) {
  _meta_class_id = id;
}


MetaClassId MetaClass::get_meta_class_id() const {
  return _meta_class_id;
}


void MetaClass::set_meta_class_of_object( Address instance ) const {
  //  ((Object*)instance)->set_meta_class( this );
}


Iterator* MetaClass::get_iterator( ConstAddress instance, Iterator::Contents contents ) const {
  return 0;
}

Iterator* MetaClass::get_iterator( const ObjectWrapper &obj,
				   Iterator::Contents contents ) const {
  return(get_iterator(obj.get_address(), contents));
}


bool MetaClass::is_elementary() const {
  return true;
}


void MetaClass::destruct( const ObjectWrapper &obj,
			  bool called_from_destructor) const {
}

void MetaClass::constructor_function( Address place ) {
  new (place) MetaClass();
}



MetaClass* MetaClass::get_link_meta_class() const {
  return 0;
}


bool MetaClass::defines_a_subtype_of( const MetaClass* m ) const {
  for ( const MetaClass* current = this;
        current;
        current = current->get_link_meta_class() ) {
    if ( current == m ) return true;
  }
  return false;
}


void emptyConstructorFunction( Address place ) {
  return;
}


void MetaClass::initialize( const ObjectWrapper &obj,
			    InputStream* inputStream ) const {
  kernel_assert(this == obj.get_meta_class());
  if ( _pre_init ) {
    _pre_init( obj, true, inputStream );
  }
  if ( _post_init ) {
    _post_init( obj, true, inputStream );
  }
}


void MetaClass::set_owning_factory( ObjectFactory* owning_factory ) {
  _owning_factory = owning_factory;
}


ObjectFactory* MetaClass::get_owning_factory() const {
  return _owning_factory;
}

Walker::ApplyStatus MetaClass::walk(const Address address,Walker &w) const {
    // does nothing - actual work in derived classes
    return Walker::Continue;
    }

Walker::ApplyStatus MetaClass::walk(const ObjectWrapper &obj,
				    Walker &w) const {
    // just reroute to the unsafe version.
    return walk(obj.get_address(), w);
    }

bool MetaClass::object_is_kind_of(const LString &className) const {
  const MetaClass *m = this;
  while (m && (className != m->get_instance_name()))
	m = m->get_link_meta_class();
  return m != NULL;
  }

// void MetaClass::debugit(const char *doing,Address to_what) {
    // printf("%s to %08X class %s\n",doing,to_what,(const char *)_meta_class_name);
    // fflush(stdout);
//     }

// void MetaClass::debugit(const char *doing,int i) {
    // printf("%s size  %d class %s\n",doing,i,(const char *)_meta_class_name);
    // fflush(stdout);
//     }

// walk the meta-classes referenced by a given meta class.
void MetaClass::walk_referenced_meta_classes(MetaClassApplier *x) {
    }

MetaClassApplier::~MetaClassApplier() 
{}
