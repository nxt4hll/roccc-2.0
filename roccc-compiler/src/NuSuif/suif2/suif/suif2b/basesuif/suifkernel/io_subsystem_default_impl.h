/**
  * InputSubSystemDefaultImplementatio is an implementation of
  *  InputSubSystem.
  * It is responsible for reading in a .suif file.
  *
  *
  * Read in a .suif file whose path is in inputFileName.
  * Returns the FileSetBlock.
  *
  *   virtual FileSetBlock *read( const String& inputFileName );
  */

/**
  * OutputSubSystemDefaultImplementatio is an implementation of
  *  OutputSystem.
  * It is responsible for writing to a .suif file.
  *
  *
  * Write the FileSetBlock in its SuifEnv to a .suif file whose
  * path is in outputFileName.
  *
  *  virtual void write( const String& outputFileName );
  */
#ifndef SUIFKERNEL__IO_SUBSYSTEM_DEFAULT_IMPL_H
#define SUIFKERNEL__IO_SUBSYSTEM_DEFAULT_IMPL_H

#include "io_subsystem.h"

class InputSubSystemDefaultImplementation : public InputSubSystem {
public:
  InputSubSystemDefaultImplementation( SuifEnv* suif_env );

  virtual FileSetBlock *read( const String& inputFileName );
};


class OutputSubSystemDefaultImplementation : public OutputSubSystem {
public:
  OutputSubSystemDefaultImplementation( SuifEnv* suif_env );

  virtual void write( const String& outputFileName );
};


class CloneSubSystemDefaultImplementation : public CloneSubSystem {
public:
  CloneSubSystemDefaultImplementation( SuifEnv* suif_env );
  virtual ~CloneSubSystemDefaultImplementation();

  virtual CloneStream* get_deep_clone_stream();

  virtual CloneStream* get_shallow_clone_stream();

  virtual void set_deep_clone_stream(CloneStream *str);

  virtual void set_shallow_clone_stream(CloneStream *str);

private:
  CloneStream * _deep_stream;
  CloneStream * _shallow_stream;
};


#endif
