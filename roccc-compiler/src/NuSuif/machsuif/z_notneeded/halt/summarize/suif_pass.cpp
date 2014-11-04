/* file "summarize/suif_pass.cpp" */

/*
    Copyright (c) 2001 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "summarize/suif_pass.h"
#endif

#include <machine/pass.h>
#include <machine/machine.h>
#include <cfg/cfg.h>
#include <halt/halt.h>

#include <summarize/suif_pass.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

extern "C" void
init_summarize(SuifEnv *suif_env)
{
    static bool init_done = false;

    if (init_done)
	return;
    init_done = true;

    ModuleSubSystem *mSubSystem = suif_env->get_module_subsystem();
    mSubSystem->register_module(new SummarizeSuifPass(suif_env));

    init_suifpasses(suif_env);
    init_machine(suif_env);
    init_cfg(suif_env);
    init_halt(suif_env);
}

SummarizeSuifPass::SummarizeSuifPass(SuifEnv *suif_env, const IdString &name)
    : PipelinablePass(suif_env, name)
{
    the_suif_env = suif_env;	// bind suif_env into our global environment
}

void
SummarizeSuifPass::initialize()
{
    OptionList *l;

    PipelinablePass::initialize();

    // Create parse tree for parsing the command line.  This must occur
    // after the parent class's initialize routine has been executed.
    _command_line->set_description(
	"summarize instrumentation events");

    // Collect optional flags.
    OptionSelection *flags = new OptionSelection(false);

    // -debug level
    l = new OptionList;
    l->add(new OptionLiteral("-debug"));
    l->add(new OptionInt("level", &debuglvl));
    l->set_description("set verbosity level for debugging messages");
    flags->add(l);

    // accept flags in any order
    _command_line->add(new OptionLoop(flags));

    // zero or more file names
    file_names_option = new OptionString("file name");
    OptionLoop *files =
	new OptionLoop(file_names_option,
		       "names of optional input and/or output files");
    _command_line->add(files);
}

bool
SummarizeSuifPass::parse_command_line(TokenStream *command_line_stream)
{
    // set defaults for optional command-line flags
    debuglvl = 0;

    bool result = PipelinablePass::parse_command_line(command_line_stream);

    debug(1, "Debug level is %d", debuglvl);

    int file_count = file_names_option->get_number_of_values();
    int i = 0;

    // Process the input file name, if any.
    if (the_suif_env->get_file_set_block()) {		// expect no input file
	claim(file_count <= 1, "Too many file names: already have input");
    } else {
	claim(file_count > 0, "No input file");
	IdString s = file_names_option->get_string(i++)->get_string();
	the_suif_env->read(s.chars());
    }

    // Process the output file name, if any.
    if (i < file_count) {
	IdString s = file_names_option->get_string(i++)->get_string();
	summarize.set_output_file(fopen(s.chars(), "w"));
	claim(summarize.get_output_file(),
	      "Couldn't open %s for writing", s.chars());
    } else {
	summarize.set_output_file(stdout);
    }
    claim(i == file_count,
	  "Too many file names: expected %d, got %d", i, file_count);
    return result;
}

void
SummarizeSuifPass::do_file_set_block(FileSetBlock *fsb) 
{
    set_opi_predefined_types(fsb);
}

void
SummarizeSuifPass::do_file_block(FileBlock *fb)
{
    debug(2, "Processing file %s", get_name(fb).chars());

    claim(has_note(fb, k_target_lib),
	  "expected target_lib annotation on file block");

    focus(fb);

    summarize.initialize();
}

void
SummarizeSuifPass::do_procedure_definition(ProcedureDefinition *pd)
{
    focus(pd);
    summarize.do_opt_unit(pd);
    defocus(pd);
}

/*
void
SummarizeSuifPass::finalize()
{
    summarize.finalize();
}
*/
