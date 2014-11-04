/* file "alpha/contexts.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "alpha/contexts.h"
#endif

#include <machine/machine.h>

#include <alpha/opcodes.h>
#include <alpha/reg_info.h>
#include <alpha/instr.h>
#include <alpha/printer.h>
#include <alpha/code_gen.h>
#include <alpha/code_fin.h>
#include <alpha/contexts.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif


/* --------------------------  SuifVmContextAlpha  -------------------------- */

CodeGen*
SuifVmContextAlpha::target_code_gen() const
{
    if (cached_code_gen)
	return cached_code_gen;

    cached_code_gen = new CodeGenAlpha;
    return cached_code_gen;
}

/* ------------------------  MachineContextAlpha  -------------------------- */

TypeId
MachineContextAlpha::type_addr() const
{
    return type_p64;
}

Printer*
MachineContextAlpha::target_printer() const
{
    if (cached_printer)
	return cached_printer;

    cached_printer = new PrinterAlpha;
    return cached_printer;
}

CPrinter*
MachineContextAlpha::target_c_printer() const
{
    claim(false, "Printing to C is not implemented for Alpha");
    return NULL;
}

CodeFin*
MachineContextAlpha::target_code_fin() const
{
    if (cached_code_fin)
	return cached_code_fin;

    cached_code_fin = new CodeFinAlpha;
    return cached_code_fin;
}


bool
MachineContextAlpha::is_ldc(Instr *mi) const
{
    return is_ldc_alpha(mi);
}

bool
MachineContextAlpha::is_move(Instr *mi) const
{
    return is_move_alpha(mi);
}

bool
MachineContextAlpha::is_cmove(Instr *mi) const
{
    return is_cmove_alpha(mi);
}

bool
MachineContextAlpha::is_line(Instr *mi) const
{
    return is_line_alpha(mi);
}

bool
MachineContextAlpha::is_ubr(Instr *mi) const
{
    return is_ubr_alpha(mi);
}

bool
MachineContextAlpha::is_cbr(Instr *mi) const
{
    return is_cbr_alpha(mi);
}

bool
MachineContextAlpha::is_call(Instr *mi) const
{
    return is_call_alpha(mi);
}

bool
MachineContextAlpha::is_return(Instr *mi) const
{
    return is_return_alpha(mi);
}

bool
MachineContextAlpha::is_binary_exp(Instr *mi) const
{
    return is_binary_exp_alpha(mi);
}

bool
MachineContextAlpha::is_unary_exp(Instr *mi) const
{
    return is_unary_exp_alpha(mi);
}

bool
MachineContextAlpha::is_commutative(Instr *mi) const
{
    return is_commutative_alpha(mi);
}

bool
MachineContextAlpha::is_two_opnd(Instr *mi) const
{
    return false;		// no two-address instrs
}

bool
MachineContextAlpha::reads_memory(Instr *mi) const
{
    return reads_memory_alpha(mi);
}

bool
MachineContextAlpha::writes_memory(Instr *mi) const
{
    return writes_memory_alpha(mi);
}

bool
MachineContextAlpha::is_builtin(Instr *mi) const
{
    return false;		// no builtins in Alpha code
}

char*
MachineContextAlpha::opcode_name(int opcode) const
{
    return opcode_name_alpha(opcode);
}

bool
MachineContextAlpha::target_implements(int opcode) const
{
    return target_implements_alpha(opcode);
}

int
MachineContextAlpha::opcode_line() const
{
    return opcode_line_alpha();
}

int
MachineContextAlpha::opcode_ubr() const
{
    return opcode_ubr_alpha();
}

int
MachineContextAlpha::opcode_move(TypeId type) const
{
    return opcode_move_alpha(type);
}

int
MachineContextAlpha::opcode_load(TypeId type) const
{
    return opcode_load_alpha(type);
}

int
MachineContextAlpha::opcode_store(TypeId type) const
{
    return opcode_store_alpha(type);
}

int
MachineContextAlpha::opcode_cbr_inverse(int opcode) const
{
    return opcode_cbr_inverse_alpha(opcode);
}

int
MachineContextAlpha::reg_count() const
{
    return reg_count_alpha();
}

const char*
MachineContextAlpha::reg_name(int reg) const
{
    return reg_name_alpha(reg);
}

int
MachineContextAlpha::reg_width(int reg) const
{
    return reg_width_alpha(reg);
}

const NatSet*
MachineContextAlpha::reg_aliases(int reg) const
{
    return reg_aliases_alpha(reg);
}

const NatSet*
MachineContextAlpha::reg_allocables(bool) const
{
    return reg_allocables_alpha(); 
}

const NatSet*
MachineContextAlpha::reg_caller_saves(bool) const
{
    return reg_caller_saves_alpha(); 
}

const NatSet*
MachineContextAlpha::reg_callee_saves(bool) const
{
    return reg_callee_saves_alpha(); 
}

InstrHandle
MachineContextAlpha::reg_fill(Opnd dst, Opnd src, InstrHandle marker,
			      bool) const
{
    return reg_fill_alpha(dst, src, marker);
}

InstrHandle
MachineContextAlpha::reg_spill(Opnd dst, Opnd src, InstrHandle marker,
			       bool) const
{
    return reg_spill_alpha(dst, src, marker);
}

int
MachineContextAlpha::reg_class_count() const
{
    return reg_class_count_alpha();
}

const NatSet*
MachineContextAlpha::reg_members(RegClassId class_id) const
{
    return reg_members_alpha(class_id);
}

void
MachineContextAlpha::reg_classify(Instr *instr, OpndCatalog *catalog,
				  RegClassMap *class_map) const
{
    reg_classify_alpha(instr, catalog, class_map);
}

RegClassId
MachineContextAlpha::reg_class_intersection(RegClassId c1, RegClassId c2) const
{
    return reg_class_intersection_alpha(c1, c2);
}

int
MachineContextAlpha::reg_choice(RegClassId class_id, const NatSet *pool,
				const NatSet *excluded, bool rotate) const
{
    return reg_choice_alpha(class_id, pool, excluded, rotate);
}
