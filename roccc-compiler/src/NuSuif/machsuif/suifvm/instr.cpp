/* file "suifvm/instr.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "suifvm/instr.h"
#endif

#include <machine/machine.h>

#include <suifvm/opcodes.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

using namespace suifvm;

bool
is_ldc_suifvm(Instr *mi)
{
    return mi->get_opcode() == LDC;
}

bool
is_move_suifvm(Instr *mi)
{
    return mi->get_opcode() == MOV;
}

bool
is_cmove_suifvm(Instr *mi)
{
    return false;
}

bool
is_line_suifvm(Instr *mi)
{
    return mi->get_opcode() == MRK && has_note(mi, k_line);
}

bool
is_ubr_suifvm(Instr *mi)
{
    return mi->get_opcode() == JMP
        || mi->get_opcode() == JMPI;
}

bool
is_cbr_suifvm(Instr *mi)
{
    switch (mi->get_opcode()) {
      case BFALSE: case BTRUE:
      case BEQ:    case BNE:
      case BGE:    case BGT:
      case BLE:    case BLT:
	return true;
      default:
	return false;
    }
}

bool
is_call_suifvm(Instr *mi)
{
    return mi->get_opcode() == CAL;
}

bool
is_return_suifvm(Instr *mi)
{
    return mi->get_opcode() == RET;
}

bool
is_binary_exp_suifvm(Instr *mi)
{
    switch (mi->get_opcode()) {
      case ADD: case SUB: case MUL: case DIV:
      case REM: case MOD: 
      case MIN: case MAX:
      /*
      case AND: case IOR: case XOR:
      */
      case BAND: case LAND: 
      case LOR: case BIOR: case BXOR: 

      case ASR: case LSL: case LSR: case ROT:
      case SEQ: case SNE: case SL:  case SLE:
	return true;
      default:
	return false;
    }
}

bool
is_unary_exp_suifvm(Instr *mi)
{
    switch (mi->get_opcode()) {
      case NEG:
      case NOT:
      case ABS:
	return true;
      default:
	return false;
    }
}

bool
is_commutative_suifvm(Instr *mi)
{
    switch (mi->get_opcode()) {
      case ADD: case MUL:
      //case MIN: case MAX:
      //case AND: case IOR: case XOR:
      case MIN2: case MAX2: 
      case BAND: case LAND: 
      case LOR: case BIOR: case BXOR: 


      case SEQ: case SNE:
	return true;
      default:
	return false;
    }
}

bool
reads_memory_suifvm(Instr *instr)
{
    int opcode = get_opcode(instr);

    return opcode == LOD || opcode == MEMCPY;
}

bool
writes_memory_suifvm(Instr *instr)
{
    int opcode = get_opcode(instr);

    return opcode == STR || opcode == MEMCPY;
}

/* is_builtin() -- Return true for an instruction representing
 * a "builtin" operation.  E.g. va_start is implemented as a builtin
 * by some front ends.
 */
bool
is_builtin_suifvm(Instr *instr)
{
    return get_opcode(instr) == ANY
	&& has_note(instr, k_builtin_args);
}
