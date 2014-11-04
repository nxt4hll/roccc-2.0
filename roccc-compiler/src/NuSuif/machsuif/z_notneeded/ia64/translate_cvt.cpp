/* file "ia64/translate_cvt.cpp" */

/*
    Copyright (c) 2002 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#include <machine/machine.h>

#include <ia64/opcodes.h>
#include <ia64/reg_info.h>
#include <ia64/code_gen.h>
#include <ia64/instr.h>
#include <ia64/init.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

using namespace ia64;

/*
 * PROCEDURE:	Translate_cvt()
 * This may be a naive way to do things, but since there is an
 * instruction that moves a value from FPR to GPR or back,
 * I'm just gonna use it and assume it does the right thing!
 * 
 * The completers for SETF/GETF specify how the conversion should go
 * I always choose double precision form (FIXME?)
 */

void
CodeGenIa64::translate_cvt(Instr *cvt)
{
    Opnd d = get_dst(cvt);
    TypeId dtype = get_type(d);

    if (is_floating_point(dtype)) 
        cvt_to_fp(cvt);
    else if (is_scalar(dtype)) 
        cvt_to_non_fp_scalar(cvt);
    else if (is_void(dtype)) {
        if (has_notes(cvt)) {
            Instr *mi = new_instr_alm(opcode_null);
            copy_notes(cvt, mi);
            emit(mi);
        }
        delete cvt;
    }
    else 
        claim(false, "expected destination type for CVT");
}

void
CodeGenIa64::cvt_to_fp(Instr *cvt)
{
    Opnd s = get_src(cvt, 0);
    TypeId stype = get_type(s);
    Opnd d = get_dst(cvt, 0);

    claim((is_reg(s) || is_var(s)), "Assumed cvt srcs are regs/vars");
    claim((is_reg(d) || is_var(d)), "Assumed cvt dsts are regs/vars");

    if (is_floating_point(stype)) {
       // simply do move instruction
       set_opcode(cvt, make_opcode(MOV_FR, EMPTY,EMPTY,EMPTY));
       emit(cvt);
    }
    else if (is_scalar(stype)) {
	// put integer source into significand of FR destination
        set_opcode(cvt, make_opcode(SETF, _SD_FORM_SIG,EMPTY,EMPTY));
        emit(cvt);
	// do fixed-to-floating conversion in the FR destination
	int fcvt_xf = make_opcode(FCVT_XF, EMPTY,EMPTY,EMPTY);
	emit(new_instr_alm(d, fcvt_xf, d));
    }
    else {
        claim(false, "Unexpected source type for cvt_to_fp");
    }
}

void
CodeGenIa64::cvt_to_non_fp_scalar(Instr *cvt)
{
    Opnd s = get_src(cvt, 0);
    TypeId stype = get_type(s);
    int ssize = get_bit_size(stype);
    Opnd d = get_dst(cvt);
    TypeId dtype = get_type(d);
    int dsize = get_bit_size(dtype);

    claim((is_reg(s) || is_var(s)), "Assumed cvt srcs are regs/vars");
    claim((is_reg(d) || is_var(d)), "Assumed cvt dsts are regs/vars");

    if (is_floating_point(stype)) {
	// Truncate value into the significand of a FP register,
	// then extract that to the GR dst.
	Opnd whole = opnd_reg(stype);
	int fcvt_opcode =
	    (is_integral(dtype) && is_signed(dtype))
		? make_opcode(FCVT_FX, _ROUND_TRUNC, EMPTY,EMPTY)
		: make_opcode(FCVT_FXU,_ROUND_TRUNC, EMPTY,EMPTY);
	emit(new_instr_alm(whole, fcvt_opcode, s));

        set_opcode(cvt, make_opcode(GETF, _SD_FORM_SIG,EMPTY,EMPTY));
	set_src(cvt, 0, whole);
        emit(cvt);
    }
    else if (is_scalar(stype)) {
        // If src and dst are both 64-bit values, or if dst is at least
        // wide as src and its signedness is the same, then the register
        // representation of src can just be moved to dst.  Otherwise,
        // extract the low-order field of src whose size is that of dst,
        // using sign-extension if dst is signed, or else zero-extension.

        if (((dsize == 64) && (ssize == 64)) ||
	    ((dsize >= ssize) &&
	     ((is_integral(dtype) && is_signed(dtype)) ==
	      (is_integral(stype) && is_signed(stype)))))
            set_opcode(cvt, make_opcode(MOV_GR, EMPTY,EMPTY,EMPTY));
        else {
	    int stem = is_signed(dtype) ? SXT : ZXT; 
	    int xsz = (dsize == 32) ? XSZ_4 :
		      (dsize == 16) ? XSZ_2 :
		      (dsize ==  8) ? XSZ_1 :
		      -1;
	    claim(xsz != -1, "Unexpected destination size (%d) for CVT", dsize);

	    set_opcode(cvt, make_opcode(stem, xsz, EMPTY,EMPTY));
        }
        emit(cvt);
    }
    else {
        claim(false, "Unexpected source type for CVT (from scalar)");
    }
}
