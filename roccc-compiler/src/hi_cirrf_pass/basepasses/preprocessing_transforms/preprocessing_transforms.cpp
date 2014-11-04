// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include "common/system_specific.h"
#include "suifkernel/suif_env.h"
#include "suifkernel/module_subsystem.h"

#include "dowhile_to_while_transform_pass.h"
#include "hw_sw_boundary_mark_pass.h"
#include "switch_case_to_if_stmts_pass.h"
#include "dfa_state_table_expansion_pass.h"

extern "C" void init_suifnodes(SuifEnv *);
extern "C" void init_cfenodes(SuifEnv *);

extern "C" void init_preprocessing_transforms( SuifEnv* suif_env ) {
  ModuleSubSystem* module_subsystem = suif_env->get_module_subsystem();

  suif_env->require_module("suifnodes");
  suif_env->require_module("cfenodes");
  suif_env->require_module("utils");

  module_subsystem->register_module( new DoWhileToWhileTransformPass(suif_env) );
  module_subsystem->register_module( new HW_SW_BoundaryMarkPass(suif_env) );
  module_subsystem->register_module( new SwitchCaseToIfStmtsPass(suif_env) );
  module_subsystem->register_module( new DFA_StateTableExpansionPass(suif_env) );
}
