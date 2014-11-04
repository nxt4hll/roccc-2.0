/* file "m2a/suif_pass.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file. 
*/

#ifndef M2A_SUIF_PASS_H
#define M2A_SUIF_PASS_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "m2a/suif_pass.h"
#endif

#include <machine/pass.h>
#include <machine/machine.h>

#include <m2a/m2a.h>

class M2aSuifPass : public PipelinablePass {
  protected:
    M2a m2a;

    // for gathering command line information
    OptionString *nonprinting_notes_option;
    OptionString *file_names_option;

  public:
    M2aSuifPass(SuifEnv* suif_env, const IdString &name = "m2a");
    ~M2aSuifPass(void) { }

    void initialize();

    // We only need a single copy of this pass, since we should never
    // need more than one m2a pass in any compiler.
    Module* clone() const { return (Module *)this; }

    bool parse_command_line(TokenStream* command_line_stream);

    void do_file_set_block(FileSetBlock*);
    void do_file_block(FileBlock*);
    void do_procedure_definition(ProcedureDefinition* pd);
};

extern "C" void init_m2a(SuifEnv *suif_env);


#endif /* M2A_SUIF_PASS_H */
