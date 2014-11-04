/* file "x86/translate_cvt.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#include <machine/machine.h>

#include <x86/opcodes.h>
#include <x86/reg_info.h>
#include <x86/code_gen.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

using namespace x86;

/*
 * PROCEDURE:	Translate_cvt()
 * 
 * DESCRIPTION:	
 *
 * This routine translates the CVT into the appropriate x86 convert
 * assembly instruction(s).  Any converts between integer and floating-point
 * values must always pass through memory.
 *
 * This routine assumes that conversions between floats and non-floats will
 * always be with a WORD-sized non-float (SUIF currently ensures that this
 * is true).  If a case other than this occurs, I'll add the code then
 * to handle cases like 'cvt (s.16) (f.64) nr#1,nr#2'.
 *
 * This routine also assumes that conversions from a FP number to an integer
 * are always done through a truncation (like C and FORTRAN specs).
 */

/* Operations done for the integer convert operations */
enum cvt_enum {
    move, // simple move
    zapd, // zap to destination type size (zero extend)
    sexd  // sign-extend to destination type size
};

/* Table of conversion operations.  In x86, we use the grains of
 * the registers for smaller types, and thus we cannot guarantee
 * that the upper bits are correct on a conversion from a smaller
 * type to a larger type.  Nothing special happens when converting 
 * from a larger to a smaller (inclusive) type since the upper
 * bits of the register are ignored.  See the operand printing
 * routine in the implementation of class PrinterX86.
 */
static cvt_enum cvt_table[6][6] = {
    /*  +------- source      {signed,unsigned}.{size} */
    /*  |        destination {signed,unsigned}.{size} */
    /* 	V        u.8   s.8   u.16  s.16  u.32  s.32 */
    /* u.8  */ { move, move, zapd, zapd, zapd, zapd },
    /* s.8  */ { move, move, sexd, sexd, sexd, sexd },
    /* u.16 */ { move, move, move, move, zapd, zapd }, 
    /* s.16 */ { move, move, move, move, sexd, sexd }, 
    /* u.32 */ { move, move, move, move, move, move },
    /* s.32 */ { move, move, move, move, move, move }
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
CodeGenX86::translate_cvt(Instr *mi)
{
    Instr *mj = NULL;
    Opnd d = get_dst(mi), s = get_src(mi, 0);
    TypeId dtype = get_type(d), stype = get_type(s);
    claim(is_scalar(dtype) && is_scalar(stype));

    VarSym *scratch_sym;	// scratch symbol

    int di, si;			// used in integer conversions

    if (is_floating_point(dtype)) {
	// x86 internal rep is always 80-bit extended real
	if (is_floating_point(stype)) {			// FLOAT ==> FLOAT
	    // deal with source operand
	    if (is_var(s)) {
		// put s at ST
		mj = fp_stack.push(s);
		emit(mj);
	    } else if (is_reg(s)) {
		// move s to ST
		mj = fp_stack.exchange(s);
		if (mj) emit(mj);
	    }

	    // deal with destination operand
	    if (is_var(d)) {
		// pop off ST and place its contents in memory
		mj = fp_stack.pop(d);
		emit(mj);
	    } else if (is_reg(d)) {
		// rename the result value at ST to be this reg's name
		fp_stack.rename_st(get_reg(d));
	    }

	} else {					// INT ==> FLOAT
	    if (get_bit_size(stype) < 32) {
		Opnd s32 = opnd_reg(is_signed(stype) ? type_s32 : type_u32);
		translate_cvt(new_instr_alm(s32, suifvm::CVT, s));
		s = s32;
	    }
	    // run converion through memory
	    scratch_sym = new_unique_var(type_s32);
	    set_addr_taken(scratch_sym, true);
	    mj = new_instr_alm(opnd_addr_sym(scratch_sym), x86::MOV, s);
	    emit(mj);					// mov s,(ss)

	    mj = fp_stack.push_int(opnd_var(scratch_sym));
	    emit(mj);

	    // deal with destination operand
	    if (is_var(d)) {
		// pop off ST and place its contents in memory
		mj = fp_stack.pop(d);
		emit(mj);
	    } else if (is_reg(d)) {
		// rename the result value at ST to be this reg's name
		fp_stack.rename_st(get_reg(d));
	    }
	}

    } else {		// int, ptr, enum, or void
	if (is_floating_point(stype)) {			// FLOAT ==> INT
	    claim(get_bit_size(dtype) <= 32);

	    // Make sure source is in ST
	    if (is_var(s)) {
		mj = fp_stack.push(s);
		emit(mj);
	    } else if (is_reg(s)) {
		mj = fp_stack.exchange(s);
		if (mj) emit(mj);
	    }

	    // Store the FPU control word into two separate memory cells,
	    // save_cw and temp_cw.  In the temp_cw cell, set rounding control
	    // to round-toward-zero mode, then load the modified CW value.
	    //
	    // If the destination is unsigned, truncate the source to a 64-bit
	    // integer to avoid overflow and discard the high-order 32 bits.
	    //
	    // FIXME: OPEN ISSUE: how should the rounding mode be determined?
	    // We use round-toward-zero because C dictates it.  What about
	    // Fortran and other languages?
	    // And do we need to set the rounding mode when writing SP/DP FP
	    // numbers in memory?

	    VarSym *save_cw_sym = new_unique_var(type_u16);
	    VarSym *temp_cw_sym = new_unique_var(type_u16);
	    set_addr_taken(save_cw_sym, true);
	    set_addr_taken(temp_cw_sym, true);
	    Opnd save_cw = opnd_addr_sym(save_cw_sym);
	    Opnd temp_cw = opnd_addr_sym(temp_cw_sym);
	    
	    TypeId result_type = is_signed(dtype) ? type_s32 : type_s64;
	    VarSym *result_sym = new_unique_var(result_type);
	    set_addr_taken(result_sym, true);
	    Opnd result = opnd_addr_sym(result_sym);

	    Opnd fpflags = opnd_reg(FPFLAGS, type_u16);
	    emit(new_instr_alm(save_cw, FSTCW, fpflags));
	    emit(new_instr_alm(temp_cw, FSTCW, fpflags));
	    emit(new_instr_alm(temp_cw, OR, temp_cw,
			       opnd_immed(0xc00, type_u16)));
	    emit(new_instr_alm(fpflags, FLDCW, temp_cw));

	    // Round and pop the FP number from ST, store the integer result
	    // in memory, then move it's low-order 32 bits to the destination.
	    // Finally restore the original FPU control word from save_cw.
	    emit(fp_stack.pop_int(result));
	    emit(mj = new_instr_alm(d, x86::MOV, result));
	    emit(new_instr_alm(fpflags, FLDCW, save_cw));

	} else {					// INT ==> INT
	    // trickier than you might think
	    si = cvt_idx_table[get_bit_size(stype)];
	    di = cvt_idx_table[get_bit_size(dtype)];
	    claim(si >= 0);
	    claim(di >= 0);
	    si += ((is_integral(stype) && is_signed(stype)) ? 1 : 0);
	    di += ((is_integral(dtype) && is_signed(dtype)) ? 1 : 0);
	    switch (cvt_table[si][di]) {
	      case move:
		mj = new_instr_alm(d, x86::MOV, s);
		break;

	      case zapd:
		mj = new_instr_alm(d, MOVZX, s);
		break;

	      case sexd:
		mj = new_instr_alm(d, MOVSX, s);
		break;
	    }
	    emit(mj);
	}
    }

    // annotations are placed on the last instruction generated, if any
    if (mj)
	copy_notes(mi, mj);
    delete mi;			// no longer needed
}
