#include "common/system_specific.h"
#include "suif_clone_stream.h"

#include "iokernel/metaclass_visitor.h"

#include "common/suif_map.h"


SuifCloneStream::SuifCloneStream( ObjectFactory* of,
                                  MetaClassVisitor<ObjectClone>* object_clone_visitor ) :
    CloneStream( of ),
    _object_clone_visitor( object_clone_visitor ),
    _finish_map( new FinishMap ) {

}


SuifCloneStream::~SuifCloneStream() {
  // the delete below shouldn't be there but if I remove it gcc screams about some
  // undefined type. therefor the _object_clone_visitor = 0 assignment
  _object_clone_visitor = 0;
  delete _object_clone_visitor;
}


void SuifCloneStream::clonestream_write(const ObjectWrapper &obj,
					bool addressable,
					FinishClone finish_function ) {
  Address instance = obj.get_address();
  const MetaClass* metaClass = obj.get_meta_class();

  if ( finish_function ) {
    FinishInfo finish;
    finish._being_finished = false;
    finish._meta_class =  metaClass;
    finish._finish_function = finish_function;
    //    (*_finish_map)[ instance ] = finish;
    _finish_map->enter_value(instance, finish);
  }

  CloneStream::write( obj, addressable );
}


void SuifCloneStream::write( const ObjectWrapper &obj,
			     bool addressable ) {
  //Address instance = obj.get_address();
  const MetaClass* metaClass = obj.get_meta_class();

  ObjectClone clone_function = _object_clone_visitor->retrieveEntity( metaClass );
  if ( !clone_function ) {
    clonestream_write( obj, addressable );
  } else {
    clone_function( this, obj, addressable );
  }
}

ObjectClone SuifCloneStream::retrieve_object_clone_behavior(
                 MetaClass* meta_class) {
  return _object_clone_visitor->retrieveEntity( meta_class );
}


void SuifCloneStream::finish( Address address ) {
  FinishMap::iterator it = _finish_map->find( address );
  if ( it != _finish_map->end() ) {
    FinishInfo& info = (*it).second;
    suif_assert_message( !info._being_finished, ( "Called finish on an object already being finished (Cycle) ") );
    info._being_finished = true;
    info._finish_function( this, ObjectWrapper(address, info._meta_class) );
  }
}




#ifdef AG

// --------------------- DEFAULT CLONING -------------------

void default_object_clone( SuifCloneStream* scs,
                    Address instance,
                    MetaClass* meta_class,
                    bool addressable ) {
  scs->clonestream_write( instance, meta_class, addressable );
}



// --------------------- EXECUTABLE CLONING -------------------

void executable_object_clone( SuifCloneStream* scs,
                    Address instance,
                    MetaClass* meta_class,
                    bool addressable ) {
  scs->clonestream_write( instance, meta_class, addressable );
  ExecutableObject* object = (ExecutableObject*)instance;
  for ( Iter<LabelSymbol*> iter = object->get_label_symbol_iterator();
        iter.is_valid();
        iter.next() ) {
    LabelSymbol* label = iter->current();
    scs->clone_stream_write( label, label->getMetaClass(),
                             true, &sto_finish_clone );
  }
}

#endif
