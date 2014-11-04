/* file "ssa2cfg/suif_pass.h" */

/*
    Copyright (c) 2001 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef SSA2CFG_SUIF_PASS_H
#define SSA2CFG_SUIF_PASS_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "ssa2cfg/suif_pass.h"
#endif

#include <machine/pass.h>
#include <machine/machine.h>

#include <ssa2cfg/ssa2cfg.h>

class Ssa2cfgSuifPass : public PipelinablePass {
  protected:
    Ssa2cfg ssa2cfg;			// the OPI-pass object

    // command-line arguments
    bool restore_orig_names;	// use original (non-SSA) location names

    OptionString *file_names;	// names of input and/or output files
    IdString o_fname;		// optional output file name

  public:
    Ssa2cfgSuifPass(SuifEnv* suif_env, const IdString &name = "ssa2cfg");
    ~Ssa2cfgSuifPass(void);

    // We only need a single copy of this pass, since we should never
    // need more than one ssa2cfg pass in any compiler.
    Module* clone() const { return (Module*)this; }

    void initialize();
    bool parse_command_line(TokenStream* command_line_stream);
    void execute();

    void do_file_set_block(FileSetBlock*);
    void do_file_block(FileBlock*);
    void do_procedure_definition(ProcedureDefinition*);

    void finalize();
};

extern "C" void init_ssa2cfg(SuifEnv *suif_env);


#endif /* SSA2CFG_SUIF_PASS_H */
