// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include "common/system_specific.h"
#include "suifkernel/suif_env.h"
#include "suifkernel/module_subsystem.h"

#include "ForLoopPreprocessing.h"
#include "RobyPreprocessingPass.h"
#include "InitializationPass.h"

extern "C" void init_suifnodes(SuifEnv *);
extern "C" void init_cfenodes(SuifEnv *);

extern "C" void init_gcc_preprocessing_transforms( SuifEnv* suif_env ) {
  ModuleSubSystem* module_subsystem = suif_env->get_module_subsystem();

  suif_env->require_module("suifnodes");
  suif_env->require_module("cfenodes");
  suif_env->require_module("utils");

  module_subsystem->register_module(new ForLoopPreprocessingPass(suif_env)) ;
  module_subsystem->register_module(new RobyPreprocessingPass(suif_env)) ;
  module_subsystem->register_module(new InitializationPass(suif_env)) ;
}
