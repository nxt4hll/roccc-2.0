/* file "cfg2ssa/suif_pass.cpp" */

/*
    Copyright (c) 2001 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "cfg2ssa/suif_pass.h"
#endif

#include <machine/pass.h>
#include <machine/machine.h>
#include <cfg/cfg.h>
#include <ssa/ssa.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

#include <cfg2ssa/suif_pass.h>

extern "C" void
init_cfg2ssa(SuifEnv *suif_env)
{
    static bool init_done = false;

    if (init_done)
	return;
    init_done = true;

    ModuleSubSystem *mSubSystem = suif_env->get_module_subsystem();
    mSubSystem->register_module(new Cfg2ssaSuifPass(suif_env));

    // initialize the libraries required by this OPI pass
    init_suifpasses(suif_env);
    init_machine(suif_env);
    init_cfg(suif_env);
    init_ssa(suif_env);
}

Cfg2ssaSuifPass::Cfg2ssaSuifPass(SuifEnv *suif_env, const IdString& name)
    : PipelinablePass(suif_env, name)
{
    the_suif_env = suif_env;	// bind suif_env into our global environment
}

Cfg2ssaSuifPass::~Cfg2ssaSuifPass()
{
    // empty
}

void
Cfg2ssaSuifPass::initialize()
{
    PipelinablePass::initialize();

    // Create grammar for parsing the command line.  This must occur
    // after the parent class's initialize routine has been executed.
    _command_line->set_description("convert from Cfg to SsaCfg form");

    // Collect optional flags.
    OptionSelection *flags = new OptionSelection(false);
    OptionList *l;
    OptionLiteral *f;

    // -debug <level>
    l = new OptionList;
    l->add(new OptionLiteral("-debug"));
    l->add(new OptionInt("level", &debuglvl));
    l->set_description("set verbosity level for debugging messages");
    flags->add(l);

    // -build_minimal_form
    f = new OptionLiteral("-build_minimal_form", &build_minimal_form, true);
    f->set_description("build minimal SSA form");
    flags->add(f);

    // -build_semi_pruned_form
    f = new OptionLiteral("-build_semi_pruned_form",
			  &build_semi_pruned_form, true);
    f->set_description("build semi-pruned SSA form");
    flags->add(f);

    // -build_pruned_form
    f = new OptionLiteral("-build_pruned_form", &build_pruned_form, true);
    f->set_description("build pruned SSA form");
    flags->add(f);

    // -build_def_use_chains
    f = new OptionLiteral("-build_def_use_chains",
			  &build_def_use_chains, true);
    f->set_description("build def-use chains during SSA conversion");
    flags->add(f);

    // -fold_copies
    f = new OptionLiteral("-fold_copies", &fold_copies, true);
    f->set_description("fold copy instructions during SSA conversion");
    flags->add(f);

    // -omit_useless_phi_nodes
    f = new OptionLiteral("-omit_useless_phi_nodes",
			  &omit_useless_phi_nodes, true);
    f->set_description("omit useless phi-nodes during SSA conversion");
    flags->add(f);

    // -print_warnings
    f = new OptionLiteral("-print_warnings", &print_warnings, true);
    f->set_description("enable printing of warnings about SSA form");
    flags->add(f);

    // Accept tagged options in any order.
    _command_line->add(new OptionLoop(flags));

    // zero or more file names
    file_names = new OptionString("file name");
    OptionLoop *files =
	new OptionLoop(file_names,
		       "names of optional input and/or output files");
    _command_line->add(files);
}

bool
Cfg2ssaSuifPass::parse_command_line(TokenStream *command_line_stream)
{
    // Set flag defaults each time command line is parsed.  Otherwise, the
    // wrong values will be seen by a second or subsequent in-memory
    // instance of the pass.

    debuglvl = 0;

    build_minimal_form     = false;
    build_semi_pruned_form = false;
    build_pruned_form      = false;
    build_def_use_chains   = false;
    fold_copies            = false;
    omit_useless_phi_nodes = false;
    print_warnings         = false;

    o_fname = empty_id_string;

    if (!PipelinablePass::parse_command_line(command_line_stream))
	return false;

    using namespace ssa;

    unsigned build_flags = 0;
    if (build_minimal_form)
	build_flags |= BUILD_MINIMAL_FORM;
    if (build_semi_pruned_form)
	build_flags |= BUILD_SEMI_PRUNED_FORM;
    if (build_pruned_form)
	build_flags |= BUILD_PRUNED_FORM;
    if (build_def_use_chains)
	build_flags |= BUILD_DEF_USE_CHAINS;
    if (fold_copies)
	build_flags |= FOLD_COPIES;
    if (omit_useless_phi_nodes)
	build_flags |= OMIT_USELESS_PHI_NODES;
    if (print_warnings)
	build_flags |= PRINT_WARNINGS;
    cfg2ssa.set_build_flags(build_flags);

    o_fname = process_file_names(file_names);

    return true;
}

void
Cfg2ssaSuifPass::execute()
{
    PipelinablePass::execute();

    // Process the output file name, if any.
    if (!o_fname.is_empty())
        the_suif_env->write(o_fname.chars());
}

void
Cfg2ssaSuifPass::do_file_set_block(FileSetBlock *fsb)
{
    claim(o_fname.is_empty() || fsb->get_file_block_count() == 1,
	  "Command-line output file => file set must be a singleton");

    set_opi_predefined_types(fsb);
}

void
Cfg2ssaSuifPass::do_file_block(FileBlock *fb)
{
    claim(has_note(fb, k_target_lib),
	  "expected target_lib annotation on file block");

    focus(fb);

    cfg2ssa.initialize();
}

void
Cfg2ssaSuifPass::do_procedure_definition(ProcedureDefinition *pd)
{
    focus(pd);
    cfg2ssa.do_opt_unit(pd);
    defocus(pd);
}

void
Cfg2ssaSuifPass::finalize()
{ 
    cfg2ssa.finalize();
}
