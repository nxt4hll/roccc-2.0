/* file "cfg2ssa/suif_pass.h" */

/*
    Copyright (c) 2001 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef CFG2SSA_SUIF_PASS_H
#define CFG2SSA_SUIF_PASS_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "cfg2ssa/suif_pass.h"
#endif

#include <machine/pass.h>
#include <machine/machine.h>

#include <cfg2ssa/cfg2ssa.h>

class Cfg2ssaSuifPass : public PipelinablePass {
  protected:
    Cfg2ssa cfg2ssa;			// the OPI-pass object

    // SSA-build flags
    bool build_minimal_form;
    bool build_semi_pruned_form;
    bool build_pruned_form;
    bool build_def_use_chains;
    bool fold_copies;
    bool omit_useless_phi_nodes;
    bool print_warnings;

    // command-line arguments
    OptionString *file_names;	// names of input and/or output files
    IdString o_fname;		// optional output file name

  public:
    Cfg2ssaSuifPass(SuifEnv* suif_env, const IdString &name = "cfg2ssa");
    ~Cfg2ssaSuifPass(void);

    // We only need a single copy of this pass, since we should never
    // need more than one cfg2ssa pass in any compiler.
    Module* clone() const { return (Module*)this; }

    void initialize();
    bool parse_command_line(TokenStream* command_line_stream);
    void execute();

    void do_file_set_block(FileSetBlock*);
    void do_file_block(FileBlock*);
    void do_procedure_definition(ProcedureDefinition*);

    void finalize();
};

extern "C" void init_cfg2ssa(SuifEnv *suif_env);


#endif /* CFG2SSA_SUIF_PASS_H */
