/* file "x86/contexts.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "x86/contexts.h"
#endif

#include <machine/machine.h>

#include <x86/reg_info.h>
#include <x86/opcodes.h>
#include <x86/instr.h>
#include <x86/code_gen.h>
#include <x86/code_fin.h>
#include <x86/printer.h>
#include <x86/contexts.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif


/* --------------------------  SuifVmContextX86  -------------------------- */


CodeGen*
SuifVmContextX86::target_code_gen() const
{
    if (cached_code_gen)
	return cached_code_gen;

    cached_code_gen = new CodeGenX86;
    return cached_code_gen;
}

/* ------------------------  MachineContextX86  -------------------------- */

TypeId
MachineContextX86::type_addr() const
{
    return type_p32;
}

CodeFin*
MachineContextX86::target_code_fin() const
{
    if (cached_code_fin)
	return cached_code_fin;

    cached_code_fin = new CodeFinX86;
    return cached_code_fin;
}

Printer*
MachineContextX86::target_printer() const
{
    if (cached_printer)
	return cached_printer;

    cached_printer = new PrinterX86;
    return cached_printer;
}


CPrinter*
MachineContextX86::target_c_printer() const
{
    claim(false, "Printing to C is not implemented for x86");
    return NULL;
}

bool
MachineContextX86::is_ldc(Instr *mi) const
{
    return is_ldc_x86(mi);
}

bool
MachineContextX86::is_move(Instr *mi) const
{
    return is_move_x86(mi);
}

bool
MachineContextX86::is_cmove(Instr *mi) const
{
    return is_cmove_x86(mi);
}

bool
MachineContextX86::is_line(Instr *mi) const
{
    return is_line_x86(mi);
}

bool
MachineContextX86::is_ubr(Instr *mi) const
{
    return is_ubr_x86(mi);
}

bool
MachineContextX86::is_cbr(Instr *mi) const
{
    return is_cbr_x86(mi);
}

bool
MachineContextX86::is_call(Instr *mi) const
{
    return is_call_x86(mi);
}

bool
MachineContextX86::is_return(Instr *mi) const
{
    return is_return_x86(mi);
}

bool
MachineContextX86::is_binary_exp(Instr *mi) const
{
    return is_binary_exp_x86(mi);
}

bool
MachineContextX86::is_unary_exp(Instr *mi) const
{
    return is_unary_exp_x86(mi);
}

bool
MachineContextX86::is_commutative(Instr *mi) const
{
    return is_commutative_x86(mi);
}

bool
MachineContextX86::is_two_opnd(Instr *mi) const
{
    return is_two_opnd_x86(mi);
}

bool
MachineContextX86::reads_memory(Instr *mi) const
{
    return reads_memory_x86(mi);
}

bool
MachineContextX86::writes_memory(Instr *mi) const
{
    return writes_memory_x86(mi);
}

bool
MachineContextX86::is_builtin(Instr *mi) const
{
    return false;		// no "builtins" currently
}

char*
MachineContextX86::opcode_name(int opcode) const
{
    return opcode_name_x86(opcode);
}

bool
MachineContextX86::target_implements(int opcode) const
{
    return target_implements_x86(opcode);
}

int
MachineContextX86::opcode_line() const
{
    return opcode_line_x86();
}

int
MachineContextX86::opcode_ubr() const
{
    return opcode_ubr_x86();
}

int
MachineContextX86::opcode_move(TypeId type) const
{
    return opcode_move_x86(type);
}

int
MachineContextX86::opcode_load(TypeId type) const
{
    return opcode_load_x86(type);
}

int
MachineContextX86::opcode_store(TypeId type) const
{
    return opcode_store_x86(type);
}

int
MachineContextX86::opcode_cbr_inverse(int opcode) const
{
    return opcode_cbr_inverse_x86(opcode);
}


int
MachineContextX86::reg_count() const
{
    return reg_count_x86();
}

const char*
MachineContextX86::reg_name(int reg) const
{
    return reg_name_x86(reg);
}

int
MachineContextX86::reg_width(int reg) const
{
    return reg_width_x86(reg);
}

const NatSet*
MachineContextX86::reg_aliases(int reg) const
{
    return reg_aliases_x86(reg);
}

const NatSet*
MachineContextX86::reg_allocables(bool maximals) const
{
    return reg_allocables_x86(maximals);
}

const NatSet*
MachineContextX86::reg_caller_saves(bool maximals) const
{
    return reg_caller_saves_x86(maximals);
}

const NatSet*
MachineContextX86::reg_callee_saves(bool maximals) const
{
    return reg_callee_saves_x86(maximals);
}

int
MachineContextX86::reg_maximal(int reg) const
{
    return reg_maximal_x86(reg);
}

InstrHandle
MachineContextX86::reg_fill(Opnd dst, Opnd src, InstrHandle marker,
			    bool) const
{
    return reg_fill_x86(dst, src, marker);
}

InstrHandle
MachineContextX86::reg_spill(Opnd dst, Opnd src, InstrHandle marker,
			     bool) const
{
    return reg_spill_x86(dst, src, marker);
}

int
MachineContextX86::reg_class_count() const
{
    return reg_class_count_x86();
}

const NatSet*
MachineContextX86::reg_members(RegClassId class_id) const
{
    return reg_members_x86(class_id);
}

void
MachineContextX86::reg_classify(Instr *instr, OpndCatalog *catalog,
				RegClassMap *class_map) const
{
    reg_classify_x86(instr, catalog, class_map);
}

RegClassId
MachineContextX86::reg_class_intersection(RegClassId c1, RegClassId c2) const
{
    return reg_class_intersection_x86(c1, c2);
}

int
MachineContextX86::reg_choice(RegClassId class_id, const NatSet *pool,
			      const NatSet *excluded, bool rotate) const
{
    return reg_choice_x86(class_id, pool, excluded, rotate);
}
