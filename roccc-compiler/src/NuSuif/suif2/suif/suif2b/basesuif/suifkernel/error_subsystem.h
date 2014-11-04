#ifndef SUIFKERNEL_ERROR_SUBSYSTEM_H
#define SUIFKERNEL_ERROR_SUBSYSTEM_H

#include "subsystem.h"
#include "suifkernel_forwarders.h"
#include "common/i_integer.h"

#include <stdarg.h>

class ObjectLocation {
  bool _is_known;
  String _file;
  IInteger _line;
  bool _has_column;
  IInteger _column;
public:
  ObjectLocation();
  ObjectLocation(String file, IInteger line);
  ObjectLocation(String file, IInteger line, IInteger column);
  bool get_is_known() const;
  String get_file() const;
  IInteger get_line() const;
  bool get_has_column() const;
  IInteger get_column() const;
  String print_to_string() const;
};

typedef ObjectLocation ( *LocationDispatch )( const SuifObject * );


class ErrorSubSystem : public SubSystem {
public:
  ErrorSubSystem( SuifEnv* suif_env );

  virtual void print( const String& message );

  // use the macro suif_error
  virtual void error(  const SuifObject* obj, const char* file_name,
                       int line_number,
                       const char* module_name,
                       const char* description, va_list ap );

  // use the macro suif_error
  virtual void error(  const char* file_name,
                       int line_number,
                       const char* module_name,
                       const char* description, va_list ap );


  // use the macro suif_warning
  virtual void warning( const SuifObject* obj, const char* file_name,
                       int line_number,
                       const char* module_name,
                       const char* description, va_list ap );


  // use the macro suif_warning
  virtual void warning( const char* file_name,
                       int line_number,
                       const char* module_name,
                       const char* description, va_list ap );

// use the macro suif_information
  virtual void information( const SuifObject* obj, const char* file_name,
                       int line_number,
                       const char* module_name,
                       int verbosity_level,
                       const char* description, va_list ap );

  // use the macro suif_information
  virtual void information( const char* file_name,
                       int line_number,
                       const char* module_name,
                       int verbosity_level,
                       const char* description, va_list ap );

  virtual void set_default_location_module(const LString &name);
  virtual LString get_default_location_module() const;
  virtual String get_location(const SuifObject *obj) const;
private:
  // Name of the module that provides the "get_object_location()"
  // implementation
  LString _default_location_module;
};


#endif
