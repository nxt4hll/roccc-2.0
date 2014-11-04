/* file "cfg2il/suif_pass.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef CFG2IL_SUIF_PASS_H
#define CFG2IL_SUIF_PASS_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "cfg2il/suif_pass.h"
#endif

#include <machine/pass.h>
#include <machine/machine.h>

#include <cfg2il/cfg2il.h>

class Cfg2ilSuifPass : public PipelinablePass {
  protected:
    Cfg2il cfg2il;			// the OPI-pass object

    // command-line arguments
    OptionString *file_names;	// names of input and/or output files
    IdString o_fname;		// optional output file name

  public:
    Cfg2ilSuifPass(SuifEnv* suif_env, const IdString &name = "cfg2il");
    ~Cfg2ilSuifPass(void);

    // We only need a single copy of this pass, since we should never
    // need more than one cfg2il pass in any compiler.
    Module* clone() const { return (Module*)this; }

    void initialize();
    bool parse_command_line(TokenStream* command_line_stream);
    void execute();

    void do_file_set_block(FileSetBlock*);
    void do_file_block(FileBlock*);
    void do_procedure_definition(ProcedureDefinition*);

    void finalize();
};

extern "C" void init_cfg2il(SuifEnv *suif_env);


#endif /* CFG2IL_SUIF_PASS_H */
