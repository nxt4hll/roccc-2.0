/* file "instrument/suif_pass.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef INSTRUMENT_SUIF_PASS_H
#define INSTRUMENT_SUIF_PASS_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "instrument/suif_pass.h"
#endif

#include <machine/pass.h>
#include <machine/machine.h>

#include <instrument/instrument.h>

class InstrumentSuifPass : public PipelinablePass {
  protected:
    InstrumentPass instrument;

    // command-line arguments
    int debug_arg;		// debugging message verbosity
    String target_lib;		// target library with instrumentation routines
    String main_name;		// optional name of main proc (for STARTUP)
    OptionString *file_names;	// names of input and/or output files
    IdString o_fname;		// optional output file name

  public:
    InstrumentSuifPass(SuifEnv* suif_env, const IdString &name = "instrument");
    ~InstrumentSuifPass(void);

    // We only need a single copy of this pass, since we should never
    // need more than one instrument pass in any compiler.
    Module* clone() const { return (Module*)this; }

    void initialize();
    bool parse_command_line(TokenStream* command_line_stream);
    void execute();

    void do_file_set_block(FileSetBlock*);
    void do_file_block(FileBlock*);
    void do_procedure_definition(ProcedureDefinition*);

    void finalize();
};

extern "C" void init_instrument(SuifEnv *suif_env);


#endif /* INSTRUMENT_SUIF_PASS_H */
