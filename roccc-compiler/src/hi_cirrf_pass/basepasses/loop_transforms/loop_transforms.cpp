// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include "common/system_specific.h"
#include "suifkernel/suif_env.h"
#include "suifkernel/module_subsystem.h"

#include "unroll_pass.h"
#include "invariant_code_motion_pass.h"
#include "fuse_pass.h"
#include "peel_pass.h"
#include "normalize_pass.h"
#include "forward_substitution_pass.h"
#include "induction_var_substitution_pass.h"
#include "unroll_constant_bounds_pass.h"
#include "unswitch_pass.h"
#include "tile_pass.h"
#include "interchange_pass.h"
#include "strip_mine_pass.h"
#include "skew_pass.h"
#include "unroll_pass2.h"
#include "cleanup_unrolled_calls.h"
#include "handle_call_statements.h"
#include "handle_copy_statements.h"
#include "fullUnroll.h"
#include "pseudoSSA.h"

extern "C" void init_suifnodes(SuifEnv *);
extern "C" void init_cfenodes(SuifEnv *);

extern "C" void init_loop_transforms( SuifEnv* suif_env ) {
  ModuleSubSystem* module_subsystem = suif_env->get_module_subsystem();

  suif_env->require_module("suifnodes");
  suif_env->require_module("cfenodes");
  suif_env->require_module("utils");

  module_subsystem -> register_module( new UnrollPass(suif_env) );
  module_subsystem -> register_module( new InvariantCodeMotionPass(suif_env) );
  module_subsystem -> register_module( new FusePass(suif_env) );
  module_subsystem -> register_module( new PeelPass(suif_env) );
  module_subsystem -> register_module( new NormalizePass(suif_env) );
  module_subsystem -> register_module( new ForwardSubstitutionPass(suif_env) );
  module_subsystem -> register_module( new InductionVariableSubstitutionPass(suif_env) );
  module_subsystem -> register_module( new UnrollConstantBoundsPass(suif_env) );
  module_subsystem -> register_module( new UnswitchPass(suif_env) );
  module_subsystem -> register_module( new TilePass(suif_env) );
  module_subsystem -> register_module( new InterchangePass(suif_env) );
  module_subsystem -> register_module( new StripMinePass(suif_env) );
  module_subsystem -> register_module( new SkewPass(suif_env) );
  module_subsystem->register_module(new UnrollPass2(suif_env)) ;
  module_subsystem->register_module(new CleanupUnrolledCalls(suif_env)) ;
  module_subsystem->register_module(new HandleCallStatements(suif_env)) ;
  module_subsystem->register_module(new HandleCopyStatements(suif_env)) ;
  module_subsystem->register_module(new FullUnrollPass(suif_env)) ;
  module_subsystem->register_module(new PseudoSSA(suif_env)) ;
}
