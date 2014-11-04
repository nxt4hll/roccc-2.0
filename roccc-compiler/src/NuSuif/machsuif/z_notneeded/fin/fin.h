/* file "fin/fin.h" */

/*
    Copyright (c) 1995-99 The President and Fellows of Harvard University

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef FIN_FIN_H
#define FIN_FIN_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "fin/fin.h"
#endif

#include <machine/machine.h>
#include <cfg/cfg.h>

/*
 * Purpose of program: This program finalizes a target-specific
 * Machine-SUIF file by laying out the stack frame and adding
 * prologue/epilogue code.
 */

class Fin {
  public:
    void initialize();
    void do_opt_unit(OptUnit*);
    void finalize() { }

  protected:
    void prepare_unit(OptUnit*);
    void analyze_unit();
    void modify_unit();

  private:
    // per-file variable
    CodeFin *code_fin;

    // per-unit variables
    OptUnit *cur_unit;
    Cfg *cur_cfg;
    InstrList *cur_il;
};

#endif /* FIN_FIN_H */
