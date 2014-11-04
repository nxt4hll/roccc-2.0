/* file "ia64/contexts.cpp" */

/*
    Copyright (c) 2002 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "ia64/contexts.h"
#endif

#include <machine/machine.h>

#include <ia64/opcodes.h>
#include <ia64/reg_info.h>
#include <ia64/instr.h>
#include <ia64/printer.h>
#include <ia64/code_gen.h>
#include <ia64/code_fin.h>
#include <ia64/contexts.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif


/* --------------------------  SuifVmContextIa64  -------------------------- */

CodeGen*
SuifVmContextIa64::target_code_gen() const
{
    if (cached_code_gen)
	return cached_code_gen;

    cached_code_gen = new CodeGenIa64;
    return cached_code_gen;
}

/* ------------------------  MachineContextIa64  -------------------------- */

TypeId
MachineContextIa64::type_addr() const
{
    return type_p64;
}

Printer*
MachineContextIa64::target_printer() const
{
    if (cached_printer)
	return cached_printer;

    cached_printer = new PrinterIa64;
    return cached_printer;
}

CPrinter*
MachineContextIa64::target_c_printer() const
{
    claim(false, "Printing to C is not implemented for Ia64");
    return NULL;
}

CodeFin*
MachineContextIa64::target_code_fin() const
{
    if (cached_code_fin)
	return cached_code_fin;

    cached_code_fin = new CodeFinIa64;
    return cached_code_fin;
}


bool
MachineContextIa64::is_ldc(Instr *mi) const
{
    return is_ldc_ia64(mi);
}

bool
MachineContextIa64::is_move(Instr *mi) const
{
    return is_move_ia64(mi);
}

bool
MachineContextIa64::is_cmove(Instr *mi) const
{
    return is_cmove_ia64(mi);
}

bool
MachineContextIa64::is_predicated(Instr *mi) const
{
    return is_predicated_ia64(mi);
}

bool
MachineContextIa64::is_line(Instr *mi) const
{
    return is_line_ia64(mi);
}

bool
MachineContextIa64::is_ubr(Instr *mi) const
{
    return is_ubr_ia64(mi);
}

bool
MachineContextIa64::is_cbr(Instr *mi) const
{
    return is_cbr_ia64(mi);
}

bool
MachineContextIa64::is_call(Instr *mi) const
{
    return is_call_ia64(mi);
}

bool
MachineContextIa64::is_return(Instr *mi) const
{
    return is_return_ia64(mi);
}

bool
MachineContextIa64::is_binary_exp(Instr *mi) const
{
    return is_binary_exp_ia64(mi);
}

bool
MachineContextIa64::is_unary_exp(Instr *mi) const
{
    return is_unary_exp_ia64(mi);
}

bool
MachineContextIa64::is_commutative(Instr *mi) const
{
    return is_commutative_ia64(mi);
}

bool
MachineContextIa64::is_two_opnd(Instr *mi) const
{
    return is_two_opnd_ia64(mi);
}

bool
MachineContextIa64::reads_memory(Instr *mi) const
{
    return reads_memory_ia64(mi);
}

bool
MachineContextIa64::writes_memory(Instr *mi) const
{
    return writes_memory_ia64(mi);
}

bool
MachineContextIa64::is_builtin(Instr *mi) const
{
    return false;		// no builtins currently
}

char*
MachineContextIa64::opcode_name(int opcode) const
{ 
    return opcode_name_ia64(opcode);
}

bool
MachineContextIa64::target_implements(int opcode) const
{
    return target_implements_ia64(opcode);
}

int
MachineContextIa64::opcode_line() const
{
    return opcode_line_ia64();
}

int
MachineContextIa64::opcode_ubr() const
{
    return opcode_ubr_ia64();
}

int
MachineContextIa64::opcode_move(TypeId type) const
{
    return opcode_move_ia64(type);
}

int
MachineContextIa64::opcode_load(TypeId type) const
{
    return opcode_load_ia64(type);
}

int
MachineContextIa64::opcode_store(TypeId type) const
{
    return opcode_store_ia64(type);
}

int
MachineContextIa64::opcode_cbr_inverse(int opcode) const
{
    return opcode_cbr_inverse_ia64(opcode);
}

int
MachineContextIa64::reg_count() const
{
    return reg_count_ia64();
}

const char*
MachineContextIa64::reg_name(int reg) const
{
    return reg_name_ia64(reg);
}

int
MachineContextIa64::reg_width(int reg) const
{
    return reg_width_ia64(reg);
}

const NatSet*
MachineContextIa64::reg_aliases(int reg) const
{
    return reg_aliases_ia64(reg);
}

const NatSet*
MachineContextIa64::reg_allocables(bool) const
{
    return reg_allocables_ia64(); 
}

const NatSet*
MachineContextIa64::reg_caller_saves(bool) const
{
    return reg_caller_saves_ia64(); 
}

const NatSet*
MachineContextIa64::reg_callee_saves(bool) const
{
    return reg_callee_saves_ia64(); 
}

InstrHandle
MachineContextIa64::reg_fill(Opnd dst, Opnd src, InstrHandle marker,
			     bool post_reg_alloc) const
{
    return reg_fill_ia64(dst, src, marker, post_reg_alloc);
}

InstrHandle
MachineContextIa64::reg_spill(Opnd dst, Opnd src, InstrHandle marker,
			      bool post_reg_alloc) const
{
    return reg_spill_ia64(dst, src, marker, post_reg_alloc);
}

int
MachineContextIa64::reg_class_count() const
{
    return reg_class_count_ia64();
}

const NatSet*
MachineContextIa64::reg_members(RegClassId class_id) const
{
    return reg_members_ia64(class_id);
}

void
MachineContextIa64::reg_classify(Instr *instr, OpndCatalog *catalog,
				 RegClassMap *class_map) const
{
    reg_classify_ia64(instr, catalog, class_map);
}

RegClassId
MachineContextIa64::reg_class_intersection(RegClassId c1, RegClassId c2) const
{
    return reg_class_intersection_ia64(c1, c2);
}

int
MachineContextIa64::reg_choice(RegClassId class_id, const NatSet *pool,
			       const NatSet *excluded, bool rotate) const
{
    return reg_choice_ia64(class_id, pool, excluded, rotate);
}
