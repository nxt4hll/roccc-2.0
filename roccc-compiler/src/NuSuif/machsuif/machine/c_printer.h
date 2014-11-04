/* file "machine/c_printer.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef MACHINE_CPRINTER_H
#define MACHINE_CPRINTER_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "machine/cprinter.h"
#endif

#include <machine/substrate.h>
#include <machine/machine_ir.h>

class Declarator;

class CPrinter {
  public:
    virtual ~CPrinter();

    void clear();
    FILE *get_file_ptr() { return out; }
    void set_file_ptr(FILE *the_file_ptr) { out = the_file_ptr; }

    bool get_omit_unwanted_notes() const { return !omit_unwanted_notes; }
    void set_omit_unwanted_notes(bool omit) { omit_unwanted_notes = omit; }

    virtual void print_notes(FileBlock*);
    virtual void print_notes(Instr*);

    virtual void print_instr(Instr *mi) = 0;
    virtual void print_opnd(Opnd o);
    virtual void print_type(TypeId t);

    virtual void print_sym(Sym *s);
    virtual void print_sym_decl(Sym *s);
    virtual void print_sym_decl(char *s, TypeId t);
    virtual void print_var_def(VarSym *v, bool no_init);

    virtual void print_global_decl(FileBlock *fb) = 0;
    virtual void print_proc_decl(ProcSym *p);
    virtual void print_proc_begin(ProcDef *pd);

  //protected:
  public:
    // table of instr printing functions -- filled in by derived class
    typedef void (CPrinter::*print_instr_f)(Instr*);
    print_instr_f *print_instr_table;

    // instr printing functions that populate the print_instr_table
    virtual void print_instr_alm(Instr*) = 0;
    virtual void print_instr_cti(Instr*) = 0;
    virtual void print_instr_dot(Instr*) = 0;
    virtual void print_instr_label(Instr*);

    virtual void print_instr_user_defd(Instr*) = 0;

    // helper methods

    virtual void print_annote(Annote*);

    virtual void print_immed(Opnd);
    virtual void print_addr(Opnd, TypeId goal = 0, int context = ANY);
    virtual void print_addr_disp(Opnd addr, Opnd disp, TypeId goal,
				 int context, char *op);

    virtual bool process_value_block(ValueBlock*, TypeId);

    virtual void print_decl(TypeId, const Declarator&);
    virtual void print_pointer_cast(TypeId referent);
    virtual bool print_type_ref(TypeId, const char *keyword);
    virtual void print_group_def(TypeId);
    virtual void print_enum_def (TypeId);
    virtual void print_atomic_type(TypeId);

    virtual void print_string_literal(IdString literal);

    CPrinter();

    FILE *out;
    bool omit_unwanted_notes;

    List<TypeId> noted_types;
    int next_type_tag;

    // syntactic contexts
    enum { ANY, ASSIGN, BINARY, UNARY, PRIMARY };
};

CPrinter* target_c_printer();

#endif /* MACHINE_CPRINTER_H */
