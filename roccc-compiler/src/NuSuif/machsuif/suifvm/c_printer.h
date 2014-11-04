/* file "suifvm/c_printer.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef SUIFVM_CPRINTER_H
#define SUIFVM_CPRINTER_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "suifvm/cprinter.h"
#endif

#include <machine/machine.h>

class CPrinterSuifVm : public CPrinter {
  public:
    CPrinterSuifVm();
    virtual ~CPrinterSuifVm() { delete [] print_instr_table; }

    virtual void print_instr(Instr *mi);
    virtual void print_global_decl(FileBlock *fb);

  protected:
    virtual void print_instr_alm(Instr *);
    virtual void print_instr_cti(Instr *);
    virtual void print_instr_dot(Instr *);
    virtual void print_instr_mbr(Instr *);
    virtual void print_instr_cal(Instr *);

    virtual void print_instr_user_defd(Instr *) { }
    virtual void print_opcode(Instr *mi);
    virtual void print_addr_binop(Instr *mi);
};

#endif /* SUIFVM_CPRINTER_H */
