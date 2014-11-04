#ifndef IOKERNEL__BINARY_STREAMS_H
#define IOKERNEL__BINARY_STREAMS_H

#include "object_stream.h"
#include "iokernel_forwarders.h"

class BinaryOutputStream : public OutputStream {
public:
  BinaryOutputStream( std::ostream& o );
  ~BinaryOutputStream();

  virtual void write_byte( Byte b );

private:
  std::ostream& _output_stream;
};


class BinaryInputStream : public InputStream {
public:
  BinaryInputStream( ObjectFactory* of, std::istream& i );
  ~BinaryInputStream();

  virtual Byte read_byte();

private:
  std::istream& _input_stream;
};


#endif

