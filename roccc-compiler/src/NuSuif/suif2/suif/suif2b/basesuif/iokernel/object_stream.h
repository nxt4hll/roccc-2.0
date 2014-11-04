#ifndef IOKERNEL__OBJECTSTREAM_H
#define IOKERNEL__OBJECTSTREAM_H

#include "object.h"
#include "iokernel_forwarders.h"
#include "object_wrapper.h"

/** An AddressMap is a mapping between AddressId (unsinged int)
  * and Address (void *).
  * It is used in the serialization of objects.
  *
  *
  * Add a new pair into this map.
  *
  * virtual void add( AddressId id, Address address );
  *
  *
  *
  * Copy all entries from this map to the one in object_stream.
  *
  *  virtual void add_to_stream( ObjectStream* object_stream );
  *
  *
  * 
  * Remove an entry from this map.
  *
  *  virtual void remove_from_address_map( Address address );
  *
  *
  *
  * Print the content of this map in human readable form to ostream o.
  *
  * virtual void print( ostream& o );
  */ 

class AddressMap {
private:
  typedef suif_map<AddressId,Address > StorageType;

public:
  AddressMap();

  virtual ~AddressMap();

  virtual void add( AddressId id, Address address );

  virtual void add_to_stream( ObjectStream* object_stream );

  virtual void remove_from_address_map( Address address );

  virtual void print( std::ostream& o );

private:
  StorageType* _address_map;

  // no implementation for these declarations
  AddressMap(const AddressMap &);
  AddressMap &operator=(const AddressMap &);
};





/**
  * A super class for InputSteam and OutputStream.
  * An ObjectStream is responsible for reading and writing to/from
  * a .suif file.
  * Each ObjectSteam contains a (AddressId, Address) mapping.
  *
  *
  *
  * Add an entry to the internal address map.
  *
  * virtual void add_address_pair( AddressId id, Address address );
  *
  *
  *
  * A NOP.  What is it suppose to do ???
  *
  * virtual void close();
  *
  *
  *
  * ???
  *  virtual void set_temp_storage( const LString& key, Address address, 
  *				 CleanupFunction cleanup_function );
  *
  *
  * ???
  *  virtual Address get_temp_storage( const LString& key ) const;
  *
  */




class ObjectStream {
public:
  virtual void add_address_pair( AddressId id, Address address ) = 0;

  virtual void close();

  // Methods and types for temp storage
  struct TempStorage;  
  typedef suif_hash_map<LString, TempStorage> TempStorageMap;
  typedef void (*CleanupFunction)( Address address );
  virtual void set_temp_storage( const LString& key, Address address, 
				 CleanupFunction cleanup_function );
  virtual Address get_temp_storage( const LString& key ) const;
  

protected:
  ObjectStream();

  virtual ~ObjectStream();
  
  //  The temp storage is used for caching on input/output
  TempStorageMap* _temp_storage;

private:
  // override stupid defaults, don't implement these
  ObjectStream &operator=(const ObjectStream &);
  ObjectStream(const ObjectStream &);
};


struct InputStreamFixUp {
    enum State { ReferencedOnly, ReadIn };
    State _state;
    union {
      Address _start_of_fixup_list;
      Address _target_address;
    };
};


struct InputStreamFixUp;
struct TempStorageData;
typedef suif_hash_map<AddressId, InputStreamFixUp> AddressMapType;
typedef suif_hash_map<Address, TempStorageData> TempStorageMapType;

class InputStream : public ObjectStream {
public:
  InputStream( ObjectFactory* of );
  ~InputStream();

  virtual Address read_object( const MetaClass* metaClass, Address space = 0 /* if 0 will create space */ );

  // produces something that's never addressable
  virtual void read( const ObjectWrapper &obj, bool addressable = false  );

  // must be invoked before using objects read in
  // This method might do things like pointer fix-ups after cloing
  virtual void read_close();
  virtual void read_start(); // obsolete ??

  // make one
  virtual void read_static_pointer( const PointerWrapper &ptr_obj);
  virtual void read_defining_pointer( const PointerWrapper &ptr_obj);
  virtual void read_owning_pointer( const PointerWrapper &ptr_obj);
  virtual void read_reference( const PointerWrapper &ptr_obj);

  virtual unsigned int read_address_id();
  virtual unsigned int read_unsigned_int();
  virtual void read_sized_int( Address instance, size_t instanceSize,
			       bool is_signed );
  virtual void read_byte_array( Byte* address, unsigned int len );
  virtual MetaClass* read_meta_class();
  virtual Byte read_byte() = 0;

  // obsolete ??
  virtual Address create_empty_nonaddressable_object( const MetaClass* mc );
  virtual Address create_empty_object( const MetaClass* mc );  // used
  virtual void* create_empty_object_in_space( Address a, const MetaClass* mc); // unused
  virtual bool is_already_read( unsigned int id ) const; //used

  virtual void fixup_address( Address, AddressId  );

  virtual bool exists_in_input_stream( Address object_address ) const ;
  virtual void set_already_visited( Address object_address );
  virtual bool was_already_visited( Address object_address ) const;

  virtual void store_data( Address objectAddress, Address someData );
  virtual Address retrieve_data( Address object_address ) const;

  virtual void remap_address( Address old_address, Address new_address );
  virtual ObjectFactory* get_object_factory() const;

  virtual void add_address_pair( AddressId id, Address address );

protected:
  virtual void map_id_to_address( AddressId id, Address address ) const;
  virtual Address get_address( AddressId id ) const;

  /*
  struct RootObject {
    Address _address;
    const MetaClass* _meta_class; };
  */

  ObjectFactory* _of;
  TempStorageMapType* _data;
  AddressMapType* _address_map;
  list<ObjectWrapper>* _root_objects;
  size_t _last_id; // used for an optimization
  InputStream&operator=(const InputStream&);
  InputStream(const InputStream&);
};


struct AddressInfo {
    AddressId _id;
    bool _exists;
};

typedef suif_hash_map<Address, AddressInfo > MyMap;

class OutputStream : public ObjectStream {
public:
  OutputStream();
  virtual ~OutputStream();

  virtual void write_object( const ObjectWrapper &obj );

  // writing an object
  virtual void write( const ObjectWrapper &obj, bool addressable = true );

  // this final writing operation has to be called before reading can take place
  virtual void write_close();

  virtual void write_static_pointer( const PointerWrapper &ptr_obj);
  virtual void write_defining_pointer( const PointerWrapper &ptr_obj);
  virtual void write_owning_pointer( const PointerWrapper &ptr_obj);
  virtual void write_reference( const PointerWrapper &ptr_obj);

  virtual void write_address_id( Address address, bool inStream );
  virtual void write_unsigned_int( unsigned int number );
  virtual void write_sized_int( Address instance, size_t instanceSize,
				bool is_signed );
  virtual void write_byte_array( Byte* start, unsigned int len );
  virtual void write_meta_class( const MetaClass* m );
  virtual void write_byte( Byte b ) = 0;

  virtual bool is_already_written( Address address ) const;

  virtual AddressMap* get_address_map_of_owned_objects() const;

  virtual void add_address_pair( AddressId id, Address address );

protected:
  virtual AddressId map_address_to_id( Address address, bool inStream );

  size_t _last_id;
  size_t _last_id_max;
  long _bytes_written;
  MyMap* _address_map;
  // no implementation for these declarations
  OutputStream& operator=(const OutputStream&);
  OutputStream(const OutputStream&);

};


#endif




















