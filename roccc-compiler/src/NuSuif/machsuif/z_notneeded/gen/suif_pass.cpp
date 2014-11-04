/* file "gen/suif_pass.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "gen/suif_pass.h"
#endif

#include <machine/pass.h>
#include <machine/machine.h>
#include <suifvm/suifvm.h>

#include <gen/suif_pass.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

extern "C" void
init_gen(SuifEnv *suif_env)
{
    static bool init_done = false;

    if (init_done)
	return;
    init_done = true;

    ModuleSubSystem *mSubSystem = suif_env->get_module_subsystem();
    mSubSystem->register_module(new GenSuifPass(suif_env));


    init_suifpasses(suif_env);
    init_machine(suif_env);
    init_suifvm(suif_env);
}


GenSuifPass::GenSuifPass(SuifEnv* suif_env, const IdString& name)
    : PipelinablePass(suif_env, name)
{
    the_suif_env = suif_env;	// bind suif_env into our global environment
}

GenSuifPass::~GenSuifPass()
{
    // empty
}

void
GenSuifPass::initialize()
{
    PipelinablePass::initialize();

    // Create parse tree for parsing the command line.  This must occur
    // after the parent class's initialize routine has been executed.
    _command_line->set_description(
	"translate SUIFvm file to real-target instructions");

    // Collect optional flags.
    OptionSelection *flags = new OptionSelection(false);

    OptionList *l;

    // -debug level
    l = new OptionList;
    l->add(new OptionLiteral("-debug"));
    l->add(new OptionInt("level", &debuglvl));
    l->set_description("set verbosity level for debugging messages");
    flags->add(l);

    // target library
    l = new OptionList;
    l->add(new OptionLiteral("-target_lib"));
    target_lib = new OptionString("target library");
    target_lib->
	set_description("select target machine by giving its library name");
    l->add(target_lib);
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
GenSuifPass::parse_command_line(TokenStream *command_line_stream)
{
    debuglvl = 0;

    if (!PipelinablePass::parse_command_line(command_line_stream))
	return false;

    debug(1, "Debug level is %d", debuglvl);

    gen.target_lib = get_option_string_value(target_lib);
    o_fname = process_file_names(file_names);

    return true;
}

void
GenSuifPass::execute()
{
    PipelinablePass::execute();

    // Process the output file name, if any.
    if (!o_fname.is_empty())
	the_suif_env->write(o_fname.chars());
}

void
GenSuifPass::do_file_set_block(FileSetBlock *fsb)
{
    claim(o_fname.is_empty() || fsb->get_file_block_count() == 1,
	  "Command-line output file => file set must be a singleton");

    set_opi_predefined_types(fsb);
}

void
GenSuifPass::do_file_block(FileBlock *fb)
{
    debug(2, "Processing file %s", get_name(fb).chars());

    OneNote<IdString> note = take_note(fb, k_target_lib);

    // sanity check
    claim(!is_null(note) && note.get_value() == k_suifvm);

    if (gen.target_lib.is_empty()) {
	char *env_target_lib = getenv("TARGET_LIB");
	claim(env_target_lib, "No target_lib for code generation");
	gen.target_lib = env_target_lib;
    }
    
    // Define lib<TARGET_LIB> as the target-specific library.  (Only
    // needs to be done in code-generation passes.)
	  
    note.set_value(gen.target_lib);
    set_note(fb, k_target_lib, note);

    focus(fb);

    gen.initialize();
}

void
GenSuifPass::do_procedure_definition(ProcedureDefinition *pd)
{
    focus(pd);
    gen.do_opt_unit(pd);
    defocus(pd);
}
