/* file "ssa2cfg/suif_pass.cpp" */

/*
    Copyright (c) 2001 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "ssa2cfg/suif_pass.h"
#endif

#include <machine/pass.h>
#include <machine/machine.h>
#include <cfg/cfg.h>
#include <ssa/ssa.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

#include <ssa2cfg/suif_pass.h>

extern "C" void
init_ssa2cfg(SuifEnv *suif_env)
{
    static bool init_done = false;

    if (init_done)
	return;
    init_done = true;

    ModuleSubSystem *mSubSystem = suif_env->get_module_subsystem();
    mSubSystem->register_module(new Ssa2cfgSuifPass(suif_env));

    // initialize the libraries required by this OPI pass
    init_suifpasses(suif_env);
    init_machine(suif_env);
    init_cfg(suif_env);
    init_ssa(suif_env);
}

Ssa2cfgSuifPass::Ssa2cfgSuifPass(SuifEnv *suif_env, const IdString& name)
    : PipelinablePass(suif_env, name)
{
    the_suif_env = suif_env;	// bind suif_env into our global environment
}

Ssa2cfgSuifPass::~Ssa2cfgSuifPass()
{
    // empty
}

void
Ssa2cfgSuifPass::initialize()
{
    PipelinablePass::initialize();

    // Create grammar for parsing the command line.  This must occur
    // after the parent class's initialize routine has been executed.
    _command_line->set_description("convert from SsaCfg to Cfg form");

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

    // -restore_orig_names
    f = new OptionLiteral("-restore_orig_names", &restore_orig_names, true);
    f->set_description("restore original (non-SSA) location names");
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
Ssa2cfgSuifPass::parse_command_line(TokenStream *command_line_stream)
{
    // Set flag defaults each time command line is parsed.  Otherwise, the
    // wrong values will be seen by a second or subsequent in-memory
    // instance of the pass.

    debuglvl = 0;
    o_fname = empty_id_string;

    restore_orig_names = false;

    if (!PipelinablePass::parse_command_line(command_line_stream))
	return false;

    ssa2cfg.set_restore_orig_names(restore_orig_names);

    o_fname = process_file_names(file_names);

    return true;
}

void
Ssa2cfgSuifPass::execute()
{
    PipelinablePass::execute();

    // Process the output file name, if any.
    if (!o_fname.is_empty())
        the_suif_env->write(o_fname.chars());
}

void
Ssa2cfgSuifPass::do_file_set_block(FileSetBlock *fsb)
{
    claim(o_fname.is_empty() || fsb->get_file_block_count() == 1,
	  "Command-line output file => file set must be a singleton");

    set_opi_predefined_types(fsb);
}

void
Ssa2cfgSuifPass::do_file_block(FileBlock *fb)
{
    claim(has_note(fb, k_target_lib),
	  "expected target_lib annotation on file block");

    focus(fb);

    ssa2cfg.initialize();
}

void
Ssa2cfgSuifPass::do_procedure_definition(ProcedureDefinition *pd)
{
    focus(pd);
    ssa2cfg.do_opt_unit(pd);
    defocus(pd);
}

void
Ssa2cfgSuifPass::finalize()
{ 
    ssa2cfg.finalize();
}
