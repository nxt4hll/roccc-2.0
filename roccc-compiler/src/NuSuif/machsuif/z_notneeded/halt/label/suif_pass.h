/* file "label/suif_pass.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef LABEL_SUIF_PASS_H
#define LABEL_SUIF_PASS_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "label/suif_pass.h"
#endif

#include <machine/pass.h>
#include <machine/machine.h>

#include <label/label.h>

class LabelSuifPass : public PipelinablePass {
  protected:
    LabelPass label;

    // command-line arguments
    int debug_arg;		// debugging message verbosity
    String debug_proc;		// if set, only procedure that we need to instrument
    String setjmp_proc;		// optional alternate setjmp name
    int start;                  // the value at which to start assigning unique numbers
    OptionString *file_names;	// names of input and/or output files
    IdString o_fname;		// optional output file name

    // instrumentation options
    bool label_cbr;
    bool label_mbr;
    bool label_entry;
    bool label_exit;
    bool label_setlongjmp;
    bool label_lod;
    bool label_str;
    bool label_bb;

  public:
    LabelSuifPass(SuifEnv* suif_env, const IdString &name = "label");
    ~LabelSuifPass(void);

    // We only need a single copy of this pass, since we should never
    // need more than one label pass in any compiler.
    Module* clone() const { return (Module*)this; }

    void initialize();
    bool parse_command_line(TokenStream* command_line_stream);
    void execute();

    void do_file_set_block(FileSetBlock*);
    void do_file_block(FileBlock*);
    void do_procedure_definition(ProcedureDefinition*);

    void finalize();
};

extern "C" void init_label(SuifEnv *suif_env);


#endif /* LABEL_SUIF_PASS_H */
