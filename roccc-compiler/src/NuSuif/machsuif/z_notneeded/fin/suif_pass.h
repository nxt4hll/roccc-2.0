/* file "fin/suif_pass.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file. 
*/

#ifndef FIN_SUIF_PASS_H
#define FIN_SUIF_PASS_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "fin/suif_pass.h"
#endif

#include <machine/pass.h>
#include <machine/machine.h>

#include <fin/fin.h>

class FinSuifPass : public PipelinablePass {
  protected:
    Fin fin;

    // command-line arguments
    OptionString *file_names;	// names of input and/or output files
    IdString o_fname;		// optional output file name

  public:
    FinSuifPass(SuifEnv *suif_env, const IdString &name = "fin");
    ~FinSuifPass(void);

    // We only need a single copy of this pass, since we should never
    // need more than one fin pass in any compiler.
    Module* clone() const { return (Module*)this; }

    void initialize();
    bool parse_command_line(TokenStream *command_line_stream);
    void execute();

    void do_file_set_block(FileSetBlock*);
    void do_file_block(FileBlock *fb);
    void do_procedure_definition(ProcDef *pd);
};

extern "C" void init_fin(SuifEnv *suif_env);


#endif /* FIN_SUIF_PASS_H */
