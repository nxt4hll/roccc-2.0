#ifndef IOKERNEL__FORWARDERS_H
#define IOKERNEL__FORWARDERS_H

#include <stddef.h>

// new pointertype modifiers to make descriptions of pointer discipline easier
//   sf_only    - this is only pointer to object (similar to auto_ptr<>)
//   sf_shared  - not sf_only
//   sf_owned   - receiver of pointer owns it but may be other
//                   pointers to it (similar to old auto_ptr<>)
//   sf_refed   - someone else owns it
//   sf_tricky  - ownership is more complex than this static model
//
//   const      - may not be modified
//   sf_mutable - may be modified
#define sf_only
#define sf_shared
#define sf_owned
#define sf_refed
#define sf_tricky
#define sf_mutable

#define SHARED
#define UNSHARED
#define NON_CONST
#define TRICKY

typedef unsigned char Byte;
typedef unsigned int AddressId;

typedef void* Address;
typedef const void* ConstAddress;
void emptyConstructorFunction( Address place );

#ifdef MEMORY_STATISTICS

void* memory_statistics_allocate( size_t size, const char* file, int line );
void* memory_statistics_deallocate( void* address );
void* memory_check();

#define SUIF_ALLOCATE( x )  (new ( memory_statistics_allocate( sizeof( x ), __FILE__, __LINE__ ) ) x )
#define SUIF_DEALLOCATE( x ) ( memory_statistics_deallocate( x ) )
#else
#define SUIF_ALLOCATE( x ) (new x)
#define SUIF_DEALLOCATE( x ) ( delete x )
#endif

// ------ iokernel

class Object;
class ObjectFactory;
class TypeLessSTLDescriptor;
class STLMetaClass;
class ObjectAggregateMetaClass;
class MetaClass;
class AggregateMetaClass;
class UnionMetaClass;
class StringMetaClass;
class LStringMetaClass;
class IntegerMetaClass;
class PointerMetaClass;
class ListMetaClass;
class FieldDescription;
class Allocator;
class VirtualNode;
class AggregateVirtualNode;
class PointerVirtualNode;
class ListVirtualNode;
class VirtualIterator;
class AggregateIterator;
class IteratorState;
struct AggregateElement;
class MetaClassApplier;


class AddressMap;
class ObjectStream;
class InputStream;
class OutputStream;
class CloneStream;
class Iterator;
class Synchronizer;
struct GenericList;
template< class T > class MetaClassVisitor;

// Wrappers for clean and safe access to MetaClass information
class ObjectWrapper;
class PointerWrapper; 
class AggregateWrapper; 
class FieldWrapper; 

typedef void (*ConstructorFunction)( Address address_of_where_to_generate_object );
typedef void (*DestructorFunction) ( const ObjectWrapper &obj );


// ------ basic data structures



#include "common/MString.h"

#ifdef NO_FORWARDING
#include "common/suif_map.h"
#include "common/suif_vector.h"
#include "common/suif_hash_map.h"
#include "common/suif_list.h"
#else

#if defined(USE_STL) || defined(STL_LIST)
#include <list>
#else
template <class T> class list;
#endif
#if USE_STL || USE_VECTOR_STL
#include <vector>
#else
template <class T> class suif_vector;
#endif

template <class domain, class range> class suif_map;
template <class domain, class range> class suif_hash_map;
#endif

// ------ suifkernel --------

class SuifEnv;
class SuifObject;
class RealObjectFactory;
class Module;
class VisitorMap;

#include <iostream>//jul modif

typedef int MetaClassId;

#include "iokernel_messages.h"

#endif
