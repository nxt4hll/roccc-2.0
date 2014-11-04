#include "common/system_specific.h"
#include "iokernel_messages.h"
#include "object_factory.h"

#include <stdio.h>
#include <stdarg.h>
#if defined(PGI_BUILD)|| defined(MSVC)
#include <stdlib.h>
#endif

IOKernelMessage::IOKernelMessage(
                   const char* file_name,
                   int line_number,
                   const char* module_name ) :
   _file_name( file_name ),
   _line_number( line_number ),
   _module_name( module_name ) {
}

IOKernelMessage& IOKernelMessage::create( const char* file_name,
                   int line_number,
                   const char* module_name ) {
  return *new IOKernelMessage( file_name, line_number, module_name );
}


void IOKernelMessage::assert_message( const char* format, ... ) {
  va_list args;
  va_start( args, format );
  fprintf( stderr, "ASSERTION FAILURE IN\n" );
  fprintf( stderr, "FILE: %s   LINE: %d MODULE: %s\n", _file_name,
                                                       _line_number, _module_name );
  vfprintf( stderr, format, args );
  abort();
}


IOKernelMessage* IOKernelMessage::error( ObjectFactory* of, 
                                         const char* format, ... ) {
  va_list args;
  va_start( args, format );
  of->error( _file_name, _line_number, _module_name, format, args ); 
  return this;
}


IOKernelMessage* IOKernelMessage::warning( ObjectFactory* of,
                                           const char* format, ... ) {
  va_list args;
  va_start( args, format );
  of->warning( _file_name, _line_number, _module_name, format, args ); 
  return this;
}


IOKernelMessage* IOKernelMessage::information( ObjectFactory* of, 
                                               int verbosity_level,
                                               const char* format, ... ) {
  va_list args;
  va_start( args, format );
  of->information( _file_name, _line_number, _module_name, verbosity_level, format, args ); 
  return this;
}

IOKernelMessage& IOKernelMessage::operator=(const IOKernelMessage&) {
  kernel_assert(false);
  return(*this);
}
IOKernelMessage::IOKernelMessage(const IOKernelMessage&) :
  _file_name(0), _line_number(0), _module_name(0)
{
  kernel_assert(false);
}
