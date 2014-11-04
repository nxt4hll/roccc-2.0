/* file "suifvm/printer.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef SUIFVM_PRINTER_H
#define SUIFVM_PRINTER_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "suifvm/printer.h"
#endif

#include <machine/machine.h>

class PrinterSuifVm : protected Printer {
  public:
    virtual void print_instr_alm(Instr*);
    virtual void print_instr_cti(Instr*);
    virtual void print_instr_dot(Instr*);
    virtual void print_instr_label(Instr*);
    virtual void print_instr_user_defd(Instr*) { }

    virtual void print_opcode(Instr*);
    virtual void print_sym_disp(Opnd addr_sym, Opnd disp);
    virtual void print_address_exp(Opnd addr_exp);

    virtual char* size_directive(TypeId);
    virtual void process_value_block(ValueBlock*);

    // local variables used in print_var_def and its helpers
    char *cur_directive;
    int cur_opnd_cnt;

  public:
    PrinterSuifVm();

    virtual void start_comment() { fprintf(out, "\t# "); }

    virtual void print_instr(Instr*);
    virtual void print_opnd(Opnd);
    virtual void print_predicate(Instr*);

    virtual void print_extern_decl(VarSym*);
    virtual void print_file_decl(int fnum, IdString fnam);
    virtual void print_var_def(VarSym*);

    virtual void print_global_decl(FileBlock*);
    virtual void print_proc_begin(ProcDef*);
    virtual void print_proc_decl(ProcSym*) { }
    virtual void print_proc_entry(ProcDef*, int file_no_for_1st_line);
    virtual void print_proc_end(ProcDef*);
};

#endif /* SUIFVM_PRINTER_H */
