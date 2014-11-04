#include "common/system_specific.h"
#include "object_factory.h"

#include "iokernel_forwarders.h"
#include "meta_class.h"
#include "aggregate_meta_class.h"
#include "union_meta_class.h"
#include "integer_meta_class.h"
#include "pointer_meta_class.h"
#include "string_meta_class.h"
#include "lstring_meta_class.h"
#include "i_integer_meta_class.h"
#include "clone_stream.h"
#include "binary_streams.h"
#include "list_meta_class.h"
#include "stl_meta_class.h"
#include "helper.h"
#include "field_description.h"

#include "common/suif_vector.h"
#include "common/suif_map.h"
#include "common/suif_hash_map.h"
#include "common/suif_indexed_list.h"

// #ifndef MSVC
// #include <strstream.h>
// #else
// #include <strstrea.h>
// #endif
#include <sstream>//jul modif
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

using namespace std; //jul modif

ObjectFactory::ObjectFactory() :
  _last_id( 0 ),
  _rudimentary_address_map( 0 ),
  _meta_class_dictionary( new MetaClassDictionary( /* 1024 */ ) ),
  _aggregate_meta_class_mc( 0 ),
  _object_aggregate_meta_class_mc( 0 ),
  _pointer_meta_class_mc( 0 ),
  _union_meta_class_mc( 0 ),
  _integer_meta_class_mc( 0 ),
  _list_meta_class_mc( 0 ),
  _suif_env( 0 ) {
}


ObjectFactory::~ObjectFactory() {
  MetaClassDictionary::iterator current = _meta_class_dictionary->begin(),
                          end = _meta_class_dictionary->end();
  // the elements in this list are not necessarily owned by this
  // ObjectFactory (example: cloning of an ObjectFactory during a
  // read operation)

    while ( current != end ) {
       MetaClass* m = (*current).second;
       ObjectFactory* of = m->get_owning_factory();
       if ( ( !of ) || ( of == this ) ) {
         delete m;
       }
       current++;
    }
  delete _meta_class_dictionary; // the contents is deleted above
  delete _rudimentary_address_map;
  _meta_class = 0;
}


void ObjectFactory::constructor_function( Address place ) {
  new(place) ObjectFactory();
}

struct X_char {
    char x;
    char field;
    };

struct X_short {
    char x;
    short field;
    };

struct X_int {
    char x;
    int field;
    };

struct X_long {
    char x;
    long field;
    };

struct X_double {
    char x;
    double field;
    };

struct X_bool {
    char x;
    bool field;
    };

struct X_size_t {
    char x;
    size_t field;
    };



void ObjectFactory::init( SuifEnv* suif_env ) {
  // -------------------------------------------------------
  // --- set the INSTANCE variables needed for           ---
  // --- creating other classes (including MetaClasses ) ---
  // -------------------------------------------------------

  _suif_env = suif_env;
  _object_aggregate_meta_class_mc = 0; // this is important since the
                                  // create_object_meta_class  uses it!!
  _object_aggregate_meta_class_mc = create_object_meta_class(
                                 ObjectAggregateMetaClass::get_class_name(),
                                 sizeof( ObjectAggregateMetaClass ),
                                 ObjectAggregateMetaClass::constructor_function,
                                 0 );

  _aggregate_meta_class_mc = create_object_meta_class(
                             AggregateMetaClass::get_class_name(),
                             sizeof( AggregateMetaClass ),
                             AggregateMetaClass::constructor_function );


  _object_aggregate_meta_class_mc -> inherits_from( _aggregate_meta_class_mc );



  AggregateMetaClass*
        metaClassMC = create_object_meta_class(
                              MetaClass::get_class_name(),
                              sizeof( MetaClass ),
                              MetaClass::constructor_function );


  _pointer_meta_class_mc = create_object_meta_class(
              PointerMetaClass::get_class_name(),
              sizeof( PointerMetaClass ),
              PointerMetaClass::constructor_function,
              metaClassMC );

  PointerMetaClass*
        pointerToAggregateMetaClassMCReference =
                    get_pointer_meta_class( _aggregate_meta_class_mc, false );

  // --- make 'forward' declarations for the MetaClassMetaClass and pointers to it

  PointerMetaClass*
        pointerToMetaClassMCOwning = get_pointer_meta_class( metaClassMC );
  PointerMetaClass*
        pointerToMetaClassMCReference = get_pointer_meta_class( metaClassMC, false );

  // ----------------------------------------------------------------
  // ---------------- make the basic integer data types -------------
  // ----------------------------------------------------------------
  // --- make the metaClass for the other integerMetaClasses ---
  _integer_meta_class_mc = create_object_meta_class(
                               IntegerMetaClass::get_class_name(),
                               sizeof( IntegerMetaClass ),
                               IntegerMetaClass::constructor_function,
                               metaClassMC );

 // --- metaClass for C/C++ signed long int
  /* IntegerMetaClass* sLongIntMC = */
  get_integer_meta_class("long", sizeof( signed long ), true ,
			 OFFSETOF(X_long,field));

  // --- metaClass for C/C++ int
  IntegerMetaClass*
        intMC = get_integer_meta_class(
                    "int", sizeof( int ), true ,OFFSETOF(X_int,field));

  // --- metaClass for C/C++ int
        get_integer_meta_class(
                    "short", sizeof( short ), true ,OFFSETOF(X_short,field));

  // --- metaClass for C/C++ int
        get_integer_meta_class(
                    "char", sizeof( char ), true ,OFFSETOF(X_char,field));


  // --- metaClass for C/C++ double WRONG HACK !!!! @@@
  /* IntegerMetaClass* doubleMC = */
  get_integer_meta_class("double", sizeof( double ), true ,
			 OFFSETOF(X_double,field));

  // --- metaClass for C/C++ bool
  IntegerMetaClass*
        boolMC = get_integer_meta_class(
                   "bool", sizeof( bool ), false ,OFFSETOF(X_bool,field));

  // --- metaClass for C/C++ size_t
  IntegerMetaClass*
        sizeTMC = get_integer_meta_class(
             "size_t", sizeof( size_t ), false ,OFFSETOF(X_size_t,field));

  // --- metaClass for IInteger: infinite precision integers with
  // --- undetermined, +/- infinity, unsigned infinity
  IIntegerMetaClass*
        IIntegerMC = new IIntegerMetaClass( "IInteger" );
  IIntegerMC -> set_meta_class( metaClassMC );
  enter_meta_class( IIntegerMC );

  // ----------------------------------------------------------------
  // -------------------- make the string data types ----------------
  // ----------------------------------------------------------------

  // --- metaClass for String ---
  StringMetaClass*
        stringMC = new StringMetaClass( "String" );
  stringMC -> set_meta_class( metaClassMC );
  enter_meta_class( stringMC );

  // --- metaClass for LString ---
  LStringMetaClass*
        lStringMC = new LStringMetaClass( "LString" );
  lStringMC -> set_meta_class( metaClassMC );
  enter_meta_class( lStringMC );

#ifndef MSVC
  // --- metaClass for Object
  struct X_object {
	char x;
	Object field;
	};
#else
  class MSVC_object {
	
  };

  struct X_object{
	char x;
	MSVC_object field;
  };
#endif

  AggregateMetaClass*
        objectMC = create_object_meta_class(
                        Object::get_class_name(),
                        sizeof( Object ),
                        Object::constructor_function );
  objectMC->set_alignment(OFFSETOF(X_object,field));
  
  /*PointerMetaClass* pointerToObjectOwning = */
  get_pointer_meta_class( objectMC, true );
  /*PointerMetaClass* pointerToObjectR = */
  get_pointer_meta_class( objectMC, false );


  // --- metaClass for MetaClass
  metaClassMC -> inherits_from( objectMC );
  metaClassMC -> add_field_description( "_meta_class_name", lStringMC, OFFSETOF(MetaClass,_meta_class_name) );
  metaClassMC -> add_field_description( "_size", sizeTMC, OFFSETOF(MetaClass,_size) );



  _list_meta_class_mc = create_object_meta_class(
                             ListMetaClass::get_class_name(),
                             sizeof( ListMetaClass ),
                             ListMetaClass::constructor_function,
                             metaClassMC );
  _list_meta_class_mc -> add_field_description( "_element_meta_class",  pointerToMetaClassMCReference, OFFSETOF(ListMetaClass, _element_meta_class ) );


  //  --- make the metaClass for AggregateMetaClasses ---
  AggregateMetaClass*
        fieldDescriptionMC = create_aggregate_meta_class(
                                  "AggregateMetaClass::FieldDescription",
                                  sizeof( FieldDescription ) );

  fieldDescriptionMC -> add_field_description( "offset", intMC, OFFSETOF(FieldDescription,offset) );
  fieldDescriptionMC -> add_field_description( "metaClass", pointerToMetaClassMCReference, OFFSETOF(FieldDescription,metaClass) );
  fieldDescriptionMC -> add_field_description( "memberName", lStringMC, OFFSETOF(FieldDescription,memberName) );

  PointerMetaClass* pointerToFieldDescriptionMCOwning = get_pointer_meta_class( fieldDescriptionMC, true );

  STLMetaClass* stlMetaClass =
    get_stl_meta_class( "LIST:suif_vector<FieldDescription*>",
                           new STLDescriptor<suif_vector<FieldDescription* > >
                                  (pointerToFieldDescriptionMCOwning ) );

  _aggregate_meta_class_mc -> inherits_from( metaClassMC );
  _aggregate_meta_class_mc -> add_field_description( "_base_class", pointerToAggregateMetaClassMCReference, OFFSETOF(AggregateMetaClass, _base_class ) );
  _aggregate_meta_class_mc -> add_field_description( "_fields", get_pointer_meta_class( stlMetaClass, true, true ), OFFSETOF(AggregateMetaClass,_fields) );




 AggregateMetaClass* lstring_pair_mc = create_aggregate_meta_class(
                    "VirtualFieldDescription::mapField",
                     sizeof(  MetaClassDictionary::value_type ) );

  typedef VirtualFieldDescription::value_type vft_value_type;
  lstring_pair_mc -> add_field_description( "name",        lStringMC, OFFSETOF( vft_value_type, first ) );
  lstring_pair_mc -> add_field_description( "description", lStringMC, OFFSETOF( vft_value_type, second ) );

  STLMetaClass* virtual_field_description_mc =
      get_stl_meta_class( "LIST:suif_map<LString, LString>",
                             new STLDescriptor< suif_map< LString,LString> >(lstring_pair_mc) );

 _aggregate_meta_class_mc -> add_field_description( "_virtual_field_description", get_pointer_meta_class( virtual_field_description_mc, true, true ), OFFSETOF(AggregateMetaClass,_virtual_field_description) );

  // --- make the metaClass for ObjectAggregateMetaClass ---

  // --- make the metaClass for UnionMetaClasses ---

  _union_meta_class_mc = create_object_meta_class(
                        UnionMetaClass::get_class_name(),
                        sizeof( UnionMetaClass ),
                        UnionMetaClass::constructor_function,
                        _aggregate_meta_class_mc );

  _union_meta_class_mc -> add_field_description( "_tag_offset", intMC, OFFSETOF(UnionMetaClass, _tag_offset ) );
  _union_meta_class_mc -> add_field_description( "_union_fields", get_pointer_meta_class( stlMetaClass, true, true ), OFFSETOF(UnionMetaClass,_union_fields) );

  // --- make the metaClass for PointerMetaClasses ---
  _pointer_meta_class_mc -> add_field_description( "_base_type",          pointerToMetaClassMCReference, OFFSETOF( PointerMetaClass, _base_type ) );
  _pointer_meta_class_mc -> add_field_description( "_pointer_owns_object", boolMC,               OFFSETOF( PointerMetaClass, _pointer_owns_object ) );
  _pointer_meta_class_mc -> add_field_description( "_is_static", boolMC,               OFFSETOF( PointerMetaClass, _is_static ) );
  _pointer_meta_class_mc -> add_field_description( "_needs_cloning",boolMC,
        OFFSETOF( PointerMetaClass,_needs_cloning));


  // --- make the metaClass for IntegerMetaClass ---
  _integer_meta_class_mc -> add_field_description( "_is_signed", boolMC, OFFSETOF( IntegerMetaClass,_is_signed ) );

  // --- make the metaClass for ObjectFactory()
  AggregateMetaClass* metaClassObjectFactory =
                     create_object_meta_class(
                         "ObjectFactory",
                         sizeof( ObjectFactory ),
                         ObjectFactory::constructor_function,
                         objectMC );

  AggregateMetaClass* mapMC = create_aggregate_meta_class(
                    "ObjectFactory::mapField",
                     sizeof(  MetaClassDictionary::value_type ) );

  typedef MetaClassDictionary m;
  typedef m::value_type t;
  mapMC -> add_field_description( "lstring",    lStringMC,             OFFSETOF( t, first ) );
  mapMC -> add_field_description( "metaClass", pointerToMetaClassMCOwning, OFFSETOF( t, second ) );

  STLMetaClass* mapMetaClassMC =
      get_stl_meta_class( "LIST:suif_hash_map<LString, MetaClass*>",
                             new STLDescriptor< MetaClassDictionary >(mapMC) );


  PointerMetaClass* mapMetaClassMCOwning = get_pointer_meta_class( mapMetaClassMC, true, true );

  metaClassObjectFactory -> add_field_description( "_meta_class_dictionary", mapMetaClassMCOwning, OFFSETOF( ObjectFactory, _meta_class_dictionary ) );

  set_meta_class( metaClassObjectFactory );

  // --- make the _rudimentary_address_map (needed for I/O) ---
  ostringstream temp;
  OutputStream* o = new BinaryOutputStream( temp );
  o->write( ObjectWrapper(this, this->get_meta_class()), false );
  o->write_close();
  _rudimentary_address_map = o->get_address_map_of_owned_objects();
  //  _rudimentary_address_map -> remove_from_address_map( this );
  _rudimentary_address_map -> remove_from_address_map( _meta_class_dictionary );
  delete o;


}



Address ObjectFactory::create_empty_object_by_name( const LString& meta_class_name ) {
  return create_empty_object( find_meta_class( meta_class_name ) );
}


Address ObjectFactory::create_empty_object( const MetaClass* metaClass ) const {
  size_t size = metaClass->get_size_of_instance();
  Address object = (Address)::operator new( size );
  // metaClass->debugit("allocated object size ",size);

  return create_empty_object_in_space( metaClass, object );
}

Address ObjectFactory::create_empty_object_in_space( const MetaClass* meta_class, Address space ) const {
  // zero initialize. necessary for static pointers.
  // since they may optionally be initialized by the constructor
  size_t size = meta_class->get_size_of_instance();
  memset( space, 0, size );

  meta_class->construct_object( space );
  meta_class->set_meta_class_of_object( space );
  return space;
}


MetaClass* ObjectFactory::lookupMetaClass( const LString& metaClassName ) {
  MetaClass* metaClass = 0;
  if ( _meta_class_dictionary->find( metaClassName ) != _meta_class_dictionary->end() ) {
    //    metaClass =  (*_meta_class_dictionary)[ metaClassName ];
    metaClass = _meta_class_dictionary->lookup(metaClassName);
    //.second /* .second  this is necessary for a suif_hash_map but it shouldn't! */;
  }
  return metaClass;
}


void ObjectFactory::enter_meta_class( MetaClass* mc ) {
  kernel_assert( mc->get_instance_name().size() != 0 );
  MetaClassId id = ++_last_id;
  kernel_assert( id != 0 );  // the id 0 must never be used
                          //  id 0 is the common root for get_link_meta_class()
  mc->set_meta_class_id( id );
  mc->set_owning_factory( this );
  //(*_meta_class_dictionary)[mc->get_instance_name()] = mc;
  _meta_class_dictionary->enter_value(mc->get_instance_name(), mc);
}


AddressMap* ObjectFactory::get_rudimentary_address_map() const {
  return _rudimentary_address_map;
}


MetaClassId ObjectFactory::get_last_id() const {
  return _last_id;
}


// ---- FOR DEBUGGING ----

void ObjectFactory::print_contents() {
  MetaClassDictionary::iterator it = _meta_class_dictionary->begin();
    cerr<<"-------- MetaClass Dictionary -------"<<endl;
    while ( it != _meta_class_dictionary->end() ) {
      cerr<<(*it).first<<" "<<(*it).second<<endl;
      it++;
    }
    cerr<<"-------------- end ------------------"<<endl;
 }


bool ObjectFactory::is_a_list( MetaClass* m ) {
  const LString& instanceName = m->get_instance_name();
  const LString magicStart = "LIST:";
  String tmp = (const char*)instanceName;
  String str = tmp.Mid(0, magicStart.size() );
  return str == magicStart;
}



AggregateMetaClass* ObjectFactory::find_aggregate_meta_class(
                        const LString& metaClassName ) {
  MetaClass* mc = find_meta_class( metaClassName );

  kernel_assert( (!mc) || (mc && mc->isKindOf(AggregateMetaClass::get_class_name())));
  return (AggregateMetaClass*)mc;
}

MetaClass* ObjectFactory::find_meta_class(
                       const LString& metaClassName ) {
  LString lookupName = metaClassName;
  MetaClass* metaClass = lookupMetaClass( lookupName );

  return metaClass;
}


UnionMetaClass *ObjectFactory::find_union_meta_class(
                        const LString& metaClassName ) {
  MetaClass* mc = find_meta_class( metaClassName );

  kernel_assert( (!mc) || (mc && mc->isKindOf("UnionMetaClass")) );
  return (UnionMetaClass*)mc;
}


SuifEnv* ObjectFactory::get_suif_env() const {
  return _suif_env;
}

//	This should work as a template but does not on some compilers, 
//	so use a macro instead
// template <class x> size_t alignment_of() {
//     struct X {
// 	char ch;
// 	x field;
// 	};
//   return OFFSETOF(X,field);
//     }

/*
static size_t alignment_of_int() {
    return OFFSETOF(X_int,field);
    }
    */

struct X_void_ptr {
    char ch;
    void * field;
    };

static size_t alignment_of_void_ptr() {
    return OFFSETOF(X_void_ptr,field);
    }

struct X_GenericList {
    char ch;
    GenericList field;
    };

static size_t alignment_of_GenericList() {
    return OFFSETOF(X_GenericList,field);
    }



IntegerMetaClass* ObjectFactory::get_integer_meta_class(
                     const LString& name,
                     size_t size,
                     bool is_signed,
		     size_t align ) {
  kernel_assert( size );
  kernel_assert( _integer_meta_class_mc );
  IntegerMetaClass* intMC;

  MetaClass* mc = lookupMetaClass( name );

  if ( mc ) {
    if ( mc->isA( IntegerMetaClass::get_class_name() ) ) {
      intMC = (IntegerMetaClass*)mc;
      if ( ( intMC->get_size_of_instance() == size ) &&
	 ( intMC->get_alignment_of_instance() == align) &&
         ( intMC->is_signed() == is_signed ) ) {
        return intMC;
      }
    }
    kernel_assert( false); // already registered with different parameters
                 // or different MetaClass
  }

  intMC = new IntegerMetaClass( name );
  intMC->set_size( size );
  intMC->set_alignment(align);
  intMC->set_is_signed( is_signed );
  intMC->set_meta_class( _integer_meta_class_mc );
  enter_meta_class( intMC );
  return intMC;
}


PointerMetaClass* ObjectFactory::get_pointer_meta_class(
                 MetaClass* base_type,
                 bool pointer_owns_object,
                 bool is_static,
		bool must_be_cloned ) {
  String name = String("PTR:") +
                String( pointer_owns_object ? "O:" : "R:" ) +
                String( is_static ? "S:" : ":" ) +
		String( must_be_cloned ? "C:" : ":") +
                base_type->get_instance_name();
  PointerMetaClass* pointer_mc;
  MetaClass* mc = lookupMetaClass( name );

  if ( mc ) {
    if ( mc->isA( PointerMetaClass::get_class_name() ) ) {
      pointer_mc = (PointerMetaClass*)mc;

      if ( ( pointer_mc->is_owning_pointer() == pointer_owns_object ) &&
           ( pointer_mc->get_base_type() == base_type ) ) {
        return pointer_mc;
      }
    }
    kernel_assert( false );
  }

  pointer_mc = new PointerMetaClass( name, base_type, pointer_owns_object, is_static,must_be_cloned );
  pointer_mc->set_size( sizeof( void* ) );
  pointer_mc->set_alignment(alignment_of_void_ptr());
  pointer_mc->set_meta_class( _pointer_meta_class_mc );
  enter_meta_class( pointer_mc );

  return pointer_mc;
}

struct x_indexed_list
    {
    char ch;
    indexed_list<int,int> field;
    };

struct x_list
    {
    char ch;
    list<int> field;
    };

struct x_searchable_list
    {
    char ch;
    searchable_list<int> field;
    };

struct x_suif_hash_map
    {
    char ch;
    suif_hash_map<int,int> field;
    };

struct x_suif_map
    {
    char ch;
    suif_map<int,int> field;
    };


struct x_suif_vector
    {
    char ch;
    suif_vector<int> field;
    };

struct x_vector
    {
    char ch;
    suif_vector<int> field;
    };

ListMetaClass* ObjectFactory::get_list_meta_class(
                                   MetaClass* element_type,
				   const LString &real_type) {
 ListMetaClass* list_meta_class;
 
 String name = real_type;
 if (!name.ends_in(":GENERIC"))
    name = name + String(":GENERIC");
 MetaClass* mc = lookupMetaClass( name );

 if ( mc ) {
    if ( mc->isA( ListMetaClass::get_class_name() ) ) {
      list_meta_class = (ListMetaClass*)mc;

      if ( ( list_meta_class->get_element_meta_class() == element_type ) ) {
        return list_meta_class;
      }
    }
    kernel_assert( false );
  }

 list_meta_class = new ListMetaClass( name );
 list_meta_class -> set_meta_class( _list_meta_class_mc );
 list_meta_class -> set_element_meta_class(  element_type );
 list_meta_class -> set_size( sizeof( GenericList ) );
 list_meta_class -> set_alignment(alignment_of_GenericList());
 list_meta_class -> set_constructor_function( emptyConstructorFunction );

 // try to get size and alignment right based on real type
 // There must be a better way to do this?

 String lname = real_type;
 if (lname.starts_with("LIST:")) {
    lname = lname.Mid(5,lname.length());
    int i = lname.find("<");
    lname = lname.Left(i);
    }
  // printf(" type of string is %s\n",(const char *)lname);
  if (lname == String("indexed_list")) {
	list_meta_class->set_size(sizeof(indexed_list<int,int>));
	list_meta_class->set_alignment(OFFSETOF(x_indexed_list,field));
	}
  else if (lname == String("list")) {
        list_meta_class->set_size(sizeof(list<int>));
        list_meta_class->set_alignment(OFFSETOF(x_list,field));
        }
  else if (lname == String("searchable_list")) {
        list_meta_class->set_size(sizeof(searchable_list<int>));
        list_meta_class->set_alignment(OFFSETOF(x_searchable_list,field));
        }
  else if (lname == String("suif_hash_map")) {
        list_meta_class->set_size(sizeof(suif_hash_map<int,int>));
        list_meta_class->set_alignment(OFFSETOF(x_suif_hash_map,field));
        }
  else if (lname == String("suif_map")) {
        list_meta_class->set_size(sizeof(suif_map<int,int>));
        list_meta_class->set_alignment(OFFSETOF(x_suif_map,field));
        }
  else if (lname == String("suif_vector")) {
        list_meta_class->set_size(sizeof(suif_vector<int>));
        list_meta_class->set_alignment(OFFSETOF(x_suif_vector,field));
        }
  else if (lname == String("vector")) {
        list_meta_class->set_size(sizeof(suif_vector<int>));
        list_meta_class->set_alignment(OFFSETOF(x_vector,field));
        }

  // else printf(" don't know\n");
	 
  enter_meta_class( list_meta_class );
  return list_meta_class;
  }




STLMetaClass* ObjectFactory::get_stl_meta_class(
                 const LString& name,
                 TypeLessSTLDescriptor* stl_descriptor ) {
  STLMetaClass* stl_meta_class;
  MetaClass* mc = lookupMetaClass( name );

  if ( mc ) {
    if ( mc->isA( STLMetaClass::get_class_name() ) ) {
      stl_meta_class = (STLMetaClass*)mc;

      if ( ( stl_meta_class->get_instance_name() == name ) ) {
	if (stl_meta_class->get_element_meta_class() == 
	    stl_descriptor->get_element_meta_class()) {
	  delete stl_descriptor;
	  return stl_meta_class;
	}
	kernel_assert_message( false,
			       ("Requested STL metaclass '%s'\n"
				" with a different element '%s'\n"
				" than previously registered '%s'\n",
		name.c_str(),
		stl_meta_class->get_element_meta_class()
				->get_instance_name().c_str(),
		stl_descriptor->get_element_meta_class()
				->get_instance_name().c_str()));
      }
    }
  }

  kernel_assert( stl_descriptor );
  stl_meta_class = new STLMetaClass( name );
  stl_meta_class -> set_meta_class( _list_meta_class_mc );
  stl_meta_class -> set_descriptor( stl_descriptor );
  enter_meta_class( stl_meta_class );
  return stl_meta_class;
}


AggregateMetaClass* ObjectFactory::create_aggregate_meta_class(
                 const LString& name,
                 size_t size,
                 ConstructorFunction constructor,
                 AggregateMetaClass* base_class ) {
  assert( name.size() );
  assert( !lookupMetaClass( name ) );
  assert( _aggregate_meta_class_mc );

  AggregateMetaClass* aggregate_mc = new AggregateMetaClass( name );

  aggregate_mc->set_meta_class( _aggregate_meta_class_mc );
  aggregate_mc->set_constructor_function( constructor );
  aggregate_mc->inherits_from( base_class );
  if (base_class)
    aggregate_mc->set_alignment(base_class->get_alignment_of_instance());
  else
    aggregate_mc->set_alignment(alignment_of_void_ptr());
  aggregate_mc->set_size( size );
  enter_meta_class( aggregate_mc );

  return aggregate_mc;
}


ObjectAggregateMetaClass* ObjectFactory::create_object_meta_class(
                 const LString& name,
                 size_t size,
                 ConstructorFunction constructor,
                 AggregateMetaClass* base_class ) {
  
  if (lookupMetaClass( name ))
    {
      kernel_assert_message(0,
			    ("Attempt to create MetaClass '%s' "
			     "that has already been loaded.\n"
			     "Try to import all needed nodes before loading "
			     "any files\n",name.c_str()));
    }

  ObjectAggregateMetaClass* object_mc = new ObjectAggregateMetaClass( name );
  AggregateMetaClass* object_meta_class_mc = this->_object_aggregate_meta_class_mc ?
                               this->_object_aggregate_meta_class_mc :
                               object_mc;
  object_mc -> set_meta_class( object_meta_class_mc );
  object_mc -> set_constructor_function( constructor );
  object_mc -> set_size( size );
  object_mc -> inherits_from( base_class );
  if (base_class)
    object_meta_class_mc->set_alignment(base_class->get_alignment_of_instance());
  else
    object_meta_class_mc->set_alignment(alignment_of_void_ptr());
  enter_meta_class( object_mc );

  return object_mc;
}


UnionMetaClass* ObjectFactory::create_union_meta_class(
                 const LString& name,
                 size_t size,
                 ConstructorFunction constructor,
                 AggregateMetaClass* base_class ) {

  assert( !lookupMetaClass( name ) );

  UnionMetaClass* union_mc = new UnionMetaClass( name );

  union_mc -> set_meta_class( _union_meta_class_mc );
  union_mc -> set_constructor_function( constructor );
  union_mc -> set_size( size );
  union_mc -> inherits_from( base_class );
  if (base_class)
    union_mc->set_alignment(base_class->get_alignment_of_instance());
  else
    union_mc->set_alignment(alignment_of_void_ptr());
  enter_meta_class( union_mc );

  return union_mc;
}

void ObjectFactory::apply_to_metaclasses( MetaClassApplier* applier ) {
  MetaClassDictionary::iterator it  = _meta_class_dictionary->begin();
  MetaClassDictionary::iterator end = _meta_class_dictionary->end();
  for( ; it != end; it++ ) {
    MetaClass* m = (*it).second;
    if (!(*applier)(m))
	break;;
    }
  }

// generic error handling which should be overriden by subclass
void ObjectFactory::error( const char* file_name,
                       int line_number,
                       const char* module_name,
                       const char* description, va_list args ) {
  fprintf( stderr, "ERROR IN\n" );
  fprintf( stderr, "FILE: %s   LINE: %d MODULE: %s\n", file_name,
                                                       line_number, module_name );
  vfprintf( stderr, description, args );
  abort();
}

// generic warning handling which should be overriden by subclass
void ObjectFactory::warning( const char* file_name,
                       int line_number,
                       const char* module_name,
                       const char* description, va_list args ) {
  fprintf( stderr, "WARNING IN:\n" );
  fprintf( stderr, "FILE: %s   LINE: %d MODULE: %s\n", file_name,
                                                       line_number, module_name );
  vfprintf( stderr, description, args );
}

// generic info handling which should be overriden by subclass
void ObjectFactory::information( const char* file_name,
                       int line_number,
                       const char* module_name,
                       int verbosity_level,
                       const char* description, va_list args ) {
  fprintf( stderr, "INFORMATIONAL MESSAGE:\n" );
  fprintf( stderr, "FILE: %s   LINE: %d MODULE: %s\n", file_name,
                                                       line_number, module_name );
  vfprintf( stderr, description, args );
}

