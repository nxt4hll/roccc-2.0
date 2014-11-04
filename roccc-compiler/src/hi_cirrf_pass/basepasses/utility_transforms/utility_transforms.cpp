// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include "common/system_specific.h"
#include "suifkernel/suif_env.h"
#include "suifkernel/module_subsystem.h"

#include "flatten_statement_lists_pass.h"
#include "flatten_scope_statements_pass.h"
#include "strip_annotes_pass.h"
#include "mem_dp_boundary_mark_pass.h"
#include "remove_loop_label_loc_stmts_pass.h"
#include "bit_resize_datapath_constants_pass.h"
#include "normalize_statement_lists.h"

extern "C" void init_suifnodes(SuifEnv *);
extern "C" void init_cfenodes(SuifEnv *);

extern "C" void init_utility_transforms( SuifEnv* suif_env ) {
  ModuleSubSystem* module_subsystem = suif_env->get_module_subsystem();

  suif_env->require_module("suifnodes");
  suif_env->require_module("cfenodes");
  suif_env->require_module("utils");

  module_subsystem->register_module( new FlattenStatementListsPass(suif_env) );
  module_subsystem->register_module( new FlattenScopeStatementsPass(suif_env) );
  module_subsystem->register_module( new StripAnnotesPass(suif_env) );
  module_subsystem->register_module( new MEM_DP_BoundaryMarkPass(suif_env) );
  module_subsystem->register_module( new RemoveLoopLabelLocStmtsPass(suif_env) );
  module_subsystem->register_module( new BitResizeDatapathConstantsPass(suif_env) );
  module_subsystem->register_module(new NormalizeStatementListsPass(suif_env));

}
