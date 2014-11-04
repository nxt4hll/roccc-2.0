/* file "m2c/suif_pass.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "m2c/suif_pass.h"
#endif

#include <machine/pass.h>
#include <machine/machine.h>

#include <m2c/suif_pass.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

extern "C" void
init_m2c(SuifEnv *suif_env)
{
    static bool init_done = false;

    if (init_done)
	return;
    init_done = true;

    ModuleSubSystem *mSubSystem = suif_env->get_module_subsystem();
    mSubSystem->register_module(new M2cSuifPass(suif_env));

    init_suifpasses(suif_env);
    init_machine(suif_env);
}

M2cSuifPass::M2cSuifPass(SuifEnv *suif_env, const IdString& name)
    : PipelinablePass(suif_env, name)
{
    the_suif_env = suif_env;	// bind suif_env into our global environment
}

void
M2cSuifPass::initialize()
{
    OptionList *l;
    OptionLiteral *f;

    PipelinablePass::initialize();

    // Create parse tree for parsing the command line.  This must occur
    // after the parent class's initialize routine has been executed.
    _command_line->set_description(
	"translate Machine-SUIF file to assembly language");

    // Collect optional flags.
    OptionSelection *flags = new OptionSelection(false);

    // -debug level
    l = new OptionList;
    l->add(new OptionLiteral("-debug"));
    l->add(new OptionInt("level", &debuglvl));
    l->set_description("set verbosity level for debugging messages");
    flags->add(l);

    // -all_notes -- print all instruction notes
    f = new OptionLiteral("-all_notes", &m2c.print_all_notes, true);
    f->set_description("force all instruction notes to appear in output");
    flags->add(f);
    
    // -noprint note -- do not print this note
    l = new OptionList;
    l->add(new OptionLiteral("-noprint"));
    nonprinting_notes_option = new OptionString("annotation key");
    l->add(nonprinting_notes_option);
    l->set_description("suppress annotations with given key");
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
M2cSuifPass::parse_command_line(TokenStream *command_line_stream)
{
    // set defaults for optional command-line flags
    debuglvl = 0;
    m2c.print_all_notes = false;

    bool result = PipelinablePass::parse_command_line(command_line_stream);

    debug(1, "Debug level is %d", debuglvl);
    if (m2c.print_all_notes)
	debug(1, "Printing all notes");

    int n = nonprinting_notes_option->get_number_of_values();
    claim((n == 0) || !m2c.print_all_notes, 
	  "cannot specify -noprint with -all_notes");

    // process non-printing notes, if any
    for (int i = 0; i < n; i++) {
	IdString s = nonprinting_notes_option->get_string(i)->get_string();
	nonprinting_notes.insert(s);
	debug(1, "Suppressing `%s' annotations", s.chars());
    }

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
	m2c.out = fopen(s.chars(), "w");
	claim(m2c.out, "Couldn't open %s for writing", s.chars());
    } else {
	m2c.out = stdout;
    }
    claim(i == file_count,
	  "Too many file names: expected %d, got %d", i, file_count);
    return result;
}

void
M2cSuifPass::do_file_set_block(FileSetBlock *fsb) 
{
    set_opi_predefined_types(fsb);
}

void
M2cSuifPass::do_file_block(FileBlock *fb)
{
    debug(2, "Processing file %s", get_name(fb).chars());

    claim(has_note(fb, k_target_lib),
	  "expected target_lib annotation on file block");

    focus(fb);

    m2c.do_file_block(fb);
}

void
M2cSuifPass::do_procedure_definition(ProcedureDefinition *pd)
{
    focus(pd);
    m2c.do_proc_def(pd);
    defocus(pd);
}
