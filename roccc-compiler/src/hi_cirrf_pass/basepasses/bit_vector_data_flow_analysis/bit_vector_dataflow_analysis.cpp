// The ROCCC Compiler Infrastructure 
//  This file is distributed under the University of California Open Source 
//  License.  See ROCCCLICENSE.TXT for details.

#include "common/system_specific.h"
#include "suifkernel/suif_env.h"
#include "suifkernel/module_subsystem.h"

#include "solve_available_expressions_pass.h"
#include "availability_chain_builder_pass.h"

#include "ud_du_chain_builder_pass2.h"
#include "solve_pass2.h"
#include "solve_feedback_variables_pass_2.0.h"
#include "solve_feedback_variables_pass_2.0_new.h"
#include "solve_feedback_calls.h"

extern "C" void init_suifnodes(SuifEnv *);
extern "C" void init_cfenodes(SuifEnv *);

extern "C" void init_bit_vector_dataflow_analysis( SuifEnv* suif_env ) {
  ModuleSubSystem* module_subsystem = suif_env->get_module_subsystem();

  suif_env->require_module("suifnodes");
  suif_env->require_module("cfenodes");
  suif_env->require_module("utils");

  module_subsystem->register_module( new SolveAvailableExpressionsPass(suif_env) );
  module_subsystem->register_module( new AvailabilityChainBuilderPass(suif_env) );

  module_subsystem->register_module( new DataFlowSolvePass2(suif_env)) ;
  module_subsystem->register_module( new UD_DU_ChainBuilderPass2(suif_env) );

  module_subsystem->register_module(new SolveFeedbackVariablesPass2(suif_env));
  
  module_subsystem->register_module(new SolveFeedbackCalls(suif_env)) ;
  module_subsystem->register_module(new SolveFeedbackVariablesPass3(suif_env));


}
