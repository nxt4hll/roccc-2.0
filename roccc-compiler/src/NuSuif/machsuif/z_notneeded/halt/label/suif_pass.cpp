/* file "label/suif_pass.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "label/suif_pass.h"
#endif

#include <machine/pass.h>
#include <machine/machine.h>
#include <cfg/cfg.h>
#include <halt/halt.h>

#include <label/suif_pass.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

extern "C" void
init_label(SuifEnv *suif_env)
{
    static bool init_done = false;

    if (init_done)
	return;
    init_done = true;

    ModuleSubSystem *mSubSystem = suif_env->get_module_subsystem();
    mSubSystem->register_module(new LabelSuifPass(suif_env));

    init_suifpasses(suif_env);
    init_machine(suif_env);
    init_cfg(suif_env);
    init_halt(suif_env);
}

LabelSuifPass::LabelSuifPass(SuifEnv *suif_env, const IdString& name)
    : PipelinablePass(suif_env, name)
{
    the_suif_env = suif_env;	// bind suif_env into our global environment
}

LabelSuifPass::~LabelSuifPass()
{
    // empty
}

void
LabelSuifPass::initialize()
{
    PipelinablePass::initialize();

    OptionList *l;
    OptionLiteral *f;
    OptionString *s;

    // Create grammar for parsing the command line.  This must occur
    // after the parent class's initialize routine has been executed.
    _command_line->set_description("HALT's default labelling pass");

    // collect optional flags
    OptionSelection *flags = new OptionSelection(false);

    // -debug level
    l = new OptionList;
    l->add(new OptionLiteral("-debug"));
    l->add(new OptionInt("level", &debug_arg));
    l->set_description("set verbosity level for debugging messages");
    flags->add(l);

    // -debug_proc procedure
    l = new OptionList;
    l->add(new OptionLiteral("-p"));
    l->add(new OptionString("procedure name", &debug_proc));
    l->set_description("instrumentation only this procedure");
    flags->add(l);

    // instrumentation options
    // -cbr
    f = new OptionLiteral("-cbr", &label_cbr, true);
    f->set_description("label conditional branches");
    flags->add(f);

    // -mbr
    f = new OptionLiteral("-mbr", &label_mbr, true);
    f->set_description("label multiway branches");
    flags->add(f);

    // -entry
    f = new OptionLiteral("-entry", &label_entry, true);
    f->set_description("label procedure entries");
    flags->add(f);

    // -exit
    f = new OptionLiteral("-exit", &label_exit, true);
    f->set_description("label procedure exits");
    flags->add(f);

    // -setlongjmp
    f = new OptionLiteral("-setlongjmp", &label_setlongjmp, true);
    f->set_description("label setlongjmps");
    flags->add(f);

    // -load
    f = new OptionLiteral("-load", &label_lod, true);
    f->set_description("label loads");
    flags->add(f);

    // -store
    f = new OptionLiteral("-store", &label_str, true);
    f->set_description("label stores");
    flags->add(f);

    // -block
    f = new OptionLiteral("-block", &label_bb, true);
    f->set_description("label basic blocks");
    flags->add(f);

    // -unique start
    l = new OptionList;
    l->add(new OptionLiteral("-unique"));
    l->add(new OptionInt("start", &start));
    l->set_description("set the value at which to start assigning unique numbers");
    flags->add(l);

    // -setjmp string
    l = new OptionList;
    l->add(new OptionLiteral("-setjmp"));
    s = new OptionString("setjmp procedure name", &setjmp_proc);
    s->set_description("indicate name of setjmp procedure");
    l->add(s);
    flags->add(l);

    // accept tagged options in any order
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
LabelSuifPass::parse_command_line(TokenStream *command_line_stream)
{
    // set defaults for command line parameters
    debug_arg = 0;
    start = 0;
    label_cbr = false;
    label_mbr = false;
    label_entry = false;
    label_exit = false;
    label_setlongjmp = false;
    label_lod = false;
    label_str = false;
    label_bb = false;

    if (!PipelinablePass::parse_command_line(command_line_stream))
	return false;

    debuglvl = debug_arg;
    label.set_debug_proc(debug_proc);
    label.set_unique_id(start);
    label.set_setjmp_proc(setjmp_proc);

    // set instrumentation options
    label.label_cbr = label_cbr;
    label.label_mbr = label_mbr;
    label.label_entry = label_entry;
    label.label_exit = label_exit;
    label.label_setlongjmp = label_setlongjmp;
    label.label_lod = label_lod;
    label.label_str = label_str;
    label.label_bb = label_bb;

    // pass implementation incomplete -- check if user asked for
    // something we cannot do yet
    //    claim(!label_setlongjmp,
    //	  "implementation incomplete, cannot label setlongjmps");


    o_fname = process_file_names(file_names);

    return true;
}

void
LabelSuifPass::execute()
{
    PipelinablePass::execute();

    // Process the output file name, if any.
    if (!o_fname.is_empty())
	the_suif_env->write(o_fname.chars());
}

void
LabelSuifPass::do_file_set_block(FileSetBlock *fsb)
{
    claim(o_fname.is_empty() || fsb->get_file_block_count() == 1,
	  "Command-line output file => file set must be a singleton");

    debug(1, "Debug level is %d", debuglvl);

    set_opi_predefined_types(fsb);
}

void
LabelSuifPass::do_file_block(FileBlock *fb)
{
    debug(2, "Processing file %s", fb->get_source_file_name().c_str());
    if (debug_proc.is_empty())
	debug(2, "debug_proc is %s\n", debug_proc.c_str());

    claim(has_note(fb, k_target_lib),
	  "expected target_lib annotation on file block");

    focus(fb);

    label.initialize();
}

void
LabelSuifPass::do_procedure_definition(ProcedureDefinition *pd)
{
    focus(pd);
    label.do_opt_unit(pd);
    defocus(pd);
}

void
LabelSuifPass::finalize()
{ 
    printf("%d\n", label.get_unique_id());
    label.finalize();
}
