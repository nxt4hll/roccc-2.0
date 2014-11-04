/* file "ia64/instr.cpp" */

/*
    Copyright (c) 2002 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "ia64/instr.h"
#endif

#include <machine/machine.h>

#include <ia64/opcodes.h>
#include <ia64/reg_info.h>
#include <ia64/instr.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

using namespace ia64;


/* ---------------- is_* helper routines ------------------- */

bool
is_ldc_ia64(Instr *mi)
{
    int o = get_opcode(mi);
    int s = get_stem(o);
    return ((s == MOV_IMM) || (s == MOVL));
}

/*
 *  Unconditional register-to-register copy within a single register file.
 */
bool
is_move_ia64(Instr *mi)
{
    int o = get_opcode(mi);
    int s = get_stem(o);
    if (((s == MOV_GR) || (s == MOV_FR)) && !is_predicated(mi)) return true;
    else return false;
}

bool
is_cmove_ia64(Instr *mi)
{
    int o = get_opcode(mi);
    int s = get_stem(o);
    if ((s >= MOV_AR) && (s <= MOVL) && is_predicated(mi)) return true;
    else return false;
}

bool
has_qpred(Instr *mi)
{
    if (srcs_size(mi) < 1) return false;

    int loc = srcs_size(mi) - 1;
    Opnd predOpnd = get_src(mi, loc);
    if (is_preg(predOpnd)) return true;
    else return false;
}

/*
 * Set qualifying predicate
 *      This routine adds a qualifying predicate to the instruction
 *      i.e. (qp) mov dst = src
 */
void
set_qpred(Instr *mi, Opnd qp)
{
    claim(is_preg(qp), "Attempting to add qp that isn't a preg");
    if (has_qpred(mi))
      set_src(mi, (srcs_size(mi)-1), qp);
    else
      append_src(mi, qp);
}

/*
 * Returns the qualifying predicate of the instruction or an error
 */
Opnd 
get_qpred(Instr *mi)
{
    Opnd predOpnd;
    if (has_qpred(mi)) 
        predOpnd = get_src(mi, (srcs_size(mi)-1));
    else 
       claim(false, "Oops, no predicate set on instruction!");
    return predOpnd;
}

bool
is_predicated_ia64(Instr *mi)
{
    if (!has_qpred(mi)) return false;
    Opnd predOpnd = get_qpred(mi);
    if (predOpnd != opnd_pr_const1) return true;
    else return false;
}

bool
is_line_ia64(Instr *mi)
{
    int o = get_opcode(mi);
    int s = get_stem(o);
    return (s == LN);
}

bool
is_ubr_ia64(Instr *mi)
{
    int o = get_opcode(mi);
    int s = get_stem(o);
    return  (((s == BR) || (s == BRL)) &&
	     !is_predicated(mi) &&
	     !is_call_ia64(mi) &&
	     !is_return_ia64(mi) &&
	     !is_mbr(mi));
}

bool
is_cbr_ia64(Instr *mi)
{
    int o = get_opcode(mi);
    int s = get_stem(o);
    return  (((s == BR) || (s == BRL)) &&
	     is_predicated(mi) &&
	     !is_call_ia64(mi) &&
	     !is_return_ia64(mi) &&
	     !is_mbr(mi));
}

bool
is_call_ia64(Instr *mi)
{
    int o = get_opcode(mi);
    int s = get_stem(o);
    if ((s == BR) || (s == BRL)) {
      if (get_br_ext(o, 1) == BTYPE_CALL) return true;
    }
    return false;
}

bool
is_return_ia64(Instr *mi)
{
    int o = get_opcode(mi);
    int s = get_stem(o);
    if ((s == BR) || (s == BRL)) {
      if (get_br_ext(o, 1) == BTYPE_RET) return true;
    }
    else if (s == RFI) return true;
    return false;
}

bool
is_triary_exp_ia64(Instr *mi)
{
    int o = get_opcode(mi);
    int s = get_stem(o);
    switch (s) {
      case FMA: case FMS: case FNMA: case FPMA: case FPMS:
      case FPNMA: case FSELECT: case PSHLADD: case PSHRADD:
      case SHLADD: case SHLADDP4: case SHRP: case XMA:
	return true;
      default:
	return false;
    }
}

bool
is_binary_exp_ia64(Instr *mi)
{
    int o = get_opcode(mi);
    int s = get_stem(o);
    switch (s) {
      case ADD: case ADDP4: case AND: case ANDCM: case CMP:
      case CMP4: case CMPXCHG: case FADD: case FAMAX: case FAMIN:
      case FAND: case FANDCM: case FCMP: case FMAX: case FMERGE:
      case FMIN: case FMIX: case FMPY: case FNMPY: case FOR:
      case FPACK: case FPAMAX: case FPAMIN: case FPCMP:
      case FPMAX: case FPMERGE: case FPMIN: case FPMPY:
      case FPNMPY: case FPRCPA: case FRCPA: case FSUB:
      case FSWAP: case FSXT: case FXOR: case MIX: case OR:
      case PACK: case PADD: case PAVG: case PAVGSUB: case PCMP:
      case PMAX: case PMIN: case PMPY: case PROBE: case PSAD:
      case PSHL: case PSHR: case PSUB: case SHL: case SHR:
      case SUB: case TBIT: case UNPACK: case XMPY: case XOR:
	return true;
      default:
	return false;
    }
}

bool
is_unary_exp_ia64(Instr *mi)
{
    int o = get_opcode(mi);
    int s = get_stem(o);
    switch (s) {
      case CZX: case FABS: case FCVT_FX: case FCVT_XF: case FCVT_XUF:
      case FNEG: case FNEGABS: case FNORM: case FPCVT_FX: case FPNEG:
      case FPNEGABS: case FPRSQRTA: case FRSQRTA: case GETF:
      case POPCNT: case SETF: case SXT: case TAK: case THASH:
      case TNAT: case TPA: case TTAG: case ZXT:
	return true;
      default:
	return false;
    }
}

bool
is_commutative_ia64(Instr *mi)
{
    int opc = get_opcode(mi);
    int st = get_stem(opc);
    switch (st) {

      case ADD: case ADDP4: case AND: case OR:
	/* The above have an immediate form that cannot be reversed */
	return !is_immed(get_src(mi, 1));

      case CMP: case CMP4:
        /* The next set have relational operators that can only
	be reversed if they are set to "EQUAL" */
	return (get_ext(opc, 1) == CREL_EQ);

      case FCMP: case FPCMP:
	return (get_ext(opc, 1) == FREL_EQ);

      case PCMP:
	return (get_ext(opc, 1) == PREL_EQ);

      case FADD: case FAMAX: case FAMIN: case FAND:
      case FMAX: case FMIN: case FMPY: case FNMPY:
      case FOR: case FPAMAX: case FPAMIN: case FPMAX:
      case FPMIN: case FPMPY: case FPNMPY: case FXOR:
      case PADD: case PAVG: case PMAX: case PMIN:
      case PMPY: case PSAD: case XMPY: case XOR:
	return true;

      default:
	return false;
    }
}

bool
is_two_opnd_ia64(Instr *instr)
{
    return false;
}

bool
reads_memory_ia64(Instr *instr)
{  
    int o = get_opcode(instr);
    int s = get_stem(o);
    switch (s) {

      case CMPXCHG: case FETCHADD: case LD: case LDF:
      case LDFP: case LFETCH: case XCHG:
	return true;

      default:
	return false;
    }
}

bool
writes_memory_ia64(Instr *instr)
{
    int o = get_opcode(instr);
    int s = get_stem(o);
    switch (s) {
      case ST:
      case STF:
	return true;
      default:
	return false;
    }
}

