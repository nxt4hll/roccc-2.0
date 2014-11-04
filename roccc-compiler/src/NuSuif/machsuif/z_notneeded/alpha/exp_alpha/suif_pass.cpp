/* file "exp_alpha/suif_pass.cpp" */

/*
    Copyright (c) 2001 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "exp_alpha/suif_pass.h"
#endif

#include <machine/pass.h>
#include <machine/machine.h>
#include <cfg/cfg.h>

#include <exp_alpha/suif_pass.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

extern "C" void
init_exp_alpha(SuifEnv *suif_env)
{
    static bool init_done = false;

    if (init_done)
	return;
    init_done = true;

    init_suifpasses(suif_env);
    init_machine(suif_env);
    init_cfg(suif_env);

    ModuleSubSystem *mSubSystem = suif_env->get_module_subsystem();
    mSubSystem->register_module(new ExpAlphaSuifPass(suif_env));
}

ExpAlphaSuifPass::ExpAlphaSuifPass(SuifEnv *suif_env, const IdString& name)
    : PipelinablePass(suif_env, name)
{
    the_suif_env = suif_env;	// bind suif_env into our global environment
}

void
ExpAlphaSuifPass::initialize()
{
    PipelinablePass::initialize();

    OptionList *l;
    OptionLiteral *f;

    // Create parse tree for parsing the command line.  This must occur
    // after the parent class's initialize routine has been executed.
    _command_line->set_description("expand synthesized Alpha instructions");

    // Collect optional flags.
    OptionSelection *flags = new OptionSelection(false);

    // -debug level
    l = new OptionList;
    l->add(new OptionLiteral("-debug"));
    l->add(new OptionInt("level", &debug_arg));
    l->set_description("set verbosity level for debugging messages");
    flags->add(l);

    // -no_reloc
    f = new OptionLiteral("-no_reloc", &no_reloc, true);
    f->set_description("do NOT expand using relocations");
    flags->add(f);

    // -no_lituse
    f = new OptionLiteral("-no_lituse", &no_lituse, true);
    f->set_description("inhibit the generation of lituse");
    flags->add(f);

    // -use_virtual_regs
    f = new OptionLiteral("-use_virtual_regs", &use_virtual_regs, true);
    f->set_description("assume register allocation follows this pass");
    flags->add(f);

    // -spill_volatiles
    f = new OptionLiteral("-spill_volatiles", &spill_volatiles, true);
    f->set_description("spill non-local or addressed variables");
    flags->add(f);

    // -exp_sublong_ld_st
    f = new OptionLiteral("-exp_sublong_ld_st", &exp_sublong_ld_st, true);
    f->set_description("expand LDB/LDW/STB/STW instructions");
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
ExpAlphaSuifPass::parse_command_line(TokenStream *command_line_stream)
{
    // Set flag defaults.
    debug_arg = 0;
    no_reloc = false;
    no_lituse = false;
    use_virtual_regs = false;
    spill_volatiles = false;
    exp_sublong_ld_st = false;
    o_fname = empty_id_string;

    if (!PipelinablePass::parse_command_line(command_line_stream))
	return false;

    exp_alpha.set_debug_arg(debug_arg);
    exp_alpha.set_no_reloc(no_reloc);
    exp_alpha.set_no_lituse(no_lituse);
    exp_alpha.set_use_virtual_regs(use_virtual_regs);
    exp_alpha.set_spill_volatiles(spill_volatiles);
    exp_alpha.set_exp_sublong_ld_st(exp_sublong_ld_st);

    o_fname = process_file_names(file_names);

    return true;
}

void
ExpAlphaSuifPass::execute()
{
    PipelinablePass::execute();

    // Process the output file name, if any.
    if (!o_fname.is_empty())
	the_suif_env->write(o_fname.chars());
}

void
ExpAlphaSuifPass::do_file_set_block(FileSetBlock *fsb)
{
    claim(o_fname.is_empty() || fsb->get_file_block_count() == 1,
	  "Command-line output file => file set must be a singleton");

    set_opi_predefined_types(fsb);
}

void
ExpAlphaSuifPass::do_file_block(FileBlock* fb)
{
    debug(2, "Processing file %s", get_name(fb).chars());

    claim(has_note(fb, k_target_lib),
	  "expected target_lib annotation on file block");

    // Focus the OPI.  (Should be done in all Machine-SUIF passes.)
    focus(fb);

    exp_alpha.initialize();
}

void
ExpAlphaSuifPass::do_procedure_definition(ProcDef* pd)
{
    focus(pd);
    exp_alpha.do_opt_unit(pd);
    defocus(pd);
}

void
ExpAlphaSuifPass::finalize()
{ 
    exp_alpha.finalize();
}
