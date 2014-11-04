#include "common/system_specific.h"
#include "string_meta_class.h"

#include "object_stream.h"
#include "iokernel_forwarders.h"
#include "common/suif_hash_map.h"
#include "common/suif_vector.h"
// #ifdef DEBUG
// #include <iostream.h>
// #endif
#include <iostream>// jul modif
#include <new>// jul modif
// #ifdef PGI_BUILD
// #include <new>
// #else
// #include <new.h>
// #endif
using namespace std;
static const LString string_meta_class_class_name("StringMetaClass");

#if DEBUG
extern char * indent_text();
#endif

const LString& StringMetaClass::get_class_name() {return string_meta_class_class_name;}

size_t hash(const String &s) {
  const char* ptr = s.c_str();
  unsigned int hash_value = 0;
  while ( *ptr ) { hash_value+=*ptr;ptr++; }
  return hash( hash_value );
}


class StringOutputCache {
  typedef suif_hash_map<String, int> StringMap;
  StringMap _string_map;
  suif_vector<String> _strings;
public:
  StringOutputCache() {}
  void write(const String &s, OutputStream *outputStream) {
    StringMap::iterator iter = _string_map.find(s);
    if (iter != _string_map.end()) {
      size_t id = (*iter).second;
      outputStream->write_unsigned_int( id );
    } else {
      size_t id = _string_map.size();
      outputStream->write_unsigned_int( id );
      _string_map.enter_value(s, id);
      _strings.push_back(s);
      int len = s.length();

      outputStream->write_unsigned_int( len );
      outputStream->write_byte_array( (unsigned char*)s.c_str(), len );
    }
  }
  ~StringOutputCache() {
    //    while (!_is_written.empty()) { _is_written.pop_back(); }
  }
};
static void string_output_cache_destructor(StringOutputCache *cache) {
  delete cache;
}

class StringInputCache {
  suif_vector<bool> _is_read;
  suif_vector<String> _value;
public:
  StringInputCache() {}
  ~StringInputCache() {
    //    while (!_is_read.empty()) { _is_read.pop_back(); }
    //    while (!_value.empty()) { _value.pop_back(); }
  }
  void read(Address instance, InputStream *inputStream) {
    size_t id = inputStream->read_unsigned_int();
    while (id >= _is_read.size()) { _is_read.push_back(0); }
    while (id >= _value.size()) { _value.push_back(emptyString); }
    if (_is_read[id]) {
      new (instance) String(_value[id]);
      return;
    }
    _is_read[id] = true;
    unsigned int len = inputStream->read_unsigned_int();
    char* s = new char[ len+1 ];
    inputStream->read_byte_array( (Byte*)s, len );
    s[len]=0;
    String str(s);
    delete [] s;
    _value[id] = str;
    new (instance) String(str);
  }
};
static void string_input_cache_destructor(StringInputCache *cache) {
  delete cache;
}



void StringMetaClass::write( const ObjectWrapper &obj,
                             OutputStream* outputStream ) const {
  Address instance = obj.get_address();
  kernel_assert(obj.get_meta_class() == this);

  static LString string_key = "String";
  StringOutputCache *cache = 
    (StringOutputCache*)outputStream->get_temp_storage(string_key);
  if (!cache) {
    cache = new StringOutputCache;
    outputStream->
      set_temp_storage(string_key, (void *)cache, 
		       (ObjectStream::CleanupFunction)
		       string_output_cache_destructor);
  }
  String *str = (String *)instance;
  cache->write(*str, outputStream);
}


void  StringMetaClass::read ( const ObjectWrapper &obj,
                              InputStream* inputStream ) const {
  Address instance = obj.get_address();
  kernel_assert(obj.get_meta_class() == this);

  static LString string_key = "String";
  StringInputCache *cache = 
    (StringInputCache *)inputStream->get_temp_storage(string_key);
  if (!cache) {
    cache = new StringInputCache;
    inputStream->
      set_temp_storage(string_key, 
		       (void *)cache, 
		       (ObjectStream::CleanupFunction)
		       string_input_cache_destructor);
   }
   cache->read(instance, inputStream);
   // for debugger visibility
   //String *str = (String *)instance;
   
}


StringMetaClass::StringMetaClass( LString metaClassName ) : MetaClass( metaClassName ) {
  _size = sizeof(String);
}


void StringMetaClass::destruct( const ObjectWrapper &obj,
				bool called_from_destructor ) const {
  Address address = obj.get_address();
  kernel_assert(obj.get_meta_class() == this);
 
  ((String*)address)->~String();
}


void StringMetaClass::constructor_function( Address place ) {
  new (place) StringMetaClass;
}

Walker::ApplyStatus StringMetaClass::walk(const Address instance,Walker &walk) const {
    // do nothing
    return Walker::Continue;
    }








