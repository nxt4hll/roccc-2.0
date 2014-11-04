#ifndef IOKERNEL__OBJECT_FACTORY_H
#define IOKERNEL__OBJECT_FACTORY_H

/*** An ObjectFactory serves the following purposes:
   * . creates suif objects,
   * . managing meta classes,
   * . helps in reading and writing of suif objects to output stream,
   * . report error, warning, and informational messages.
   *
   *
   * Representation:
   *
   *	the next meta class id.
   * int _last_id;
   *
   *	map: AddressId -> rudimentary meta class
   * AddressMap* _rudimentary_address_map;
   *
   *	map: meta class id -> meta class
   * MetaClassDictionary* _meta_class_dictionary;
   *
   *	The following are the meta classes that describe the basic
   *	meta class objects.
   * AggregateMetaClass* _aggregate_meta_class_mc;
   * AggregateMetaClass* _object_aggregate_meta_class_mc;
   * AggregateMetaClass* _pointer_meta_class_mc;
   * AggregateMetaClass* _union_meta_class_mc;
   * AggregateMetaClass* _integer_meta_class_mc;
   * AggregateMetaClass* _list_meta_class_mc;
   *
   *	the SuifEnv to which this object belongs
   * SuifEnv* _suif_env;
   *
   */


#include "object.h"
#include "iokernel_forwarders.h"

#include <stdarg.h>

class ObjectFactory : public Object {
  // suif_hash_maps currently dont work correctly. Their behavior is most likely
  // not identical. Suspicion: The iterator doesn't iterate over all the elements
  // or the system assumes an ordering property that the hash map cannot fulfill
  typedef suif_hash_map<LString,MetaClass*> MetaClassDictionary;
  //typedef suif_map<LString,MetaClass*> MetaClassDictionary;

public:
  ObjectFactory();

/**
  * Intialize this object, notably create all rudimentary
  * meta-meta-class objects.
  */
  virtual void init( SuifEnv* suif );

  virtual ~ObjectFactory();

  /* Lookup or create a new meta class object.
   * If meta class of the same name already exist but it is not compatible
   *   with the input argument, these get_ methods will terminate this
   *   process.
   */
  virtual IntegerMetaClass* get_integer_meta_class(
                                   const LString& name,
                                   size_t size,
                                   bool is_signed,
				   size_t align );

  virtual PointerMetaClass* get_pointer_meta_class(
                                   MetaClass* base_type,
                                   bool pointer_owns_object = true,
                                   bool is_static = false,
				   bool must_be_cloned = false );

  virtual ListMetaClass* get_list_meta_class(
                                   MetaClass* element_type,
				   const LString &real_type);


  virtual STLMetaClass* get_stl_meta_class(
                                   const LString& name,
                                   TypeLessSTLDescriptor* stl_descriptor );

  /*
   * If the name crashes with an existing meta class, these create_ methods
   *   will terminate this process.
   */
  virtual AggregateMetaClass* create_aggregate_meta_class(
                                   const LString& name,
                                   size_t size = 0,
                                   ConstructorFunction constructor =
                                        emptyConstructorFunction,
                                   AggregateMetaClass* base_class = 0 );
  virtual ObjectAggregateMetaClass* create_object_meta_class(
                                   const LString& name,
                                   size_t size = 0,
                                   ConstructorFunction constructor =
				       emptyConstructorFunction,
                                   AggregateMetaClass* base_class = 0 );
  virtual UnionMetaClass* create_union_meta_class(
                                   const LString& name,
                                   size_t size = 0,
                                   ConstructorFunction constructor =
                                                  emptyConstructorFunction,
                                   AggregateMetaClass* base_class = 0 );

  /*
   * Find the meta class with the same name as metaClassName.
   * Return a null pointer if not found.
   */
  virtual MetaClass* find_meta_class(
                const LString& metaClassName );
  // obsolete
  virtual MetaClass* lookupMetaClass( const LString& metaClassName );

  /*
   * Find the aggregate meta class with the same name as metaClassName.
   * If no such meta class exist or if the meta class (with the same name)
   *  is not an aggregate meta class, terminate this process.
   */
  virtual AggregateMetaClass* find_aggregate_meta_class(
                const LString& metaClassName );

  /*
   * Find the union meta class with the same name as metaClassName.
   * If no such meta class exist or if the meta class (with the same name)
   *  is not an union meta class, terminate this process.
   */
  virtual UnionMetaClass * find_union_meta_class(
                const LString& metaClassName );

  /*
   * Create an object instance.
   */
  virtual Address create_empty_object_by_name( const LString& meta_class_name );

  virtual Address create_empty_object( const MetaClass* metaClass ) const;

  virtual Address create_empty_object_in_space( const MetaClass* metaClass,
						Address space ) const;


  /*
   * Register a meta class to this objec factory.
   * 'mc' must already have a non-null name (or it will bomb).
   * A unique id will be assigned to 'mc'
   */
  virtual void enter_meta_class( MetaClass* mc );

  /*
   * Print the content to cerr.  For debugging only.
   */
  virtual void print_contents();

  virtual AddressMap* get_rudimentary_address_map() const;

  virtual MetaClassId get_last_id() const;

  virtual SuifEnv* get_suif_env() const;

  virtual void apply_to_metaclasses( MetaClassApplier* applier );

  // error message handling

  /*
   * Print an error message to stderr.
   */
  virtual void error(  const char* file_name,
                       int line_number,
                       const char* module_name,
                       const char* description, va_list ap );

  /*
   * Print an warning message to stderr.
   */
  virtual void warning( const char* file_name,
                       int line_number,
                       const char* module_name,
                       const char* description, va_list ap );

  /*
   * Print an informational message to stderr.
   */
  virtual void information( const char* file_name,
                       int line_number,
                       const char* module_name,
                       int verbosity_level,
                       const char* description, va_list ap );

  /*
   * Return true iff 'm's name starts with "LIST:"
   */
  static bool is_a_list( MetaClass* m );

protected:

  /*
   * Invoke ObjectFactory::ObjectFactory() on 'place'.
   */
  static void constructor_function( void* place );

private:
  int _last_id;
  AddressMap* _rudimentary_address_map;
  MetaClassDictionary* _meta_class_dictionary;
  AggregateMetaClass* _aggregate_meta_class_mc;
  AggregateMetaClass* _object_aggregate_meta_class_mc;
  AggregateMetaClass* _pointer_meta_class_mc;
  AggregateMetaClass* _union_meta_class_mc;
  AggregateMetaClass* _integer_meta_class_mc;
  AggregateMetaClass* _list_meta_class_mc;
  SuifEnv* _suif_env;
private:
  // there should be no definitions for these declarations
  ObjectFactory(const ObjectFactory &);
  ObjectFactory &operator=(const ObjectFactory &);
};


#endif














