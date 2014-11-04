#ifndef IOKERNEL__AGGREGATE_META_CLASS_H
#define IOKERNEL__AGGREGATE_META_CLASS_H

#include "meta_class.h"
#include "iokernel_forwarders.h"


typedef suif_vector<AggregateMetaClass*> AggregateMetaClassVector;
typedef suif_map<LString, LString> VirtualFieldDescription;

class AggregateMetaClass  : public MetaClass  {
  friend class ObjectFactory;
  friend class AggregateIterator;
  friend class NewAggregateIterator;
public:
  AggregateMetaClass( LString metaClassName = LString() );
  virtual ~AggregateMetaClass();

  virtual FieldDescription* add_field_description( LString fieldName,
                                    MetaClass* metaClass,
                                    size_t offset );

  // The ones without the address will return a field description
  // whether or not it is valid for the current union class.
  // Use this with CARE
  virtual FieldDescription* get_field_description( const LString& field_name ) const;

  /**
   * Count the total number of field in this
   * metaclass and its parents. Uses get_proper_field_count to 
   * count the fields for this metaclass.
   */
  virtual size_t get_field_count(const Address address) const;
  virtual FieldDescription* get_field_description(const Address address,
						  const LString& field_name ) const;
  virtual FieldDescription* get_field_description(const Address address,
						  size_t i) const;

  // The following three methods are used to make available the fields as a list
  // in MacroObjects. The address is only used for union objects

  /**
   * Get a count of the number of fields in this AggregateMetaclass alone.
   */
  virtual size_t get_proper_field_count(const Address address) const;

  virtual FieldDescription* get_proper_field_description(const Address address,
							 const LString& field_name ) const;
  virtual FieldDescription* get_proper_field_description(const Address address,
							 size_t i) const;

  virtual void inherits_from( AggregateMetaClass* metaClass );

  virtual void write( const ObjectWrapper &obj,
                      OutputStream* outputStream ) const;

  virtual void read( const ObjectWrapper &obj,
                     InputStream* inputStream ) const;


  virtual Iterator* get_iterator( ConstAddress instance,
                                  Iterator::Contents = Iterator::All ) const;


  // returns an iterator containing all the instance variables in the
  // range [start_class, end_class]. An empty start_class starts
  // iterating at the first class in the inheritance hierarchy.
  // An empty end_class will iterate till the end
  // [ Statement, IfStatement ] will iterate over the instance variables defined in both
  //    Statement and IfStatement
  // [ IfStatement, IfStatement ] will iterate over the instance variables defined in
  //    IfStatement (it will not iterate over the instance variables in SuifObject, ..., ExecutionObject
  //    Statement )
  virtual Iterator* get_aggregate_iterator( ConstAddress instance,
                                  const LString& start_class,
                                  const LString& end_class = emptyString ) const;

  virtual Iterator* get_aggregate_iterator( ConstAddress instance,
                                  const MetaClass* start_class = 0,
                                  const MetaClass* end_class = 0 ) const;


  // returns an Iterator that just iterates over the instances this
  // MetaClass defines (this doesn't include the instances defined
  // in the inheritance link)
  virtual Iterator* get_local_iterator( Address instance ) const;

  virtual bool is_elementary() const;

  static const LString &get_class_name();

  //  virtual bool isKindOf( const LString& name ) const;

  virtual const MetaClass* get_meta_class( Address address ) const;

  virtual void set_meta_class_of_object( Address instance ) const;

  virtual void initialize( const ObjectWrapper &obj,
			   InputStream* inputStream ) const;

  virtual void adjust_field_offsets();

  virtual MetaClass* get_link_meta_class() const;

  virtual void add_virtual_field( const LString& name, const String& description );

  virtual Iterator* get_virtual_iterator( ConstAddress instance, const LString& name ) const;

  virtual VirtualNode* get_virtual_node( const LString &name, const String &what ) const;

  virtual void set_destructor_function( DestructorFunction destructor_function );

  virtual void destruct( const ObjectWrapper &obj,
			 bool called_from_destructor ) const;

  virtual Walker::ApplyStatus walk(const Address address,Walker &w) const;

  virtual AggregateMetaClass* get_base_class() const;

  virtual ConstructorFunction get_constructor_function() const;

  virtual void construct_object( Address address ) const;

  // walk the meta-classes referenced by a given meta class.
  virtual void walk_referenced_meta_classes(MetaClassApplier *x);

  virtual String get_debug_text() const;

protected:
  virtual void init_virtual_nodes();

  virtual bool has_its_own_constructor() const;
  static void constructor_function( Address address );

  virtual void initialize_base_types() const;

  virtual void call_destructor( const AggregateWrapper &agg_obj ) const;

  AggregateMetaClass* _base_class;  // single inheritance only

protected:
  typedef suif_map<LString, VirtualNode*> NodeMap;


  suif_vector<FieldDescription*>* _fields;

  sf_mutable NodeMap* _virtual_nodes;
  sf_mutable VirtualFieldDescription* _virtual_field_description;

  AggregateMetaClassVector* _base_types; // caching of the inheritance links
  int _number_of_base_classes; // caching of base classes
                               // this number is valid only if _base_types is non
                               // null
  DestructorFunction _destructor_function;

  Walker::ApplyStatus walk_fields(const Address address,Walker &w) const;
private:

  AggregateMetaClass(const AggregateMetaClass &);
  AggregateMetaClass &operator=(const AggregateMetaClass &);
};


class ObjectAggregateMetaClass : public AggregateMetaClass {
  friend class ObjectFactory;
public:
  ObjectAggregateMetaClass( LString metaClassName = LString() );

  virtual const MetaClass* get_meta_class( Address address ) const;

  virtual void set_meta_class_of_object( Address instance ) const;

  virtual void set_destructor_function( DestructorFunction destructor_function );


  virtual void construct_object( Address instance ) const;

  Walker::ApplyStatus walk(const Address address,Walker &w) const;


  static const LString &get_class_name();

protected:
  virtual void call_destructor( const AggregateWrapper &agg_obj ) const;

  static void constructor_function( Address address );
};


#endif








