/* file "zero.h" */


/*
       Copyright (c) 1997 Stanford University

       All rights reserved.

       This software is provided under the terms described in
       the "suif_copyright.h" include file.
*/

#include <common/suif_copyright.h>


#ifndef STY_ZERO_H
#define STY_ZERO_H


/*
      This is the definition of ``zero'' functions for sty, the
      first-level main library of the SUIF system.
*/


/*
 *  Defining ``zero()'' functions for each type gives a way to specify
 *  what type should be used as the ``zero'' element for templates
 *  such as list templates.  The template can call ``zero()'' and get
 *  whatever result is appropriate for the type in question.  When
 *  some new type is to be used in such a template, the user must
 *  define a ``zero()'' function for that type.
 */

inline long zero(long *)  { return 0; }
inline unsigned long zero(unsigned long *)  { return 0; }
inline int zero(int *)  { return 0; }
inline unsigned zero(unsigned *)  { return 0; }
inline short zero(short *)  { return 0; }
inline unsigned short zero(unsigned short *)  { return 0; }
inline char zero(char *)  { return 0; }
inline signed char zero(signed char *)  { return 0; }
inline unsigned char zero(unsigned char *)  { return 0; }
inline long double zero(long double *)  { return 0; }
inline double zero(double *)  { return 0; }
inline float zero(float *)  { return 0; }
#ifndef NOBOOL
inline bool zero(bool *)  { return false; }
#endif

template <class elem_t> inline elem_t *zero(elem_t **)  { return 0; }


#endif /* STY_ZERO_H */
