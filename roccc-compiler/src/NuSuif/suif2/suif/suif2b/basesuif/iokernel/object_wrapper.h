#ifndef OBJECT_WRAPPER
#define OBJECT_WRAPPER

#include "iokernel_forwarders.h"

// The following objects is Immutable.

class ObjectWrapper {
public:
  ObjectWrapper();
  ObjectWrapper(Address address, const MetaClass *metaclass);
  ObjectWrapper(Object *obj);
  // We freely mix const-ness within the Object wrapper
  // in order to avoid creating 2 wrapping objects, one
  // that is const and one that is not.
  ObjectWrapper(const Object * const obj);
  ObjectWrapper(const ObjectWrapper &);
  ObjectWrapper &operator=(const ObjectWrapper &);
  bool operator==(const ObjectWrapper &);
  Address get_address() const;
  const MetaClass *get_meta_class() const;
  Object *get_object() const; // return 0 if not a subclass of object
  void print_debug() const;
  bool is_null() const; // return 0 if address is 0


  // Now, add an interface that looks like the meta_class interface
  //Walker::ApplyStatus walk(Walker &walk) const;

  // The object life cycle.
  // we should NOT be able to construct on an objectwrapper
  // because the object does not yet have it's metaclass set.
  //  void construct() const;

  // if the metaclass is a subclass of an object,
  // return the more precise metaclass stored in the Object's meta_class field
  ObjectWrapper update_meta_class() const;
  
  void initialize(InputStream *inputStream) const;
  void destruct(bool called_from_destructor) const;


private:
  Address _address;
  const MetaClass *_meta_class;
};

#endif /* OBJECT_WRAPPER */
