#ifndef IOKERNEL__SYNCHRONIZER_H
#define IOKERNEL__SYNCHRONIZER_H

#include "iokernel_forwarders.h"
#include "clone_stream.h"

class Synchronizer : public CloneStream {
public:
  Synchronizer(ObjectFactory *factory);
  virtual ~Synchronizer();

  virtual void synchronize( ObjectFactory* system_factory,
                            ObjectFactory* new_object_factory,
                            InputStream* input_stream );

  virtual ObjectFactory* get_object_factory() const;
  MetaClass * get_replacement(Object *) const;
  virtual void object_enquiry(Object *,CloneStreamObjectInstance *,PTR_TYPE ptr_type);
private:
  ObjectFactory* _system_factory;
  ObjectFactory* _new_object_factory;
  InputStream * _input_stream;
private:
  Synchronizer(const Synchronizer&);
  Synchronizer &operator=(const Synchronizer&);
};




#endif
