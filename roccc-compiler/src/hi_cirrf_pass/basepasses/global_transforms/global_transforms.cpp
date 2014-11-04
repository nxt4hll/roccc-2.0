// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include "common/system_specific.h"
#include "suifkernel/suif_env.h"
#include "suifkernel/module_subsystem.h"

#include "constant_propagation_n_folding_pass.h"
#include "constant_propagation_n_folding_pass2.h"
#include "copy_propagation_pass.h"
#include "cse_pass.h"
#include "dead_code_elimination_pass.h"
#include "unrchable_code_elimination_pass.h"
#include "div_by_const_elimination_pass2.h"
#include "reverse_copy_propagation_pass.h"
#include "mult_by_const_elimination_pass.h"
#include "scalar_renaming_pass.h"
#include "code_hoisting_pass.h"
#include "code_sinking_pass.h"
// #include "temporal_cse_pass.h"

#include "scalar_renaming_pass2.h"
#include "copy_propagation_pass2.h"
#include "leftoverRemoval.h"
#include "if_conversion_pass2.h"
#include "temporal_cse_pass2.h"
#include "output_identification_pass.h"
#include "predication_pass.h"
#include "transform_systems_to_modules.h"
#include "remove_unused_variables.h"

#include "cleanup_store_statements.h"
#include "pointer_conversion.h"
#include "evalTransform.h"
#include "redundancy_pass.h"
#include "mark_redundant.h"
#include "redundant_to_redundant.h"
#include "redundant_cleanup.h"

#include "available_elimination.h"
#include "cleanup_boolsels.h"
#include "summation_pass.h"
#include "combine_summation_pass.h"
#include "inlining_pass.h"
#include "reference_cleanup_pass.h"
#include "miniScalarReplacement.h"
#include "inlineAllModules.h"
#include "cleanup_load_pass.h"
#include "miniConstantPropagation.h"
#include "identify_parameters_pass.h"
#include "remove_unsupported_statements.h"

extern "C" void init_suifnodes(SuifEnv *);
extern "C" void init_cfenodes(SuifEnv *);

extern "C" void init_global_transforms( SuifEnv* suif_env ) {
  ModuleSubSystem* module_subsystem = suif_env->get_module_subsystem();

  suif_env->require_module("suifnodes");
  suif_env->require_module("cfenodes");
  suif_env->require_module("utils");

  module_subsystem->register_module(new ConstantPropagationAndFoldingPass(suif_env) );
  module_subsystem->register_module(new ConstantPropagationAndFoldingPass2(suif_env) );
  module_subsystem->register_module(new CopyPropagationPass(suif_env) );
  module_subsystem->register_module(new CommonSubExpressionEliminationPass(suif_env) );
  module_subsystem->register_module(new DeadCodeEliminationPass(suif_env) );
  module_subsystem->register_module(new UnreachableCodeEliminationPass(suif_env) );
  module_subsystem->register_module(new DivByConstEliminationPass2(suif_env) );
  module_subsystem->register_module(new ReverseCopyPropagationPass(suif_env) );
  module_subsystem->register_module(new MultiplyByConstEliminationPass2(suif_env) );
  module_subsystem->register_module(new ScalarRenamingPass(suif_env) );
  module_subsystem->register_module(new CodeHoistingPass(suif_env) );
  module_subsystem->register_module(new CodeSinkingPass(suif_env) );
  module_subsystem->register_module(new TemporalCSEPass(suif_env) );

  module_subsystem->register_module(new ScalarRenamingPass2(suif_env) ) ;

  module_subsystem->register_module(new CopyPropagationPass2(suif_env) ) ;

  module_subsystem->register_module(new LeftoverRemovalPass(suif_env) ) ;
  
  module_subsystem->register_module(new IfConversionPass2(suif_env)) ;

  module_subsystem->register_module(new OutputIdentificationPass(suif_env)) ;

  module_subsystem->register_module(new PredicationPass(suif_env)) ;
  
  module_subsystem->register_module(new TransformSystemsToModules(suif_env)) ;

  module_subsystem->register_module(new RemoveUnusedVariables(suif_env)) ;

  module_subsystem->register_module(new CleanupStoreStatementsPass(suif_env));
  
  module_subsystem->register_module(new PointerConversionPass(suif_env)) ;

  module_subsystem->register_module(new EvalTransformPass(suif_env)) ;
  
  module_subsystem->register_module(new RedundancyPass(suif_env)) ;

  module_subsystem->register_module(new MarkRedundantPass(suif_env)) ;

  module_subsystem->register_module(new RedundantToRedundantPass(suif_env)) ;

  module_subsystem->register_module(new CleanupRedundantVotes(suif_env)) ;

  module_subsystem->register_module(new AvailableCodeEliminationPass(suif_env));

  module_subsystem->register_module(new CleanupBoolSelsPass(suif_env)) ;
  module_subsystem->register_module(new SummationPass(suif_env)) ;
  module_subsystem->register_module(new InliningPass(suif_env)) ;
  module_subsystem->register_module(new ReferenceCleanupPass(suif_env)) ;
  module_subsystem->register_module(new MiniScalarReplacementPass(suif_env)) ;
  module_subsystem->register_module(new InlineAllModulesPass(suif_env)) ;
  module_subsystem->register_module(new CombineSummationPass(suif_env)) ;
  module_subsystem->register_module(new CleanupLoadPass(suif_env)) ;
  module_subsystem->register_module(new MiniConstantPropagationPass(suif_env));
  module_subsystem->register_module(new IdentifyParametersPass(suif_env)) ;
  module_subsystem->register_module(new RemoveUnsupportedStatements(suif_env));
}
