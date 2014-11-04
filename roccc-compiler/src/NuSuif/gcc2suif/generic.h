/*

  This file contains the declarations of miscellaneous useful functions and 
   enums that don't really fit anywhere else or had to be hoisted to this
   level.

*/

#ifndef __GENERIC_DOT_H__
#define __GENERIC_DOT_H__

// This function dynamically determines if a pointer is pointing to
//  a certain class
template <class X, class Y>
bool is_a(Y* parameter)
{
  return (dynamic_cast<X>(parameter)) != NULL ;
}

#endif
