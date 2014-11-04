
#ifndef TYPE_UTILITIES_DOT_H
#define TYPE_UTILITIES_DOT_H

#include <suifnodes/suif.h>

bool IsARocccType(QualifiedType* q) ;

IntegerType* GetBaseInt(SuifEnv* theEnv) ;
QualifiedType* GetQualifiedBaseInt(SuifEnv* theEnv) ;

DataType* DeReference(DataType* t) ;

//String StringType(Type* t) ;

#endif
