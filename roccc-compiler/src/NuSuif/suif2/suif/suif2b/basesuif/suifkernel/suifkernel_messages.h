#ifndef SUIFKERNEL__MESSAGES_H
#define SUIFKERNEL__MESSAGES_H

#include "suifkernel_forwarders.h"

#ifndef SUIF_MODULE
#define SUIF_MODULE "Unknown"
#endif

/**
 * \file suifkernel_messages.h
 * Macros for assertions, warnings and errors
 */

/**
 * \class SuifKernelMessage suifkernel_messages.h suifkernel/suifkernel_messages.h
 * This is an internal class that is
 * only used to help dispatch error, warning, information, and
 * assertion messages
 * Use the MACROS defined below
 */
class SuifKernelMessage {
public:
  static SuifKernelMessage& create( const char* file_name,
                   int line_number,
                   const char* module_name );

  void assert_message( const char* format, ... );

  SuifKernelMessage* error( const char* format, ... );
  SuifKernelMessage* warning( const char* format, ... );
  SuifKernelMessage* information( int verbosity_level, const char* format, ... );

  void assert_message_on_object( SuifObject *so, const char* format, ... );
  SuifKernelMessage* error/*_on_object*/( SuifObject *so, const char* format, ... );
  SuifKernelMessage* warning/*_on_object*/( SuifObject *so, const char* format, ... );
  SuifKernelMessage* information/*_on_object*/( SuifObject *so, int verbosity_level, const char* format, ... );

  SuifKernelMessage* error( SuifEnv *env, const char* format, ... );
  SuifKernelMessage* warning( SuifEnv *env, const char* format, ... );
  SuifKernelMessage* information( SuifEnv* env, int verbosity_level, const char* format, ... );
protected:
  SuifKernelMessage( const char* file_name,
                   int line_number,
                   const char* module_name );

   const char* _file_name;
   int _line_number;
   const char* _module_name;

private:
  SuifKernelMessage& operator=(const SuifKernelMessage&);
  SuifKernelMessage(const SuifKernelMessage&);
};

/**
 * \section Outputting Errors and Warnings
 *
 *
 */


/**
 * Assert that the expression is true.
 * If it is not, expect the system to halt
 *
 * This macro was designed so that if it
 * is redefined to ignore assertions, there 
 * will be no performance penalty. (and no checking)
 *
 * \par Usage:
 * \code
 * suif_assert( value == 0 );
 * \endcode
 */

#define suif_assert( expr ) if (expr) ; else SuifKernelMessage::create( __FILE__, __LINE__, SUIF_MODULE ).assert_message("") 

/**
 * Assert that the expression is true.
 * If it is not, print a message
 *
 * This macro was designed so that if it
 * is redefined to ignore assertions, there 
 * will be no performance penalty. (and no checking)
 *
 * \par Usage:
 * \code
 * suif_assert_message( value == 0, ("Value(%d) != 0\n", value))
 * \endcode
 */

#define suif_assert_message( expr, params ) if (expr) ; else SuifKernelMessage::create( __FILE__, __LINE__, SUIF_MODULE ).assert_message params 

/**
 * Assert that the expression is true.
 * If it is not, print a message that
 * contains information about a relevant object
 *
 * This macro was designed so that if it
 * is redefined to ignore assertions, there 
 * will be no performance penalty. (and no checking)
 *
  * \par Usage:
  *
  * \code
  * suif_assert_on_object( value == 0, suif_obj );
  * \endcode
  */

#define suif_assert_on_object( expr, obj ) if (expr) ; else SuifKernelMessage::create( __FILE__, __LINE__, SUIF_MODULE ).assert_message_on_object("", obj) ;

/**
 * Assert that the expression is true.
 * If it is not, print a message that
 * contains information about a relevant object
 * and some other user-defined code.
 *
 * This macro was designed so that if it
 * is redefined to ignore assertions, there 
 * will be no performance penalty. (and no checking)
 *
  * \par Usage:
  *
  * \code
  * suif_assert_message_on_object( value == 0, ( suif_obj, "message %s", str) );
  * \endcode
  */

#define suif_assert_message_on_object( expr, params ) if (expr) ; else SuifKernelMessage::create( __FILE__, __LINE__, SUIF_MODULE ).assert_message_on_object params ;

/**
 * An error has occurred.
 * print a message, possibly about 
 * a suifobject that it occurred on
 * Expect that the system may stop.
 * 
  * \par Usage:
  *
  * \code
  * suif_error("error message %s", str)
  * suif_error(suif_env, "error message %s", str)
  * suif_error(suif_obj, "error message %s", str)
  * \endcode
  */



#define suif_error delete SuifKernelMessage::create( __FILE__, __LINE__, SUIF_MODULE ).error

/**
 * An unexpected event has occurred that
 * can be dealt with in a safe way
 * print a message about it to 
 * warn the user that there is a problem
 * but processing can continue.
 *
  * \par Usage:
  *
  * \code
  * suif_warning("warning message %s", str)
  * suif_warning(suif_env, "warning message %s", str)
  * suif_warning(suif_obj, "warning message %s", str)
  * \endcode
  */

#define suif_warning delete SuifKernelMessage::create( __FILE__, __LINE__, SUIF_MODULE ).warning

/**
 * Print information pertaining to a
 * given SuifObject or environment.
 *
  * \par Usage:
  *
  * \code
  * suif_information("info message %s", str)
  * suif_information(suif_env, "info message %s", str)
  * suif_information(suif_obj, "info message %s", str)
  * \endcode
  */

#define suif_information delete SuifKernelMessage::create( __FILE__, __LINE__, SUIF_MODULE ).information




#endif
