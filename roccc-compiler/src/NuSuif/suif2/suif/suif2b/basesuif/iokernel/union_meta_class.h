#ifndef IOKERNEL_UNIONMETACLASS_H_
#define IOKERNEL_UNIONMETACLASS_H_

#include "aggregate_meta_class.h"
#include "iokernel_forwarders.h"



// used for writing/initializing only if the
// UnionMetaClass::_offset field is not set!!
typedef int (*UnionSelector)( Address address );


class UnionMetaClass : public AggregateMetaClass {
  friend class ObjectFactory;
  friend class UnionIterator;
  void zero_field(Address address) const;
public:
  UnionMetaClass( LString meta_class_name = LString() );
  virtual ~UnionMetaClass();

  virtual void add_union_field( LString field_name,
                 MetaClass* meta_class,
                 size_t offset );

  // use exactly one of the following two methods
  virtual void set_union_selector( UnionSelector union_selector ); // the function returning the tag
  virtual void set_offset( int offset ); // the offset for the tag which is of type 'int'

  virtual void write( const ObjectWrapper &obj,
                      OutputStream* outputStream ) const;

  virtual void read( const ObjectWrapper &obj,
                     InputStream* inputStream ) const;

  virtual void initialize( const ObjectWrapper &obj,
			   InputStream* inputStream ) const;

  virtual void adjust_field_offsets();

  virtual Iterator* get_local_iterator( Address instance ) const;

  virtual const MetaClass* get_meta_class( Address address ) const;

  virtual void set_meta_class_of_object( Address instance ) const;

  Walker::ApplyStatus walk(const Address address,Walker &walk) const;

  static const LString &get_class_name();

  virtual size_t get_proper_field_count(const Address address) const;

  virtual FieldDescription* get_proper_field_description(const Address address,
							 const LString& field_name ) const;
  virtual FieldDescription* get_proper_field_description(const Address address,
							 size_t i) const;

  virtual void walk_referenced_meta_classes(MetaClassApplier *x);

protected:
  static void constructor_function( Address address );

  virtual int get_tag_num( Address instance ) const;
private:

   suif_vector<FieldDescription*>* _union_fields;
   // the UnionMetaClass uses either the _tag_offset or the _union_selector, but not both!
   // the other one is 0 (_union_selector) or -1 (_tag_offset) to specifiy an invalide entry
   int _tag_offset;
   UnionSelector _union_selector;
private:
  UnionMetaClass(const UnionMetaClass &);
  UnionMetaClass &operator=(const UnionMetaClass &);

};



#endif






