/* file "instrument/suif_pass.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "instrument/suif_pass.h"
#endif

#include <machine/pass.h>
#include <machine/machine.h>
#include <halt/halt.h>

#include <instrument/suif_pass.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

extern "C" void
init_instrument(SuifEnv *suif_env)
{
    static bool init_done = false;

    if (init_done)
	return;
    init_done = true;

    ModuleSubSystem *mSubSystem = suif_env->get_module_subsystem();
    mSubSystem->register_module(new InstrumentSuifPass(suif_env));

    init_suifpasses(suif_env);
    init_machine(suif_env);
    init_cfg(suif_env);
    init_bvd(suif_env);
    init_halt(suif_env);
}

InstrumentSuifPass::InstrumentSuifPass(SuifEnv *suif_env, const IdString& name)
    : PipelinablePass(suif_env, name)
{
    the_suif_env = suif_env;	// bind suif_env into our global environment
}

InstrumentSuifPass::~InstrumentSuifPass()
{
    // empty
}

void
InstrumentSuifPass::initialize()
{
    PipelinablePass::initialize();

    OptionList *l;

    // Create grammar for parsing the command line.  This must occur
    // after the parent class's initialize routine has been executed.
    _command_line->set_description("Adds instrumentation code");

    // Collect optional flags.
    OptionSelection *flags = new OptionSelection(false);

    // -debug level
    l = new OptionList;
    l->add(new OptionLiteral("-debug"));
    l->add(new OptionInt("level", &debug_arg));
    l->set_description("set verbosity level for debugging messages");
    flags->add(l);

    // -target_lib library
    l = new OptionList;
    l->add(new OptionLiteral("-target_lib"));
    l->add(new OptionString("target library", &target_lib));
    l->set_description("target library with instrumentation code");
    flags->add(l);

    // -main name
    l = new OptionList;
    l->add(new OptionLiteral("-main"));
    l->add(new OptionString("main-procedure name", &main_name));
    l->set_description("name of main procedure, for STARTUP instrumentation");
    flags->add(l);

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
InstrumentSuifPass::parse_command_line(TokenStream *command_line_stream)
{
    // set flag defaults
    debug_arg = 0;
    main_name = "main";

    if (!PipelinablePass::parse_command_line(command_line_stream))
	return false;

    debuglvl = debug_arg;
    instrument.set_main_name(main_name);

    o_fname = process_file_names(file_names);

    return true;
}

void
InstrumentSuifPass::execute()
{
    PipelinablePass::execute();

    // Process the output file name, if any.
    if (!o_fname.is_empty())
	the_suif_env->write(o_fname.chars());
}

void
InstrumentSuifPass::do_file_set_block(FileSetBlock *fsb)
{
    claim(o_fname.is_empty() || fsb->get_file_block_count() == 1,
	  "Command-line output file => file set must be a singleton");

    debug(1, "Debug level is %d", debuglvl);

    set_opi_predefined_types(fsb);
}

void
InstrumentSuifPass::do_file_block(FileBlock *fb)
{
    debug(2, "Processing file %s", fb->get_source_file_name().c_str());

    claim(has_note(fb, k_target_lib),
	  "expected target_lib annotation on file block");

    focus(fb);

    // If the user specified a new target library, use that one as
    // the_context instead of the one on the file_block.
    if (!target_lib.is_empty())
	the_context = find_context(target_lib);

    instrument.initialize();
}

void
InstrumentSuifPass::do_procedure_definition(ProcedureDefinition *pd)
{
    focus(pd);
    instrument.do_opt_unit(pd);
    defocus(pd);
}

void
InstrumentSuifPass::finalize()
{ 
    instrument.finalize();
}
