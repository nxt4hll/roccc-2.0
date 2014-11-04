/* file "machine/contexts.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef MACHINE_CONTEXT_H
#define MACHINE_CONTEXT_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "machine/contexts.h"
#endif

#include <machine/substrate.h>
#include <machine/problems.h>
#include <machine/opnd.h>
#include <machine/instr.h>
#include <machine/reg_info.h>
#include <machine/code_fin.h>
#include <machine/printer.h>
#include <machine/c_printer.h>

class Context {
  public:
    Context() { }
    virtual ~Context() { }
};

class MachineContext {
  public:
    MachineContext();
    virtual ~MachineContext();

    
    virtual TypeId type_addr() const = 0;

    
    virtual int reg_count() const
	{ claim(false); return -1; }
    virtual const char* reg_name(int reg) const
	{ claim(false); return NULL; }
    virtual int reg_width(int reg) const
	{ claim(false); return -1; }
    virtual const NatSet* reg_aliases(int reg) const
	{ claim(false); return NULL; }

    virtual int reg_class_count() const
	{ claim(false); return -1; }
    virtual const NatSet* reg_members(RegClassId) const
	{ claim(false); return NULL; }
    virtual const NatSet* reg_allocables(bool maximals = false) const
	{ claim(false); return NULL; }
    virtual const NatSet* reg_caller_saves(bool maximals = false) const
	{ claim(false); return NULL; }
    virtual const NatSet* reg_callee_saves(bool maximals = false) const
	{ claim(false); return NULL; }
    virtual int reg_maximal(int reg) const
	{ return reg; }
    virtual InstrHandle reg_fill(Opnd dst, Opnd src, InstrHandle marker,
				 bool post_reg_alloc = false) const
	{ claim(false); return marker; }
    virtual InstrHandle reg_spill(Opnd dst, Opnd src, InstrHandle marker,
				  bool post_reg_alloc = false) const
	{ claim(false); return marker; }
    virtual void reg_classify(Instr*, OpndCatalog*, RegClassMap*) const
	{ claim(false); }
    virtual RegClassId reg_class_intersection(RegClassId, RegClassId) const
	{ claim(false); return REG_CLASS_NONE; }
    virtual int reg_choice(RegClassId, const NatSet *pool,
			   const NatSet *excluded, bool rotate) const
	{ claim(false); return -1; }

    
    virtual Printer* target_printer() const = 0;
    virtual CPrinter* target_c_printer() const = 0;

    
    virtual CodeFin* target_code_fin() const = 0;

    
    virtual bool is_ldc(Instr*) const = 0;
    virtual bool is_move(Instr*) const = 0;
    virtual bool is_cmove(Instr*) const = 0;
    virtual bool is_predicated(Instr*) const { return false; }
    virtual bool is_line(Instr*) const = 0;
    virtual bool is_ubr(Instr*) const = 0;
    virtual bool is_cbr(Instr*) const = 0;
    virtual bool is_call(Instr*) const = 0;
    virtual bool is_return(Instr*) const = 0;
    virtual bool is_binary_exp(Instr*) const = 0;
    virtual bool is_unary_exp(Instr*) const = 0;
    virtual bool is_commutative(Instr*) const = 0;
    virtual bool is_two_opnd(Instr*) const = 0;
    virtual bool reads_memory(Instr*) const = 0;
    virtual bool writes_memory(Instr*) const = 0;
    virtual bool is_builtin(Instr*) const = 0;

    
    virtual int opcode_line() const = 0;
    virtual int opcode_ubr() const = 0;
    virtual int opcode_move(TypeId) const = 0;
    virtual int opcode_load(TypeId) const = 0;
    virtual int opcode_store(TypeId) const = 0;
    virtual int opcode_cbr_inverse(int cbr_opcode) const = 0;

    
    virtual bool target_implements(int opcode) const = 0;
    virtual char* opcode_name(int opcode) const = 0;

  protected:
    
    mutable Printer *cached_printer;
    mutable CPrinter *cached_c_printer;
    mutable CodeFin *cached_code_fin;
};

extern Context *the_context;

Context* target_context(FileBlock*);
Context* find_context(IdString lib_name);


extern Map<IdString,Context*(*)()> the_context_creator_registry;

#endif /* MACHINE_CONTEXT_H */
