/* file "const_cast.h" */


/*
       Copyright (c) 1996, 1997 Stanford University

       All rights reserved.

       This software is provided under the terms described in
       the "suif_copyright.h" include file.
*/

#include <common/suif_copyright.h>


#ifndef STY_CONST_CAST_H
#define STY_CONST_CAST_H


/*
      This is a CONST_CAST definition for sty, the first-level main
      library of the SUIF system.
 */


/*
 *  The const_cast<>() construct is a new feature in C++.  The idea is
 *  to have a special kind of cast that is only to cast away ``const''
 *  modifiers on types being pointed to.  You get a little more
 *  compiler type checking by using this syntax rather than a regular
 *  cast for this special case, and it's potentially easier to read
 *  the code because you can tell why the cast is there.
 *
 *  However, not all C++ implementations currently support the
 *  const_cast<> syntax.  So we define a CONST_CAST() macro here that
 *  uses const_cast<> if it is available but uses a standard cast
 *  otherwise.  If const_cast<> is not supported on a given
 *  implementation, ``-DNO_CONST_CAST'' should be added to the
 *  standard list of command-line flags for that C++ compiler when
 *  compiling SUIF code.
 */

#ifndef NO_CONST_CAST
#define CONST_CAST(type, object)  const_cast<type>(object)
#else
#define CONST_CAST(type, object)  ((type)(object))
#endif


#endif /* STY_CONST_CAST_H */
