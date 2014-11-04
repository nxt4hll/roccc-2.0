#ifndef SUIFKERNEL__REAL_OBJECT_FACTORY_H
#define SUIFKERNEL__REAL_OBJECT_FACTORY_H

#include "suifkernel_forwarders.h"

class RealObjectFactory {
public:
  RealObjectFactory();
  virtual void init( SuifEnv* suif_env );
  virtual ~RealObjectFactory();

  virtual void init_io( ObjectFactory* of );
  virtual void init_cloning( CloneSubSystem* css );
  virtual void init_printing( PrintSubSystem* pss );

  virtual void* create_empty_object( const MetaClass* metaClass );

  virtual const LString& getName();

  SuifEnv *get_suif_environment();

protected:
  virtual MetaClass* lookupMetaClass( const LString& metaClassName );

  virtual ObjectFactory* get_object_factory();

  ObjectFactory* _object_factory;

  SuifEnv* _suif_env;
private:
  RealObjectFactory(const RealObjectFactory &);
  RealObjectFactory& operator=(const RealObjectFactory &);
};


#endif














