#ifndef IOKERNEL__STL_META_CLASS_H
#define IOKERNEL__STL_META_CLASS_H

#include "iokernel_forwarders.h"
#include "list_meta_class.h"


/* #ifdef PGI_BUILD */
/* #include <new> */
/* #else */
/* #include <new.h> */
/* #endif */
#include <new>

template<class T> class STLDescriptor;


class TypeLessSTLDescriptor {
public:
    virtual Iterator* get_iterator( ConstAddress address ) const=0;

    virtual MetaClass* get_element_meta_class() const;
    virtual ConstructorFunction get_constructor_function() const;
    virtual DestructorFunction get_destructor_function() const;
    virtual size_t get_size_of_instance() const;
    virtual ~TypeLessSTLDescriptor(){}
protected:
  // the following instance variables are initialized by the
  // STLDescriptor subclass
  MetaClass* _element_meta_class;
  size_t _size;
  ConstructorFunction _constructor_function;
  DestructorFunction _destructor_function;

  TypeLessSTLDescriptor(MetaClass *element_meta_class,
			size_t size,
			ConstructorFunction constructor_function,
			DestructorFunction destructor_function) :
    _element_meta_class(element_meta_class),
    _size(size),
    _constructor_function(constructor_function),
    _destructor_function(destructor_function) {}
private:
  // no definitions for these constructors
  TypeLessSTLDescriptor(const TypeLessSTLDescriptor &other);
  TypeLessSTLDescriptor &operator=(const TypeLessSTLDescriptor &other);
};


template <class T>
class STLIterator : public BaseListIterator {
  friend class STLDescriptor<T>;
 public:
  STLIterator( Address address, const MetaClass* elementMetaClass ) :
    BaseListIterator( elementMetaClass ),
      collectionObject( *(T*)address )
    //,it (collectionObject.begin())
    {
      it = collectionObject.begin();

    _is_valid = it != collectionObject.end();
  }

  virtual ~STLIterator(){}

  virtual Address current() const {
    if ( _is_valid ) {
      return (Address)&(*it);
    }
    return 0;
  }

    // iteration operations
    virtual void next() {
      if ( !_is_valid ) return;
      it++; _is_valid = ( it != collectionObject.end() );
    }

    virtual void previous() {
      if ( !_is_valid ) return;
      _is_valid = !( it == collectionObject.begin() );
      if ( _is_valid ) it--;
    }

    virtual void first() {
      it = collectionObject.begin();
      _is_valid = it != collectionObject.end();
    }

    virtual size_t length() const {
        return collectionObject.size();
    }

    virtual void add( void* object ) {
      it = collectionObject.end();
      collectionObject.insert( it, *((typename T::value_type*)object) );
      it = collectionObject.end();
    }

    virtual Iterator *clone() const {
      //	return new STLIterator<T>((T &)collectionObject,
      //				  (typename T::iterator)it);
      	return new STLIterator<T>(_element_meta_class,
				  collectionObject,
				  (typename T::iterator &)it);
    }


private:
  STLIterator(const MetaClass *element_meta_class,
	      const T& co,
	      typename T::iterator &i) :
	BaseListIterator(element_meta_class),
	collectionObject((T&)co),it(i) {}
  // We can't even consider making a const one of these
  // because our STL implementation can not deal with const.
  T& collectionObject;
  typename T::iterator it;
private:
  // no definitions for these declarations
  STLIterator(const STLIterator<T> &other);
  STLIterator &operator=(const STLIterator<T> &other);


};


template<class T> class STLDescriptor : public TypeLessSTLDescriptor {

  static void STLMetaClassConstructorFunction( Address place ) {
    new(place) T;
  }

  static void STLMetaClassDestructorFunction( const ObjectWrapper &obj ) {
    Address place = obj.get_address();
    ((T*)place)->~T();
  }

public:
  STLDescriptor( MetaClass* element_meta_class ) :
    TypeLessSTLDescriptor(element_meta_class, sizeof(T),
			  STLMetaClassConstructorFunction,
			  STLMetaClassDestructorFunction)
	{}

  virtual Iterator* get_iterator( ConstAddress address ) const {
    return new STLIterator<T>( (Address)address, _element_meta_class );
  }
};




class STLMetaClass : public ListMetaClass {
  friend class ObjectFactory;
public:
  STLMetaClass( LString metaClassName = LString() );
  virtual ~STLMetaClass();

  virtual void set_descriptor( TypeLessSTLDescriptor* stl_descriptor );

  virtual Iterator* get_iterator( ConstAddress instance,
                                  Iterator::Contents contents = Iterator::Owned ) const;

  virtual ConstructorFunction get_constructor_function() const;

  virtual void set_constructor_function( ConstructorFunction constructorFunction );

  virtual void destruct( const ObjectWrapper &obj,
			 bool called_from_destructor ) const;

  static void constructor_function( void* place );

  static const LString &get_class_name();

protected:
  virtual void copy_over( Address target, GenericList* source ) const;

private:
  TypeLessSTLDescriptor* _stl_descriptor;
private:
  // no definitions for these declarations
  STLMetaClass(const STLMetaClass &);
  STLMetaClass &operator=(const STLMetaClass &);
};




#endif

