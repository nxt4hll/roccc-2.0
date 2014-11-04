#include "common/system_specific.h"
#include "lstring_meta_class.h"
#include "object_stream.h"
#include "common/suif_vector.h"

#include <iostream>//jul modif
using namespace std;//jul modif

// #ifdef PGI_BUILD
// #include <new>
// #else
// #include <new.h>
// #endif
#include <new>//jul modif

#ifdef DEBUG
//#include <iostream.h>
extern const char * indent_text();
#endif

static const LString lstring_meta_class_class_name("LStringMetaClass");

const LString& LStringMetaClass::get_class_name() {return lstring_meta_class_class_name;}

class LStringOutputCache {
  suif_vector<bool> _is_written;
public:
  LStringOutputCache() {}
  void write(const LString &s, OutputStream *outputStream) {
    size_t id = s.get_ordinal();
    while (id >= _is_written.size()) { _is_written.push_back(0); }
    if (_is_written[id]) {
      outputStream->write_unsigned_int( id );
    } else {
      _is_written[id] = true;
      outputStream->write_unsigned_int( id );
      int len = s.length();
      outputStream->write_unsigned_int( len );
      outputStream->write_byte_array( (unsigned char*)s.c_str(), len );
    }
#ifdef DEBUG
    cerr << indent_text() << "  wrote LString " << s.c_str() << endl;
#endif
  }
  ~LStringOutputCache() {}
};
static void lstring_output_cache_destructor(LStringOutputCache *cache) {
   delete cache;
}

class LStringInputCache {
  suif_vector<bool> _is_read;
  suif_vector<LString> _value;
public:
  LStringInputCache() {}
  ~LStringInputCache() {}
  void read(Address instance, InputStream *inputStream) {
    size_t id = inputStream->read_unsigned_int();
    while (id >= _is_read.size()) { _is_read.push_back(0); }
    while (id >= _value.size()) { _value.push_back(emptyLString); }
    if (_is_read[id]) {
      new (instance) LString(_value[id]);
      return;
    }
    _is_read[id] = true;
    unsigned int len = inputStream->read_unsigned_int();
    char* s = new char[ len+1 ];
    inputStream->read_byte_array( (Byte*)s, len );
    s[len]=0;
    LString str(s);
    delete [] s;
    _value[id] = str;
    new (instance) LString(str);
#ifdef DEBUG
    cerr << indent_text() << "  read LString " << str.c_str() << len << endl;
#endif
  }
};
static void lstring_input_cache_destructor(LStringInputCache *cache) {
  delete cache;
}

void LStringMetaClass::write( const ObjectWrapper &obj,
                              OutputStream* outputStream ) const {
  Address instance = obj.get_address();
  kernel_assert(obj.get_meta_class() == this);

  static LString lstring_key = "LString";
  LStringOutputCache *cache = 
    (LStringOutputCache*)outputStream->get_temp_storage(lstring_key);
  if (!cache) {
    cache = new LStringOutputCache;
    outputStream->set_temp_storage(lstring_key, 
				   (void *)cache, 
				   (ObjectStream::CleanupFunction)
				   lstring_output_cache_destructor);
  }
  LString *str = (LString *)instance;
  cache->write(*str, outputStream);
}


void LStringMetaClass::read ( const ObjectWrapper &obj,
			      InputStream* inputStream ) const {
  Address instance = obj.get_address();
  kernel_assert(obj.get_meta_class() == this);

  static LString lstring_key = "LString";
  LStringInputCache *cache = 
    (LStringInputCache *)inputStream->get_temp_storage(lstring_key);
  if (!cache) {
    cache = new LStringInputCache;
    inputStream->set_temp_storage(lstring_key, 
				  (void *)cache, 
				  (ObjectStream::CleanupFunction)
				  lstring_input_cache_destructor);
  }
  cache->read(instance, inputStream);
  //LString *str = (LString *)instance; // for debugger visibility.
}


LStringMetaClass::LStringMetaClass( LString metaClassName ) :
    MetaClass( metaClassName ) {
  _size = sizeof(LString);
}

void LStringMetaClass::destruct( const ObjectWrapper &obj,
				 bool called_from_destructor ) const {
  Address address = obj.get_address();
  kernel_assert(obj.get_meta_class() == this);
  ((LString*)address)->~LString();
}


void LStringMetaClass::constructor_function( Address place ) {
  new (place) LStringMetaClass;
}


Walker::ApplyStatus LStringMetaClass::walk(const Address address,Walker &walk) const {
    // do nothing - not a suif object, does not contain suif objects
    return Walker::Continue;
    }
    

