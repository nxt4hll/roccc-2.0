/* file "alpha/contexts.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef ALPHA_CONTEXT_H
#define ALPHA_CONTEXT_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "alpha/contexts.h"
#endif

#include <machine/substrate.h>

class SuifVmContextAlpha : public SuifVmContext{
  public:
    CodeGen* target_code_gen() const;
};

class MachineContextAlpha : public MachineContext {
  public:
    MachineContextAlpha() { }
    virtual ~MachineContextAlpha() { }
    
    TypeId type_addr() const;
    Printer* target_printer() const;
    CPrinter* target_c_printer() const;
    CodeFin* target_code_fin() const;
    
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
    bool reads_memory(Instr *instr) const;
    bool writes_memory(Instr *instr) const;
    bool is_builtin(Instr *instr) const;

    int opcode_line() const;
    int opcode_ubr() const;
    int opcode_move(TypeId) const;
    int opcode_load(TypeId) const;
    int opcode_store(TypeId) const;
    int opcode_cbr_inverse(int cbr_opcode) const;
    
    bool target_implements(int opcode) const;
    char* opcode_name(int opcode) const;

    int reg_count() const;
    const char* reg_name(int reg) const;
    int reg_width(int reg) const;
    const NatSet* reg_aliases(int reg) const;
    const NatSet* reg_allocables(bool maximals = false) const;
    const NatSet* reg_caller_saves(bool maximals = false) const;
    const NatSet* reg_callee_saves(bool maximals = false) const;
    // use default implementation of reg_maximal

    InstrHandle reg_fill (Opnd dst, Opnd src, InstrHandle marker, bool) const;
    InstrHandle reg_spill(Opnd dst, Opnd src, InstrHandle marker, bool) const;

    int reg_class_count() const;
    const NatSet* reg_members(RegClassId) const;
    void reg_classify(Instr*, OpndCatalog*, RegClassMap*) const;
    RegClassId reg_class_intersection(RegClassId, RegClassId) const;
    int reg_choice(RegClassId, const NatSet *pool, const NatSet *excluded,
		   bool rotate) const;
};

class AlphaContext : public virtual Context,
		     public virtual MachineContextAlpha,
		     public virtual SuifVmContextAlpha
{ };

#endif /* ALPHA_CONTEXT_H */
