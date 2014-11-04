/* file "ex1/suif_pass.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef EX1_SUIF_PASS_H
#define EX1_SUIF_PASS_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "ex1/suif_pass.h"
#endif


class Ex1SuifPass : public PipelinablePass {
  protected:
    Ex1 ex1;			// the OPI-pass object

    // command-line arguments
    OptionString *file_names;	// names of input and/or output files
    IdString o_fname;		// optional output file name

  public:
    Ex1SuifPass(SuifEnv* suif_env, const IdString &name = "ex1");
    ~Ex1SuifPass(void);

    // We only need a single copy of this pass, since we should never
    // need more than one ex1 pass in any compiler.
    Module* clone() const { return (Module*)this; }

    void initialize();
    bool parse_command_line(TokenStream* command_line_stream);
    void execute();

    void do_file_set_block(FileSetBlock*);
    void do_file_block(FileBlock*);
    void do_procedure_definition(ProcedureDefinition*);

    void finalize();
};

extern "C" void init_ex1(SuifEnv *suif_env);


#endif /* EX1_SUIF_PASS_H */
