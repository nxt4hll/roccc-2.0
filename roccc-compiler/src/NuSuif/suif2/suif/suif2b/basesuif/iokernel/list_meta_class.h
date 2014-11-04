#ifndef IOKERNEL__LIST_META_CLASS_H
#define IOKERNEL__LIST_META_CLASS_H

#include "meta_class.h"
#include "iokernel_forwarders.h"

struct GenericList {
  int count;
  Address space;
};

class ListMetaClass : public MetaClass {
  friend class ObjectFactory;
public:
  ListMetaClass( LString metaClassName = LString() );

  virtual void write( const ObjectWrapper &obj,
		      OutputStream* outputStream ) const;

  virtual void read( const ObjectWrapper &obj,
		     InputStream* inputStream ) const;

  virtual Iterator* get_iterator( ConstAddress instance,
                                  Iterator::Contents contents = Iterator::Owned ) const;

  virtual bool is_elementary() const;

  virtual void initialize( const ObjectWrapper &obj,
			   InputStream* inputStream ) const;

  virtual void set_meta_class_of_object( Address address ) const;

  virtual MetaClass* get_element_meta_class() const;

  virtual VirtualNode* get_virtual_node( const LString &name, const String &what ) const;

  virtual void set_element_meta_class( MetaClass* mc );

  virtual void destruct( const ObjectWrapper &obj,
			 bool called_from_destructor ) const;

  static void constructor_function( void* place );

  virtual void adjust_field_offsets();

  Walker::ApplyStatus walk(const Address address,Walker &walk) const;

  static const LString &get_class_name();

  virtual void walk_referenced_meta_classes(MetaClassApplier *x);

protected:
  virtual void copy_over( Address target, GenericList* source ) const;

  MetaClass* _element_meta_class;
private:
  ListMetaClass(const ListMetaClass &);
  ListMetaClass &operator=(const ListMetaClass &);

};


// this is an abstract class
class BaseListIterator : public Iterator {
public:
  BaseListIterator( const MetaClass* element_meta_class );

  virtual const MetaClass* current_meta_class() const;

  virtual const LString& current_name() const;

  virtual bool is_valid() const;
protected:
  bool _is_valid;
  const MetaClass* _element_meta_class;
private:
  BaseListIterator(const BaseListIterator &);
  BaseListIterator &operator=(const BaseListIterator &);
};


class ListIterator : public BaseListIterator {
public:
  ListIterator( GenericList* glist, MetaClass* element_meta_class );

  virtual void* current() const;

  virtual void next();

  virtual void previous();

  virtual void first();

  virtual void set_to( size_t index );

  virtual size_t length() const;

  virtual Iterator *clone() const;

private:
  int currentIndex;
  GenericList* genericList;
private:
  ListIterator(const ListIterator &);
  ListIterator &operator=(const ListIterator &);
};








#endif
