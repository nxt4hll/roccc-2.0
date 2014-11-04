/* file "m2a/suif_pass.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "m2a/suif_pass.h"
#endif

#include <machine/pass.h>
#include <machine/machine.h>

#include <m2a/suif_pass.h>
#include <suifvm/suifvm.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

extern "C" void
init_m2a(SuifEnv *suif_env)
{
    static bool init_done = false;

    if (init_done)
	return;
    init_done = true;

    ModuleSubSystem *mSubSystem = suif_env->get_module_subsystem();
    mSubSystem->register_module(new M2aSuifPass(suif_env));

    init_suifpasses(suif_env);
    init_machine(suif_env);
    init_suifvm(suif_env);
}

M2aSuifPass::M2aSuifPass(SuifEnv* suif_env, const IdString& name)
    : PipelinablePass(suif_env, name)
{
    the_suif_env = suif_env;	// bind suif_env into our global environment
}

void
M2aSuifPass::initialize()
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
    f = new OptionLiteral("-all_notes", &m2a.print_all_notes, true);
    f->set_description("force all instruction notes to appear in output");
    flags->add(f);
    
    // -noprint note -- do not print this note
    l = new OptionList;
    l->add(new OptionLiteral("-noprint"));
    nonprinting_notes_option = new OptionString("annotation key");
    l->add(nonprinting_notes_option);
    l->set_description("suppress annotations with given key");
    flags->add(l);

    // -G max -- max byte size of datum in $gp area
    l = new OptionList;
    l->add(new OptionLiteral("-G"));
    l->add(new OptionInt("max", &m2a.Gnum));
    l->set_description("set max byte size of a datum in the $gp area");
    flags->add(l);

    // -stabs -- print with stabs information
    f = new OptionLiteral("-stabs", &m2a.want_stabs, true);
    f->set_description("add stabs debugging information to output");
    flags->add(f);

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
M2aSuifPass::parse_command_line(TokenStream* command_line_stream)
{
    // set defaults for optional command-line flags
    debuglvl = 0;
    m2a.print_all_notes = false;
    m2a.Gnum = 512;		// in bytes
    m2a.want_stabs = false;    

    bool result = PipelinablePass::parse_command_line(command_line_stream);

    debug(1, "Debug level is %d", debuglvl);
    debug(1, "Gnum = %d", m2a.Gnum);
    if (m2a.print_all_notes)
	debug(1, "Printing all notes");
    if (m2a.want_stabs)
	debug(1, "Printing with stabs information");

    int n = nonprinting_notes_option->get_number_of_values();
    claim((n == 0) || !m2a.print_all_notes, 
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
	m2a.out = fopen(s.chars(), "w");
	claim(m2a.out, "Couldn't open %s for writing", s.chars());
    } else {
	m2a.out = stdout;
    }
    claim(i == file_count,
	  "Too many file names: expected %d, got %d", i, file_count);
    return result;
}

void
M2aSuifPass::do_file_set_block(FileSetBlock* fsb) 
{
    set_opi_predefined_types(fsb);
}

void
M2aSuifPass::do_file_block(FileBlock* fb)
{
    debug(2, "Processing file %s", get_name(fb).chars());

    claim(has_note(fb, k_target_lib),
	  "expected target_lib annotation on file block");

    focus(fb);

    m2a.do_file_block(fb);
}

void
M2aSuifPass::do_procedure_definition(ProcedureDefinition* pd)
{
    focus(pd);
    m2a.do_proc_def(pd);
    defocus(pd);
}
