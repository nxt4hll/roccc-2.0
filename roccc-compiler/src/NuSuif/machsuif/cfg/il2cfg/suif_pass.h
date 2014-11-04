/* file "il2cfg/suif_pass.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef IL2CFG_SUIF_PASS_H
#define IL2CFG_SUIF_PASS_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "il2cfg/suif_pass.h"
#endif

#include <machine/pass.h>
#include <machine/machine.h>

#include <il2cfg/il2cfg.h>

class Il2cfgSuifPass : public PipelinablePass {
  protected:
    Il2cfg il2cfg;		// the OPI-pass object

    // command-line arguments
    bool keep_layout;		// flags to cfg creator
    bool break_at_call;
    bool break_at_instr;
    OptionString *file_names;	// names of input and/or output files
    IdString o_fname;		// optional output file name

  public:
    Il2cfgSuifPass(SuifEnv* suif_env, const IdString &name = "il2cfg");
    ~Il2cfgSuifPass(void);

    // We only need a single copy of this pass, since we should never
    // need more than one il2cfg pass in any compiler.
    Module* clone() const { return (Module*)this; }

    void initialize();
    bool parse_command_line(TokenStream* command_line_stream);
    void execute();

    void do_file_set_block(FileSetBlock*);
    void do_file_block(FileBlock*);
    void do_procedure_definition(ProcedureDefinition*);

    void finalize();
};

extern "C" void init_il2cfg(SuifEnv *suif_env);


#endif /* IL2CFG_SUIF_PASS_H */
