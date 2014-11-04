#ifndef SUIFKERNEL__DLL_SUBSYSTEM_H
#define SUIFKERNEL__DLL_SUBSYSTEM_H

#include "subsystem.h"
#include "suifkernel_forwarders.h"

class DLLSubSystem : public SubSystem {
public:
  DLLSubSystem( SuifEnv* suif_env );
  ~DLLSubSystem();

  virtual void load_and_initialize_DLL( const LString& libraryName );
  virtual void require_DLL( const LString& libraryName );
  virtual bool is_DLL_loaded( const LString& libraryName );
private:
  list<LString> *_loaded;
};

#endif
