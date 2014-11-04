/* file "dce/suif_pass.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef DCE_SUIF_PASS_H
#define DCE_SUIF_PASS_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "dce/suif_pass.h"
#endif

#include <machine/machine.h>
#include <dce/dce.h>

class DceSuifPass : public PipelinablePass {
  protected:
    Dce dce;

    // command-line arguments
    int debug_arg;		// debugging message verbosity
    OptionString *file_names;	// names of input and/or output files
    IdString o_fname;		// optional output file name

  public:
    DceSuifPass(SuifEnv *suif_env, const IdString &name = "dce");

    // We only need a single copy of this pass, since we should never
    // need more than one dce pass in any compiler.
    Module* clone() const { return (Module*)this; }

    void initialize();
    bool parse_command_line(TokenStream *command_line_stream);
    void execute();

    void do_file_set_block(FileSetBlock *the_file_set_block);
    void do_file_block(FileBlock *the_file_block);
    void do_procedure_definition(ProcDef *pd);

    void finalize()
	{ dce.finalize(); }
};

extern "C" void init_dce(SuifEnv *suif_env);


#endif /* DCE_SUIF_PASS_H */
