#ifndef IOKERNEL__META_CLASS_H
#define IOKERNEL__META_CLASS_H

#include "object.h"
#include "iokernel_forwarders.h"
#include "walker.h"
#include "meta_class_iter.h"


#define OFFSETOF( CLASSNAME, MEMBERNAME )    ((size_t)(&((CLASSNAME*)0)->MEMBERNAME))




typedef void (*InitializerFunction)( const ObjectWrapper &obj,
                                     bool is_owned,
                                     InputStream* input_stream );

class MetaClassApplier {
    public:
        virtual bool operator () (MetaClass *x) = 0;
        virtual ~MetaClassApplier();
        };

class MetaClass : public Object {
  friend class ObjectFactory;
public:

  // returns the name of the class (For example: "IfStatement" )
  virtual const LString& get_instance_name() const;

  // returns the instance size of an object (For example: 4 for an int)
  virtual size_t get_size_of_instance() const;

  // return the alignment for an instance. Must be on 0 mod alignment border
  virtual size_t get_alignment_of_instance() const;

  // creates an instance of an object at 'instance' and fills it with
  // contents from the input_stream
  virtual void read ( const ObjectWrapper &obj, 
		      InputStream* inputStream ) const;

  // writes an instance of 'this' at 'instance' address to the output_stream
  // 'o'
  virtual void write( const ObjectWrapper &obj, 
		      OutputStream* outputStream ) const;

  // returns an iterator that iterates over the
  // element that 'instance' contains
  // If the MetaClass doesn't encapsulate any objects
  // 0 is returned as the iterator
  virtual Iterator* get_iterator( ConstAddress instance,
                 Iterator::Contents contents = Iterator::Owned ) const;
  // This currently dispatches to the instance version
  virtual Iterator* get_iterator( const ObjectWrapper &obj,
                 Iterator::Contents contents = Iterator::Owned ) const;

  virtual bool is_elementary() const ;

  // works for all 'objects' subclassed for Object
  virtual const MetaClass* get_meta_class( Address address ) const;

  virtual void set_constructor_function( ConstructorFunction constructorFunction );

  virtual ConstructorFunction get_constructor_function() const;

  virtual void construct_object( Address address ) const;

  virtual void initialize( const ObjectWrapper &obj,
			   InputStream* inputStream ) const;

  virtual MetaClass* get_link_meta_class() const;

  virtual void adjust_field_offsets();

  // walk the meta-classes referenced by a given meta class.
  virtual void walk_referenced_meta_classes(MetaClassApplier *x);

  virtual MetaClassId get_meta_class_id() const;

  virtual VirtualNode* get_virtual_node( const LString &name, const String &what ) const;

  virtual bool defines_a_subtype_of( const MetaClass* m ) const;

  // is the object for which this is the meta class of the given kind?
  bool object_is_kind_of(const LString &className) const;
  bool has_constructed_object() const;

  // debugging help.  Print out all meta class info.
  virtual String get_debug_text() const;
  virtual void print_debug() const;

  // deletes all sub-parts of an object that the
  // default destructor does not destruct.
  // For example: The system reads in an object of type MethodCallInstruction
  // and since no MethodCallInstruction exists it is mapped to a
  // CallInstruction
  // The destructor of CallInstruction can only call the destructors
  // of the sub objects it owns and nothing else
  // This method uses the dynamic MetaClass of the object to destruct
  // the remaining objects
  virtual void destruct( const ObjectWrapper &obj,
			 bool called_from_destructor ) const;

  static const LString &get_class_name();

  virtual Walker::ApplyStatus walk(const ObjectWrapper &obj,
				   Walker &walk) const;
  virtual Walker::ApplyStatus walk(const Address address,Walker &walk) const;

public:
  MetaClass( LString metaClassName = LString() );

  virtual ~MetaClass();

  virtual void set_size( size_t size );

  virtual void set_alignment(size_t size);

  virtual size_t get_size() const;

  virtual void set_meta_class_of_object( Address instance ) const;

  virtual ObjectFactory* get_owning_factory() const;

protected:
  virtual void set_owning_factory( ObjectFactory* owning_factory );

  static void constructor_function( Address place );

  InitializerFunction _pre_init;

  InitializerFunction _post_init;

protected:
  virtual void set_meta_class_id( MetaClassId id );

  LString _meta_class_name;

  size_t _size;

  size_t _alignment;

  MetaClassId _meta_class_id;

  ObjectFactory* _owning_factory;
  
// public:
  // void debugit(const char *doing,Address to_what);
  // void debugit(const char *doing,int i);
private:
  ConstructorFunction _constructor_function;
  bool _has_constructed_object;

  MetaClass(const MetaClass &);
  MetaClass &operator=(const MetaClass &);
};





#endif







