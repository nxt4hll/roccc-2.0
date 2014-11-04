#ifndef META_CLASS_ITER_H
#define META_CLASS_ITER_H

#include "iokernel_forwarders.h"
#include "object_wrapper.h"
#include "field_wrapper.h"

class Iterator {
public:
    enum Contents { All, Referenced, Owned };

    // the current type
    virtual const MetaClass* current_meta_class() const = 0 ;

    // meaningful only for aggregate_meta_classes otherwise
    // the empty string
    virtual const LString& current_name() const = 0;

    virtual Address current() const = 0;

    virtual ObjectWrapper current_object() const;
    virtual FieldWrapper current_field() const;


    // does the iterator point to something valid
    virtual bool is_valid() const = 0;

    // iteration operations
    virtual void next() = 0;
    virtual void previous() = 0;
    virtual void set_to( size_t index );
    virtual size_t length() const;
    virtual void first() = 0;

    virtual void add( Address object );
    virtual void print_to_default() const;
    virtual Iterator *clone() const = 0;
    Iterator() {};
    virtual ~Iterator();
private:
    // override stupid defaults;
    Iterator &operator=(const Iterator &);
    Iterator(const Iterator &);
};




class SingleElementIterator : public Iterator {
public:
   SingleElementIterator( Address address, const MetaClass* mc );
   SingleElementIterator( const ObjectWrapper &obj);

    virtual const MetaClass* current_meta_class() const;

    // meaningful only for aggregate_meta_classes otherwise
    // the empty string
    virtual const LString& current_name() const;

    virtual void* current() const;                 // for pointers


    // does the iterator point to something valid
    virtual bool is_valid() const;

    // iteration operations
    virtual void next();
    virtual void previous();
    virtual void first();
    virtual size_t length() const;

    virtual Iterator *clone() const;

private:
  bool _is_valid;
  ObjectWrapper _object;
  //  Address address;
  //  const MetaClass* metaClass;
  SingleElementIterator(const SingleElementIterator &);
  SingleElementIterator &operator=(const SingleElementIterator &);
};

class EmptyIterator : public Iterator {
public:
  EmptyIterator( );

  virtual const MetaClass* current_meta_class() const;

  // meaningful only for aggregate_meta_classes otherwise
  // the empty string
  virtual const LString& current_name() const;

  virtual void* current() const;                 // for pointers

  // does the iterator point to something valid
  virtual bool is_valid() const;

  // iteration operations
  virtual void next();
  virtual void previous();
  virtual void first();
  virtual size_t length() const;
  
  virtual Iterator *clone() const;

 private:
  EmptyIterator(const EmptyIterator &);
  EmptyIterator &operator=(const EmptyIterator &);
};

#endif /* META_CLASS_ITER_H */
