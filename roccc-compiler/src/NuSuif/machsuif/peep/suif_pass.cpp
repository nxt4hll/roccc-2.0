/* file "peep/suif_pass.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "peep/suif_pass.h"
#endif

#include <machine/pass.h>
#include <machine/machine.h>
#include <cfg/cfg.h>
#include <bvd/bvd.h>

#include <peep/suif_pass.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

extern "C" void
init_peep(SuifEnv *suif_env)
{
    static bool init_done = false;

    if (init_done)
	return;
    init_done = true;

    ModuleSubSystem *mSubSystem = suif_env->get_module_subsystem();
    mSubSystem->register_module(new PeepSuifPass(suif_env));

    init_suifpasses(suif_env);
    init_machine(suif_env);
    init_cfg(suif_env);
    init_bvd(suif_env);
}

PeepSuifPass::PeepSuifPass(SuifEnv *suif_env, const IdString& name)
    : PipelinablePass(suif_env, name)
{
    the_suif_env = suif_env;	// bind suif_env into our global environment
}

void
PeepSuifPass::initialize()
{
    PipelinablePass::initialize();

    OptionList *l;
    OptionLiteral *f;

    // Create grammar for parsing the command line.  This must occur
    // after the parent class's initialize routine has been executed.
    _command_line->set_description("eliminate unneeded move instructions");

    // Collect optional flags.
    OptionSelection *flags = new OptionSelection(false);

    // -debug level
    l = new OptionList;
    l->add(new OptionLiteral("-debug"));
    l->add(new OptionInt("level", &debug_arg));
    l->set_description("set verbosity level for debugging messages");
    flags->add(l);

    // -const_prop
    f = new OptionLiteral("-const_prop", &const_prop, true);
    f->set_description("push constant operands into neighboring instructions");
    flags->add(f);

    // Accept tagged options in any order.
    _command_line->add(new OptionLoop(flags));

    // zero or more file names
    file_names = new OptionString("file name");
    OptionLoop *files =
	new OptionLoop(file_names,
		       "names of optional input and/or output files");
    _command_line->add(files);
    o_fname = empty_id_string;
}

bool
PeepSuifPass::parse_command_line(TokenStream *command_line_stream)
{
    // Set flag defaults each time command line is parsed.
    // Otherwise, the wrong values will be seen by a second or
    // subsequent in-memory instance of the pass.

    debug_arg = 0;
    const_prop = false;

    if (!PipelinablePass::parse_command_line(command_line_stream))
	return false;

    peep.set_debug_arg(debug_arg);
    peep.set_const_prop(const_prop);

    o_fname = process_file_names(file_names);

    return true;
}

void
PeepSuifPass::execute()
{
    PipelinablePass::execute();

    // Process the output file name, if any.
    if (!o_fname.is_empty())
	the_suif_env->write(o_fname.chars());
}

void
PeepSuifPass::do_file_set_block(FileSetBlock *fsb)
{
    claim(o_fname.is_empty() || fsb->get_file_block_count() == 1,
	  "Command-line output file => file set must be a singleton");

    set_opi_predefined_types(fsb);
}


void
PeepSuifPass::do_file_block(FileBlock *fb)
{
    claim(has_note(fb, k_target_lib),
	  "expected target_lib annotation on file block");

    focus(fb);

    peep.initialize();
}


void
PeepSuifPass::do_procedure_definition(ProcedureDefinition *pd)
{
    focus(pd);
    peep.do_opt_unit(pd);
    defocus(pd);
}


void
PeepSuifPass::finalize()
{ 
    peep.finalize();
}
