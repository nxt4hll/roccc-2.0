#include "common/system_specific.h"
#if defined(PGI_BUILD) || 1
#include <stdlib.h>
#endif
#include "error_subsystem.h"
#include "suifkernel_forwarders.h"
#include "suif_object.h"
#include "suif_env.h"
#include "module_subsystem.h"
#include "module.h"

#include <stdio.h>


ObjectLocation::ObjectLocation() :
  _is_known(false), _file(emptyString), _line(0), 
  _has_column(false), _column(0)
{
}
ObjectLocation::ObjectLocation(String file, IInteger line) :
  _is_known(true), _file(file), _line(line), 
  _has_column(false), _column(0)
{
}
ObjectLocation::ObjectLocation(String file, IInteger line, IInteger column) :
  _is_known(true), _file(file), _line(line), 
  _has_column(true), _column(column)
{
}


bool ObjectLocation::get_is_known() const {
  return(_is_known);
}
String ObjectLocation::get_file() const {
  return(_file);
}
IInteger ObjectLocation::get_line() const {
  return(_line);
}
bool ObjectLocation::get_has_column() const {
  return(_has_column);
}
IInteger ObjectLocation::get_column() const {
  return(_column);
}
String ObjectLocation::print_to_string() const {
  if (!get_is_known())
    return("unknown_file:");

  String location = _file;
  location += ":";
  location += _line.to_String();
  location += ":";
  if (get_has_column()) {
    location += _column.to_String();
    location += ":";
  }
  return(location);
}

  



ErrorSubSystem::ErrorSubSystem( SuifEnv* suif_env ) : SubSystem( suif_env ) {
}

void ErrorSubSystem::print( const String& message ) {
  fprintf( stderr, "%s\n", message.c_str() );
}

void ErrorSubSystem::error( const SuifObject* obj, const char* file_name,
                       int line_number,
                       const char* module_name,
                       const char* description, va_list args ) {
  // TODO something more intelligent with the object
  kernel_assert_message(obj!=NULL, ("Internal error: expecting a non-NULL argument"));
  fprintf( stderr, "ERROR IN\n" );
  fprintf( stderr, "FILE: %s   LINE: %d MODULE: %s\n", file_name,
                                                       line_number, module_name );
  fprintf( stderr, "%s", get_location(obj).c_str());
  vfprintf( stderr, description, args );
  return;
}

void ErrorSubSystem::error( const char* file_name,
                       int line_number,
                       const char* module_name,
                       const char* description, va_list args ) {
  fprintf( stderr, "ERROR IN\n" );
  fprintf( stderr, "FILE: %s   LINE: %d MODULE: %s\n", file_name,
                                                       line_number, module_name );
  vfprintf( stderr, description, args );
  return;
}

void ErrorSubSystem::warning( const SuifObject* obj, const char* file_name,
                       int line_number,
                       const char* module_name,
                       const char* description, va_list args ) {
  // TODO something more intelligent with the object
  kernel_assert_message(obj!=NULL, ("Internal error: expecting a non-NULL argument"));
  fprintf( stderr, "WARNING IN:\n" );
  fprintf( stderr, "FILE: %s   LINE: %d MODULE: %s\n", file_name,
                                                       line_number, module_name );
  fprintf( stderr, "%s", get_location(obj).c_str());
  vfprintf( stderr, description, args );
}

void ErrorSubSystem::warning( const char* file_name,
                       int line_number,
                       const char* module_name,
                       const char* description, va_list args ) {
  fprintf( stderr, "WARNING IN:\n" );
  fprintf( stderr, "FILE: %s   LINE: %d MODULE: %s\n", file_name,
                                                       line_number, module_name );
  vfprintf( stderr, description, args );
}


void ErrorSubSystem::information( const SuifObject* obj, const char* file_name,
                       int line_number,
                       const char* module_name,
                       int verbosity_level,
                       const char* description, va_list args ) {
  // TODO something more intelligent with the object
  kernel_assert_message(obj!=NULL, 
			("Internal error: expecting a non-NULL argument"));
  fprintf( stderr, "INFORMATIONAL MESSAGE:\n" );
  fprintf( stderr, "FILE: %s   LINE: %d MODULE: %s\n", file_name,
                                                       line_number, module_name );
  fprintf( stderr, "%s", get_location(obj).c_str());
  vfprintf( stderr, description, args );
}


void ErrorSubSystem::information( const char* file_name,
                       int line_number,
                       const char* module_name,
                       int verbosity_level,
                       const char* description, va_list args ) {
  fprintf( stderr, "INFORMATIONAL MESSAGE:\n" );
  fprintf( stderr, "FILE: %s   LINE: %d MODULE: %s\n", file_name,
                                                       line_number, module_name );
  vfprintf( stderr, description, args );
}

String ErrorSubSystem::get_location(const SuifObject *obj) const {
  if (obj == NULL) return emptyString;
  LString style = get_default_location_module();
  if (style == emptyLString) {
    return(obj->getClassName());
  }

  ModuleSubSystem *ms = get_suif_env()->get_module_subsystem();
  Module *module = ms->retrieve_module(style);
  LString location_iface = "get_object_location";
  if (!module->supports_interface(location_iface))
    {
      return(obj->getClassName());
    }
  LocationDispatch pfunc = 
    (LocationDispatch)module->get_interface(location_iface);
  //TODO:
#ifdef MSVC
  if((int)pfunc==0x1)pfunc=NULL;
#endif
  if (pfunc)
    {
      return ((*pfunc)(obj).print_to_string());
    }
  else
    {
      //      cerr << "installed location style" 
      //      	     << style.c_str() << "has no valid print function" << endl;
      return(obj->getClassName());
    }
}


LString ErrorSubSystem::get_default_location_module() const {
  return(_default_location_module);
}

void ErrorSubSystem::set_default_location_module( const LString &name ) {
  _default_location_module = name;
}


























#ifdef AG


void ErrorSubSystem::error( const char* module,
                            const char* file_name, 
                            int line, 
                            const char* text ) {
  print( String(" ERROR: " ) + String( module ) + 
         String(":") + String( file_name ) + 
         String(" ") + String( text ) );
}

void ErrorSubSystem::warning( const char* module, 
                           const char* file_name, 
                           int line, 
                           const char* text ) {
  print( String( "WARNING: " ) + String( module ) + 
         String(":") + String( file_name ) + 
         String(" ") + String( text ) );
}

void ErrorSubSystem::info( const char* module, 
                           const char* file_name, 
                           int line, 
                           const char* text ) {
  print( String("INFO: ") + 
         String( module ) + 
         String(":") + String( file_name ) + 
         String(" ") + String( text ) );
}

#endif

void suif_assert_function( const char* module, 
                           const char* file_name, 
                           int line, 
                           const char* text ) {
  fprintf( stderr,"SUIF Assertion:  %s\n", text );
  fprintf( stderr, "Module: %s\n", module );
  fprintf( stderr, "File Name: %s\n", file_name );
  fprintf( stderr, "Line: %d\n", line );
  abort();
}




