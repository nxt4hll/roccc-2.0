#ifndef IOKERNEL__CAST_H
#define IOKERNEL__CAST_H

#include "object.h"
#include "iokernel_forwarders.h"

/**
  * @file
  * This file contains functions to do casting and type checking for Object.
  */

/**
  * Need to have these ifdefs because of MSVC doesn't mangle 
  *   template parameters into function names. 
  */


/** \class is_a cast.h iokernel/cast.h
  * Test if an object is an instance of a class.
  * @param T   a subclass of Object.
  * @param o   an instance.
  * @return true iff \a o is an instance of class \a T.
  * @internal Need to have these ifdefs because of MSVC doesn't mangle 
  *           template parameters into function names. 
  */
#ifndef MSVC
template< class T >
bool is_a( const Object* o ) {
  return ( (o) && o->isA( T::get_class_name() ) );
}
#else
template< class T >
bool is_a( const Object* o, T* t=NULL ) {
  return ( (o) && o->isA( T::get_class_name() ) );
}
#endif /* MSVC */





/**
  * Test if an object is a_kind_of instance of a class.
  * object \a o is_kind_of class \a T if \a o is an instance of \a T or
  * a subclass of \a T.
  *
  * E.g. if (is_kind_of<IfStatement>(object)) ...
  *
  * @param T   a subclass of Object.
  * @param o   an instance.
  * @return true iff \a o is a_kind_of class \a T.
  */
#ifndef MSVC
template< class T >
bool is_kind_of( const Object* o ) {
  return ( (o) && o->isKindOf( T::get_class_name() ) );
}
#else
template< class T >
bool is_kind_of( const Object* o, T* t=NULL ) {
  return ( (o) && o->isKindOf( T::get_class_name() ) );
}
#endif /* MSVC */


/**
  * Cast an object to a class.
  * If the cast is illegal, this function will bomb.
  *
  * E.g. IfStatement *ifstmt = to<IfStatement>(stmt);
  *
  * @param T   a subclass of Object.
  * @param o   an instance.
  * @return the same object \a o but as class \a T.
  */
template< class T >
T* to( const Object* o ) {
  if (!o) return (T*)o;
  if((o) && o->isKindOf( T::get_class_name())) return (T*)o;
  kernel_assert_message( false, 
			 ("Can't cast object of type %s to type %s\n", 
			  o->getClassName().c_str(),
			  T::get_class_name().c_str()));
  return 0;
}


/**
  * Cast an object to a const class.
  * If the cast is illegal, this function will bomb.
  *
  * @param T   a subclass of Object.
  * @param o   an instance.
  * @return the same object \a o but as const class \a T.
  */
template< class T >
const T* to_const( const Object* o ) {
  if (!o) return (T*)o;
  if((o) && o->isKindOf( T::get_class_name())) return (const T*)o;
  kernel_assert_message( false, 
			 ("Can't const cast object %s to const %s\n", 
			  o->getClassName().c_str(),
			  T::get_class_name().c_str()) );
  return 0;
}



#endif /* IOKERNEL__CAST_H */
