/* file "suifvm/contexts.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef SUIFVM_CONTEXT_H
#define SUIFVM_CONTEXT_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "suifvm/contexts.h"
#endif

#include <machine/machine.h>
#include <suifvm/suifvm.h>

class SuifVmContext {
  public:
    SuifVmContext();
    virtual ~SuifVmContext();

    virtual CodeGen* target_code_gen() const = 0;

  protected:
    
    mutable CodeGen *cached_code_gen;
};

class MachineContextSuifVm
      : public virtual Context, public virtual MachineContext {
  public:
    MachineContextSuifVm() { }
    virtual ~MachineContextSuifVm() { }

    
    TypeId type_addr() const;

    
    Printer* target_printer() const;
    CPrinter* target_c_printer() const;

    
    CodeFin* target_code_fin() const { claim(false); return NULL; }

    
    bool is_ldc(Instr*) const;
    bool is_move(Instr*) const;
    bool is_cmove(Instr*) const;
    bool is_line(Instr*) const;
    bool is_ubr(Instr*) const;
    bool is_cbr(Instr*) const;
    bool is_call(Instr*) const;
    bool is_return(Instr*) const;
    bool is_binary_exp(Instr*) const;
    bool is_unary_exp(Instr*) const;
    bool is_commutative(Instr*) const;
    bool is_two_opnd(Instr*) const;
    bool reads_memory(Instr*) const;
    bool writes_memory(Instr*) const;
    bool is_builtin(Instr*) const;

    
    int opcode_line() const;
    int opcode_ubr() const;
    int opcode_move(TypeId) const;
    int opcode_load(TypeId) const;
    int opcode_store(TypeId) const;
    int opcode_cbr_inverse(int cbr_opcode) const;

    
    bool target_implements(int opcode) const;
    char* opcode_name(int opcode) const;

    
    int reg_count() const
	{ return 0; }
    const NatSet* reg_allocables(bool maximals = false) const
	{ static NatSetSparse empty; return &empty; }
    const NatSet* reg_caller_saves(bool maximals = false) const
	{ static NatSetSparse empty; return &empty; }
    const NatSet* reg_callee_saves(bool maximals = false) const
	{ static NatSetSparse empty; return &empty; }
};

#endif /* SUIFVM_CONTEXT_H */
