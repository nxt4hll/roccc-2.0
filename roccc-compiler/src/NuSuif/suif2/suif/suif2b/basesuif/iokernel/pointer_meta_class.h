#ifndef IOKERNEL__POINTER_META_CLASS_H
#define IOKERNEL__POINTER_META_CLASS_H

#include "meta_class.h"
#include "iokernel_forwarders.h"


class PointerMetaClass : public MetaClass {
 friend class ObjectFactory;
public:
  virtual void write( const ObjectWrapper &obj,
		      OutputStream* outputStream ) const;

  virtual void read( const ObjectWrapper &obj, 
		     InputStream* inputStream ) const;

  virtual Iterator* get_iterator( ConstAddress instance, Iterator::Contents contents = Iterator::All ) const;

  virtual bool is_elementary() const;

  virtual bool is_owning_pointer() const;

  virtual bool needs_cloning() const;

  virtual MetaClass* get_base_type() const;

  virtual void initialize( const ObjectWrapper &obj,
			   InputStream* inputStream ) const;

  virtual MetaClass* get_link_meta_class() const;

  virtual void adjust_field_offsets();

  virtual VirtualNode* get_virtual_node( const LString &name, const String &what ) const;

  virtual void destruct( const ObjectWrapper &obj, 
			 bool called_from_destructor ) const;

  Walker::ApplyStatus walk(const Address address,Walker &walk) const;

  static const LString &get_class_name();

  virtual void walk_referenced_meta_classes(MetaClassApplier *x);

protected:
  PointerMetaClass( LString name = emptyLString,
                    MetaClass* baseType = 0,
                    bool pointerOwnsObject = false,
		    bool is_static = false,
		    bool needs_cloning = false);

  static void constructor_function( void* place );

private:
  MetaClass* _base_type;
  MetaClass* _ptr_to_base_type; // non persistent cache
  bool _pointer_owns_object;

  // a pointer that has the static attribute has the following properties:
  //  1.) PointerMetaClass::_pointer_owns_object must be true
  // 2.) the pointer either points to NULL or to an object of exactly _base_type
  //        (that's why it is called static)
  // 3.) The object may either be created by the constructor of the surrounding
  //      object or if the field is NULL and it is needed by the PointerMetaClass
  //      during a read in operation
  // 4.) The address of the object to which it points must not be taken and stored
  //       by another instance variable (the object is not addressable)
  //       It is semantically equivalent to an instance of the surrounding object.
  bool _is_static;
  bool _needs_cloning;

private:
  PointerMetaClass(const PointerMetaClass &);
  PointerMetaClass &operator=(const PointerMetaClass &);
};


#endif
