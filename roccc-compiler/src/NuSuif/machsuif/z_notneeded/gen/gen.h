/* file "gen/gen.h" */

/*
    Copyright (c) 1995-99 The President and Fellows of Harvard University

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef GEN_GEN_H
#define GEN_GEN_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "gen/gen.h"
#endif

#include <machine/machine.h>
#include <suifvm/suifvm.h>

/* Purpose of program: This program translates a Machine-SUIF file
 * from SUIFvm instructions to code for a real target. */

class Gen {
  private:
    // per file variables
    CodeGen *code_gen;

  public:
    Gen() { }
    ~Gen() { }

    // command-line arguments
    IdString target_lib;  	// optional target library name

    void initialize();
    void do_opt_unit(OptUnit*);
    void finalize() { }
};

#endif /* GEN_GEN_H */
