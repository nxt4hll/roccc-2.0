/* file "dcessa/suif_pass.h" */

/*
    Copyright (c) 2001 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef DCESSA_SUIF_PASS_H
#define DCESSA_SUIF_PASS_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "dcessa/suif_pass.h"
#endif

#include <machine/pass.h>
#include <machine/machine.h>

#include <dcessa/dcessa.h>

class DcessaSuifPass : public PipelinablePass {
  protected:
    Dcessa dcessa;

    // command-line arguments
    int debug_arg;		// debugging message verbosity
    OptionString *file_names;	// names of input and/or output files
    IdString o_fname;		// optional output file name

  public:
    DcessaSuifPass(SuifEnv*, const IdString &name = "dcessa");

    // We only need a single copy of this pass, since we should never
    // need more than one DCE pass in any compiler.
    Module* clone() const { return (Module*)this; }

    void initialize();
    bool parse_command_line(TokenStream *command_line_stream);
    void execute();

    void do_file_set_block(FileSetBlock*);
    void do_file_block(FileBlock*);
    void do_procedure_definition(ProcDef*);

    void finalize()
	{ dcessa.finalize(); }
};

extern "C" void init_dcessa(SuifEnv*);


#endif /* DCESSA_SUIF_PASS_H */
