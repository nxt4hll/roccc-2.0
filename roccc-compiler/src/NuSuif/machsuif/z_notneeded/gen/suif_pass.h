/* file "gen/suif_pass.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file. 
*/

#ifndef GEN_SUIF_PASS_H
#define GEN_SUIF_PASS_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "gen/suif_pass.h"
#endif

#include <machine/pass.h>
#include <machine/machine.h>

#include <gen/gen.h>

class GenSuifPass : public PipelinablePass {
  protected:
    Gen gen;

    // command-line arguments
    OptionString *target_lib;	// optional target library name
    OptionString *file_names;	// names of input and/or output files
    IdString o_fname;		// optional output file name

  public:
    GenSuifPass(SuifEnv* suif_env, const IdString &name = "gen");
    ~GenSuifPass(void);

    // We only need a single copy of this pass, since we should never
    // need more than one gen pass in any compiler.
    Module* clone() const { return (Module *)this; }

    void initialize();
    bool parse_command_line(TokenStream* command_line_stream);
    void execute();

    void do_file_set_block(FileSetBlock* the_file_set_block);
    void do_file_block(FileBlock* the_file_block);
    void do_procedure_definition(ProcDef* pd);
};

extern "C" void init_gen(SuifEnv *suif_env);


#endif /* GEN_SUIF_PASS_H */
