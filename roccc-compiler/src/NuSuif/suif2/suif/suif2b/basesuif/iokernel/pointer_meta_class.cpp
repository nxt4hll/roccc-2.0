#include "common/system_specific.h"
#include "pointer_meta_class.h"
#include "pointer_wrapper.h"

#include "object_stream.h"
#include "iokernel_forwarders.h"
#include "object_factory.h"

#ifdef DEBUG
//#include <iostream.h>
#include <iostream> //jul modif

#endif

// #if defined(PGI_BUILD) || defined(MSVC)
// #include <new>
// #else
// #include <new.h>
// #endif
#include <new>//jul modif

using namespace std;

static const LString pointer_meta_class_class_name("PointerMetaClass");

const LString &PointerMetaClass::get_class_name() {
  return pointer_meta_class_class_name;
}

struct X_void_ptr {
    char ch;
    void * field;
    };

static size_t alignment_of_void_ptr() {
    return OFFSETOF(X_void_ptr,field);
    }


void PointerMetaClass::write(const ObjectWrapper &obj,
			     OutputStream* outputStream ) const {
    Address instance = obj.get_address();
    kernel_assert(obj.get_meta_class() == this);
    PointerWrapper ptr_obj(instance, this);

    if ( _pointer_owns_object ) {
    	if ( _is_static ) {
	    outputStream->write_static_pointer( ptr_obj );
    	    } 
        else {
      	    outputStream->write_owning_pointer( ptr_obj );
    	    }
    	} 
    else if (_needs_cloning) {
	outputStream->write_defining_pointer( ptr_obj );
	}
    else {
    	outputStream->write_reference( ptr_obj );
  	}
    }

void PointerMetaClass::read( const ObjectWrapper &obj,
			     InputStream* inputStream ) const {
    Address instance = obj.get_address();
    kernel_assert(obj.get_meta_class() == this);
    PointerWrapper ptr_obj(instance, this);

    if (_needs_cloning) {
	inputStream->read_defining_pointer( ptr_obj );
        }
    else if ( _pointer_owns_object ) {
    	if ( _is_static ) {
	    inputStream->read_static_pointer( ptr_obj );
    	    } 
	else {
      	    inputStream->read_owning_pointer( ptr_obj );
    	    }
  	} 
    else {
    	inputStream->read_reference( ptr_obj );
  	}
    }


MetaClass* PointerMetaClass::get_base_type() const {
  return _base_type;
}

Iterator* PointerMetaClass::get_iterator( ConstAddress instance, Iterator::Contents contents )  const {
  Iterator* return_value = 0;

  if ( ( _pointer_owns_object &&  (contents != Iterator::Referenced) ) ||
       ( (!_pointer_owns_object) && (contents == Iterator::Referenced) ) ) {
    Address address = *(Address*)instance;
    if ( address ) {
      const MetaClass* metaClass = _base_type->get_meta_class( address );
      if ( !metaClass ) metaClass = _base_type;
      return_value = new SingleElementIterator( address, metaClass );
    }
  }
  return return_value;
}

bool PointerMetaClass::is_elementary() const {
  return false;
}


PointerMetaClass::PointerMetaClass( LString name, MetaClass* base_type, 
				    bool pointer_owns_object, 
				    bool is_static,
				    bool needs_cloning ) :
   MetaClass( name ),
   _base_type( base_type ),
   _ptr_to_base_type( 0 ),
   _pointer_owns_object( pointer_owns_object ),
   _is_static( is_static ),
   _needs_cloning(needs_cloning)  {
     // if _is_static => _pointer_owns_object
  kernel_assert( !_is_static || ( _is_static && _pointer_owns_object ) );
}


void PointerMetaClass::constructor_function( Address place ) {
  new (place) PointerMetaClass;
}


void PointerMetaClass::destruct( const ObjectWrapper &obj,
				 bool called_from_destructor ) const {
  Address address = obj.get_address();
  kernel_assert(obj.get_meta_class() == this);

  // is never called from a destructor
  kernel_assert( !called_from_destructor );

  if ( _pointer_owns_object ) {
    Address object_address = *(Address*)address;
    if ( object_address ) {
      const MetaClass *mc = _base_type -> get_meta_class( object_address );
      mc-> destruct( ObjectWrapper(object_address, mc), false );
      delete (char*)object_address;
      *(Address*)address = 0; // just to be nice
    }
  }
}


void PointerMetaClass::initialize( const ObjectWrapper &obj,
				   InputStream* inputStream ) const {
  Address address = obj.get_address(); 
  kernel_assert(obj.get_meta_class() == this);

   // debugit("initializing pointer ",address);
  if ( _pre_init ) {
    _pre_init( obj, true, inputStream );
  }
  Address objectAddress =  *(Address*)address;


  // test for whether it's already there
  if ( _is_static ||
       (  _pointer_owns_object && inputStream->exists_in_input_stream( objectAddress ) &&
          (!inputStream->was_already_visited( objectAddress ) ) ) ) {
    if ( !_is_static ) {
      inputStream->set_already_visited( objectAddress );
    }
    const MetaClass *mc = 
      get_base_type()->get_meta_class( (Byte*)objectAddress );
    mc->initialize( ObjectWrapper(*(Address*)address, mc), inputStream );
  }
  if ( _post_init ) {
    _post_init( obj, true, inputStream );
  }
}

MetaClass* PointerMetaClass::get_link_meta_class() const {
  PointerMetaClass* non_const_this = (PointerMetaClass*)this;
  if ( !_ptr_to_base_type ) {
    // initialize cache
   MetaClass* base_type_link = _base_type->get_link_meta_class();
   non_const_this->_ptr_to_base_type = base_type_link ? 
              _owning_factory->get_pointer_meta_class( base_type_link, _pointer_owns_object ) : 
              0;
 }
 return non_const_this->_ptr_to_base_type;
}


bool PointerMetaClass::is_owning_pointer() const {
  return _pointer_owns_object;
}

bool PointerMetaClass::needs_cloning() const {
  return _needs_cloning;
  }

void PointerMetaClass::adjust_field_offsets() {
    int size_of_void_ptr = sizeof(void *);
    set_size( size_of_void_ptr);
    set_alignment(alignment_of_void_ptr());

    MetaClass::adjust_field_offsets();
    }



Walker::ApplyStatus PointerMetaClass::walk(const Address address,Walker &w) const {
    MetaClass *link = get_base_type();

    Address ** ptr = (Address **)address;
    if (!ptr)
	return Walker::Continue;
    if (!(*ptr))
	return Walker::Continue;
    if (w.is_changed(*ptr))
	return Walker::Continue;
    if (w.is_walkable(*ptr,_pointer_owns_object,link))
        return link->walk(*ptr,w);
    return Walker::Continue;
    }

void PointerMetaClass::walk_referenced_meta_classes(MetaClassApplier *x) {
    if (_ptr_to_base_type)
	(*x)(_ptr_to_base_type);
    }
