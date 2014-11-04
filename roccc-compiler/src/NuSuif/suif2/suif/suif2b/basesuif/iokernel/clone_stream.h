#ifndef IOKERNEL__CLONESTREAM_H
#define IOKERNEL__CLONESTREAM_H

#include "object_stream.h"
#include "iokernel_forwarders.h"
#include "common/suif_vector.h"

#include <sstream>

//	Cloner. 

class CloneStreamObjectInstance;

enum PTR_HANDLING {ENQUIRE_PTR,NULL_PTR,COPY_PTR,CLONE_PTR};
enum PTR_TYPE     {NO_PTR,REF_PTR,DEFINE_PTR,OWN_PTR};

class CloneStream : public OutputStream, public InputStream  {
public:
  CloneStream( ObjectFactory* of );
  virtual ~CloneStream();

  // The following clone a tree rooted on a single object. These two
  // calls are equivalent - thefirst finds the metaclass of the object
  // and calls the second
  virtual Object* clone( const Object* object );

  virtual Address clone( Address instance, 
			 const MetaClass* meta_class_of_instance, 
			 Address target = 0);

  // Or, you can call the following routines to present a forest of
  // trees to be cloned, and then call for cloning. To use these, you must
  // call open before cloning and close after cloning.

  // open the cloning first
  virtual void open();

  bool get_is_open() const;

  virtual void push_for_clone(const Object *object);

  // as well as pushing for cloning, you can push for pointer replacement.
  // This is not the same as pushing with a target because in that case,
  // the object is copied

  virtual void push_for_replacement(Object *old_obj,Object *new_obj);

  virtual void push_for_clone(Address instance,
			      const MetaClass* meta_class_of_instance,
			      Address target = 0);

  virtual void perform_cloning();

  // There are also some status actions available when you clone
  // explicitly (using open/push_for_clone etc)
  // don't call close until you finish with status information

  // was the given address cloned?
  bool is_cloned(Address addr) const;

  // find address of clone. Returns NULL if not cloned
  Address clone_address(Address addr) const;

  // is this object cloned?
  bool get_is_cloned(Address addr) const;

  // how many objects were cloned?
  int get_clone_count() const;

  // get information about nth cloned object

  Address get_old_address(int n) const;
  Address get_new_address(int n) const;
  const MetaClass *get_meta_class(int n) const;
  bool get_is_cloned(int n) const;
  bool get_is_orphan(int n) const;

  // finally, close the cloner
  virtual void close();

  // Finish cloning of an orphan object. An orphan object is
  // an object that is cloned but is not owned by any cloned
  // object.

  virtual void finish_orphan_object_cloning(
	Object *old_object,
	Object *new_object);

  // set how objects are to be handled
  void set_pointer_handling( PTR_HANDLING refed_objects,
                         PTR_HANDLING defined_objects,
                         PTR_HANDLING owned_objects);
  void set_refed_object_handling(PTR_HANDLING handling);
  PTR_HANDLING get_refed_object_handling() const;
  void set_defined_object_handling(PTR_HANDLING handling);
  PTR_HANDLING get_defined_object_handling() const;
  void set_owned_object_handling(PTR_HANDLING handling);
  PTR_HANDLING get_owned_object_handling() const;

  // For objects that are to be enquired upon, the following routine is
  // called. The status must be set by calling one of the following
  // routines using the same status.

  virtual void object_enquiry(Object *,CloneStreamObjectInstance *,PTR_TYPE);

  // Clone the object
  void set_clone_object(CloneStreamObjectInstance *);

  // Reference the object
  void set_reference(CloneStreamObjectInstance *);

  // replace pointers to the object by other pointer value (can be NULL)
  void set_replacement(CloneStreamObjectInstance *,Object *);

  // End of object status routines


  // A couple of useful settings pre-packaged

  void set_deep_clone();	// the default clone owned and defined objects
  void set_shallow_clone(); // NULL all pointers

  virtual void set_already_visited( Address object_address );
  virtual bool was_already_visited( Address object_address ) const;

protected:
  suif_hash_map<Address,CloneStreamObjectInstance *> *_clone_table;
  suif_vector<CloneStreamObjectInstance *> *_work_list;
  CloneStreamObjectInstance *find_or_add_object(Address addr,
						const MetaClass *meta);
private:

  CloneStreamObjectInstance * write_ptr(PTR_HANDLING handling,
					PTR_TYPE ptr_type,
					const PointerWrapper &ptr_obj);

  void read_ptr(const PointerWrapper &ptr_obj);

  virtual Byte read_byte();

  virtual void read_static_pointer( const PointerWrapper &ptr_obj);
  virtual void read_defining_pointer( const PointerWrapper &ptr_obj);
  virtual void read_owning_pointer( const PointerWrapper &ptr_obj);
  virtual void read_reference( const PointerWrapper &ptr_obj);

  virtual void write_byte( Byte b );

  virtual void write_static_pointer( const PointerWrapper &ptr_obj);
  virtual void write_owning_pointer( const PointerWrapper &ptr_obj);
  virtual void write_defining_pointer( const PointerWrapper &ptr_obj);
  virtual void write_reference( const PointerWrapper &ptr_obj);
  
  std::stringstream* _object_buffer;
  CloneStream(const CloneStream&);
  CloneStream& operator=(const CloneStream&);

  PTR_HANDLING _refed_objects;
  PTR_HANDLING _defined_objects;
  PTR_HANDLING _owned_objects;

  bool _is_open;
  ObjectFactory *_of;

};

//	Shallow clone no longer exists - instead derive from CloneStream and
//	call shallow_clone method

#endif
