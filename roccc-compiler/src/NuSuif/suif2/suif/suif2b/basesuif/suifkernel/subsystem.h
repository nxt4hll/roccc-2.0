#ifndef SUIFKERNEL__SUBSYSTEM_H
#define SUIFKERNEL__SUBSYSTEM_H

#include "suifkernel_forwarders.h"

/** This is the base class for all of the SuifEnv subsystems.
 *
 *  The only use it seems to have is to encapsulate the SuifEnv
 *  that built it
 */
class SubSystem /* : Module */ {
public:
  SubSystem( SuifEnv* s );
  virtual ~SubSystem();
  virtual SuifEnv* get_suif_env() const;
protected:
  SuifEnv* _suif_env;
private:
  SubSystem(const SubSystem &);
  SubSystem& operator=(const SubSystem &);
};

#endif
