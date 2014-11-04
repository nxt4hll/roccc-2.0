/* file "il2cfg/suif_pass.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "il2cfg/suif_pass.h"
#endif

#include <machine/pass.h>
#include <machine/machine.h>
#include <cfg/cfg.h>

#include <il2cfg/suif_pass.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

extern "C" void
init_il2cfg(SuifEnv *suif_env)
{
    static bool init_done = false;

    if (init_done)
	return;
    init_done = true;

    ModuleSubSystem *mSubSystem = suif_env->get_module_subsystem();
    mSubSystem->register_module(new Il2cfgSuifPass(suif_env));

    // initialize the libraries required by this OPI pass
    init_suifpasses(suif_env);
    init_machine(suif_env);
    init_cfg(suif_env);
}

Il2cfgSuifPass::Il2cfgSuifPass(SuifEnv *suif_env, const IdString& name)
    : PipelinablePass(suif_env, name)
{
    the_suif_env = suif_env;	// bind suif_env into our global environment
}

Il2cfgSuifPass::~Il2cfgSuifPass()
{
    // empty
}

void
Il2cfgSuifPass::initialize()
{
    PipelinablePass::initialize();

    OptionLiteral *f;

    // Create grammar for parsing the command line.  This must occur
    // after the parent class's initialize routine has been executed.
    _command_line->set_description("convert from InstrList to Cfg form");

    // Collect optional flags.
    OptionSelection *flags = new OptionSelection(false);

    // -keep_layout
    f = new OptionLiteral("-keep_layout", &keep_layout, true);
    f->set_description("maintain the program layout");
    flags->add(f);

    // -break_at_call
    f = new OptionLiteral("-break_at_call", &break_at_call, true);
    f->set_description("treat call as a block-terminating CTI");
    flags->add(f);

    // -break_at_instr
    f = new OptionLiteral("-break_at_instr", &break_at_instr, true);
    f->set_description("give each instruction a CFG node");
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
Il2cfgSuifPass::parse_command_line(TokenStream *command_line_stream)
{
    // Set flag defaults.
    keep_layout = false;
    break_at_call = false;
    break_at_instr = false;
    o_fname = empty_id_string;

    if (!PipelinablePass::parse_command_line(command_line_stream))
	return false;

    il2cfg.set_keep_layout(keep_layout);
    il2cfg.set_break_at_call(break_at_call);
    il2cfg.set_break_at_instr(break_at_instr);

    o_fname = process_file_names(file_names);

    return true;
}

void
Il2cfgSuifPass::execute()
{
    PipelinablePass::execute();

    // Process the output file name, if any.
    if (!o_fname.is_empty())
        the_suif_env->write(o_fname.chars());
}

void
Il2cfgSuifPass::do_file_set_block(FileSetBlock *fsb)
{
    claim(o_fname.is_empty() || fsb->get_file_block_count() == 1,
	  "Command-line output file => file set must be a singleton");

    set_opi_predefined_types(fsb);
}

void
Il2cfgSuifPass::do_file_block(FileBlock *fb)
{
    claim(has_note(fb, k_target_lib),
	  "expected target_lib annotation on file block");

    focus(fb);

    il2cfg.initialize();
}

void
Il2cfgSuifPass::do_procedure_definition(ProcedureDefinition *pd)
{
    focus(pd);
    il2cfg.do_opt_unit(pd);
    defocus(pd);
}

void
Il2cfgSuifPass::finalize()
{ 
    il2cfg.finalize();
}
