/* file "x86/instr.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "x86/machine_instr.h"
#endif

#include <machine/machine.h>

#include <x86/opcodes.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

using namespace x86;

/* ---------------- Is_* helper routines ------------------- */

bool
is_ldc_x86(Instr *mi)
{
    int o = get_opcode(mi);
    return ((o == MOV) && is_immed(get_src(mi, 0))
	    && !writes_memory(mi));
}

/*
 * is_move_x86() -- Return true for moves that are (potentially)
 * register-to-register.  Rule out a load, ldc, or store, which have the
 * same opcode as a register move: get_src(mi,0) can't be an effective
 * address or immediate value, and the instruction can't write memory.
 *
 * Even rule out cases where the operands have different sizes.  Moving a
 * big source to a small destination is a conversion, not a simple copy.
 * Moving a small source value to a large destination without sign or zero
 * extension is "not done".
 */
bool
is_move_x86(Instr *mi)
{
    int o = get_opcode(mi);
    return (o == MOV)
	&& !reads_memory(mi)
	&& !writes_memory(mi)
	&& !is_immed(get_src(mi, 0))
	&& (get_bit_size(get_type(get_src(mi, 0))) ==
	    get_bit_size(get_type(get_dst(mi, 0))));
}

bool
is_cmove_x86(Instr *)
{
    return false;
}

bool
is_line_x86(Instr *mi)
{
    int o = get_opcode(mi);
    return (o == LOC);
}

bool
is_ubr_x86(Instr *mi)
{
    int o = get_opcode(mi);
    return  ((o == JMP) 
	     && is_null(get_note(mi, k_instr_ret))
	     && is_null(get_note(mi, k_instr_mbr_tgts)));
}

bool
is_cbr_x86(Instr *mi)
{
    int o = get_opcode(mi);
    return ((o >= JA) && (o <= LOOPZ));
}

bool
is_call_x86(Instr *mi)
{
    int o = get_opcode(mi);
    return (o == CALL);
}

bool
is_return_x86(Instr *mi)
{
    int o = get_opcode(mi);
    return (o == RET); 
}

bool
is_binary_exp_x86(Instr *mi)
{
    claim(false, "is_binary_exp_x86 not implemented");
    return false;
}

bool
is_unary_exp_x86(Instr *mi)
{
    claim(false, "is_unary_exp_x86 not implemented");
    return false;
}

bool
is_commutative_x86(Instr *mi)
{
    claim(false, "is_commutative_x86 not implemented");
    return false;
}

/*
 * The meaning of "two operand" instruction is that there is a first source
 * operand that is identical to the destination.  To the assembler, this is
 * implicit, but in a machsuif instruction it is represented explicitly.
 * It must equal the destination operand, however.
 *
 * Given this definition, even the unary operators NOT and NEG are
 * two-operand instructions, even though the assembler sees each as having
 * one operand.
 */
bool
is_two_opnd_x86(Instr *mi)
{
    switch (get_opcode(mi)) {
      case ADC:
      case ADD:
      case AND:
      case DIV:
      case IDIV:
      case IMUL:
      case NEG:
      case NOT:
      case OR:
      case SAL:
      case SAR:
      case SHL:
      case SHR:
      case SUB:
      case XOR:
	return true;
      default:
	return false;
    }
}

bool
reads_memory_x86(Instr *instr)
{
    for (int i = 0; i < srcs_size(instr); i++) 
	if (is_addr(get_src(instr, i)))
	    return (get_opcode(instr) != LEA);
    return false;
}

bool
writes_memory_x86(Instr *instr)
{
    for (int i = 0; i < dsts_size(instr); i++) 
	if (is_addr(get_dst(instr, i)))
	    return true;
    return false;
}
