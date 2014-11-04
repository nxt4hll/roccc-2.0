#ifndef IOKERNEL__OBJECT_H
#define IOKERNEL__OBJECT_H

#include "iokernel_forwarders.h"



/**
  * @file
  * This file contains definition of Object class.
  */

class Object {
  friend class ObjectFactory;
public:

  /**
    * Get the meta class object of this object.
    * @internal
    */
  virtual const MetaClass* get_meta_class() const;

  /**
    * Get the aggregate meta class object of this object.
    * @internal
    */
  virtual const AggregateMetaClass* get_aggregate_meta_class() const;

  /**
    * Get the name of the class of which this object is an instance of.
    * This returns the instance name of the actual dynamic instance.
    */
  virtual const LString& getClassName() const;

  /**
    * Return true if this object is an instance of the class named by
    * \a className.
    * @param className  name of a class.
    * @return true iff this object is an instance of the class \a className.
    */
  virtual bool isA( const LString& className ) const;

  /**
    * Return true if this object is a_kind_of the class named by \a className.
    * Object \a o is a_kind_of class \a c iff \a o is an instance of \a c
    * or a subclass of \a c.
    *
    * @param className  name of a class.
    * @return true iff this object is a_kind_of the class \a className.
    */
  virtual bool isKindOf( const LString& className ) const;

  /**
    * @internal
    */
  virtual void set_meta_class( const MetaClass* metaClass );

  /**
    * This returns the instance name of this Class - "Object".
    */
  static const LString &get_class_name();

  /**
    * one input name => returns the address of the object and its meta_class.
    * @param name    name of a field.
    * @param address location to store the field value.
    * @param meta_class location to store the type of the field.
    * @internal
    */
  virtual FieldWrapper get_member( const LString &name) const;
  //  virtual Address get_member( const LString& name, 
  //			      Address& address, 
  //			      const MetaClass*& meta_class ) const;

  /**
    * Destructor.
    */
  virtual ~Object();

protected:
  static void constructor_function( void *instance );

  Object();


  const AggregateMetaClass* _meta_class;
private:
  // there should be no definitions for these declarations
  Object( const Object& x );
  Object& operator=(const Object& x );

}; /* Object */



/**
  * @internal
  */
bool is_kind_of_object_meta_class(const MetaClass *mc);

#endif

