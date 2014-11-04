#ifndef IOKERNEL__INTEGER_META_CLASS_H
#define IOKERNEL__INTEGER_META_CLASS_H

#include "meta_class.h"
#include "iokernel_forwarders.h"


class IntegerMetaClass : public MetaClass {
  friend class ObjectFactory;
public:
  virtual void write( const ObjectWrapper &obj,
		      OutputStream* outputStream ) const;

  virtual void read( const ObjectWrapper &obj,
		     InputStream* inputStream ) const;

  virtual bool is_signed() const;

  Walker::ApplyStatus walk(const Address address,Walker &walk) const;

  static const LString &get_class_name();

protected:
  IntegerMetaClass( LString metaClassName = emptyLString );

  static void constructor_function( Address place );

  virtual void set_is_signed( bool isSigned );

  static int test_int;

  static bool is_little_endian;

private:
  bool _is_signed;

};


#endif
