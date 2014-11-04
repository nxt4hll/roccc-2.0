#include "common/system_specific.h"
#include "suifpasses.h"
#include "suifkernel/module_subsystem.h"
#include "suifkernel/suif_env.h"

void init_suifpasses( SuifEnv* suif_env ) {
  ModuleSubSystem* module_subsystem = suif_env->get_module_subsystem();

  if (!module_subsystem->retrieve_module(Driver::get_class_name())) {
    module_subsystem -> register_module( new Driver( suif_env ) );
    module_subsystem -> register_module( new Pipeliner( suif_env ) );

    module_subsystem -> register_module( new XLoadModule( suif_env ) );
    module_subsystem -> register_module( new SaveModule( suif_env ) );
    module_subsystem -> register_module( new ImportModule( suif_env ) );
    module_subsystem -> register_module( new RequireModule( suif_env ) );
    module_subsystem -> register_module( new PrintModule( suif_env ) );
    module_subsystem -> register_module( new ListModulesModule( suif_env ) );
    module_subsystem -> register_module( new ListInterfacesModule( suif_env ) );
    module_subsystem->set_interface_description
    ("Pass",
     "Module is a subclass of the Pass class."
     );
    module_subsystem->set_interface_description
    ("PipelinablePass",
     "Module is a subclass of the PipelinablePass class."
     );
    module_subsystem->set_interface_description
    ("FrontendPass",
     "Module is a subclass of the FrontendPass class."
     );
  }
}
