#include "common/system_specific.h"
#include "suif_env.h"

#include "iokernel/object_factory.h"
#include "iokernel/aggregate_meta_class.h"
#include "iokernel/object_stream.h"
#include "iokernel/binary_streams.h"
#include "iokernel/pointer_meta_class.h"
#include "iokernel/helper.h"

#include "dll_subsystem.h"
#include "module_subsystem.h"
#include "error_subsystem.h"
#include "print_subsystem.h"
#include "io_subsystem_default_impl.h"
#include "real_object_factory.h"
#include "suif_object.h"

#include "common/suif_list.h"

#include <stdarg.h>
#include <stdio.h>



class SuifEnvObjectFactory : public ObjectFactory {
public:
  SuifEnvObjectFactory();

  virtual void error(  const char* file_name,
                       int line_number,
                       const char* module_name,
                       const char* description, va_list ap );

  virtual void warning( const char* file_name,
                       int line_number,
                       const char* module_name,
                       const char* description, va_list ap );

  virtual void information( const char* file_name,
                       int line_number,
                       const char* module_name,
                       int verbosity_level,
                       const char* description, va_list ap );

};

SuifEnvObjectFactory::SuifEnvObjectFactory() {
}


void SuifEnvObjectFactory::error(  const char* file_name,
                       int line_number,
                       const char* module_name,
                       const char* description, va_list ap ) {
  get_suif_env()->error( file_name, line_number, module_name, description, ap );
}


void SuifEnvObjectFactory::warning( const char* file_name,
                       int line_number,
                       const char* module_name,
                       const char* description, va_list ap ) {
  get_suif_env()->warning( file_name, line_number, module_name, description, ap );
}


void SuifEnvObjectFactory::information( const char* file_name,
                       int line_number,
                       const char* module_name,
                       int verbosity_level,
                       const char* description, va_list ap ) {
  get_suif_env()->information( file_name, line_number, module_name, verbosity_level, description, ap );
}




SuifEnv* create_suif_env() {
     SuifEnv* suif_env = new SuifEnv;
     suif_env->init();
     return suif_env;
}


SuifEnv::SuifEnv() :
  input_sub_system( 0 ),
  output_subsystem( 0 ),
  cloneSubSystem( 0 ),
  _dll_subsystem( 0 ),
  _module_subsystem( 0 ),
  _error_subsystem( 0 ),
  _print_subsystem( 0 ),
  _type_builder( 0 ),
  _object_factory( 0 ),
  factories( new list<RealObjectFactory*> ),
  _file_set_block( 0 ),
  rudimentaryAddressMap( 0 )
{
}


SuifEnv::~SuifEnv() {
  delete (SuifObject*)_file_set_block;
  delete_list_and_elements( factories );
  delete rudimentaryAddressMap;
  delete _object_factory;

  delete input_sub_system;
  delete output_subsystem;
  delete cloneSubSystem;
  delete _dll_subsystem;
  delete _module_subsystem;
  delete _error_subsystem;
  delete _print_subsystem;
}


void SuifEnv::init() {
  ObjectFactory* of = new SuifEnvObjectFactory;
  set_object_factory( of );
  of -> init( this );

  AggregateMetaClass*
        suifObjectMC = of->create_object_meta_class(
                                 SuifObject::get_class_name(),
                                 sizeof( SuifObject ),
                                 SuifObject::constructorFunction,
            (AggregateMetaClass*)of->lookupMetaClass( Object::get_class_name() ) );

  PointerMetaClass* pointerToSuifObjectR = of->get_pointer_meta_class( suifObjectMC, false );

  suifObjectMC -> add_field_description( "parent", pointerToSuifObjectR, OFFSETOF( SuifObject, parent ) );

  // Brick::init_meta_class(this);

  input_sub_system = new InputSubSystemDefaultImplementation( this );
  output_subsystem = new OutputSubSystemDefaultImplementation( this );
  cloneSubSystem = new CloneSubSystemDefaultImplementation( this );
  _dll_subsystem = new DLLSubSystem( this );
  _module_subsystem = new ModuleSubSystem( this );
  _error_subsystem = new ErrorSubSystem( this );
  _print_subsystem = new PrintSubSystem( this );
}




void SuifEnv::set_object_factory( ObjectFactory* _object_factory ) {
    this->_object_factory = _object_factory;
}


ObjectFactory* SuifEnv::get_object_factory() const {
  return _object_factory;
}


void SuifEnv::add_object_factory( RealObjectFactory* of ) {
  of->init( this );
  factories->push_back( of );
}


RealObjectFactory* SuifEnv::get_object_factory( const LString& name ) const {
  list<RealObjectFactory*>::iterator it = factories->begin(), end = factories->end();
  while ( it != end ) {
    if ( (*it)->getName() == name ) return (*it);
    ++it;
  }
  return 0;
}

void SuifEnv::read( const String& inputFileName ) {
  FileSetBlock *file_set_block = read_more(inputFileName);
  delete (SuifObject *)_file_set_block;
  _file_set_block = file_set_block;
}

FileSetBlock *SuifEnv::read_more( const String& inputFileName ) const {
  suif_assert( input_sub_system );
  return input_sub_system->read( inputFileName );
}

void SuifEnv::write( const String& outputFileName ) const {
  suif_assert( output_subsystem );
  output_subsystem->write( outputFileName );
}



CloneSubSystem* SuifEnv::get_clone_subsystem() const {
  return cloneSubSystem;
}


void SuifEnv::set_file_set_block( FileSetBlock* file_set_block ) {
    _file_set_block = file_set_block;
}


FileSetBlock* SuifEnv::get_file_set_block() const {
  return _file_set_block;
}


ModuleSubSystem* SuifEnv::get_module_subsystem() const {
  return _module_subsystem;
}

void SuifEnv::set_module_subsystem( ModuleSubSystem* subSystem ) {
  _module_subsystem = subSystem;
}


DLLSubSystem* SuifEnv::get_dll_subsystem() const {
  return _dll_subsystem;
}


void SuifEnv::set_dll_subsystem( DLLSubSystem* subSystem ) {
  _dll_subsystem = subSystem;
}


ErrorSubSystem* SuifEnv::get_error_subsystem() const {
  return _error_subsystem;
}


void SuifEnv::set_error_subsystem( ErrorSubSystem* subSystem ) {
  _error_subsystem = subSystem;
}


PrintSubSystem* SuifEnv::get_print_subsystem() const {
  return _print_subsystem;
}


void SuifEnv::set_print_subsystem( PrintSubSystem* subSystem ) {
  _print_subsystem = subSystem;
}


TypeBuilder* SuifEnv::get_type_builder() const {
  return _type_builder;
}


void SuifEnv::set_type_builder( TypeBuilder* type_builder ) {
  _type_builder = type_builder;
}


void SuifEnv::error( SuifObject* obj, const char* file_name,
                       int line_number,
                       const char* module_name,
                       const char* description, va_list args ) {
  suif_assert( _error_subsystem );
  _error_subsystem->error( obj, file_name, line_number, module_name, description, args );
}


void SuifEnv::warning( SuifObject* obj, const char* file_name,
                       int line_number,
                       const char* module_name,
                       const char* description, va_list args ) {
  suif_assert( _error_subsystem );
  _error_subsystem->warning( obj, file_name, line_number, module_name, description, args );
}


void SuifEnv::information( SuifObject* obj, const char* file_name,
                       int line_number,
                       const char* module_name,
                       int verbosity_level,
                       const char* description, va_list args ) {
  suif_assert( _error_subsystem );
  _error_subsystem->information( obj, file_name, line_number, module_name, verbosity_level, description, args );
}


void SuifEnv::error( const char* file_name,
                       int line_number,
                       const char* module_name,
                       const char* description, va_list args ) {
  suif_assert( _error_subsystem );
  _error_subsystem->error( file_name, line_number, module_name, description, args );
}


void SuifEnv::warning( const char* file_name,
                       int line_number,
                       const char* module_name,
                       const char* description, va_list args ) {
  suif_assert( _error_subsystem );
  _error_subsystem->warning( file_name, line_number, module_name, description, args );
}


void SuifEnv::information( const char* file_name,
                       int line_number,
                       const char* module_name,
                       int verbosity_level,
                       const char* description, va_list args ) {
  suif_assert( _error_subsystem );
  _error_subsystem->information( file_name, line_number, module_name, verbosity_level, description, args );
}

void SuifEnv::import_module(const LString &module_name )
{
  get_dll_subsystem()->load_and_initialize_DLL( module_name );
}

void SuifEnv::require_module(const LString &module_name )
{
  get_dll_subsystem()->require_DLL( module_name );
}
void SuifEnv::require_DLL(const LString &module_name )
{
  get_dll_subsystem()->require_DLL( module_name );
}

void SuifEnv::register_module(Module *module) {
  get_module_subsystem()->register_module(module);
}

String SuifEnv::get_location(const SuifObject *obj) const
{
  return(get_error_subsystem()->get_location(obj));
}
String SuifEnv::to_string(const LString &style,
			  const SuifObject *obj) const {
  return(get_print_subsystem()->print_to_string(style, obj));
}

/* Use the default printer */
String SuifEnv::to_string(SuifObject *obj) const {
  return(get_print_subsystem()->print_to_string(obj));
}


SuifEnv::SuifEnv(const SuifEnv &other) :
  input_sub_system( 0 ),
  output_subsystem( 0 ),
  cloneSubSystem( 0 ),
  _dll_subsystem( 0 ),
  _module_subsystem( 0 ),
  _error_subsystem( 0 ),
  _print_subsystem( 0 ),
  _type_builder( 0 ),
  _object_factory( 0 ),
  factories( new list<RealObjectFactory*> ),
  _file_set_block( 0 ),
  rudimentaryAddressMap( 0 )
{
  suif_assert(false);
}

SuifEnv& SuifEnv::operator=(const SuifEnv &) {
  suif_assert(false);
  return(*this);
}




