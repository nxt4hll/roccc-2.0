#ifndef __SUIF_MALLOC_H_
#define __SUIF_MALLOC_H_
#include <stddef.h>
void* operator new( size_t size );
void operator delete( void* address );
#endif
