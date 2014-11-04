#ifndef SUIFPASSES__LOCATION_MODULES_H
#define SUIFPASSES__LOCATION_MODULES_H

#include "suifkernel/module.h"
#include "suifkernel/suifkernel_forwarders.h"
#include "suifkernel/error_subsystem.h"

  
  

class LocationModule : public Module {
public:
  LocationModule( SuifEnv* suif_env );

  virtual void execute(); // a nop
  virtual void initialize();

  virtual Module* clone() const;

  static const LString get_class_name();
private:
  static ObjectLocation get_object_location(SuifObject *obj);
};


#endif









