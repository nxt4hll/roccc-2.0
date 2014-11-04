/* file "m2c/m2c.h" */

/*
    Copyright (c) 1995-99 The President and Fellows of Harvard University

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef M2C_M2C_H
#define M2C_M2C_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "m2c/m2c.h"
#endif

#include <machine/machine.h>

/* Purpose of program: Outputs a Machine-SUIF file in C. */

class M2c {
  private:
    CPrinter *printer;

    InstrList   *cur_body;
    InstrHandle cur_handle;

    // variables whose initializers are deferred during global
    // symbol-table processing
    List<VarSym *> postponed_vars;

    // helper methods
    void process_sym_table(SymTable *st);
    
  public:
    M2c();
    ~M2c() { }

    // command arguments
    FILE *out;		// output file
    bool print_all_notes;

    // this pass does not follow the usual OPI-pass conventions
    void do_file_block(FileBlock *fb);
    void do_proc_def(ProcDef *pd);

    // per procedure variables
    TypeId *vr_table;	// entry non-null at index i if vr == i exists

    // helper methods
    void process_symtab(SymTable *st, bool descend);
    void process_vr_decls(ProcDef *pd);
};

#endif /* M2C_M2C_H */



