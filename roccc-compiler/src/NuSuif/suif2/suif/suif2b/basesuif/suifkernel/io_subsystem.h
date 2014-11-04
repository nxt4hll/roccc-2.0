/**
  * InputSubSystem is responsible for reading in a .suif file.
  * An InputSubSystem contains a SuifEnv.
  *
  *
  * Read in a .suif file whose path is in inputFileName.
  * Returns the FileSetBlock.
  *
  *   virtual FileSetBlock *read( const String& inputFileName );
  */

/**
  * OutputSubSystem is responsible for writing to a .suif file.
  *
  *
  * Write the FileSetBlock in its SuifEnv to a .suif file whose
  * path is in outputFileName.
  *
  *  virtual void write( const String& outputFileName );
  */

#ifndef SUIFKERNEL__IO_SUBSYSTEM_H
#define SUIFKERNEL__IO_SUBSYSTEM_H

#include "suifkernel_forwarders.h"
#include "subsystem.h"

// called whenever cloning an object of type 'metaClass' is cloned
typedef void (*ObjectClone)( SuifCloneStream* scs,
			     const ObjectWrapper &obj,
                             bool addressable );

class InputSubSystem : public SubSystem {
public:
  InputSubSystem( SuifEnv* suif_env );

  virtual FileSetBlock *read( const String& inputFileName ) = 0;

  virtual ~InputSubSystem();
};


class OutputSubSystem : public SubSystem {
public:
  OutputSubSystem( SuifEnv* suif_env );

  virtual void write( const String& outputFileName ) = 0;

  virtual ~OutputSubSystem();
};


class CloneSubSystem : public SubSystem {
public:
  CloneSubSystem( SuifEnv* suif_env );

  virtual SuifObject* deep_clone( const SuifObject* object );
  virtual SuifObject* shallow_clone( const SuifObject* object );

  // the target parameter makes only sense for the cloning of value classes
  // in all other cases it should not be used
  virtual Address deep_clone( Address address, const MetaClass* metaClass, Address target = 0 );
  virtual Address shallow_clone( Address address, const MetaClass* metaClass, Address target = 0 );

  virtual CloneStream* get_deep_clone_stream() = 0;
  virtual CloneStream* get_shallow_clone_stream() = 0;
  virtual void set_deep_clone_stream(CloneStream *str) = 0;
  virtual void set_shallow_clone_stream(CloneStream *str) = 0;

  virtual ~CloneSubSystem();
};

#endif



