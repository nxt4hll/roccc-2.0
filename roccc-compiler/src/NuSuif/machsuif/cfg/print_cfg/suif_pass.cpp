/* file "print_cfg/suif_pass.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "print_cfg/suif_pass.h"
#endif

#include <machine/pass.h>
#include <machine/machine.h>
#include <cfg/cfg.h>

#include <print_cfg/suif_pass.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

extern "C" void
init_print_cfg(SuifEnv *suif_env)
{
    static bool init_done = false;

    if (init_done)
	return;
    init_done = true;

    ModuleSubSystem *mSubSystem = suif_env->get_module_subsystem();
    mSubSystem->register_module(new PrintCfgSuifPass(suif_env));

    // initialize the libraries required by this OPI pass
    init_suifpasses(suif_env);
    init_machine(suif_env);
    init_cfg(suif_env);
}

PrintCfgSuifPass::PrintCfgSuifPass(SuifEnv *suif_env, const IdString& name)
    : PipelinablePass(suif_env, name)
{
    the_suif_env = suif_env;	// bind suif_env into our global environment
}

PrintCfgSuifPass::~PrintCfgSuifPass()
{
    // empty
}

void
PrintCfgSuifPass::initialize()
{
    PipelinablePass::initialize();

    OptionLiteral *f;

    // Create grammar for parsing the command line.  This must occur
    // after the parent class's initialize routine has been executed.
    _command_line->set_description("convert from InstrList to Cfg form");

    // Collect optional flags.
    OptionSelection *flags = new OptionSelection(false);

    // -show_layout
    f = new OptionLiteral("-show_layout", &show_layout, true);
    f->set_description("show the program layout");
    flags->add(f);

    // -show_code
    f = new OptionLiteral("-show_code", &show_code, true); 
    f->set_description("show block content"); 
    flags->add(f);


    // -debug_proc procedure
    OptionList *l = new OptionList;
    l->add(new OptionLiteral("-proc"));
    proc_names = new OptionString("procedure name");
    proc_names->set_description("indicate procedures that need to be printed ");
    l->add(proc_names);
    _flags->add(l);

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
PrintCfgSuifPass::parse_command_line(TokenStream *command_line_stream)
{
    // Set flag defaults
    show_layout = false;
    show_code = false;
    o_fname = empty_id_string;

    if (!PipelinablePass::parse_command_line(command_line_stream))
	return false;

    print_cfg.set_show_layout(show_layout);
    print_cfg.set_show_code(show_code);

    int n = proc_names->get_number_of_values();
    for (int i = 0; i < n; i++) {
	String s = proc_names->get_string(i)->get_string();
	out_procs.insert(s);
	std::cout << s.c_str() << std::endl;
    }

    o_fname = process_file_names(file_names);

    return true;
}

void
PrintCfgSuifPass::execute()
{
    PipelinablePass::execute();

    // Process the output file name, if any.
    if (!o_fname.is_empty())
        the_suif_env->write(o_fname.chars());
}

void
PrintCfgSuifPass::do_file_set_block(FileSetBlock *fsb)
{
    claim(o_fname.is_empty() || fsb->get_file_block_count() == 1,
	  "Command-line output file => file set must be a singleton");

    set_opi_predefined_types(fsb);
}

void
PrintCfgSuifPass::do_file_block(FileBlock *fb)
{
    claim(has_note(fb, k_target_lib),
	  "expected target_lib annotation on file block");

    focus(fb);

    print_cfg.initialize();
}

void
PrintCfgSuifPass::do_procedure_definition(ProcedureDefinition *pd)
{
    IdString pname = get_name(get_proc_sym(pd));
    if (!out_procs.empty() && out_procs.find(pname) == out_procs.end()) {
	return;
    }

    focus(pd);
    print_cfg.do_opt_unit(pd);
    defocus(pd);
}

void
PrintCfgSuifPass::finalize()
{ 
    print_cfg.finalize();
}
