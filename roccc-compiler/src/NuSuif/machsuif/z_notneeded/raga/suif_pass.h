/* file "raga/suif_pass.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef RAGA_SUIF_PASS_H
#define RAGA_SUIF_PASS_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "raga/suif_pass.h"
#endif

#include <machine/pass.h>
#include <machine/machine.h>

#include <raga/raga.h>

class RagaSuifPass : public PipelinablePass {
  protected:
    Raga raga;

    // command-line arguments
    int debug_arg;		// debugging message verbosity
    bool reserve_t0;		// don't allocate 0th GPR temp register
    bool note_spills;		// annotate spill/reload instructions
    bool min_caller_saved;	// don't do round-robin temp assignment
    bool just_coalesce;		// coalesce but don't assign registers
    bool time_total;		// measure total allocation time for file
    bool time_each;		// measure allocation time for each proc
#ifdef NEIGHBOR_CENSUS
    bool neighbor_census;	// Maintain neighbor counts by class
#endif
    OptionString *debug_procs;	// procedures that need debugging output
    OptionString *file_names;	// names of input and/or output files
    IdString o_fname;		// optional output file name

  public:
    RagaSuifPass(SuifEnv *suif_env, const IdString &name = "raga");

    void initialize();
    void execute();

    // This pass might be used more than once in a pipeline, so we give it
    // a clone method that really clones.

    Module* clone() const { return new RagaSuifPass(the_suif_env); }

    bool parse_command_line(TokenStream *command_line_stream);

    void do_file_set_block(FileSetBlock *fsb);
    void do_file_block(FileBlock *fb);
    void do_procedure_definition(ProcDef *pd);
    void finalize() { raga.finalize(); }
};

extern "C" void init_raga(SuifEnv *suif_env);


#endif /* RAGA_SUIF_PASS_H */
