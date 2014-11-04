/* file "ex3/suif_pass.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "ex3/suif_pass.h"
#endif

#include <machine/pass.h>
#include <machine/machine.h>

#include "ex3.h"
#include "suif_pass.h"

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

extern "C" void
init_ex3(SuifEnv *suif_env)
{
    static bool init_done = false;

    if (init_done)
	return;
    init_done = true;

    ModuleSubSystem *mSubSystem = suif_env->get_module_subsystem();
    mSubSystem->register_module(new Ex3SuifPass(suif_env));

    // initialize the libraries required by this OPI pass
    init_suifpasses(suif_env);
    init_machine(suif_env);
}

Ex3SuifPass::Ex3SuifPass(SuifEnv *suif_env, const IdString& name)
    : PipelinablePass(suif_env, name)
{
    the_suif_env = suif_env;	// bind suif_env into our global environment
}

Ex3SuifPass::~Ex3SuifPass()
{
    // empty
}

void
Ex3SuifPass::initialize()
{
    PipelinablePass::initialize();

    OptionList *l;

    // Create grammar for parsing the command line.  This must occur
    // after the parent class's initialize routine has been executed.
    _command_line->set_description("cookbook example #3");

    // Collect optional flags.
    OptionSelection *flags = new OptionSelection(false);

    // -reserve_reg
    l = new OptionList;
    l->add(new OptionLiteral("-reserve_reg"));
    l->add(new OptionString("reg", &reserved_reg_name));
    l->set_description("specifies name of register to reserve");
    flags->add(l);

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
Ex3SuifPass::parse_command_line(TokenStream *command_line_stream)
{
    reserved_reg_name.make_empty();
    o_fname = empty_id_string;

    if (!PipelinablePass::parse_command_line(command_line_stream))
	return false;

    ex3.set_reserved_reg(reserved_reg_name);

    o_fname = process_file_names(file_names);

    return true;
}

void
Ex3SuifPass::execute()
{
    PipelinablePass::execute();

    // Process the output file name, if any.
    if (!o_fname.is_empty())
        the_suif_env->write(o_fname.chars());
}

void
Ex3SuifPass::do_file_set_block(FileSetBlock *fsb)
{
    claim(o_fname.is_empty() || fsb->get_file_block_count() == 1,
	  "Command-line output file => file set must be a singleton");

    set_opi_predefined_types(fsb);
}

void
Ex3SuifPass::do_file_block(FileBlock *fb)
{
    claim(has_note(fb, k_target_lib),
	  "expected target_lib annotation on file block");

    focus(fb);

    ex3.initialize();
}

void
Ex3SuifPass::do_procedure_definition(ProcedureDefinition *pd)
{
    focus(pd);
    ex3.do_opt_unit(pd);
    defocus(pd);
}

void
Ex3SuifPass::finalize()
{ 
    ex3.finalize();
}
