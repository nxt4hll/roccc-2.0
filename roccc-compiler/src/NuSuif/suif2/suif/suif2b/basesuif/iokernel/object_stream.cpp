#include "common/system_specific.h"
#include "object_stream.h"
#include "iokernel_forwarders.h"
#include "meta_class.h"
#include "pointer_meta_class.h"
#include "pointer_wrapper.h"
#include "object_factory.h"

#include "common/suif_list.h"
#include "common/suif_map.h"
#include "common/suif_hash_map.h"

#include <stdio.h>
//#include <iostream.h>
#include <iostream>//jul modif
using namespace std;//jul modif



struct TempStorageData {
  Address _address;
  bool  _already_visited;
};

struct ObjectStream::TempStorage {
  Address _address;
  CleanupFunction _cleanup_function;
};





AddressMap::AddressMap(): _address_map(new StorageType) {
}



AddressMap::~AddressMap() {
  delete _address_map;
}

// This is used for initialization of objects that MUST be
// defined before we are able to read anything
// i.e. metaclasses for metaclasses
void AddressMap::add_to_stream( ObjectStream* object_stream ) {
  StorageType::iterator current  = _address_map->begin(),
                        end      = _address_map->end();

  while ( current != end ) {
    object_stream->add_address_pair( (*current).first, (*current).second );
    current++;
  }
}


void AddressMap::add( AddressId id, Address address ) {
  _address_map->enter_value( id, address );
}


void AddressMap::print( ostream& o ) {
  StorageType::iterator current  = _address_map->begin(),
                        end      = _address_map->end();

  o<<"--- ADRESSMAP CONTENTS ---"<<endl;
  while ( current != end ) {
      o<<"ID: "<<(*current).first<<
         "  ADDRESS: "<<(Address)((*current).second)<<endl;
      current++;
    }
    o<<"--------- END -----------"<<endl;
}

void AddressMap::remove_from_address_map( Address address ) {
  StorageType::iterator current  = _address_map->begin(),
                        end      = _address_map->end();
  while ( current != end ) {
    if ( (*current).second == address ) {
       _address_map->erase( current );
       break;
    }
    current++;
  }
}






void ObjectStream::close() {
}


ObjectStream::ObjectStream() :
  _temp_storage(0)
{
}


void ObjectStream::set_temp_storage( const LString& key,
                                     Address address,
                                     CleanupFunction cleanup_function ) {
  // Build on demand.
  if (!_temp_storage) { _temp_storage = new TempStorageMap; }
  TempStorage temp_data;
  temp_data._address = address;
  temp_data._cleanup_function = cleanup_function;
  // (*_temp_storage)[ key ] = temp_data;
  _temp_storage->enter_value(key, temp_data);
}

Address ObjectStream::get_temp_storage( const LString& key ) const {
  Address address = 0;
  if (_temp_storage) {
    TempStorageMap::iterator elem = _temp_storage->find( key );
    if ( elem != _temp_storage->end() ) {
      address = (*elem).second._address;
    }
  }
  return address;
}


ObjectStream::~ObjectStream() {
  if (_temp_storage) {
    for ( TempStorageMap::iterator iter = _temp_storage->begin();
	  iter != _temp_storage->end(); iter++) {
      CleanupFunction cleanup_function = (*iter).second._cleanup_function;
      if ( cleanup_function ) {
	cleanup_function( (*iter).second._address );
      }
    }
    delete _temp_storage;
  }

}


OutputStream::OutputStream() :
  _last_id( 1 ),
  _last_id_max(0),
  _bytes_written(0),
  _address_map( new MyMap ) {
}


OutputStream::~OutputStream() {
  delete _address_map;
}


void OutputStream::write_object( const ObjectWrapper &obj ) {
  write( obj, true );
}




InputStream::InputStream( ObjectFactory* of ) :
  _of(of), _data(new TempStorageMapType),
  _address_map(new AddressMapType),
  _root_objects(new list<ObjectWrapper>),
  _last_id(1) {
}

InputStream::~InputStream() {
  delete _data;
  delete _address_map;
  delete _root_objects;
}

void InputStream::read_close() {

  // resolves all fixup lists
#ifdef AG
  AddressMapType::iterator it = _address_map->begin();
  AddressMapType::iterator end = _address_map->end();
  for (; it != end; ++it ) {
    InputStreamFixUp& info = (*it).second;
     if ( info._state == CloneStreamAddressInfo::ReferencedOnly ) {
       Address address = (*it).first;
       transformReferencedToAlreadyCloned( (Byte*)address, (Byte*)address );
     }
   }
#endif
  // initialize the tree
  while ( !_root_objects->empty() ) {
    ObjectWrapper root = _root_objects->front();
    _root_objects->pop_front();
    Address address = root.get_address();
    set_already_visited( address );
    root.get_meta_class()->initialize( root, this );
  }

}

void InputStream::read_start() {
}



void InputStream::map_id_to_address( AddressId id, Address address ) const {
    AddressMapType::iterator it = _address_map->find( id  );
    if ( it != _address_map->end() ) {
      InputStreamFixUp& info = (*it).second;
      kernel_assert( info._state == InputStreamFixUp::ReferencedOnly );
      Address start_of_fixup_list = info._start_of_fixup_list;

      while ( start_of_fixup_list ) {
        Address temp = *(Address*)start_of_fixup_list;
        *(Address*)start_of_fixup_list = address;
        start_of_fixup_list = temp;
      }
      info._state = InputStreamFixUp::ReadIn;
      info._target_address = address;
    } else {
      InputStreamFixUp info;
      info._state = InputStreamFixUp::ReadIn;
      info._target_address = address;
      _address_map->enter_value( id, info );
    }
}

Address InputStream::get_address( AddressId id ) const {
    AddressMapType::iterator it = _address_map->find( id  );
    kernel_assert( it != _address_map->end() );
    InputStreamFixUp& info = (*it).second;
    kernel_assert( info._state == InputStreamFixUp::ReadIn );
    return info._target_address;
}



bool InputStream::is_already_read( AddressId id ) const {
  if (!id) return true;
  AddressMapType::iterator it = _address_map->find( id );
  return it != _address_map->end() ;
}


AddressMap* OutputStream::get_address_map_of_owned_objects() const {
    AddressMap* m = new AddressMap;


    MyMap::iterator current = _address_map->begin();
    MyMap::iterator end = _address_map->end();
    while ( current != end ) {
      if ( (*current).second._exists ) {
        AddressId id = (*current).second._id;
        Address address = (*current).first;
        m->add( id, address );
      }
      ++current;
    }
    return m;
  }


void OutputStream::add_address_pair( AddressId id, Address address ) {
  kernel_assert(id != 0);  // these ids are special.
  kernel_assert(id != 1);
  AddressInfo i = { id, true };
  _address_map->enter_value( address, i );
  _last_id = id > _last_id ? id : _last_id;
}

void OutputStream::write_defining_pointer( const PointerWrapper &ptr_obj) {
  write_owning_pointer( ptr_obj );
  }

void OutputStream::write_owning_pointer( const PointerWrapper &ptr_obj) {
  Address addressOfPointer = ptr_obj.get_address();
  const PointerMetaClass* pointerMetaClass = ptr_obj.get_meta_class();

  Address address = *(Address*)addressOfPointer;

  // CURRENT WRITING STRATEGY: write object on first encounter
  bool alreadyWritten = is_already_written( address );
  write_address_id(address, !alreadyWritten);
  //  write_unsigned_int( map_address_to_id( address, !alreadyWritten ) );
  if ( alreadyWritten ) {
    return;
  }
  const MetaClass* baseMetaClass = 
    pointerMetaClass->get_base_type();
  const MetaClass* realMetaClass = 
    baseMetaClass->get_meta_class( address );
  write_meta_class( realMetaClass );
  if ( address ) {
     write( ObjectWrapper(address, realMetaClass), false );
   }
}

// CURRENT WRITING STRATEGY: write object on first encounter
void OutputStream::write_reference( const PointerWrapper &ptr_obj) {
  write_owning_pointer( ptr_obj );
}

void OutputStream::write_static_pointer( const PointerWrapper &ptr_obj) {
    Address instance = ptr_obj.get_address();
    const PointerMetaClass *meta_class_of_pointer = ptr_obj.get_meta_class();

    Address address = *(Address*)instance;
    MetaClass *_base_type = meta_class_of_pointer->get_base_type();
    if ( address ) {
        write_byte( 1 );
        write( ObjectWrapper(address, _base_type), false );
        }
    else {
        write_byte( 0 );
        }
    }


// addressable == true  ==> write_object
//                false ==> writePartOfObject
void OutputStream::write( const ObjectWrapper &obj,
			  bool addressable ) {
  Address instance = obj.get_address();
  const MetaClass* metaClass = obj.get_meta_class();
  
  // some more temp stuff here
  if ( addressable ) {
    if ( is_already_written( instance ) ) {
      write_address_id(instance, false);
      return;
    } else {
      write_address_id(instance, true);
    }
  }

  metaClass->write( obj, this );
}


void OutputStream::write_meta_class( const MetaClass* m ) {
  write_address_id((Byte*)m, false);
}


void OutputStream::write_close() {
  unsigned int unresolved = 0;
  for ( MyMap::iterator it = _address_map->begin(); it != _address_map->end(); ++it ) {
      if ( !(*it).second._exists ) {
        write_unsigned_int( (*it).second._id );
        //FIXME - 64bit problem
        //write_unsigned_int( (unsigned int)(*it).first );
        write_unsigned_int( (unsigned long int)(*it).first );
        ++unresolved;
      }
  }
  write_unsigned_int( unresolved );
}


void OutputStream::write_byte_array( Byte* start, unsigned int len ) {
  while ( len-- ) {
    write_byte( *(start) );
    ++start;
  }
}



void OutputStream::write_unsigned_int( unsigned int number ) {
  write_sized_int(&number, sizeof(number), false);
}

#define NORMAL_INT_BIT (1<<(BITSPERBYTE-1))		// 0x80 
#define NORMAL_INT_SIGN_BIT (1<<(BITSPERBYTE-2))	// 0x40 
#define NORMAL_INT_LENGTH_MASK ((1<<(BITSPERBYTE-2))-1)	// 0x3f

#define SIGNED_BYTE_FILL ((1<<(BITSPERBYTE))-1) 	// 0xff 
#define IS_BYTE_SIGNED_BIT (1<<(BITSPERBYTE-1))		// 0x80 

// quick little endian test
static int test_int = 1;
static bool is_little_endian = (*((Byte*)&test_int))!=0;


void OutputStream::write_sized_int( Address instance, 
				    size_t instanceSize,
				    bool is_signed ) {
  //  kernel_assert( instanceSize <= INT_SIZE_MAX ); // maximum 64/128
  // highest bit used for single-byte encoding. next used for sign bit
  // or highest bit used for sign byte

  size_t sizeOfBytesToWrite = instanceSize;
  Byte* currentAddress;
  int direction;
  if ( is_little_endian ) {
    currentAddress = ((Byte*)instance) + instanceSize -1;
    direction = -1;
  } else {
    currentAddress = ((Byte*)instance);
    direction = 1;
  }

  bool isCurrentIntSigned = 
    is_signed && ((*currentAddress) & (IS_BYTE_SIGNED_BIT));
  Byte fillByte = isCurrentIntSigned ? SIGNED_BYTE_FILL : 0;

  while ( sizeOfBytesToWrite && ( (*currentAddress) == fillByte ) ) {
     sizeOfBytesToWrite--;
     currentAddress+=direction;
  }



  // special case optimization - for values 0..127 just write it
  // 
  // i.e. unsigned value 0..127 just write it.
  // otherwise, we mark the top bit with 1, the next bit with the
  // signed-ness, then write the bytes.
  //   
  if (sizeOfBytesToWrite == 0 && !isCurrentIntSigned) {
    write_byte( 0 );
    return;
  }
  if (( sizeOfBytesToWrite == 1 ) &&
      (!isCurrentIntSigned) &&
      (!((*currentAddress) & (IS_BYTE_SIGNED_BIT)))) {
    // for 0 bytes, the value will be zero anyway.
    write_byte( *currentAddress );
    return;
  }
  kernel_assert( (sizeOfBytesToWrite & NORMAL_INT_LENGTH_MASK) 
		 == sizeOfBytesToWrite); // maximum 64/128

  // Encoded length
  Byte size_val = sizeOfBytesToWrite | NORMAL_INT_BIT;
  if (isCurrentIntSigned) { size_val |= NORMAL_INT_SIGN_BIT; }
  write_byte( size_val );

  while ( sizeOfBytesToWrite ) {

    // write it out in network order.
    write_byte( *currentAddress );
    currentAddress+=direction;
    sizeOfBytesToWrite--;
  }

}
void OutputStream::write_address_id(Address address, bool inStream) {
  unsigned int id = map_address_to_id( address, inStream );

  if (id == _last_id) {
    if (_last_id_max == id -1) {
      _last_id_max = id;
      write_unsigned_int(1); 
      return;
    }
    _last_id_max = id;
  }
  write_unsigned_int(id); 
}

unsigned int InputStream::read_address_id() {
  unsigned id = read_unsigned_int();
  if (id == 1) {
    _last_id++;
    id = _last_id;
  } else {
    if (id > _last_id) { _last_id = id; }
  }

  return(id);
}
    
   

unsigned int OutputStream::map_address_to_id( Address address, bool inStream ) {
    if ( !address ) return 0;
    MyMap::iterator it = _address_map->find( address );
    if ( it == _address_map->end() ) {
      AddressInfo info = { ++_last_id, inStream };
      _address_map->enter_value(address,info);
      //return(1);
      return info._id;
    } else {
      kernel_assert( !( (*it).second._exists & inStream ) );
      (*it).second._exists |= inStream;
      return (*it).second._id;
    }
}




bool OutputStream::is_already_written( Address address ) const {
  if (!address) return true;
  MyMap::iterator it = _address_map->find( address );
  if ( it == _address_map->end() ) return false;
  return (*it).second._exists;
}



void InputStream::read( const ObjectWrapper &obj,
			bool addressable  ) {
  Address instance = obj.get_address();
  const MetaClass* metaClass = obj.get_meta_class();

  if ( addressable ) {
     unsigned int id = read_address_id();
     map_id_to_address( id, instance );
  }// else {
  //    _last_id++;
  //  }

  metaClass->read( obj, this );
  // If you are ever looking at this spot in the debugger,
  // This is a good place to print metaClass->print_debug()
}


void InputStream::read_owning_pointer( const PointerWrapper &ptr_obj ) {
  Address addressOfPointer = ptr_obj.get_address();
  //const PointerMetaClass* pointerMetaClass = ptr_obj.get_meta_class();
  // --- temp code for checking output ---
  unsigned int id = read_address_id();
  if ( !is_already_read( id ) ) {
    Address instance;
    MetaClass* realMetaClass = read_meta_class();
    if ( realMetaClass ) {
      instance = create_empty_object( realMetaClass );
      map_id_to_address( id, (Byte*) instance );
      read( ObjectWrapper(instance, realMetaClass), false );
    } else {
      id = 0;
    }
  }
 fixup_address( addressOfPointer, id );
    // *(Byte**)addressOfPointer=(Byte*)instance;
 return;
  }

void InputStream::read_defining_pointer( const PointerWrapper &ptr_obj) {
    read_owning_pointer( ptr_obj );
    }


void InputStream::read_static_pointer( const PointerWrapper &ptr_obj) {
    Address instance = ptr_obj.get_address();
    const PointerMetaClass* pointerMetaClass = ptr_obj.get_meta_class();

    Address ptr = *(Address*)instance;
    MetaClass *_base_type = pointerMetaClass->get_base_type();
    Byte read_an_object = read_byte();
    if ( read_an_object ) {
        if ( !ptr ) {
            ptr = create_empty_nonaddressable_object( _base_type );
            *(Address*)instance = ptr;
            }
        read( ObjectWrapper(ptr, _base_type), false );
        }
    }

void InputStream::read_reference( const PointerWrapper &ptr_obj ) {
  read_owning_pointer( ptr_obj );
  }



void InputStream::fixup_address( Address addressOfPointer, AddressId id ) {
  if ( !id ) {
    *(Address*)addressOfPointer = 0;
    return;
}
 AddressMapType::iterator it = _address_map->find( id );
  if ( it != _address_map->end() ) {
    InputStreamFixUp& info = (*it).second;
    switch( info._state ) {
    case InputStreamFixUp::ReferencedOnly:
      *(Address*)addressOfPointer = (Address*)info._start_of_fixup_list;
      info._start_of_fixup_list = addressOfPointer;
      break;
    case InputStreamFixUp::ReadIn:
//      fprintf( stderr, "COPIED FIX PTR AT: %p WITH: %p (%d)\n", addressOfPointer, (Address)info._target_address, id  );
      *(Address*)addressOfPointer = (Address*)info._target_address;
      break;
    }
  } else {
    InputStreamFixUp info;
    info._state = InputStreamFixUp::ReferencedOnly;
    info._start_of_fixup_list = addressOfPointer;
    *(Address*)addressOfPointer = 0;
    //(*_address_map)[id] = info;
    _address_map->enter_value(id, info);
  }
}


MetaClass* InputStream::read_meta_class() {
  unsigned int id = read_address_id();
  AddressMapType::iterator it = _address_map->find( id  );
  kernel_assert_message( id, ("Invalid metaclass code 0"));
  kernel_assert_message( it != _address_map->end(),
			 ("Metaclass id: %d not found in address map", id));
  MetaClass *mc = (MetaClass*)((*it).second._target_address);
  return mc;
}


unsigned int InputStream::read_unsigned_int() {
  unsigned int number = 0;
  read_sized_int(&number, sizeof(number), false);
  return number;
}


void InputStream::read_sized_int( Address instance, size_t instanceSize,
				  bool is_signed ) {

  // This is the original code from integer_meta_class::read
  Byte b = read_byte();


  if ((b & NORMAL_INT_BIT) == 0) {
    // we have the actual VALUE in B and it is unsigned
    // just SLAM it into the instance and be done with it
    // we'll set a flag

    for (unsigned i = 0; i < instanceSize; i++) {
      ((Byte*)instance)[i] = 0; // unsigned fill byte
    }
    // slam the value to the right side.
    Byte* currentAddress;
    if ( is_little_endian) {
      currentAddress = ((Byte*)instance);
    } else {
      currentAddress = ((Byte*)instance)+instanceSize-1;
    }
    *currentAddress = b;
    return;
  }
  assert(b & NORMAL_INT_BIT);

  size_t currentIntSize = b & ( NORMAL_INT_LENGTH_MASK);
  bool isCurrentIntSigned = ((b & ( NORMAL_INT_SIGN_BIT )) != 0);

  // we should never be reading a signed value into
  // an unsigned type.
  kernel_assert(is_signed || !isCurrentIntSigned);

  kernel_assert_message( currentIntSize <= instanceSize,
			 ("IO Format Error: Attempt to load a %d byte integer into %d bytes", currentIntSize, instanceSize));
  size_t remainingBytes = instanceSize - currentIntSize;
  Byte* currentAddress;
  Byte* remainingByteAddress;
  int direction;
  if ( is_little_endian ) {
    currentAddress = ((Byte*)instance) + currentIntSize - 1;
    direction = -1;
  } else {
    currentAddress = (Byte*)instance+instanceSize-currentIntSize;
    direction = 1;
  }
  remainingByteAddress = currentAddress - direction;

  while ( currentIntSize ) {
    Byte b = read_byte();

    *currentAddress = b;
    currentAddress+=direction;
    currentIntSize--;
  }

  Byte fillByte = isCurrentIntSigned ? (SIGNED_BYTE_FILL)  : 0;
  while ( remainingBytes ) {

     *remainingByteAddress = fillByte;
     remainingByteAddress-=direction;
     remainingBytes--;

  }
  
  
  

  
}

void InputStream::read_byte_array( Byte* address, unsigned int len ) {
  while ( len-- ) {
    *(address)=read_byte();
    ++address;
  }
}


Address InputStream::read_object( const MetaClass* what_to_readin, Address instance  ) {
     // better -----------
     // 1.) create object
     // 2.) read object (as addressable with bug removed)
     // 3.) push the create object on list to read in
     if ( !instance ) {
       instance = create_empty_object( what_to_readin );
     } else {
       what_to_readin->construct_object( instance );
     }
     ObjectWrapper root(instance, what_to_readin);
     read( root, true );
     _root_objects->push_back( root );
     return instance;
  }



void InputStream::add_address_pair( AddressId id, Address address ) {
  kernel_assert(id != 0);
  kernel_assert(id != 1);
  map_id_to_address( id, (Byte*)address );
}


Address InputStream::create_empty_nonaddressable_object( const MetaClass* mc ) {
  return  (Address)_of->create_empty_object( mc );
}


Address InputStream::create_empty_object( const MetaClass* mc ) {
  Address objectAddress =  (Address)_of->create_empty_object( mc );
  TempStorageData tempData = { 0, false };
  //  (*_data)[objectAddress]= tempData;
  _data->enter_value(objectAddress, tempData);
  return objectAddress;
}


Address InputStream::create_empty_object_in_space( Address objectAddress, const MetaClass* mc ) {
  TempStorageData tempData = { 0, false };
  //  (*_data)[objectAddress]= tempData;
  _data->enter_value(objectAddress, tempData);
  _of->create_empty_object_in_space( mc, objectAddress );
  return objectAddress;
}


bool InputStream::exists_in_input_stream( Address objectAddress ) const {
  TempStorageMapType::iterator it = _data->find( objectAddress );
  return ( it != _data->end() );
}

void InputStream::set_already_visited( Address objectAddress ) {
  TempStorageMapType::iterator it = _data->find( objectAddress );
  if ( it == _data->end() ) return;
  (*it).second._already_visited = true;
}

bool InputStream::was_already_visited( Address objectAddress ) const {
  TempStorageMapType::iterator it = _data->find( objectAddress );
  if ( it == _data->end() ) return true;
  return (*it).second._already_visited;
}

void InputStream::store_data( Address objectAddress, Address someData ) {
  TempStorageData tempData = { someData, false };
  //(*_data)[objectAddress]=tempData;
  _data->enter_value(objectAddress, tempData);
}


Address InputStream::retrieve_data( Address objectAddress ) const {
  TempStorageMapType::iterator it = _data->find( objectAddress );
  if ( it == _data->end() ) return 0;
  else return (*it).second._address;
}


void InputStream::remap_address( Address oldAddress, Address newAddress ) {
  AddressMapType::iterator it = _address_map->begin(), end = _address_map->end();
  for ( ; it != end ; ++it ) {
        InputStreamFixUp& info = (*it).second;
      if ( ( info._state == InputStreamFixUp::ReadIn ) &&
           ( info._target_address == oldAddress ) ) {
            info._target_address = newAddress;
#ifdef AG
	    // test
            AddressMapType::iterator it1 = _address_map->find( (*it).first);
            kernel_assert ( &info == &(*it1).second );
            kernel_assert( (*it1).second._target_address = newAddress );
	    // end test
#endif
            return;
      }
  }
  cerr<<"NO ADDRESS TO FIX FOUND"<<endl;
  kernel_assert( false );
}


ObjectFactory* InputStream::get_object_factory() const {
  return _of;
}














