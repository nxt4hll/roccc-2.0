/* file "ex5/suif_pass.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef EX5_SUIF_PASS_H
#define EX5_SUIF_PASS_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "ex5/suif_pass.h"
#endif


#include "ex5.h"

class Ex5SuifPass : public PipelinablePass {
  protected:
    Ex5 ex5;			// the OPI-pass object

    // command-line arguments
    OptionString *file_names;	// names of input and/or output files
    IdString o_fname;		// optional output file name

  public:
    Ex5SuifPass(SuifEnv* suif_env, const IdString &name = "ex5");
    ~Ex5SuifPass(void);

    // We only need a single copy of this pass, since we should never
    // need more than one ex5 pass in any compiler.
    Module* clone() const { return (Module*)this; }

    void initialize();
    bool parse_command_line(TokenStream* command_line_stream);
    void execute();

    void do_file_set_block(FileSetBlock*);
    void do_file_block(FileBlock*);
    void do_procedure_definition(ProcedureDefinition*);

    void finalize();
};

extern "C" void init_ex5(SuifEnv *suif_env);


#endif /* EX5_SUIF_PASS_H */
