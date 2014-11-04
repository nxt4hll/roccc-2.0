#include "common/system_specific.h"
#include "real_object_factory.h"

#include "iokernel/object_factory.h"

#include "suif_env.h"


RealObjectFactory::RealObjectFactory() :
  _object_factory(0), _suif_env(0) {
}


void RealObjectFactory::init( SuifEnv* suif_env ) {
  _suif_env = suif_env;

  _object_factory = _suif_env->get_object_factory();
  if ( _object_factory ) init_io( _object_factory );

  CloneSubSystem* css = _suif_env -> get_clone_subsystem();
  if ( css ) init_cloning( css );

  PrintSubSystem* pss = _suif_env -> get_print_subsystem();
  if ( pss ) init_printing( pss );
}


RealObjectFactory::~RealObjectFactory() {
}


void RealObjectFactory::init_io( ObjectFactory* of ) {

}


void RealObjectFactory::init_cloning( CloneSubSystem* scs ) {


}

void RealObjectFactory::init_printing( PrintSubSystem* pss ) {

}



void* RealObjectFactory::create_empty_object( const MetaClass* metaClass ) {
  suif_assert( _object_factory );
  return _object_factory->create_empty_object( metaClass );
}

MetaClass* RealObjectFactory::lookupMetaClass( const LString& metaClassName ) {
  suif_assert( _object_factory );
  return _object_factory->lookupMetaClass( metaClassName );
}

ObjectFactory* RealObjectFactory::get_object_factory() {
  return _object_factory;
}

const LString& RealObjectFactory::getName() {
  return emptyLString;
}

SuifEnv *RealObjectFactory::get_suif_environment() {
    return _suif_env;
    }



