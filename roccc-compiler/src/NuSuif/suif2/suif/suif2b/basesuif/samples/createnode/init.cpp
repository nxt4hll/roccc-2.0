#include "create_suif_complex_input.h"

extern "C" void init_createnode(SuifEnv *suif_env) {
  suif_env->require_module("suifnodes");
  suif_env->require_module("typebuilder");
  suif_env->require_module("suifcloning");

  ModuleSubSystem *ms = suif_env->get_module_subsystem();
  ms->register_module(new CreateSuifComplexInputPass(suif_env)); 
}

