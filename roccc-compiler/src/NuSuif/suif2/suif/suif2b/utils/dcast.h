#ifndef DCAST_H
#define DCAST_H

/** @file
  * Utilities for dynamic casting.
  *
  * \warning
  *	Do not use these for anything derived from SuifObject.
  *	For suif objects, use suifkernel/cast.h
  *	\see cast.h
  */

#include "suifkernel/suif_exception.h"


/** A dynamic_cast with error checking.
  * If the cast is illegal, throw an SuifDevException containing the source
  * filename and line number.
  * @param T the class to cast to.
  * @param F the class to cast from.
  * @param from the object to cast.
  * @param file the name of the file to be included in the exception.
  * @param line the line number to be included in the exception.
  * @exception SuifDevException if the cast is illegal, the error message
  *            will contain \a file and \a line.
  * @return the same object as \a from but as type \a T.
  *
  * This function is designed for the DCAST() macro.
  */
template<class T, class F>
T dcast(F* from, const char* file, int line, T* dummy=NULL)
{
  if (from == 0) return 0;
  T tmp = dynamic_cast<T>(from);
  if (tmp == 0)
    SUIF_THROW(SuifDevException(file, line, String("Illegal dynamic cast.")));
  return tmp;
}

/** A dynamic_cast with error checking.
  * @param newtype the target type.
  * @param from the object.
  * @exception SuifDevException if the cast is illegal.
  * @return the object \a from as type \a newtype.
  *
  * Example: to cast \a obj to \a NewClass*.
  * \code
  * NewClass *n = DCAST(NewClass*, obj);
  * \endcode
  */
#define DCAST(newtype, from) (dcast<newtype>(from, __FILE__, __LINE__))




/**
    A dyna-cast macro which allows you to name its output.
    Return value is of Type*, so it's different from the
    macro above.
*/
#define DCAST_AND_CALL(Type, What, CallIt) Type * CallIt = \
            dcast<Type*>(What, __FILE__, __LINE__)

/** A version of the above macro returning a const pointer */
#define CONST_DCAST_AND_CALL(Type, What, CallIt) const Type * CallIt = \
            dcast<const Type*>(What, __FILE__, __LINE__)

#endif /* DCAST_H */
