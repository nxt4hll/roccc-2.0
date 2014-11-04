/* file "peep/suif_pass.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef PEEP_SUIF_PASS_H
#define PEEP_SUIF_PASS_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "peep/suif_pass.h"
#endif

#include <machine/pass.h>
#include <machine/machine.h>

#include <peep/peep.h>


class PeepSuifPass : public PipelinablePass {
  protected:
    Peep peep;

    // command-line arguments
    int debug_arg;		// debugging message verbosity
    bool const_prop;		// push constants to adjacent use occurrences
    OptionString *file_names;	// names of input and/or output files
    IdString o_fname;		// optional output file name

  public:
    PeepSuifPass(SuifEnv* suif_env, const IdString &name = "peep");

    // We only need a single copy of this pass, since we should never
    // need more than one peep pass in any compiler.
    Module* clone() const { return (Module*)this; }

    void initialize();
    bool parse_command_line(TokenStream* command_line_stream);
    void execute();

    void do_file_set_block(FileSetBlock*);
    void do_file_block(FileBlock*);
    void do_procedure_definition(ProcedureDefinition*);

    void finalize();
};

extern "C" void init_peep(SuifEnv *suif_env);


#endif /* PEEP_SUIF_PASS_H */
