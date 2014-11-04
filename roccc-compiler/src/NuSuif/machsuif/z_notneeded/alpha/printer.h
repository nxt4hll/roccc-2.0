/* file "alpha/printer.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef ALPHA_PRINTER_H
#define ALPHA_PRINTER_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "alpha/printer.h"
#endif

/*
 * This class defines how to print Alpha instructions in assembly format.
 */
class PrinterAlpha : public Printer {
  protected:
    virtual void print_instr_alm(Instr*);
    virtual void print_instr_cti(Instr*);
    virtual void print_instr_dot(Instr*);
    virtual void print_instr_label(Instr*);
    virtual void print_instr_user_defd(Instr*) { }

    virtual void print_opcode(Instr *mi);
    virtual void print_sym_disp(Sym *s, int d);
    virtual void print_addr_exp(Opnd o);
    virtual void print_reloc(Instr *mi);

    virtual int print_size_directive(TypeId t);
    virtual int print_bit_filler(int bit_count);
    virtual int process_value_block(ValueBlock *vblk);

    // local variables used in print_var_def and its helpers
    int cur_opcode, cur_opnd_cnt;

  public:
    PrinterAlpha();

    virtual void start_comment() { fprintf(out, "\t# "); }

    virtual void print_instr(Instr *mi);
    virtual void print_opnd(Opnd o);

    virtual void print_extern_decl(VarSym *vsym);
    virtual void print_file_decl(int fnum, IdString fnam);

    virtual void print_var_def(VarSym *vsym);

    virtual void print_global_decl(FileBlock *fb);
    virtual void print_proc_begin(OptUnit *unit);
    virtual void print_proc_decl(ProcSym*) { }
    virtual void print_proc_entry(OptUnit *unit,
				  int file_no_for_1st_line);
    virtual void print_proc_end(OptUnit *unit);
};

#endif /* ALPHA_PRINTER_H */
