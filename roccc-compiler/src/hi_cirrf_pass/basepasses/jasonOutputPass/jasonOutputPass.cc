// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include "common/system_specific.h"
#include "suifkernel/suif_env.h"
#include "suifkernel/module_subsystem.h"

#include "outputPass.h"
#include "removeNonPrintable.h"
#include "intrinsicPass.h"
#include "preferencePass.h"
#include "castPass.h"
#include "identifyDebug.h"

extern "C" void init_suifnodes(SuifEnv *);
extern "C" void init_cfenodes(SuifEnv *);

extern "C" void init_jasonOutputPass( SuifEnv* suif_env ) 
{
  ModuleSubSystem* module_subsystem = suif_env->get_module_subsystem();

  suif_env->require_module("suifnodes");
  suif_env->require_module("cfenodes");
  suif_env->require_module("utils");

  module_subsystem->register_module( new OutputPass(suif_env) );
  module_subsystem->register_module( new RemoveNonPrintablePass(suif_env) ) ;
  module_subsystem->register_module( new IntrinsicPass(suif_env)) ;
  module_subsystem->register_module( new PreferencePass(suif_env)) ;
  module_subsystem->register_module( new CastPass(suif_env)) ;
  module_subsystem->register_module( new IdentifyDebugPass(suif_env)) ;
}
