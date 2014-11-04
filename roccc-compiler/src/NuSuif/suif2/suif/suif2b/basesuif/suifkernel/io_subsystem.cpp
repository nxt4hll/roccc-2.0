#include "common/system_specific.h"
#include "io_subsystem.h"
#include "suif_object.h"
#include "iokernel/clone_stream.h"
#include "iokernel/meta_class.h"

InputSubSystem::InputSubSystem( SuifEnv* s )
  : SubSystem( s ) {
}

InputSubSystem::~InputSubSystem() {
}


OutputSubSystem::OutputSubSystem( SuifEnv* s )
  : SubSystem( s ) {
}

OutputSubSystem::~OutputSubSystem() {
}


CloneSubSystem::CloneSubSystem( SuifEnv* s )
  : SubSystem( s ) {
}

CloneSubSystem::~CloneSubSystem() {
}


SuifObject* CloneSubSystem::deep_clone( const SuifObject* object ) {
    if (!object)
	return 0;
  
    return (SuifObject*)deep_clone( (Address)object, object->get_meta_class() );
    }


SuifObject* CloneSubSystem::shallow_clone( const SuifObject* object ) {
    if (!object)
        return 0;
  
    return (SuifObject*)shallow_clone( (Address)object, object->get_meta_class() );
    }

static Address clone_with_stream( Address address,
				  const MetaClass* metaClass,
				Address target,
				CloneStream* stream ) {
    suif_assert_message(stream,("No cloner available - try importing cloner first i.e. import suifcloning\n"));
    return stream->clone( address, metaClass, target );
    }

Address CloneSubSystem::deep_clone( Address address, const MetaClass* metaClass, Address target ) {
    if (address == 0)
	return address;
    return clone_with_stream( address, metaClass, target, get_deep_clone_stream() );
    }


Address CloneSubSystem::shallow_clone( Address address, const MetaClass* metaClass, Address target ) {
    return clone_with_stream( address, metaClass, target, get_shallow_clone_stream() );
    }

