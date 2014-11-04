#include "common/system_specific.h"
#include "suifkernel/module_subsystem.h"
#include "suifkernel/suif_env.h"
#include "location_modules.h"

extern "C" void init_suiflocation( SuifEnv* suif_env );

void init_suiflocation( SuifEnv* suif_env ) {
//  suif_env->require_module("basicnodes");

  ModuleSubSystem* module_subsystem = suif_env->get_module_subsystem();
  if (!module_subsystem->retrieve_module(LocationModule::get_class_name())) {
    module_subsystem -> register_module( new LocationModule( suif_env ) );
    }

  // Set the default location finder
  suif_env->get_error_subsystem()->set_default_location_module(LocationModule::get_class_name());
}
