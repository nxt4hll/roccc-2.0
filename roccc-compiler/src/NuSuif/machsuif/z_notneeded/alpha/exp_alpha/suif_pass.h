/* file "exp_alpha/suif_pass.h" */

/*
    Copyright (c) 2001 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef EXP_ALPHA_SUIF_PASS_H
#define EXP_ALPHA_SUIF_PASS_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "exp_alpha/suif_pass.h"
#endif

#include <machine/pass.h>
#include <machine/machine.h>

#include <exp_alpha/exp_alpha.h>

class ExpAlphaSuifPass : public PipelinablePass {
  protected:
    ExpAlpha exp_alpha;

    // command-line arguments
    int debug_arg;		// debugging message verbosity

    bool no_reloc;		// do NOT expand using relocations
    bool no_lituse;		// inhibit the generation of lituse
    bool use_virtual_regs;	// assume register allocation follows this pass
    bool spill_volatiles;	// spill non-local or addressed variables
    bool exp_sublong_ld_st;	// expand LDB/LDW/STB/STW instructions

    OptionString *file_names;	// names of input and/or output files
    IdString o_fname;		// optional output file name

  public:
    ExpAlphaSuifPass(SuifEnv *suif_env, const IdString &name = "exp_alpha");

    void initialize();
    void execute();

    // Unlike most passes, this one is typically used more than once in
    // chain of passes, so we give it a clone method that really clones.

    Module* clone() const { return new ExpAlphaSuifPass(the_suif_env); }

    bool parse_command_line(TokenStream *command_line_stream);

    void do_file_set_block(FileSetBlock*);
    void do_file_block(FileBlock*);
    void do_procedure_definition(ProcDef*);

    void finalize();
};

extern "C" void init_exp_alpha(SuifEnv *suif_env);


#endif /* EXP_ALPHA_SUIF_PASS_H */
