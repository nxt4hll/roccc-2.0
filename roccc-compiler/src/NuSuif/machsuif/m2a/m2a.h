/* file "m2a/m2a.h" */

/*
    Copyright (c) 1995-99 The President and Fellows of Harvard University

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef M2A_M2A_H
#define M2A_M2A_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "m2a/m2a.h"
#endif

#include <machine/machine.h>

/* Purpose of program: This program outputs a Machine-SUIF file as
 * a list of assembly language statements. */

class M2a {
  private:
    // per file variables
    Printer *printer;

    // helper methods
    void process_sym_table(SymTable *st);
    
  public:
    M2a();
    ~M2a() { }

    // command arguments
    FILE *out;		// output file
    int Gnum;
    bool print_all_notes;
    bool want_stabs;

    // this pass does not follow the usual OPI-pass conventions
    void do_file_block(FileBlock *fb);
    void do_proc_def(ProcDef *pd);
};

#endif /* M2A_M2A_H */



