/* file "alpha/instr.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "alpha/instr.h"
#endif

#include <machine/machine.h>

#include <alpha/opcodes.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

using namespace alpha;


/* ---------------- Is_* helper routines ------------------- */

bool
is_ldc_alpha(Instr *mi)
{
    int o = get_opcode(mi);
    return ((o == LDIL) || (o == LDIQ)
	|| ((o >= LDIF) && (o <= LDIT))); 
}

bool
is_move_alpha(Instr *mi)
{
    int o = get_opcode(mi);
    return ((o == MOV) || (o == FMOV));
}

bool
is_cmove_alpha(Instr *mi)
{
    int o = get_opcode(mi);
    return (((o >= CMOVEQ) && (o <= CMOVLBS))
	|| ((o >= FCMOVEQ) && (o <= CMOVGE)));
}

bool
is_line_alpha(Instr *mi)
{
    int o = get_opcode(mi);
    if (o != LOC)
	return false;
    claim(!is_null(get_note(mi, k_line)),
	  "Alpha .loc instruction lacks k_line annotation");
    return true;
}

bool
is_ubr_alpha(Instr *mi)
{
    int o = get_opcode(mi);
    return  (o == BR)
	|| ((o == JMP) && (is_null(get_note(mi, k_instr_mbr_tgts)))); 
}

bool
is_cbr_alpha(Instr *mi)
{
    int o = get_opcode(mi);
    return (((o >= BEQ) && (o <= BNE_T))
	|| ((o >= FBEQ) && (o <= FBGE))); 
}

bool
is_call_alpha(Instr *mi)
{
    int o = get_opcode(mi);
    return ((o == BSR) || (o == JSR) || (o == JSR_COROUTINE)); 
}

bool
is_return_alpha(Instr *mi)
{
    int o = get_opcode(mi);
    return (o == RET); 
}

bool
is_binary_exp_alpha(Instr *mi)
{
    switch (get_opcode(mi)) {
      case ADDL:
      case ADDLV:
      case ADDQ:
      case ADDQV:
      case AND:
      case ANDNOT:
      case BIC:
      case BIS:
      case CMPEQ:
      case DIVL:
      case DIVLU:
      case DIVQ:
      case DIVQU:
      case EQV:
      case MULL:
      case MULLV:
      case MULQ:
      case MULQV:
      case OR:
      case ORNOT:
      case REML:
      case REMLU:
      case REMQ:
      case REMQU:
      case S4ADDL:
      case S4ADDQ:
      case S8ADDL:
      case S8ADDQ:
      case S4SUBL:
      case S4SUBQ:
      case S8SUBL:
      case S8SUBQ:
      case SLL:
      case SRA:
      case SRL:
      case SUBL:
      case SUBLV:
      case SUBQ:
      case SUBQV:
      case UMULH:
      case XOR:
      case XORNOT:
      case CMPBGE:
      case EXTBL:
      case EXTWL:
      case EXTLL:
      case EXTQL:
      case EXTWH:
      case EXTLH:
      case EXTQH:
      case INSBL:
      case INSWL:
      case INSLL:
      case INSQL:
      case INSWH:
      case INSLH:
      case INSQH:
      case MSKBL:
      case MSKWL:
      case MSKLL:
      case MSKQL:
      case MSKWH:
      case MSKLH:
      case MSKQH:
      case ZAP:
      case ZAPNOT:
      case ADDF:
      case ADDG:
      case ADDS:
      case ADDT:
      case CPYS:
      case CPYSN:
      case CPYSE:
      case DIVF:
      case DIVG:
      case DIVS:
      case DIVT:
      case MULF:
      case MULG:
      case MULS:
      case MULT:
      case SUBF:
      case SUBG:
      case SUBS:
      case SUBT:
      case CMPGEQ:
      case CMPGLT:
      case CMPGLE:
      case CMPTEQ:
      case CMPTLT:
      case CMPTLE:
      case CMPTUN:
	return true;
      default:
	return false;
    }
}

bool
is_unary_exp_alpha(Instr *mi)
{
    switch (get_opcode(mi)) {
      case NEGL:
      case NEGLV:
      case NEGQ:
      case NEGQV:
      case NOT:
      case CVTQL:
      case CVTLQ:
      case CVTGQ:
      case CVTTQ:
      case CVTQF:
      case CVTQG:
      case CVTQS:
      case CVTQT:
      case CVTDG:
      case CVTGD:
      case CVTGF:
      case CVTTS:
      case CVTST:
      case FABS:
      case FNEG:
      case NEGF:
      case NEGG:
      case NEGS:
      case NEGT:
	return true;
      default:
	return false;
    }
}

bool
is_commutative_alpha(Instr *mi)
{
    switch (get_opcode(mi)) {
      case ADDL:
      case ADDLV:
      case ADDQ:
      case ADDQV:
      case AND:
      case BIS:
      case CMPEQ:
      case EQV:
      case MULL:
      case MULLV:
      case MULQ:
      case MULQV:
      case OR:
      case UMULH:
      case XOR:
	return !is_immed(get_src(mi, 1));
      case ADDF:
      case ADDG:
      case ADDS:
      case ADDT:
      case MULF:
      case MULG:
      case MULS:
      case MULT:
      case CMPGEQ:
      case CMPTEQ:
      case CMPTUN:
	return true;
      default:
	return false;
    }
}

bool
reads_memory_alpha(Instr *instr)
{
    switch (get_opcode(instr)) {
      case LDB:
      case LDBU:
      case LDW:
      case LDWU:
      case LDL:
      case LDL_L:
      case LDQ:
      case LDQ_L:
      case LDQ_U:
      case ULDW:
      case ULDWU:
      case ULDL:
      case ULDQ:
      case LDF:
      case LDG:
      case LDS:
      case LDT:
	return true;
      default:
	return false;
    }
}

bool
writes_memory_alpha(Instr *instr)
{
    switch (get_opcode(instr)) {
      case STB:
      case STW:
      case STL:
      case STL_C:
      case STQ:
      case STQ_C:
      case STQ_U:
      case USTW:
      case USTL:
      case USTQ:
      case STF:
      case STG:
      case STS:
      case STT:
	return true;
      default:
	return false;
    }
}
