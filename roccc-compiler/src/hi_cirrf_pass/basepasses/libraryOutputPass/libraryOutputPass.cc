// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include "common/system_specific.h"
#include "suifkernel/suif_env.h"
#include "suifkernel/module_subsystem.h"

#include "libOutputPass.h"
#include "dumpHeaderPass.h"
#include "removeModulePass.h"
#include "addModulePass.h"
#include "exportPass.h"
#include "cleanRepository.h"

extern "C" void init_suifnodes(SuifEnv *);
extern "C" void init_cfenodes(SuifEnv *);

extern "C" void init_libraryOutputPass( SuifEnv* suif_env ) 
{
  ModuleSubSystem* module_subsystem = suif_env->get_module_subsystem();

  suif_env->require_module("suifnodes");
  suif_env->require_module("cfenodes");
  suif_env->require_module("utils");

  module_subsystem->register_module(new LibraryOutputPass(suif_env) );
  module_subsystem->register_module(new RemoveModulePass(suif_env)) ;
  module_subsystem->register_module(new DumpHeaderPass(suif_env)) ;
  module_subsystem->register_module(new AddModulePass(suif_env)) ;
  module_subsystem->register_module(new ExportPass(suif_env)) ;
  module_subsystem->register_module(new CleanRepositoryPass(suif_env)) ;
}
