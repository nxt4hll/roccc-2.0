/* file "alpha/translate_cvt.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#include <machine/machine.h>

#include <alpha/opcodes.h>
#include <alpha/reg_info.h>
#include <alpha/code_gen.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

using namespace alpha;

/*
 * PROCEDURE:	Translate_cvt()
 * 
 * DESCRIPTION:	
 *
 * This routine translates the CVT into the appropriate ALPHA convert
 * assembly instruction.  Any converts between integer and floating-point
 * values must always pass through the memory system (i.e. perform a store
 * and a load in addtion to the cvt instruction).
 *
 * This routine assumes that conversions between floats and non-floats will
 * always be with a 32-bit-sized non-float or larger (SUIF currently ensures
 * that this is true).  If a case other than this occurs, I'll add the code
 * then to handle cases like 'cvt (s.16) (f.64) vr#1,vr#2'.
 *
 * This routine also assumes that conversions from a FP number to an integer
 * are always done through a truncation (like C and FORTRAN specs).  This
 * truncation is handled by the "chopped" qualifier on the cvt instruction
 * in ALPHA. */

Instr*
CodeGenAlpha::translate_cvt_zap(TypeId t, Opnd d, Opnd s)
{
    // helper routine for translate_cvt()
    int im, size = get_bit_size(t);

    if (size == 32)
	im = 15;
    else if (size == 16)
	im = 3;
    else if (size == 8)
	im = 1;
    else
	claim(false, "translate_cvt_zap() -- unexpected operand size");

    // inefficent to use AND since immed field is often too small
    return new_instr_alm(d, ZAPNOT, s, opnd_immed(im, type_u32));
}

Instr*
CodeGenAlpha::translate_cvt_shf(int d_sz, Opnd d, int s_sz, Opnd s)
{
    // helper routine for Translate_cvt() -- signed destination
    Instr *mi;

    if (d_sz == 32) {
	mi = new_instr_alm(d, ADDL, s, opnd_reg_const0);

    } else {
	Opnd vr = opnd_reg(type_s64);
    	int shift_amount = (s_sz > d_sz) ? (64 - d_sz) : (64 - s_sz);
	Opnd im = opnd_immed(shift_amount, type_u32);

	mi = new_instr_alm(vr, SLL, s, im);
	emit(mi);

	mi = new_instr_alm(d, SRA, vr, im);
    }

    return mi;
}


/* Operations done for the convert operations */
enum cvt_enum {
    move, // simple move
    Move, // optimized move (zap/sign-ext bits 64 and higher)
    MOVE, // weird move due to Alpha canonical form
    ZAPS, // weird zap per source size to get out of Alpha canonical form
    zapd, // zap to destination type size
    shfd, // sign-extend to destination type size
    SHFD  // weird sign-extend to match Alpha canonical form
};

/* Table of conversion operations for the Alpha architecture.  Unsigned
 * 32-bit numbers are stored in a canonical form, which replicates bit 31
 * in the upper 32 bit register locations. */
static cvt_enum cvt_table[8][8] = {
    /*  +------- source      {signed,unsigned}.{size} */
    /*  |        destination {signed,unsigned}.{size} */
    /* 	V        u.8   s.8   u.16  s.16  u.32  s.32  u.64  s.64 */
    /* u.8  */ { move, shfd, move, move, move, move, move, move },
    /* s.8  */ { zapd, move, zapd, move, MOVE, move, zapd, move },
    /* u.16 */ { zapd, shfd, move, shfd, move, move, move, move }, 
    /* s.16 */ { zapd, shfd, zapd, move, MOVE, move, zapd, move }, 
    /* u.32 */ { zapd, shfd, zapd, shfd, move, MOVE, ZAPS, ZAPS },
    /* s.32 */ { zapd, shfd, zapd, shfd, MOVE, move, Move, move },
    /* u.64 */ { zapd, shfd, zapd, shfd, SHFD, shfd, move, Move }, 
    /* s.64 */ { zapd, shfd, zapd, shfd, shfd, shfd, Move, move }
};

/* Computes type size to cvt_table index missing the least significant
 * bit which is determined by the signed/unsigned-ness of the type. */
static int cvt_idx_table[65] = {
 -1,
 -1, -1, -1, -1, -1, -1, -1,  0, -1, -1, -1, -1, -1, -1, -1,  2, 
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  4, 
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  6
};

void
CodeGenAlpha::translate_cvt(Instr *cvt)
{
    Instr *mi;

    Opnd d = get_dst(cvt);
    TypeId dtype = get_type(d);
    int dsize = get_bit_size(dtype);

    Opnd s = get_src(cvt, 0);
    TypeId stype = get_type(s);
    int ssize = get_bit_size(stype);

    VarSym *scratch_sym;				// for interbank moves

    if (is_floating_point(dtype)) {
	if (dsize == 32) {
	    if (is_floating_point(stype)) {
		if (ssize == 32)				// FPs => FPs
		    // simply do move instruction
		    set_opcode(cvt, FMOV);
		else if (ssize == 64)				// FPd => FPs
		    set_opcode(cvt, CVTTS);
		else
		    claim(false, "unexpected FP src size for CVT");
		emit(cvt);

	    } else if (is_scalar(stype)) {
		claim(ssize == 32 || ssize == 64 || ssize == 8 || ssize == 16);

		// I => FPs -- move from GP reg to FP reg via mem before cvt
		scratch_sym = new_unique_var(type_s64);
		Opnd ea = opnd_addr_sym(scratch_sym);
		mi = new_instr_alm(ea, STQ, s);		// stq s,(ss)
		emit(mi);

		// notice that the type is a lie for register allocation
		Opnd vr = opnd_reg(type_f64);
		mi = new_instr_alm(vr, LDT, ea);		// ldt vr,(ss)
		emit(mi);

		set_opcode(cvt, CVTQS);
		set_src(cvt, 0, vr);				// cvtqs vr,d
		emit(cvt);

	    } else {
		claim(false, "unexpected source type for CVT (to FPs)");
	    }

	} else if (dsize == 64) {
	    if (is_floating_point(stype)) {
		if (ssize == 32)				// FPs => FPd
		    set_opcode(cvt, CVTST);
		else if (ssize == 64)				// FPd => FPd
		    // simply do move instruction
		    set_opcode(cvt, FMOV);
		else
		    claim(false, "unexpected FP src size for CVT");
		emit(cvt);

	    } else if (is_scalar(stype)) {
		claim(ssize == 32 || ssize == 64 || ssize == 8 || ssize == 16);

		// I => FPd -- move from GP reg to FP reg via mem before cvt
		scratch_sym = new_unique_var(type_s64);
		Opnd ea = opnd_addr_sym(scratch_sym);
		mi = new_instr_alm(ea, STQ, s);		// stq s,(ss)
		emit(mi);

		// notice that the type is a lie for register allocation
		Opnd vr = opnd_reg(type_f64);
		mi = new_instr_alm(vr, LDT, ea);		// ldt vr,(ss)
		emit(mi);

		set_opcode(cvt, CVTQT);
		set_src(cvt, 0, vr);				// cvtqs vr,d
		emit(cvt);

	    } else {
		claim(false, "unexpected source type for CVT (to FPd)");
	    }

	} else {
	    claim(false, "unexpected destination size %d for CVT", dsize);
	}

    } else if (is_scalar(dtype)) {

	if (is_floating_point(stype)) {
	    // fp => long -- move from fp RF to int RF via mem after cvt
	    // Note that conversion is independent of the fp precision
	    // since Alpha stores SP numbers in DP-compatible format.
	    Opnd vr = opnd_reg(type_f64);
	    set_opcode(cvt, CVTTQ);
	    set_dst(cvt, 0, vr);				// cvttqc s,vr

	    ListNote<long> note;
	    note.set_value(0, ROUND_CHOPPED);
	    set_note(cvt, k_instr_opcode_exts, note);

	    emit(cvt);

	    scratch_sym = new_unique_var(type_s64);
	    Opnd ea = opnd_addr_sym(scratch_sym);
	    mi = new_instr_alm(ea, STT, vr);			// stt vr,(ss)
	    emit(mi);

	    if (dsize <= 32) {
		claim(dsize == 32 || dsize == 8 || dsize == 16);
		mi = new_instr_alm(d, LDL, ea);		// ldl d,(ss)
	    } else if (dsize == 64) {
		mi = new_instr_alm(d, LDQ, ea);		// ldq d,(ss)
	    } else
		claim(false, "unexpected int src size for CVT");
	    emit(mi);

	} else if (is_scalar(stype)) {

	    /* I ==> I conversion -- trickier than you might think 
	     *
	     * 'sm' stands for 'smaller of the two numbers'; 'bg'
	     * stands for 'bigger' of the two numbers.  Obviously,
	     * the upper-left and lower-right quadrants are isomorphic.
	     *
	     * Here's the conversion table for no weirdness in the
	     * representations:
	     *
	     *			destination
	     * src  |	u.sm	s.sm	u.bg	s.bg	Key: u = unsigned
	     * -----+-------------------------------	     s = signed
	     * u.sm |	mov	shf1	mov	mov	     sm = size small
	     * s.sm |	zap1	mov	zap1	mov	     bg = size big
	     * u.bg |	zapd	shfd	mov	shf1
	     * s.bg |	zapd	shfd	zap1	mov
	     *
	     * where
	     *	mov  = simply move src to dst;
	     *  shfd = perform sll + sra to sign extend proper dest-type bit;
	     *  zapd = zero appropriate upper bytes (can implement with AND);
	     *  XXX1 = if (dst->size == REGISTER_SIZE)
	     *		 mov;
	     *         else
	     *		 XXXd;
	     *
	     * Since Alpha is weird, the actual conversion operations come
	     * from the table above.
	     */
	    int si = cvt_idx_table[ssize];
	    int di = cvt_idx_table[dsize];
	    claim(si >= 0);
	    claim(di >= 0);
	    si += ((is_integral(stype) && is_signed(stype)) ? 1 : 0);
	    di += ((is_integral(dtype) && is_signed(dtype)) ? 1 : 0);
	    switch (cvt_table[si][di]) {
	      case move:
	      case Move:
	      case MOVE:
		set_opcode(cvt, alpha::MOV);
		emit(cvt);
		break;

	      case ZAPS:
		mi = translate_cvt_zap(stype, d, s);
		copy_notes(cvt, mi);
		emit(mi);
		delete cvt;
		break;

	      case zapd:
		mi = translate_cvt_zap(dtype, d, s);
		copy_notes(cvt, mi);
		emit(mi);
		delete cvt;
		break;

	      case shfd:
	      case SHFD:
		mi = translate_cvt_shf(dsize, d, ssize, s);
		copy_notes(cvt, mi);
		emit(mi);
		delete cvt;
		break;
	    };

	} else {
	    claim(false, "Unexpected source type for CVT (to INT)");
	}
    } else if (is_void(dtype)) {
	if (has_notes(cvt)) {
	    mi = new_instr_alm(opcode_null);
	    copy_notes(cvt, mi);
	    emit(mi);
	}
	delete cvt;
    } else {
	claim(false, "expected destination type for CVT");
    }
}
