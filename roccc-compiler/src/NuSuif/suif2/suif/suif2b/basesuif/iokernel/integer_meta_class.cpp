#include "common/system_specific.h"
#include "integer_meta_class.h"

#include "object_stream.h"
#include "clone_stream.h"
#include "iokernel_forwarders.h"

// #if defined(PGI_BUILD) || defined(MSVC)
// #include <new>
// #else
// #include <new.h>
// #endif
#include <new>




using namespace std;




int IntegerMetaClass::test_int = 1;
bool IntegerMetaClass::is_little_endian = (*((Byte*)&test_int))!=0;



void IntegerMetaClass::write( const ObjectWrapper &obj,
			      OutputStream* outputStream ) const {
  Address instance = obj.get_address();
  kernel_assert(obj.get_meta_class() == this);

  outputStream->write_sized_int(instance, get_size_of_instance(),
				is_signed());
}





void IntegerMetaClass::read( const ObjectWrapper &obj,
			     InputStream* inputStream ) const {
  Address instance = obj.get_address();
  kernel_assert(obj.get_meta_class() == this);
 
  inputStream->read_sized_int(instance, get_size_of_instance(),
			       is_signed());
}



IntegerMetaClass::IntegerMetaClass( LString metaClassName ) :
  MetaClass( metaClassName ), _is_signed(false) {
}


void IntegerMetaClass::set_is_signed( bool is_signed ) {
  _is_signed = is_signed;
}


bool IntegerMetaClass::is_signed() const {
  return _is_signed;
}


static const LString integer_meta_class_class_name("IntegerMetaClass");

const LString &IntegerMetaClass::get_class_name() {
  return integer_meta_class_class_name;
}




void IntegerMetaClass::constructor_function( Address place ) {
  new (place) IntegerMetaClass;
}


Walker::ApplyStatus IntegerMetaClass::walk(const Address instance,Walker &walk) const {
    // do nothing
    return Walker::Continue;
    }
