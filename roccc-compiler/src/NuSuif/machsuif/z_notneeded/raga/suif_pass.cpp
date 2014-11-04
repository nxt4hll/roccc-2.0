/* file "raga/suif_pass.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "raga/suif_pass.h"
#endif

#ifdef PERTURB_FREQ
#include <sys/types.h>
#include <time.h>
#endif // PERTURB_FREQ

#include <machine/pass.h>
#include <machine/machine.h>
#include <cfg/cfg.h>
#include <cfa/cfa.h>
#include <bvd/bvd.h>

#include <raga/suif_pass.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

extern NoteKey k_spill_code;	// Annotation key for spill code added

extern "C" void
init_raga(SuifEnv *suif_env)
{
    static bool init_done = false;

    if (init_done)
	return;
    init_done = true;

    init_suifpasses(suif_env);
    init_machine(suif_env);
    init_cfg(suif_env);
    init_cfa(suif_env);
    init_bvd(suif_env);

    k_spill_code = "spill_code";
    k_cycle_count = "cycle_count";

    ModuleSubSystem *mSubSystem = suif_env->get_module_subsystem();
    mSubSystem->register_module(new RagaSuifPass(suif_env));
}

RagaSuifPass::RagaSuifPass(SuifEnv *suif_env, const IdString& name)
    : PipelinablePass(suif_env, name)
{
    the_suif_env = suif_env;	// bind suif_env into our global environment
}

void
RagaSuifPass::initialize()
{
    PipelinablePass::initialize();

    OptionList *l;
    OptionLiteral *f;

    // Create parse tree for parsing the command line.  This must occur
    // after the parent class's initialize routine has been executed.
    _command_line->set_description("allocate registers by graph coloring");

    // Collect optional flags.
    OptionSelection *flags = new OptionSelection(false);

    // -debug level
    l = new OptionList;
    l->add(new OptionLiteral("-debug"));
    l->add(new OptionInt("level", &debug_arg));
    l->set_description("set verbosity level for debugging messages");
    flags->add(l);

    // -reserve_t0
    f = new OptionLiteral("-reserve_t0", &reserve_t0, true);
    f->set_description("reserve 0th general-purpose temp register");
    flags->add(f);

    // -note_spills
    f = new OptionLiteral("-note_spills", &note_spills, true);
    f->set_description("request annotation of added spill code");
    flags->add(f);

    // -min_caller_saved
    f = new OptionLiteral("-min_caller_saved", &min_caller_saved, true);
    f->set_description("disable round-robin temp-register allocation");
    flags->add(f);

    // -just_coalesce
    f = new OptionLiteral("-just_coalesce", &just_coalesce, true);
    f->set_description("coalesce but don't assign registers");
    flags->add(f);

    // -time_total
    f = new OptionLiteral("-time_total", &time_total, true);
    f->set_description("measure total allocation time for file");
    flags->add(f);

    // -time_each
    f = new OptionLiteral("-time_each", &time_each, true);
    f->set_description("measure allocation time for each proc");
    flags->add(f);
#ifdef NEIGHBOR_CENSUS

    // -neighbor_census
    f = new OptionLiteral("-neighbor_census", &neighbor_census, true);
    f->set_description("count neighbors by class to sharpen colorability test");
    flags->add(f);
#endif

    // -debug_proc procedure
    l = new OptionList;
    l->add(new OptionLiteral("-debug_proc"));
    debug_procs = new OptionString("procedure name");
    debug_procs->set_description("indicate procedure needing debugging output");
    l->add(debug_procs);
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

#ifdef PERTURB_FREQ
    time_t seed;
    time(&seed);
    srand48((long)seed);
#endif // PERTURB_FREQ
}

bool
RagaSuifPass::parse_command_line(TokenStream *command_line_stream)
{
    // Set flag defaults each time command line is parsed.  Otherwise, the
    // wrong values will be seen by a second or subsequent in-memory
    // instance of the pass.

    debug_arg = 0;
    reserve_t0 = false;
    note_spills = false;
    min_caller_saved = false;
    just_coalesce = false;
    time_total = false;
    time_each = false;
#ifdef NEIGHBOR_CENSUS
    neighbor_census = false;
#endif

    if (!PipelinablePass::parse_command_line(command_line_stream))
	return false;

    raga.set_debug_arg(debug_arg);
    raga.set_reserve_t0(reserve_t0);
    raga.set_note_spills(note_spills);
    raga.set_min_caller_saved(min_caller_saved);
    raga.set_just_coalesce(just_coalesce);
    raga.set_time_total(time_total || time_each);
    raga.set_time_each(time_each);
#ifdef NEIGHBOR_CENSUS
    raga.set_neighbor_census(neighbor_census);
#endif

    int n = debug_procs->get_number_of_values();
    for (int i = 0; i < n; i++) {
	String s = debug_procs->get_string(i)->get_string();
	raga.add_debug_proc(s);
    }

    o_fname = process_file_names(file_names);

    return true;
}

void
RagaSuifPass::execute()
{
    PipelinablePass::execute();

    // Process the output file name, if any.
    if (!o_fname.is_empty())
	the_suif_env->write(o_fname.chars());
}

void
RagaSuifPass::do_file_set_block(FileSetBlock *fsb)
{
    claim(o_fname.is_empty() || fsb->get_file_block_count() == 1,
	  "Command-line output file => file set must be a singleton");

    set_opi_predefined_types(fsb);
}

void
RagaSuifPass::do_file_block(FileBlock* fb)
{
    claim(has_note(fb, k_target_lib),
	  "expected target_lib annotation on file block");

    // Focus the OPI.  (Should be done in all Machine-SUIF passes.)
    focus(fb);

    raga.set_file_name(get_name(fb).chars());
    raga.initialize();
}

void
RagaSuifPass::do_procedure_definition(ProcDef* pd)
{
    focus(pd);
    raga.do_opt_unit(pd);
    defocus(pd);
}
