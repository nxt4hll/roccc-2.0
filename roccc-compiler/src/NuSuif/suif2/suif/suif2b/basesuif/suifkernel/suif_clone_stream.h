#ifndef SUIFKERNEL__SUIF_CLONE_STREAM_H
#define SUIFKERNEL__SUIF_CLONE_STREAM_H

// A compiler bug in Motana doesn't allow compilation of this class
#ifndef MONTANABUG


#include "iokernel/clone_stream.h"

#include "suifkernel_forwarders.h"
#include "io_subsystem.h"




// called after cloning has taken place
typedef void (*FinishClone)( SuifCloneStream*,
			     const ObjectWrapper &obj );


struct FinishInfo;

struct FinishInfo {
    bool _being_finished;
    const MetaClass* _meta_class;
    FinishClone _finish_function;
};



class SuifCloneStream : public CloneStream {
public:
  SuifCloneStream( ObjectFactory* of, MetaClassVisitor<ObjectClone>* object_clone_visitor );
  virtual ~SuifCloneStream();
  virtual void clonestream_write( const ObjectWrapper &obj,
                      bool addressable,
                      FinishClone finish_function = 0 );

  virtual void write( const ObjectWrapper &obj,
                      bool addressable = true );


  // the following function allow the querying of
  // already registered MetaClasses.
  // if the input MetaClass is 0 the default_clone_behavior
  // will be returned.
  virtual ObjectClone retrieve_object_clone_behavior( MetaClass* meta_class);

  virtual void finish( Address address );

private:
  MetaClassVisitor<ObjectClone>* _object_clone_visitor;

  typedef suif_map<Address,FinishInfo> FinishMap;
  FinishMap* _finish_map;
private:
  // no implementation for these declarations
  SuifCloneStream(const SuifCloneStream &);
  SuifCloneStream& operator=(const SuifCloneStream &);
};


#endif

#endif


