#include "common/system_specific.h"
#ifdef PGI_BUILD
#include <stdlib.h>
#endif
#include "suifkernel_messages.h"
#include "suif_env.h"
#include "suif_object.h"
#include "suif_exception.h"

#include <stdio.h>
#include <stdarg.h>

#ifdef MSVC
#include <stdlib.h>
#endif

SuifKernelMessage::SuifKernelMessage(
                   const char* file_name,
                   int line_number,
                   const char* module_name ) :
   _file_name( file_name ),
   _line_number( line_number ),
   _module_name( module_name ) {
}

SuifKernelMessage& SuifKernelMessage::create( const char* file_name,
                   int line_number,
                   const char* module_name ) {
  return *new SuifKernelMessage( file_name, line_number, module_name );
}


void SuifKernelMessage::assert_message( const char* format, ... ) {
  va_list args;
  va_start( args, format );
  fprintf( stderr, "%s:%d: ",
	   _file_name, _line_number );
  fprintf( stderr, "Assertion Failure: '%s': ",
	   _module_name);
  vfprintf( stderr, format, args );
  fprintf( stderr, "\n");
#ifndef USE_CPP_EXCEPTION
  abort();
#else
  SUIF_THROW(SuifDevException(_file_name, _line_number, String("")));
#endif
}


SuifKernelMessage* SuifKernelMessage::error( const char* format, ... ) {
  va_list args;
  va_start( args, format );
  //of->error( _file_name, _line_number, _module_name, format, args ); 
  fprintf( stderr, "%s/%s:%d: Error: " , 
	   _module_name, _file_name, _line_number );
  vfprintf( stderr, format, args );
  fprintf( stderr, "\n");
  return this;
}

SuifKernelMessage* SuifKernelMessage::error/*_on_object*/(
						       SuifObject *so,
						       const char* format, ... ) {
  SuifEnv *env;
  va_list args;
  va_start( args, format );
  if(so!=NULL && ((env=so->get_suif_env())!=NULL)){
    env->error( so, _file_name, _line_number, _module_name, format, args ); 
  }else{
    warning(format, args);
  }
  return this;
}

SuifKernelMessage* SuifKernelMessage::error(SuifEnv *env, const char* format, ... ) {
  va_list args;
  va_start( args, format );
  if(env!=NULL){
    env->error( _file_name, _line_number, _module_name, format, args ); 
  }else{
    error(format, args);
  }
  return this;
}

SuifKernelMessage* SuifKernelMessage::warning( const char* format, ... ) {
  va_list args;
  va_start( args, format );
  fprintf( stderr, "Warning in %s(%d)[%s]\n" , 
        _file_name, _line_number, _module_name );
  fprintf( stderr, "Warning: " );
  vfprintf( stderr, format, args );
  fprintf( stderr, "\n" );
  return this;
}


SuifKernelMessage* SuifKernelMessage::warning( SuifObject *so, const char* format, ... ) {
  SuifEnv *env;
  va_list args;
  va_start( args, format );
  
  if(so!=NULL && ((env=so->get_suif_env())!=NULL)){

    env->warning( so, _file_name, _line_number, _module_name, format, args );
  }else{
    warning( _file_name, _line_number, _module_name, format, args );
  }
  return this;
}

SuifKernelMessage* SuifKernelMessage::warning( SuifEnv* env, const char* format, ... ) {
  va_list args;
  va_start( args, format );
  
  if(env!=NULL){
    env->warning( _file_name, _line_number, _module_name, format, args );
  }else{
    warning( _file_name, _line_number, _module_name, format, args );
  }
  return this;
}


SuifKernelMessage* SuifKernelMessage::information( int verbosity_level,
                                               const char* format, ... ) {
  va_list args;
  va_start( args, format );
  fprintf( stderr, "Information in %s(%d)[%s]\n" , 
        _file_name, _line_number, _module_name );
    vfprintf( stderr, format, args );
  return this;
}


SuifKernelMessage* SuifKernelMessage::information( SuifObject *so,
                                               int verbosity_level,
                                               const char* format, ... ) {
  SuifEnv *env;
  
  va_list args;
  va_start( args, format );
  if(so!=NULL && ((env=so->get_suif_env())!=NULL)){
    env->information( so, _file_name, _line_number, _module_name, verbosity_level, format, args ); 
  }else{
    information(verbosity_level, format, args);
  }
  
  return this;
}

SuifKernelMessage* SuifKernelMessage::information( SuifEnv* env,
                                               int verbosity_level,
                                               const char* format, ... ) {
  va_list args;
  va_start( args, format );
  if(env!=NULL){
    env->information( _file_name, _line_number, _module_name, verbosity_level, format, args ); 
  }else{
    information(verbosity_level, format, args);
  }
  
  return this;
}

void SuifKernelMessage::assert_message_on_object( SuifObject *so, const char* format, ... ) {
  va_list args;
  va_start( args, format );

  SuifEnv *env = so->get_suif_env();
  if (env) {
    String s = env->get_location(so);
    fprintf( stderr, "%s ", s.c_str());
  }
  fprintf( stderr, "ASSERTION FAILURE IN\n" );
  fprintf( stderr, "FILE: %s   LINE: %d MODULE: %s\n", _file_name,
                                                       _line_number, _module_name );
  vfprintf( stderr, format, args );
#ifndef USE_CPP_EXCEPTION
  abort();
#else
  SUIF_THROW(SuifDevException(_file_name, _line_number, String("")));
#endif
}


SuifKernelMessage& SuifKernelMessage::operator=(const SuifKernelMessage&other) {
  _file_name = other._file_name;
  _line_number = other._line_number;
  _module_name = other._module_name;
  return(*this);
}

SuifKernelMessage::SuifKernelMessage(const SuifKernelMessage&other) :
  _file_name(other._file_name), _line_number(other._line_number),
  _module_name(other._module_name)
{
}

