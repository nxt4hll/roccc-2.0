#ifndef IOKERNEL__IOKERNEL_MESSAGES_H
#define IOKERNEL__IOKERNEL_MESSAGES_H

#include "iokernel_forwarders.h"

#ifndef SUIF_MODULE
#define SUIF_MODULE "Unknown"
#endif

class IOKernelMessage {
public:
  static IOKernelMessage& create( const char* file_name,
                   int line_number,
                   const char* module_name );

  void assert_message( const char* format, ... );

  IOKernelMessage* error( ObjectFactory* of, const char* format, ... );
  IOKernelMessage* warning( ObjectFactory* of, const char* format, ... );
  IOKernelMessage* information( ObjectFactory* of, int verbosity_level, const char* format, ... );
protected:
  IOKernelMessage( const char* file_name,
                   int line_number,
                   const char* module_name );

   const char* _file_name;
   int _line_number;
   const char* _module_name;
private:
  IOKernelMessage& operator=(const IOKernelMessage&);
  IOKernelMessage(const IOKernelMessage&);

};


/**
  * Same as assert() but print the source file name and line number before it 
  * bomb.
  * E.g. kernel_assert(index >= 0);
  * @param expr  a boolean expression to be evaluated.
  */
#define kernel_assert( expr ) if (expr) ; else IOKernelMessage::create( __FILE__, __LINE__, SUIF_MODULE ).assert_message("") ;


/**
  * Same as assert() but print a message before it bomb.
  * E.g. kernel_assert_message( index >= 0, ("%i is not positive", index))
  * @param expr  a boolean expression to be evaluated.
  * @param params a printf style argument list.
  */
#define kernel_assert_message( expr, params ) if (expr) ; else IOKernelMessage::create( __FILE__, __LINE__, SUIF_MODULE ).assert_message params ;


/**
  * Print an error message.
  * E.g. kernel_error("index %i is too negative", index);
  * @param implicit  a printf style argument list.
  */
#define kernel_error delete IOKernelMessage::create( __FILE__, __LINE__, SUIF_MODULE ).error


/**
  * Print a warning message.
  * E.g. kernel_warning("index %i is too negative", index);
  * @param implicit a printf style argument list.
  */
#define kernel_warning delete IOKernelMessage::create( __FILE__, __LINE__, SUIF_MODULE ).warning


/**
  * Print an information message.
  * E.g. kernel_information("index %i is too negative", index);
  * @param implicit a printf style argument list.
  */
#define kernel_information delete IOKernelMessage::create( __FILE__, __LINE__, SUIF_MODULE ).information


#endif
