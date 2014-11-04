/* file "blpp/suif_pass.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef BLPP_SUIF_PASS_H
#define BLPP_SUIF_PASS_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "blpp/suif_pass.h"
#endif


class BlppSuifPass : public PipelinablePass {
  protected:
    BlppPass blpp;			// the OPI-pass object

    // command-line arguments
    OptionString *file_names;	// names of input and/or output files
    IdString o_fname;		// optional output file name

  public:
    BlppSuifPass(SuifEnv* suif_env, const IdString &name = "blpp");
    ~BlppSuifPass(void);

    // We only need a single copy of this pass, since we should never
    // need more than one blpp pass in any compiler.
    Module* clone() const { return (Module*)this; }

    void initialize();
    bool parse_command_line(TokenStream* command_line_stream);
    void execute();

    void do_file_set_block(FileSetBlock*);
    void do_file_block(FileBlock*);
    void do_procedure_definition(ProcedureDefinition*);

    void finalize();
};

extern "C" void init_blpp(SuifEnv *suif_env);


#endif /* BLPP_SUIF_PASS_H */
