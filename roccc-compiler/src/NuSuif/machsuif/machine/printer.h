/* file "machine/printer.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef MACHINE_PRINTER_H
#define MACHINE_PRINTER_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "machine/printer.h"
#endif

#include <machine/substrate.h>
#include <machine/opnd.h>
#include <machine/machine_ir.h>

class Printer {
  public:
    virtual ~Printer() { }

    FILE* get_file_ptr() const { return out; }
    void set_file_ptr(FILE* the_file_ptr) { out = the_file_ptr; }

    int  get_Gnum() const { return Gnum; }
    void set_Gnum(int n) { Gnum = n; }

    bool get_omit_unwanted_notes() const { return !omit_unwanted_notes; }
    void set_omit_unwanted_notes(bool omit) { omit_unwanted_notes = omit; }

    virtual void print_notes(FileBlock*);
    virtual void print_notes(Instr*);

    virtual void start_comment() = 0;

    virtual void print_instr(Instr *mi) = 0;
    virtual void print_opnd(Opnd o) = 0;
    virtual void print_predicate(Instr *mi) = 0;

    virtual void print_extern_decl(VarSym *v) = 0;
    virtual void print_file_decl(int fnum, IdString fnam) = 0;

    virtual void print_sym(Sym *s);
    virtual void print_var_def(VarSym *v) = 0;

    virtual void print_global_decl(FileBlock *fb) = 0;
    virtual void print_proc_decl(ProcSym *p) = 0;
    virtual void print_proc_begin(ProcDef *pd) = 0;
    virtual void print_proc_entry(ProcDef *pd, int file_no_for_1st_line) = 0;
    virtual void print_proc_end(ProcDef *pd) = 0;

  //protected:
  public:
    // table of Instr printing functions -- filled in by derived class
    typedef void (Printer::*print_instr_f)(Instr *);
    Vector<print_instr_f> print_instr_table;

    // printing functions that populate the print_instr_table
    virtual void print_instr_alm(Instr *) = 0;
    virtual void print_instr_cti(Instr *) = 0;
    virtual void print_instr_dot(Instr *) = 0;
    virtual void print_instr_label(Instr *) = 0;

    virtual void print_instr_user_defd(Instr *) = 0;

    Printer();

    // helper
    virtual void print_annote(Annote *the_annote);

    // remaining state
    FILE *out;
    int Gnum;
    bool omit_unwanted_notes;
};

Printer* target_printer();

#endif /* MACHINE_PRINTER_H */
