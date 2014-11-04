#include "common/system_specific.h"
#include "i_integer_meta_class.h"

#include "common/i_integer.h"
// ANSI C++ says this should be <sstream>
// #ifndef MSVC
// #include <strstream.h>
// #else
// #include <strstrea.h>
// #endif

#include <iostream>//jul modif
#include <sstream>//jul modif
using namespace std;//jul modif

// #ifdef DEBUG
// #include <iostream.h>
// #endif

#include "object_stream.h"

#include "iokernel_forwarders.h"

// #ifdef PGI_BUILD
#include <new>
// #else
// #include <new.h>
// #endif

static const LString iinteger_meta_class_class_name("IIntegerMetaClass");

const LString& IIntegerMetaClass::get_class_name() {return iinteger_meta_class_class_name;}

#define LONG_IINT_BIT (1<<(BITSPERBYTE-1))		// 0x80
#define STRING_IINT_BIT (1<<(BITSPERBYTE-2))		// 0x40
#define LONG_STRING_IINT_BIT (1<<(BITSPERBYTE-3))	// 0x20
#define LONG_IINT_SIGN_BIT (1<<(BITSPERBYTE-3))		// 0x20 

#define SHORT_IINT_LENGTH_MASK ((1<<(BITSPERBYTE-1))-1)	// 0x7f

#define STRING_LENGTH_MASK (1<<(BITSPERBYTE-3)-1)	// 0x1f
#define LONG_IINT_LENGTH_MASK (1<<(BITSPERBYTE-3)-1)	// 0x1f

#define BYTE_MASK ((1<<(BITSPERBYTE))-1)	 	// 0xff 
#define SIGNED_BYTE_FILL ((1<<(BITSPERBYTE))-1) 	// 0xff 
#define IS_BYTE_SIGNED_BIT (1<<(BITSPERBYTE-1))		// 0x80 


//	The creation of strstream from an i_integer gives bogus results, so I have redone this
//	code. 
void IIntegerMetaClass::write( const ObjectWrapper &obj,
			       OutputStream* outputStream ) const {
  Address instance = obj.get_address();
  kernel_assert(obj.get_meta_class() == this);

  IInteger *iint = (IInteger *)instance;

  // Change this to an encoding:
  // 2 formats:
  // 0vvvvvvv  -> unsigned value vvvvvvv
  // 10slllll, BYTE_ARRAY -> s is SIGN, lllll is length, BYTES read like
  //                         bytes.
  // 110lllll, BYTE_ARRAY -> lllll is length, BYTE_ARRAY is String value
  // 111-----, UNSIGNED_INT, BYTE_ARRAY -> UNSIGNED_INT is length, 
  //                         BYTE_ARRAY is String value
  // Bits are named:
  // 7: LONG_IINT_BIT
  // 6: STRING_IINT_BIT
  // 5: LONG_STRING_IINT_BIT  and LONG_IINT_SIGN_BIT
  //
  // If the unsigned size in bytes doesn't fit in 5 bits (32) write as string.
  // For string representation, 
  //     If the length in characters doesn't fit in 5 bits (32) write length
  //     as unsigned_int
  //
  // When reading in, be caseful.  If the size doesn't fit into a word,
  //    then it must be read in as an i_integer.
  if (iint->is_c_long()) {
    long val = iint->c_long();
    if ((val & SHORT_IINT_LENGTH_MASK) == val) {
      outputStream->write_byte((Byte)val);
      return;
    }
    bool is_signed = (val < 0);

    unsigned num_bytes = 0;
    long final_val = is_signed ? -1 : 0; 
    for (long val_copy = val; val_copy != final_val; 
	 num_bytes++, val_copy >>= BITSPERBYTE) {}
    
    // short proof, in 2s complement, the high bit is 0 for
    // an unsigned int, shifting right by the BITSPERBYTE
    // will come to zero
    kernel_assert(num_bytes <= sizeof(long));

    // make sure it fits
    if ((num_bytes & LONG_IINT_LENGTH_MASK) == num_bytes) { 
      //Encoded length
      Byte length_encode = num_bytes | LONG_IINT_BIT;
      if (is_signed) { length_encode |= LONG_IINT_SIGN_BIT; }
      outputStream->write_byte(length_encode);
      // write it in big endian style.
      for (unsigned i = 0; i < num_bytes; i++) {
	Byte bval = BYTE_MASK & (val >> ((num_bytes - i -1)*BITSPERBYTE));
	outputStream->write_byte(bval);
      }
      return;
    }
  }

  // Write it as a string.
  
  String str;
  iint->write(str,10);
  unsigned int len = str.length();
  // Now we know we will output a string representation
  // We could use this for the string length... Naw forget it.
  if ((len & STRING_LENGTH_MASK) == len) {
    Byte length_encode = len | LONG_IINT_BIT | STRING_IINT_BIT;
    outputStream->write_byte(length_encode);
  } else {
    Byte code = LONG_IINT_BIT | STRING_IINT_BIT | LONG_STRING_IINT_BIT;
    outputStream->write_byte(code);
    outputStream->write_unsigned_int( len );
  }
  // cerr << (Byte*)str.c_str() <<  " len = " << len << endl;
  outputStream->write_byte_array( (Byte*)str.c_str(), len );
}


void  IIntegerMetaClass::read ( const ObjectWrapper &obj,
				InputStream* inputStream ) const {
  Address instance = obj.get_address();
  kernel_assert(obj.get_meta_class() == this);

  Byte b = inputStream->read_byte();
  if ((b & LONG_IINT_BIT) == 0) {
    new (instance) IInteger(b);
    return;
  }
  // try the next bit
  if ((b & STRING_IINT_BIT) == 0) {
    bool is_signed = b & (LONG_IINT_SIGN_BIT);
    unsigned num_bytes = b & LONG_IINT_LENGTH_MASK;
    if (num_bytes > sizeof(long)) { // if it doesnt fit, build an iint
      IInteger new_val = is_signed? IInteger(-1) : IInteger(0);
      for (unsigned i = 0; i < num_bytes; i++) {
	Byte next_b = inputStream->read_byte();
	// This should test the implementation of IIntegers.
	new_val = new_val << BITSPERBYTE;
	new_val |= next_b;
      }
      new (instance) IInteger(new_val);
      return;
    } else {
      long new_val = is_signed ? -1 : 0;
      for (unsigned i = 0; i < num_bytes; i++) {
	Byte next_b = inputStream->read_byte();
	// This should test the implementation of IIntegers.
	new_val = new_val << BITSPERBYTE;
	new_val |= next_b;
      }
      new (instance) IInteger(new_val);
      return;
    }
  }
  unsigned int len;
  // otherwise read the string.
  if ((b & LONG_STRING_IINT_BIT) == 0) {
    len = b & STRING_LENGTH_MASK;
  } else {
    len = inputStream->read_unsigned_int();
  }

  char* s = new char[ len+1 ];

  inputStream->read_byte_array( (Byte*)s, len );

  s[len]=0;
#ifdef DEBUG
  cerr<<s<<endl;
#endif

  new (instance) IInteger(s);
  delete [] s;
}



  struct X_IInteger {
        char x;
        IInteger field;
        };

IIntegerMetaClass::IIntegerMetaClass( LString metaClassName ) :
  MetaClass( metaClassName ) {
  _size = sizeof(IInteger);
  set_alignment(OFFSETOF(X_IInteger,field));
  }


void IIntegerMetaClass::destruct( const ObjectWrapper &obj,
				  bool called_from_destructor ) const {
  Address address = obj.get_address();
  kernel_assert(obj.get_meta_class() == this);

  ((IInteger*)address)->~IInteger();
}


void IIntegerMetaClass::constructor_function( Address place ) {
  new (place) IIntegerMetaClass;
}

Walker::ApplyStatus IIntegerMetaClass::walk(const Address instance,Walker &walk) const {
    // do nothing
    return Walker::Continue;
    }

