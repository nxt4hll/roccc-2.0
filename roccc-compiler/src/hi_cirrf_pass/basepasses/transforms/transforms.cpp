// The ROCCC Compiler Infrastructure
//  This file is distributed under the University of California Open Source
//  License.  See ROCCCLICENSE.TXT for details.
#include "common/system_specific.h"
/* file "transforms.cpp" */


/*
       Copyright (c) 1998 Stanford University

       All rights reserved.

       This software is provided under the terms described in
       the "suif_copyright.h" include file.
*/

#include <common/suif_copyright.h>


/*
      This is the main implementation file of transforms, a library of core
      passes for converting between different dialects of SUIF.
*/


#include "statement_dismantlers.h"
#include "expression_dismantlers.h"
#include "array_dismantlers.h"
#include "symbol_walkers.h"
#include "region_passes.h"
#include "padding.h"
#include "function_dismantlers.h"
#include "symbol_transforms.h"
#include "typebuilder/type_builder.h"
#include "suifkernel/command_line_parsing.h"

/* DEW */
//#include "unowned.h"

/*
#include "make_empty_file_set_symbol_table.h"
*/

//	Many passes can be derived from this template simply by instantiating with a
//	suitable walker
template <class walker> class walker_based_module : public PipelinablePass {
  String _description;
public:
  walker_based_module(SuifEnv *env, const LString &name,
		      const String &description ) 
    : PipelinablePass(env, name), _description(description) {}

        void initialize() 
            {
	    PipelinablePass::initialize();
	    _command_line->set_description(_description);
	    }
    
  	virtual ~walker_based_module(void)  { }

  	virtual void do_procedure_definition(ProcedureDefinition *proc_def)
	    {
	    if ( !proc_def )
        	return;

    	    walker walk(get_suif_env(), proc_def);
    	    proc_def->walk(walk);
    	    }


  	Module *clone() const { return(Module*)this;};
};

extern "C" void init_cfenodes(SuifEnv *suif_env);

extern "C" void init_transforms(SuifEnv *suif_env) {
  ModuleSubSystem *ms = suif_env->get_module_subsystem();
  suif_env->require_module("typebuilder");
  suif_env->require_module("suifnodes");
  suif_env->require_module("cfenodes");
  suif_env->require_module("suifcloning");
  suif_env->require_module("cfeutils");
  suif_env->require_module("suifpasses");
  suif_env->require_module("suiflocation");
  suif_env->require_module("utils");

  //  init_typebuilder(suif_env);
  /*
    static boolean init_done = FALSE;

    if (init_done)
    return;
    init_done = TRUE;
    enter_suif(argc, argv);
    enter_suifpasses(argc, argv);
  */

  /*
  ms->register_module(new make_empty_file_set_symbol_table_pass());
  */
  ms->register_module(new NameAllSymbolsPass(suif_env));
  ms->register_module(new AvoidExternCollisions(suif_env));
  ms->register_module(new AvoidFileScopeCollisions(suif_env));
  ms->register_module(new PaddingPass(suif_env));
  ms->register_module(new StructPaddingPass(suif_env));
  ms->register_module(new CombinedPass(suif_env));
  ms->register_module(new CallExpressionDismantlerPass(suif_env));
  ms->register_module(new FieldBuilderPass(suif_env));
  ms->register_module(new MultiDimArrayDismantlerPass(suif_env));
  ms->register_module(new NonConstBoundDismantlerPass(suif_env));
  ms->register_module(new SetAddrTakenPass(suif_env));
  ms->register_module(new DismantleEmptyScopeStatements(suif_env));
  ms->register_module(new DismantleCallArguments(suif_env));
  ms->register_module(new DismantleMultiEntryScopeStatements(suif_env));

/* DEW */
  // This is in usefulpasses
  //ms->register_module(new UnownedPass(suif_env));

  ms->register_module(
	new walker_based_module<if_statement_walker>(
	        suif_env, "dismantle_if_statements",
		"Dismantle IfStatements to Branches and Jumps"));
  ms->register_module(
	new walker_based_module<while_statement_walker>(
		suif_env, "dismantle_while_statements",
		"Dismantle all WhileStatements to Branches and Jumps"));
  ms->register_module(
	new walker_based_module<do_while_statement_walker>(
	        suif_env, "dismantle_do_while_statements",
		"Dismantle all DoWhileStatements to Branches and Jumps"));
  ms->register_module(
	new walker_based_module<for_statement_walker>(
		suif_env, "dismantle_for_statements",
		"Dismantle all ForStatements to Branches and Jumps"));
  ms->register_module(
        new walker_based_module<c_for_statement_walker>(
                suif_env, "dismantle_c_for_statements",
		"Dismantle all CForStatements"));

  ms->register_module(
	new walker_based_module<scope_statement_walker>(
		suif_env, "dismantle_scope_statements",
		"Dismantle all ScopeStatements and move their symbols"));

  ms->register_module(
	new walker_based_module<multi_way_branch_statement_walker>(
                suif_env, "dismantle_multi_way_branch_statements",
		"Dismantle all MultiWayBranchStatements to Branches and Jumps"));
  ms->register_module( new ArrayReferenceDismantlerPass(suif_env));

  //	A number of the expression dismantlers were not implemented in SUIFX, so they are grouped at
  //    the end of the list here
  ms->register_module(
	new walker_based_module<field_access_expression_walker>(
		suif_env, "dismantle_field_access_expressions",
		"Dismantle all FieldAccessExpressions to pointer arithmetic"));
  /*
  ms->register_module(
	new walker_based_module<call_expression_walker>(
		suif_env, "dismantle_call_expressions"));
  */

  //   A multi-way branch re-organiser
  ms->register_module(
        new walker_based_module<multi_way_branch_statement_compactor>(
                suif_env, "compact_multi_way_branch_statements",
			"convert multi-way branches to IfStatements with dense multi-way branches"));

  ms->register_module(new AvoidLabelCollisions(suif_env));
  ms->register_module(new RepeatValueBlockBuilderPass(suif_env));

//  ms->register_module(new walker_based_module<extract_fields_expression_walker>(suif_env, "dismantle_extract_fields_expressions"));
  // ms->register_module(new walker_based_module<extract_elements_expression_walker>(suif_env, "dismantle_extract_elements_expressions"));
  // ms->register_module(new walker_based_module<byte_size_of_expression_walker>(suif_env, "dismantle_byte_size_of_expressions"));

#if 0
//	These are not implemented in SUIFX or the new Suif (yet)
  ms->register_module(new walker_based_module<select_expression_walker>(suif_env, "dismantle_sc_select_expressions"));
  ms->register_module(new walker_based_module<array_reference_expression_walker>(suif_env, "dismantle_array_reference_expressions"));
  ms->register_module(new walker_based_module<set_fields_expression_walker>(suif_env, "dismantle_set_fields_expressions"));
  ms->register_module(new walker_based_module<set_elements_expression_walker>(suif_env, "dismantle_set_elements_expressions"));
  ms->register_module(new walker_based_module<bit_size_of_expression_walker>(suif_env, "dismantle_bit_size_of_expressions"));
  ms->register_module(new walker_based_module<bit_alignment_of_expression_walker>(suif_env, "dismantle_bit_alignment_of_expressions"));
  ms->register_module(new walker_based_module<byte_alignment_of_expression_walker>(suif_env, "dismantle_byte_alignment_of_expressions"));
  ms->register_module(new walker_based_module<bit_offset_of_expression_walker>(suif_env, "dismantle_bit_offset_of_expressions"));
  ms->register_module(new walker_based_module<byte_offset_of_expression_walker>(suif_env, "dismantle_byte_offset_of_expressions"));
  ms->register_module(new walker_based_module<sc_and_expression_walker>(suif_env, "dismantle_sc_and_expressions"));
  ms->register_module(new walker_based_module<sc_or_expression_walker>(suif_env, "dismantle_sc_or_expressions"));
  ms->register_module(new walker_based_module<sc_select_expression_walker>(suif_env, "dismantle_sc_select_expressions"));
  ms->register_module(new walker_based_module<load_value_block_expression_walker>(suif_env, "dismantle_load_value_block_expressions"));
  ms->register_module(new walker_based_module<multi_way_branch_expression_walker>(suif_env, "dismantle_multi_way_branch_expressions"));
#endif

  ms->register_module(new RemoveIfAndLoopPass(suif_env));
  ms->register_module(new FlattenStatementListsPass(suif_env));
  ms->register_module(new RemoveIfAndLoopPass(suif_env));
  ms->register_module(new AddProcedureEndLabelsPass(suif_env));
  ms->register_module(new One2MultiArrayExpressionPass(suif_env));
  //ms->register_module(new EliminateArrayConvertsPass(suif_env));
  ms->register_module(new CFor2ForPass(suif_env));
  ms->register_module(new DismantleStructuredReturns(suif_env));
  ms->register_module(new NormalizeProcedureReturns(suif_env));
  ms->register_module(new RequireProcedureReturns(suif_env));
  ms->register_module(new LoadExpressionDismantlerPass(suif_env));
  ms->register_module(new FoldStatementsPass(suif_env));
  ms->register_module(new DismantleStmtsWithJumpsInside(suif_env));
  ms->register_module(new AddStatementListsToProcs(suif_env));
  ms->register_module(new AddExplicitLoopLabels(suif_env));
  ms->register_module(new FixupExplicitLoopLabels(suif_env));
  ms->register_module(new RemoveExplicitLoopLabels(suif_env));
  ms->register_module(new IfConditionsToBinaryExprs(suif_env));

  ms->register_module(new GuardAllFors(suif_env));
  ms->register_module(new MarkGuardedFors(suif_env));
}

  //extern "C" void exit_transforms(void)
  //  {
    /* empty */
  //  }
