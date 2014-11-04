/* file "summarize/suif_pass.h" */

/*
    Copyright (c) 2001 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file. 
*/

#ifndef SUMMARIZE_SUIF_PASS_H
#define SUMMARIZE_SUIF_PASS_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "summarize/suif_pass.h"
#endif

#include <machine/pass.h>
#include <machine/machine.h>

#include <summarize/summarize.h>

class SummarizeSuifPass : public PipelinablePass {
  protected:
    Summarize summarize;

    // for gathering command line information
    OptionString *file_names_option;

  public:
    SummarizeSuifPass(SuifEnv *suif_env, const IdString &name = "summarize");
    ~SummarizeSuifPass(void) { }

    void initialize();

    // We only need a single copy of this pass, since we should never
    // need more than one summarize pass in any compiler.
    Module* clone() const { return (Module*)this; }

    bool parse_command_line(TokenStream *command_line_stream);

    void do_file_set_block(FileSetBlock*);
    void do_file_block(FileBlock*);
    void do_procedure_definition(ProcedureDefinition*);

//  void finalize();
};

extern "C" void init_summarize(SuifEnv *suif_env);


#endif /* SUMMARIZE_SUIF_PASS_H */
