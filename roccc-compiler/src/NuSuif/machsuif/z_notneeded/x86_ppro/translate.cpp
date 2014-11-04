/* file "x86_ppro/translate.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#include <machine/machine.h>
#include <suifvm/suifvm.h>
#include <x86/x86.h>

#include <x86_ppro/code_gen.h>
#include <x86_ppro/opcodes.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

using namespace x86;

/* Change this routine's behavior wrt fp comparisons.  Here, we take
 * advantage of the new fp instruction that compares to fp values and
 * sets the eflags register.
 *
 * All other translations are directed to the base class routine. */
void
CodeGenX86PPro::translate_fp(int opc, Instr *mi)
{
    if (opc != FCOMPP) {
	// code in x86 library works just fine, go there
	CodeGenX86::translate_fp(opc, mi);
	return;
    }

    // otherwise, take advantage of compare-real-and-set-eflags operation
    int s0_st_i = -1, s1_st_i = -1;	// ST-relative indices

    Instr *mj = NULL;
    Opnd s0 = get_src(mi, 0);
    Opnd s1 = get_src(mi, 1);
    Opnd d  = get_dst(mi);

    if (is_var(s0)) {
	// Put s0 at ST.  Must be there for single-source operations.
	mj = fp_stack.push(s0);
	emit(mj);
	s0_st_i = 0;		// record that s0 is at ST

    } else if (is_reg(s0)) {
	if (is_reg(s1)) {
	    // all work is done below, see note
	} else {
	    // move s0 to ST
	    mj = fp_stack.exchange(s0);
	    if (mj) emit(mj);
	    s0_st_i = 0;
	}
    }

    if (is_var(s1)) {
	mj = fp_stack.push(s1);
	emit(mj);
	s1_st_i = 0;		// at ST
	s0_st_i++;		// should be ST(1), check ...
	claim(s0_st_i == 1);

    } else if (is_reg(s1)) {
	// need to move s1 to ST
	mj = fp_stack.exchange(s1);
	if (mj) emit(mj);
	s1_st_i = 0;
	// Since the exchange may have affected placement of s0, only
	// now is it safe to determine the location of s0 in fpstack.
	s0_st_i = fp_stack.find(get_reg(s0));
	claim((s0_st_i >= 0),
	      "translate_fp() -- vr%d not on fpstack", get_reg(s0));
    }

    // do compare -- result placed directly in EFLAGS
    mj = new_instr_alm(opnd_reg_eflags, FCOMIP,
		       opnd_reg(FP0 + s0_st_i, get_type(s0)),
		       opnd_reg(FP0, get_type(s1)));
    emit(mj);

    // fcomip pops one of the operands off the stack
    fp_stack.pop();

    // run a nop fadd to pop off the other operand
    claim(s0_st_i == 1);
    mj = fp_stack.pop(opnd_reg(FP0, get_type(s0)));
    emit(mj);

    if (!is_reg(d) || (get_reg(d) != EFLAGS)) {
	// The user wants a boolean result of the compare in the
	// specified destination.  Create the appropriate SETCC 
	// and then sign-extend result to 32 bits.
	Opnd vr = opnd_reg(type_u8);
	opc = get_opcode(mi);
	if ((opc == suifvm::SEQ) || (opc == suifvm::SL))
	    opc = SETNE;
	else	// SNE or SLE
	    opc = SETE;
	mj = new_instr_alm(vr, opc, opnd_reg_eflags);
	emit(mj);
	mj = new_instr_alm(get_dst(mi), MOVSX, vr);
	emit(mj);
    }

    if (mj != NULL)
	copy_notes(mi, mj);	// transfer annotations onto last new instr
    else
	warn("translate_fp() -- dropping annotations");

    delete mi;
}


/* Example demonstrating the use of the integer conditional move
   instruction. */
void
CodeGenX86PPro::translate_abs(Instr *mi)
{
    if (is_floating_point(get_type(get_dst(mi)))) {
	translate_fp(FABS, mi);
	delete mi;
    } else {
	// create a scratch register
	Opnd s = get_src(mi, 0), d = get_dst(mi), vr = opnd_reg(type_s32);

	// produce a negated version of src, which sets cc register
	set_dst(mi, 0, vr);
	set_dst(mi, 1, opnd_reg_eflags);
	set_opcode(mi, x86::NEG);
	// leave src0 alone
	mi = make_2opnd(mi);
	emit(mi);				// neg  vr <- s

	// set dst based conditionally on sign-bit in cc
	mi = new_instr_alm(d, CMOVNS, s, vr);
	set_src(mi, 2, opnd_reg_eflags);
	emit(mi);				// d = (eflags) ? vr : s;
    }
}
