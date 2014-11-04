/* file "ex2/suif_pass.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef EX2_SUIF_PASS_H
#define EX2_SUIF_PASS_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "ex2/suif_pass.h"
#endif


#include "ex2.h"

class Ex2SuifPass : public PipelinablePass {
  protected:
    Ex2 ex2;			// the OPI-pass object

    // command-line arguments
    OptionString *file_names;	// names of input and/or output files
    IdString o_fname;		// optional output file name

  public:
    Ex2SuifPass(SuifEnv* suif_env, const IdString &name = "ex2");
    ~Ex2SuifPass(void);

    // We only need a single copy of this pass, since we should never
    // need more than one ex2 pass in any compiler.
    Module* clone() const { return (Module*)this; }

    void initialize();
    bool parse_command_line(TokenStream* command_line_stream);
    void execute();

    void do_file_set_block(FileSetBlock*);
    void do_file_block(FileBlock*);
    void do_procedure_definition(ProcedureDefinition*);

    void finalize();
};

extern "C" void init_ex2(SuifEnv *suif_env);


#endif /* EX2_SUIF_PASS_H */
