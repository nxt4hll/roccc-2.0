/* file "x86/translate.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#include <machine/machine.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif
//#include <suifvm/suifvm.h>

#include <x86/init.h>
#include <x86/opcodes.h>
#include <x86/reg_info.h>
#include <x86/code_gen.h>

using namespace x86;

void
add_regs_defd(Instr* instr, int l, int h)
{
    NatSetDense regs_defd;
    regs_defd.insert(l);
    regs_defd.insert(h);
    
    NatSetNote note;
    note.set_set(&regs_defd);
    set_note(instr, k_regs_defd, note);
}

void
add_regs_used(Instr* instr, int l, int h)
{
    NatSetDense regs_used;
    regs_used.insert(l);
    regs_used.insert(h);
    
    NatSetNote note;
    note.set_set(&regs_used);
    set_note(instr, k_regs_used, note);
}


/* make_2opnd() -- add a few surrounding move instructions to
 * satisfy the requirements for a 2-operand instruction,
 * appending all but the last instruction (which is returned),
 * to the emitted instruction list.  This routine is required
 * by most x86 alu instructions. */
Instr *
CodeGenX86::make_2opnd(Instr *mi)
{
    Instr *mj;
    Opnd d0 = get_dst(mi);
    Opnd s0 = get_src(mi, 0);

    Opnd tmp;
    bool need_last_move = true;
    if (is_reg(d0)) {
	// optimization: use dst reg as temp
	tmp = d0;
	need_last_move = false;
    } else
	tmp = opnd_reg(get_type(s0));

    // move s0 to a tmp register
    mj = new_instr_alm(tmp, x86::MOV, s0);
    emit(mj);

    // rewrite execute instruction in 2-operand form
    set_src(mi, 0, tmp);
    set_dst(mi, tmp);

    if (need_last_move) {
	// move result to true destination
	emit(mi);
	mj = new_instr_alm(d0, x86::MOV, tmp);
	return mj;
    } else
	return mi;
}


/* Translate function pattern #1 -- Simplest translation.  Just update the
 * opcode, set eflags as a destination, and convert instruction to 2-operand
 * format.  In this pattern, there is only one target opcode possible. */
#ifdef TRANSLATE_FUN_1
#undef TRANSLATE_FUN_1
#endif
#define TRANSLATE_FUN_1(from_opcode, to_opcode)	   \
void						   \
CodeGenX86::translate_##from_opcode(Instr *mi) { \
    set_opcode(mi, to_opcode);			   \
    set_dst(mi, 1, opnd_reg_eflags);		   \
    mi = make_2opnd(mi);			   \
    emit(mi);					   \
}

TRANSLATE_FUN_1(and, x86::AND)
TRANSLATE_FUN_1(ior, x86::OR)
TRANSLATE_FUN_1(not, x86::NOT)
TRANSLATE_FUN_1(xor, x86::XOR)


/* translate_null() -- Handle opcode_null instructions by omitting them
 * unless they have annotations.
 */
void
CodeGenX86::translate_null(Instr *mi)
{
    if (!has_notes(mi))
	return;			// drop useless null instrs on the floor

    emit(mi);
}


/* translate_nop() -- This routine handles NOP instructions.  We
 * convert nops into null instructions since a nop is useless, except
 * as a placeholder for information, before instruction scheduling. */
void
CodeGenX86::translate_nop(Instr *mi)
{
    if (has_notes(mi)) // append to instr list
	emit(mi);
    // else drop instr on the floor
}


/* translate_mrk() -- This routine currently handles "line"
 * annotations only.  The "line" annotations are translated into .loc
 * an assembler pseudo-op.  The expansion of this single .loc pseudo-op
 * into .loc and .file is postponed until ASCII print time.
 *
 * All other annotations handled by translating them into .null
 * pseudo-ops. */
void
CodeGenX86::translate_mrk(Instr *mi)
{
    if (LineNote note = get_note(mi, k_line)) {
	const char *file = note.get_file().chars();
	claim((file && file[0]), "line note with null file name");
	set_opcode(mi, LOC);
    } else
	set_opcode(mi, opcode_null);
    emit(mi);
}

/* Helper routine for translate_ldc(). */
Instr *
CodeGenX86::create_fld_const(Instr *mi)
{
    // some sanity checks
    Opnd d = get_dst(mi);
    TypeId dtype = get_type(d);
    int sz = get_bit_size(dtype);
    claim((sz == 32 || sz == 64),
	   "create_fld_const() -- unrecognized FP ldc size (%d bits)", sz);

    Opnd s = get_src(mi, 0);
    double fp_val = atof(get_immed_string(s).chars());

    delete mi;				// done with current mi

    // check for quick loads of common constants
    if (fp_val == 0.0) {
	mi = fp_stack.push_const(FLDZ);
    } else if (fp_val == 1.0) {
	mi = fp_stack.push_const(FLD1);
    } else {
	// In general, fp constants have to be pushed on the stack
	// from a location in memory, so make a new tmp variable
	// and initialize it to the constant value.  No optimization
	// to check if a memory location already created for a
	// constant value--change when this becomes a problem.
	VarSym *fp_sym = new_unique_var(s);
	mi = fp_stack.push(opnd_var(fp_sym));
    }

    // deal with destination
    if (is_var(d)) {
	emit(mi);
	// store top of stack to symbol
	mi = fp_stack.pop(d);
    } else {
	// value left on stack
	claim(is_reg(d));		// sanity check
	fp_stack.rename_st(get_reg(d));
    }
    return mi;
}


void
CodeGenX86::translate_ldc(Instr *mi)
{
    if (is_floating_point(get_type(get_dst(mi)))) {
	mi = create_fld_const(mi);
    } else {
	set_opcode(mi, x86::MOV);
    }
    emit(mi);
}

void 
CodeGenX86::translate_lda(Instr *mi) {
    set_opcode(mi, LEA);
    emit(mi);
}


/*
 * Translate floating-point instruction `mi' using the given x86 `opcode'.
 * There are four main cases:
 *
 *  (1) binary arithmetic:  dst = src0 op src1
 *  (2) unary  arithmetic:  dst = op src0
 *  (3) move:               dst = src0
 *  (4) comparison:         dst = src0 op src1
 *
 * For case (1) we get src0 into ST(1) and src1 into ST, then operate into
 * ST(1) and pop the FP stack, leaving the result in ST.
 *
 * For case (2) we get src0 into ST and operate into ST.
 *
 * For case (3), we just get src0 into ST.  [NB: in this case `opcode' is
 * the null opcode.]
 *
 * For case (4) we get src0 into ST and src1 into ST(1), then compare using
 * FCOMPP, which pops both ST and ST(1) from the FP stack.  [NB: stack
 * positions of srcs is the opposite of case (1).]
 *
 * In cases (1) - (3), we dispose of the result either by popping ST into
 * memory, if dst is a variable, or by recording the fact that dst lives in
 * ST, if dst is a register.
 *
 * In case (4), we move the FP condition code into EFLAGS if that's the dst
 * register, or we form a boolean in dst using the opcode.
 *
 * Note that placement of src0 and src1 on the FP stack is different for
 * the binary cases, (1) and (4) .
 *
 * Note also that except for case (4) (opcode == FCOMPP), the mi should be
 * deleted after calling this routine.  */
void
CodeGenX86::translate_fp(int opcode, Instr *mi)
{
    int s0_st_i, s1_st_i = -1;	// ST-relative indices

    Instr *mj = NULL;
    Opnd s0 = get_src(mi, opcode != FCOMPP ? 0 : 1);
    Opnd s1 = get_src(mi, opcode != FCOMPP ? 1 : 0);
    Opnd d  = get_dst(mi);

    if (is_var(s0)) {
	// Put s0 at ST.  Must be there for single-source operations.
	mj = fp_stack.push(s0);
	emit(mj);
	s0_st_i = 0;		// record that s0 is at ST

    } else if (is_reg(s0)) {
	// move s0 to ST
	mj = fp_stack.exchange(s0);
	if (mj) emit(mj);
	s0_st_i = 0;
    } else {
	claim(false, "Unexpected kind of floating-point operand");
    }

    if (is_var(s1)) {
	mj = fp_stack.push(s1);
	emit(mj);
	s1_st_i = 0;		// at ST
	s0_st_i = 1;		// at ST(1)

    } else if (is_reg(s1)) {
	// need to move s1 to ST
	claim(s0 != s1, "Floating operands must not be identical");
	s1_st_i = fp_stack.find(get_reg(s1));
	claim(s1_st_i == 1,
	      "translate_fp() -- vr%d not in expected position on fp_stack",
	      get_reg(s1));
	mj = fp_stack.exchange(s1);
	if (mj) emit(mj);
	s1_st_i = 0;		// at ST
	s0_st_i = 1;		// at ST(1)
    }

    if (opcode == FCOMPP) {	// case (4)
	// do compare
	mj = new_instr_alm(opcode, 
			   opnd_reg(FP0, get_type(s1)),			// src0
			   opnd_reg(FP0 + s0_st_i, get_type(s0)));	// src1
	set_dst(mj, 1, opnd_reg(FPFLAGS, type_u16));
	emit(mj);

	// pop off both operands, result in FPFLAGS
	fp_stack.pop();
	fp_stack.pop();

	// transfer result to AX
	mj = new_instr_alm(opnd_reg(AX, type_u16), FSTSW,
			   opnd_reg(FPFLAGS, type_u16));
	set_dst(mj, 1, opnd_reg(FPFLAGS, type_u16));
	add_regs_defd(mj, AL, AH);
	emit(mj);

	if (is_reg(d) && (get_reg(d) == EFLAGS)) {
	    // user just wants the result of the compare in eflags
	    set_opcode(mi, SAHF);
	    set_src(mi, 0, opnd_reg(AH, type_u8));
	    set_src(mi, 1, opnd_null());
	    emit(mi);
	    return;
	}

	// Otherwise, caller wants a boolean result of the compare in
	// the specified destination.  Create the appropriate SETCC
	// so that the right TEST and cbranch are created later.
	set_src(mi, 0, opnd_reg(AX, type_u16));
	add_regs_used(mi, AL, AH);
	int setcc_op;
	switch (get_opcode(mi)) {
	  case suifvm::SEQ:
	    setcc_op = SETNE;
	    set_src(mi, 1, opnd_immed(0x4000, type_u16));
	    break;
	  case suifvm::SNE:
	    setcc_op = SETE;
	    set_src(mi, 1, opnd_immed(0x4000, type_u16));
	    break;
	  case suifvm::SL:
	    setcc_op = SETNE;
	    set_src(mi, 1, opnd_immed(0x0100, type_u16));
	    break;
	  case suifvm::SLE:
	    setcc_op = SETNE;
	    set_src(mi, 1, opnd_immed(0x4100, type_u16));
	    break;
	  default:
	    claim(false, "translate_fp -- unexpected SETcc opcode");
	}
	translate_iscc(TEST, setcc_op, setcc_op, mi);
	return;

    } else if (opcode != opcode_null) {
	mj = new_instr_alm(opnd_reg(FP0 + s0_st_i, get_type(s0)),
			   opcode, 
			   opnd_reg(FP0 + s0_st_i, get_type(s0)),
			   opnd_reg(FP0, get_type(s1)));
	set_dst(mj, 1, opnd_reg(FPFLAGS, type_u16));
	emit(mj);
    }

    if (mj != NULL)
	copy_notes(mi, mj);	// transfer annotations onto last new instr
    else if (has_notes(mi))
	warn("translate_fp() -- dropping annotations");

    // Do execution bookkeeping for the FP stack.
    if (opcode==FADDP || opcode==FDIVRP || opcode==FMULP || opcode==FSUBRP) {
	fp_stack.pop();		// pop off old ST
    }

    // Do any processing required by destination.  Actual processing
    // depends upon where the fp instruction left its result.
    if (is_var(d)) {	// normal fp operation that left result in ST
	// pop off ST and place its contents in memory
	mj = fp_stack.pop(d);
	emit(mj);
    } else if (is_reg(d)) {
	// rename the result value at ST to be this reg's name
	fp_stack.rename_st(get_reg(d));
    }
}


void
CodeGenX86::translate_mov(Instr *mi)
{
    if (is_floating_point(get_type(get_dst(mi)))) {
	// opcode is null because copy handled by src/dst processing
	translate_fp(opcode_null, mi);
	delete mi;
    } else {
	set_opcode(mi, x86::MOV);
	emit(mi);
    }
}


/* Translate function pattern #2 -- integer translation that requires
 * a fix for the two-operand world, or a call to translate_fp if the
 * values are floating-point values.  For pointer arithmetic, it may
 * be necessary to widen (sign- or zero-extend) the integral operand
 * if its width is less than a pointer's. */
#ifdef TRANSLATE_FUN_2
#undef TRANSLATE_FUN_2
#endif
#define TRANSLATE_FUN_2(from_opcode, to_int_opcode, to_fp_opcode,	 \
			maybe_widen_index_opnd)				 \
void									 \
CodeGenX86::translate_##from_opcode(Instr *mi) {			 \
    if (is_floating_point(get_type(get_dst(mi)))) {			 \
	translate_fp(to_fp_opcode, mi);					 \
	delete mi;							 \
    } else {								 \
	maybe_widen_index_opnd(mi);					 \
	Opnd old_dst = widen_opnds(mi, to_int_opcode != x86::NEG);	 \
	set_opcode(mi, to_int_opcode);					 \
	set_dst(mi, 1, opnd_reg_eflags);				 \
	mi = make_2opnd(mi);						 \
	emit(mi);							 \
	if (!is_null(old_dst)) {					 \
	    Opnd new_dst = get_dst(mi);					 \
	    translate_cvt(new_instr_alm(old_dst, suifvm::CVT, new_dst)); \
	}								 \
    }									 \
}

#define no_widen(instr)

TRANSLATE_FUN_2(add, x86::ADD,  FADDP,  widen_index_opnd)
TRANSLATE_FUN_2(sub, x86::SUB,  FSUBRP, widen_index_opnd)
TRANSLATE_FUN_2(mul, x86::IMUL, FMULP,  no_widen)
TRANSLATE_FUN_2(neg, x86::NEG,  FCHS,   no_widen)

/*
 * widen_index_opnd -- In a pointer-valued add or sub instruction, the
 * integer index operand is not guaranteed to have the width of a
 * pointer.  For x86, two values being added must have the same width.
 * Find the index operand (if any) and convert it to pointer width.
 */
void
CodeGenX86::widen_index_opnd(Instr *mi)
{
    if (is_pointer(get_type(get_dst(mi))))
	for (int j = 1; j >= 0; --j) {
	    Opnd srcj = get_src(mi, j);
	    TypeId sjtype = get_type(srcj);
	    if (is_integral(sjtype)) {
		if (get_bit_size(sjtype) != 32) {
		    if (!is_reg(srcj) && !is_var(srcj)) {
			Opnd srcj_reg = opnd_reg(get_type(srcj));
			emit(new_instr_alm(srcj_reg, x86::MOV, srcj));
			srcj = srcj_reg;
		    }
		    TypeId t32 = is_signed(sjtype) ? type_s32 : type_u32;
		    Opnd idx32 = opnd_reg(t32);
		    translate_cvt(new_instr_alm(idx32, suifvm::CVT, srcj));
		    set_src(mi, j, idx32);
		    break;
		}
	    }
	}
}

#ifndef CFE_CONVERSION_FIXED
/*
 * widen_opnds -- FIXME: This routine should not be needed!
 *
 * The C front end should perform "usual unary or binary conversions".  The
 * old version of CFE leaves input operands smaller than int unconverted.
 * The current CFE still gets this wrong for array-subscript scaling.
 * As a workaround, check for these cases and convert to 32-bit int.
 *
 * If the destination is integral, but narrower than int, then replace it
 * by a new 32-bit virtual register and return the original destination.
 * Otherwise, return a null operand. 
 */
Opnd
CodeGenX86::widen_opnds(Instr *mi, bool is_binary)
{
    for (int j = (int)is_binary; j >= 0; --j) {
	Opnd srcj = get_src(mi, j);
	TypeId sjtype = get_type(srcj);
	if (is_integral(sjtype) && get_bit_size(sjtype) < 32) {
	    if (!is_reg(srcj) && !is_var(srcj)) {
		Opnd srcj_reg = opnd_reg(get_type(srcj));
		emit(new_instr_alm(srcj_reg, x86::MOV, srcj));
		srcj = srcj_reg;
	    }
	    TypeId t32 = is_signed(sjtype) ? type_s32 : type_u32;
	    Opnd srcj32 = opnd_reg(t32);
	    translate_cvt(new_instr_alm(srcj32, suifvm::CVT, srcj));
	    set_src(mi, j, srcj32);
	}
    }
    Opnd dst = get_dst(mi);
    TypeId dtype = get_type(dst);

    if (is_integral(dtype) && get_bit_size(dtype) < 32) {
	TypeId t32 = is_signed(dtype) ? type_s32 : type_u32;
	Opnd dst32 = opnd_reg(t32);
	set_dst(mi, dst32);
	return dst;
    }
    return opnd_null();
}
#endif

void
CodeGenX86::translate_abs(Instr *mi)
{
    if (is_floating_point(get_type(get_dst(mi)))) {
	translate_fp(FABS, mi);
	delete mi;
    } else {
	// create a scratch register and a new label
	Opnd d = get_dst(mi), vr = opnd_reg(type_s32);
	LabelSym *l_positive = new_unique_label();

	// first, set cc register based on sign-bit -- use existing mi
	set_dst(mi, 0, vr);
	set_dst(mi, 1, opnd_reg_eflags);
	set_opcode(mi, x86::SUB);
	// leave src0 alone
	set_src(mi, 1, opnd_immed(0, type_s8));
	mi = make_2opnd(mi);
	emit(mi);				// sub  vr <- s,0

	// next, branch if positive already
	mi = new_instr_cti(JNS, l_positive, opnd_reg_eflags);
	emit(mi);

	// neg if necessary
	mi = new_instr_alm(vr, x86::NEG, vr);
	set_dst(mi, 1, opnd_reg_eflags);
	emit(mi);

	// insert jump label
	mi = new_instr_label(l_positive);
	emit(mi);

	// final move to real destination of abs instruction
	mi = new_instr_alm(d, x86::MOV, vr);
	emit(mi);
    }
}


/* --------------- integer division ----------------- */

void
CodeGenX86::create_div_sequence(Instr *mi)
{
    Instr *mj;
    int opc_suifvm = get_opcode(mi);
    Opnd s0 = get_src(mi, 0), s1 = get_src(mi, 1), d = get_dst(mi);

    // determine which operation to perform
    TypeId t = get_type(d);
    int opc_x86 = (is_signed(t) ? x86::IDIV : x86::DIV);

    // first, put s0 into EAX
    mj = new_instr_alm(opnd_reg(EAX, get_type(s0)), x86::MOV, s0);
    add_regs_defd(mj, AL, AH);
    emit(mj);

    // next, initialize EDX appropriately
    if (opc_x86 == x86::IDIV) {
	// sign-extend EAX into EDX
	mj = new_instr_alm(opnd_reg(EDX, type_s32), CDQ, 
			   opnd_reg(EAX, get_type(s0)));
	add_regs_defd(mj, DL, DH);
	add_regs_used(mj, AL, AH);
	t = type_s32;
    } else {
	mj = new_instr_alm(opnd_reg(EDX, type_u32), x86::XOR,
			   opnd_reg(EDX, type_u32),
			   opnd_reg(EDX, type_u32));
	set_dst(mj, 1, opnd_reg_eflags);
	add_regs_defd(mj, DL, DH);
	add_regs_used(mj, DL, DH);
	t = type_u32;
    }
    emit(mj);

    // Now, create the chosen divide operation.  We also make
    // explicit the implicit use/def of EDX and the def of EFLAGS.
    set_opcode(mi, opc_x86);
    set_dst(mi, 0, opnd_reg(EAX, get_type(s0)));
    set_dst(mi, 1, opnd_reg_eflags);
    set_dst(mi, 2, opnd_reg(EDX, t));
    set_src(mi, 0, opnd_reg(EAX, get_type(s0)));
    // src1 = s1 already
    set_src(mi, 2, opnd_reg(EDX, t));
    add_regs_defd(mi, AL, AH);
    add_regs_defd(mi, DL, DH);
    add_regs_used(mi, AL, AH);
    add_regs_used(mi, DL, DH);
    emit(mi);

    // finally, put the result (EAX for division, EDX for modulo) in d
    if (opc_suifvm == suifvm::DIV) {
	mj = new_instr_alm(d, x86::MOV, opnd_reg(EAX, t));
	add_regs_used(mj, AL, AH);
    } else if (opc_suifvm == suifvm::REM) {
	mj = new_instr_alm(d, x86::MOV, opnd_reg(EDX, t));
	add_regs_used(mj, DL, DH);
    } else 
	claim(false, 
	      "create_div_sequence() -- unexpected opcode %d", opc_suifvm);
    emit(mj);
}

void
CodeGenX86::translate_div(Instr *mi)
{
    if (is_floating_point(get_type(get_dst(mi)))) {
	translate_fp(FDIVRP, mi);
	delete mi;
    } else {
	create_div_sequence(mi);
    }
}

void
CodeGenX86::translate_rem(Instr *mi)
{
    claim(!is_floating_point(get_type(get_src(mi, 0))));
    create_div_sequence(mi);
}


/* --------------- shift translations --------------- */

void
CodeGenX86::translate_asr(Instr *mi)
{
    Instr *mj;

    // count must be in ECX
    mj = new_instr_alm(opnd_reg(ECX, type_u32), x86::MOV, get_src(mi, 1));
    add_regs_defd(mj, CL, CH);
    emit(mj);

    // rewrite asr instruction
    set_opcode(mi, SAR);
    set_src(mi, 1, opnd_reg(CL, type_u8));
    set_dst(mi, 1, opnd_reg_eflags);
    mi = make_2opnd(mi);
    emit(mi);
}

void
CodeGenX86::translate_lsl(Instr *mi)
{
    Instr *mj;

    // count must be in ECX
    mj = new_instr_alm(opnd_reg(ECX, type_u32), x86::MOV, get_src(mi, 1));
    add_regs_defd(mj, CL, CH);
    emit(mj);

    // rewrite lsl instruction
    set_opcode(mi, SHL);
    set_src(mi, 1, opnd_reg(CL, type_u8));
    set_dst(mi, 1, opnd_reg_eflags);
    mi = make_2opnd(mi);
    emit(mi);
}

void
CodeGenX86::translate_lsr(Instr *mi)
{
    Instr *mj;

    // count must be in ECX
    mj = new_instr_alm(opnd_reg(ECX, type_u32), x86::MOV, get_src(mi, 1));
    add_regs_defd(mj, CL, CH);
    emit(mj);

    // rewrite lsr instruction
    set_opcode(mi, SHR);
    set_src(mi, 1, opnd_reg(CL, type_u8));
    set_dst(mi, 1, opnd_reg_eflags);
    mi = make_2opnd(mi);
    emit(mi);
}


/* --------------- conditional branch translations --------------- */

void
CodeGenX86::translate_jmp(Instr *mi)
{
    set_opcode(mi, x86::JMP);
    emit(mi);
}

void
CodeGenX86::translate_jmpi(Instr *mi)
{
    set_opcode(mi, x86::JMP);
    emit(mi);
}

/* branch on true, <>0 */
void
CodeGenX86::translate_btrue(Instr *mi)
{
    Opnd s = get_src(mi, 0);
    Instr *mj = new_instr_alm(opnd_reg_eflags, x86::CMP, s,
			      opnd_immed(0, get_type(s)));
    emit(mj);
    set_opcode(mi, JNE);
    set_src(mi, 0, opnd_reg_eflags);
    emit(mi);
}

/* branch on false, ==0 */
void
CodeGenX86::translate_bfalse(Instr *mi)
{
    Opnd s = get_src(mi, 0);
    Instr *mj = new_instr_alm(opnd_reg_eflags, x86::CMP, s,
			      opnd_immed(0, get_type(s)));
    emit(mj);
    set_opcode(mi, JE);
    set_src(mi, 0, opnd_reg_eflags);
    emit(mi);
}


void
CodeGenX86::translate_bcc(int signed_opcode, int unsigned_opcode,
			  Instr *mi)
{
    Instr *mj;
    Opnd s0 = get_src(mi, 0), s1 = get_src(mi, 1);
    TypeId s0_t = get_type(s0);

    if (is_floating_point(s0_t)) {
	// Generate something that will compare the source operands.
	// Note that mj's opcode doesn't matter.
	mj = new_instr_alm(opnd_reg_eflags, FCOMPP, s0, s1);
	translate_fp(FCOMPP, mj);
    } else {
	// generate a compare instruction
	mj = new_instr_alm(opnd_reg_eflags, x86::CMP, s0, s1);
	widen_opnds(mj, true);		// FIXME: front end should do this
	emit(mj);
    }

    // perform the correct conditional branch
    if (is_integral(s0_t) && is_signed(s0_t)) {
	set_opcode(mi, signed_opcode);
    } else {
	set_opcode(mi, unsigned_opcode);
    }
    set_src(mi, 0, opnd_reg_eflags);
    set_src(mi, 1, opnd_null());
    emit(mi);
}

void
CodeGenX86::translate_beq(Instr *mi)
{
    translate_bcc(JE, JE, mi);
}

void
CodeGenX86::translate_bne(Instr *mi)
{
    translate_bcc(JNE, JNE, mi);
}

void
CodeGenX86::translate_bge(Instr *mi)
{
    translate_bcc(JGE, JAE, mi);
}

void
CodeGenX86::translate_bgt(Instr *mi)
{
    translate_bcc(JG, JA, mi);
}

void
CodeGenX86::translate_ble(Instr *mi)
{
    translate_bcc(JLE, JBE, mi);
}

void
CodeGenX86::translate_blt(Instr *mi)
{
    translate_bcc(JL, JB, mi);
}


/* --------------- set conditional translations --------------- */

/* translate_iscc() -- in x86, the setcc instruction relies on a
 * preceding cmp instruction to set the condition code register.  It
 * also affects only the bottom 8 bits of the destination register, so
 * we have to perform a sign-extend to 32 bits.
 * Args:
 *  cmp_op	comparison opcode
 *  setcc_op	setcc for signed comparison
 *  usetcc_op	setcc for unsigned comparison
 *  mi		compare instr being translated
 */
void
CodeGenX86::translate_iscc(int cmp_op, int setcc_op, int usetcc_op, Instr *mi)
{
    Instr *mj, *mk;
    Opnd vr = opnd_reg(type_u8);
    widen_opnds(mi, true);		// FIXME: front end should do this

    TypeId arg_type = get_type(get_src(mi, 0));
    bool have_sign = is_integral(arg_type) && is_signed(arg_type);

    // First, create the version of the setcc instruction that uses
    // the cc register and an instruction to sign-extend its result
    // to 32 bits.
    mj = new_instr_alm(vr, (have_sign ? setcc_op : usetcc_op), opnd_reg_eflags);
    mk = new_instr_alm(get_dst(mi), MOVSX, vr);

    // Change the original mi so that it now sets the cc register.
    // The instruction used to compute the condition codes depends
    // upon the type of the operands in the original SUIF setcc
    // instruction.  Integer setcc comparisons use CMP while FP
    // setcc comparisons use TEST.
    set_opcode(mi, cmp_op);
    set_dst(mi, opnd_reg_eflags);
    emit(mi);				// compare/test
    emit(mj);				// setcc
    emit(mk);				// sign-extend
}


/* Translate function pattern #4 */
#ifdef TRANSLATE_FUN_4
#undef TRANSLATE_FUN_4
#endif
#define TRANSLATE_FUN_4(from_opcode, to_int_opcode, to_uint_opcode)  \
void								     \
CodeGenX86::translate_##from_opcode(Instr *mi) {		     \
    if (is_floating_point(get_type(get_src(mi, 0)))) {		     \
	translate_fp(FCOMPP, mi);				     \
    } else {							     \
	translate_iscc(x86::CMP, to_int_opcode, to_uint_opcode, mi); \
    }								     \
}

TRANSLATE_FUN_4(seq, SETE,  SETE)
TRANSLATE_FUN_4(sne, SETNE, SETNE)
TRANSLATE_FUN_4(sl,  SETL,  SETB)
TRANSLATE_FUN_4(sle, SETLE, SETBE)


/* --------------- no translation --------------- */

void
CodeGenX86::translate_min(Instr *mi)
{
    claim(false);
}

void
CodeGenX86::translate_max(Instr *mi)
{
    claim(false);
}

void
CodeGenX86::translate_mod(Instr *mi)
{
    claim(false);
}

void
CodeGenX86::translate_rot(Instr *mi)
{
    claim(false);
}

/*
 * translate_any() -- Translate a generic instruction.
 *
 * Currently, the only expected ANY instructions are __builtin_va_start,
 * __builtin_va_arg, and __builtin_va_end, all of which are produced by
 * expansions of the macros in varargs.h/stdarg.h
 */
void
CodeGenX86::translate_any(Instr *mi)
{
    ListNote<IrObject*> args_note = take_note(mi, k_builtin_args);
    claim(!is_null(args_note),
	  "Found an ANY instruction that's not a builtin");

    OneNote<IdString> opcode_name_note = take_note(mi, k_instr_opcode);
    claim(!is_null(opcode_name_note));
    const char *opcode_name = opcode_name_note.get_value().chars();

    if (strcmp(opcode_name, "__builtin_va_start") == 0) {
	claim(is_varargs && get_formal_param_count(cur_unit) > 0);

	// Expecting three sources: (1) the symbol for the variable that is
	// to hold the non-fixed arg pointer (nicknamed "ap"): (2) an
	// argument (nicknamed "pn" for "parameter n") from the varargs
	// procedure argument list (va_alist for varargs; the last fixed
	// (named) argument for stdarg); and (3) an immediate-integer flag:
	// 0 for varargs.h style and 1 for stdarg.h style.

	claim(srcs_size(mi) == 3);
	claim(dsts_size(mi) == 0);
	Opnd ap_src = get_src(mi, 0);
	Opnd pn_src = get_src(mi, 1);
	Opnd is_std = get_src(mi, 2);

	claim(is_var(ap_src));

	// The 3rd parameter to __builtin_va_start is either zero for
	// varargs.h style or one for stdarg.h style.
	claim(is_immed_integer(is_std));
	bool is_stdarg = get_immed_integer(is_std) == 1;
	claim(is_stdarg || get_immed_integer(is_std) == 0);

	// Under Linux/x86, variable `ap' has type void* and it points to
	// the next unnamed arg value.  In the stdarg.h case, init ap to
	// the 4-byte boundary just above the contents of `parmn'.  In the
	// varargs.h case, initialize it to the address of the first arg,
	// which is 8(%ebp), allowing for saved %ebp and the return address.

	if (is_stdarg) {
	    VarSym *pn_var = get_var(pn_src);
	    int pn_size = get_bit_size(get_type(pn_var));
	    Opnd disp = opnd_immed(((pn_size + 31) / 32) * 4, type_s32);

	    Opnd pn_addr = opnd_addr_sym(pn_var);
	    set_addr_taken(pn_var, true);
	    emit(new_instr_alm(ap_src, LEA, SymDispOpnd(pn_addr, disp)));
	} else {
	    Opnd p0_addr = BaseDispOpnd(opnd_reg_fp, opnd_immed(8, type_s32));
	    emit(new_instr_alm(ap_src, LEA, p0_addr));
	}
	delete mi;
    }
    else if (strcmp(opcode_name, "__builtin_va_arg") == 0) {
	// Expecting one source: the variable (ap) that holds the struct
	// that identifies the current unnamed arg.  Expecting one
	// destination: the register in which to leave the address of the
	// va_arg(...) value.  Unlike most VRs, this opnd has a precise
	// pointer type.  It's referent gives the type of the va_arg value,
	// which is needed to locate the value and to advance ap.

	claim(srcs_size(mi) == 1);
	Opnd ap = get_src(mi, 0);

	claim(dsts_size(mi) == 1);
	Opnd dst = get_dst(mi);

	// First, copy ap to the destination.
	emit(new_instr_alm(dst, x86::MOV, ap));

	// Then advance ap by the least multiple of 4 bytes that can
	// contain the current value.
	TypeId value_type = get_referent_type(get_type(dst));
	int value_size = ((get_bit_size(value_type) + 31) / 32) * 4;

	Instr *mj =
	    new_instr_alm(ap, x86::ADD, ap, opnd_immed(value_size, type_s32));
	set_dst(mj, 1, opnd_reg_eflags);
	mj = make_2opnd(mj);
	emit(mj);
	delete mi;
    }
    else if (strcmp(opcode_name, "__builtin_va_end") == 0) {
	// Do nothing at all for va_end(ap).
	delete mi;
    } else {
	claim(false, "translate_any() -- unexpected builtin name");
    }
}


/* --------------- complex cti's --------------- */

/*
 * translate_mbr() -- Assume that the case constants form an increasing,
 * non-empty sequence of integers.  Assume that the source operand is in
 * range.  (The current SUIF front end only creates a multi-way branch
 * under these circumstances.)
 *
 * Create a dispatch table.  This creation will update the
 * instr_mbr_tgts note by incorporating the dispatch-table symbol as
 * its first element.
 *
 * Emit code to compute the dispatch-table index in VR `idx', then more
 * code to load the dispatch-table entry address into VR `ptr', adding a
 * mbr_index_def note to the last of these instructions for HALT's
 * sake.  Then emit a load-indirect through `ptr' to put the target address
 * in VR `tgt'.
 *
 * Finally, convert the original SUIFvm mbr into an indirect jump via
 * `tgt'.  */
void
CodeGenX86::translate_mbr(Instr *mi)
{
    MbrNote note = get_note(mi, k_instr_mbr_tgts);
    claim(!is_null(note));

    VarSym *dtsym = new_dispatch_table_var(note);

    Instr *mj;
    Opnd idx, tgt = opnd_reg(type_addr());
    int lower = note.get_case_constant(0);

    // define idx
    if (lower == 0)
	idx = get_src(mi, 0);			// let idx be src itself
    else {
	idx = opnd_reg(type_s32);
	mj = new_instr_alm(idx, x86::ADD, get_src(mi, 0),
			   opnd_immed(-lower, type_s32));
	mj = make_2opnd(mj);
	emit(mj);				// addl src,-lower -> idx
    }

    // set tgt to address of idx label in dispatch table
    mj = new_instr_alm(tgt, x86::MOV,
		       IndexScaleDispOpnd(idx, opnd_immed(4, type_u8),
					  opnd_addr_sym(dtsym)));
    set_note(mj, k_mbr_table_use,  note_flag());
    set_note(mj, k_mbr_target_def, note_flag());
    emit(mj);					// movl dtsym(,idx,4) -> tgt

    set_opcode(mi, x86::JMP);
    set_src(mi, 0, tgt);
    set_target(mi, NULL);			// jmp *tgt
    emit(mi);
}

/*
 * Translate a call instruction.
 */
void
CodeGenX86::translate_cal(Instr *mi)
{
    Opnd ea;
    Instr *mj;

    // On the x86, the caller is responsible for fixing the stack pointer
    // after a call returns.  This variable keeps track of how many bytes
    // were pushed on the stack for arguments.
    int arg_area = 0;
    int argi;			// argument index
    int num_args = srcs_size(mi) - 1;

    // Plow through the parameters, creating argument build
    // instructions (or information) as we go.  Since we push arguments
    // onto the x86 stack, we push the last argument first.
    for (argi = num_args; argi >= 1; argi--) {
	Opnd arg_opnd = get_src(mi, argi);
	TypeId arg_type = get_type(arg_opnd);

	// Perform argument area size calculation.  All arguments are
	// expanded to be a multiple of 4 bytes in size.  Double words
	// do not necessarily begin at a double-word-aligned address.
	// After this code, arg_size represents the total number of
	// bytes (after expansion) on the stack allocated to this argument.
	int arg_size = get_bit_size(arg_type) >> 3;
	int bytes_over_word_boundary;
	if ((bytes_over_word_boundary = (arg_size & 3)))
	    arg_size += (4 - bytes_over_word_boundary);
	arg_area += arg_size;

	// Parameter build instructions -- 3 scenarios
	//  (1)	structure (or union) operand;
	//  (2) FP operand;
	//  (3) integer operand.

	if (!is_scalar(arg_type)) {				// case 1
	    // first, create space on stack for structure argument
	    mj = new_instr_alm(opnd_reg_sp, x86::SUB, opnd_reg_sp,
			       opnd_immed(arg_size, type_s32));
	    set_dst(mj, 1, opnd_reg_eflags);
	    emit(mj);

	    // next, find or generate corresponding structure load
	    Instr *mi_ld = NULL;

	    for (InstrHandle h=struct_lds.begin(); h!=struct_lds.end(); ++h)
		if (get_dst(*h) == arg_opnd) {
		    mi_ld = *h;
		    struct_lds.erase(h);
		    break;
		}

	    if (mi_ld == NULL) {
		// need to create structure load
		VarSym *v = get_var(arg_opnd);
		arg_opnd = opnd_reg(arg_type);
		mi_ld = new_instr_alm(arg_opnd, suifvm::LOD, opnd_addr_sym(v));
	    }

	    // generate the struct store into parameter space
	    ea = BaseDispOpnd(opnd_reg_sp, opnd_immed(0, type_s32), arg_type);
	    Instr *mi_st = new_instr_alm(ea, suifvm::STR, arg_opnd);

	    // expand the structure move into real ops
	    expand_struct_move(mi_ld, mi_st);

	} else if (is_floating_point(arg_type)) {		// case 2
	    // first, create space on stack for fp argument
	    mj = new_instr_alm(opnd_reg_sp, x86::SUB, opnd_reg_sp,
			       opnd_immed(arg_size, type_s32));
	    set_dst(mj, 1, opnd_reg_eflags);
	    emit(mj);

	    // next, get argument at FP st(0)
	    if (is_var(arg_opnd)) {
		mj = fp_stack.push(arg_opnd);
		emit(mj);
	    } else if (is_reg(arg_opnd)) {
		mj = fp_stack.exchange(arg_opnd);
		if (mj) emit(mj);
	    } else {
		claim(false,
		      "translate_cal() -- unexpected opnd for arg %n", argi);
	    }

	    // finally, push argument on memory stack
	    ea = BaseDispOpnd(opnd_reg_sp, opnd_immed(0, type_s32), arg_type);
	    mj = fp_stack.pop(ea);
	    emit(mj);

	} else {						// case 3
	    // Push the operand on the stack after widening to 32 bits.
	    // Use dst opnds to note that PUSH writes memory and updates ESP.

	    if (get_bit_size(arg_type) < 32) {
		Opnd orig_arg = arg_opnd;
		arg_opnd = opnd_reg(is_signed(arg_type) ? type_s32 : type_u32);
		translate_cvt(new_instr_alm(arg_opnd, suifvm::CVT, orig_arg));
	    }
	    ea = BaseDispOpnd(opnd_reg_sp, opnd_immed(-4, type_s32));
	    mj = new_instr_alm(ea, PUSH, arg_opnd);
	    set_dst(mj, 1, opnd_reg_sp);
	    emit(mj);
	}
    }

    // FP stack is caller-saved.  If it's not empty, save active contents.
    int fp_stack_depth = fp_stack.count();
    for (int d = 0; d < fp_stack_depth; ++d) {
	Opnd disp = opnd_immed(8 * d, type_s32);
	Opnd addr = SymDispOpnd(opnd_addr_sym(fp_stack_safe), disp, type_f64);
	mj = new_instr_alm(addr, FSTP, opnd_reg(FP0, type_f64));
	set_dst(mj, 1, opnd_reg(FPFLAGS, type_u16));
	emit(mj);
    }

    // Generate actual call instruction -- call pushes EIP onto the
    // stack (and thus updates the stack pointer).
    ea = BaseDispOpnd(opnd_reg_sp, opnd_immed(0, type_s32));
    mj = new_instr_cti(ea, x86::CALL, get_target(mi), get_src(mi, 0));
    set_dst(mj, 1, opnd_reg_sp);
    copy_notes(mi, mj);
    emit(mj);

    Instr *call_mi = mj;		// remember this ptr

    fp_stack.reflect_call(mi);		// model the call in fp_stack

    // Caller removes arguments pushed onto the call stack, if any.
    if ((arg_area) > 0) {
	mj = new_instr_alm(opnd_reg_sp, x86::ADD, opnd_reg_sp, 
			   opnd_immed(arg_area, type_s32));
	set_dst(mj, 1, opnd_reg_eflags);
	emit(mj);
    }

    // Set regs_defd annotation to indicate that caller-saved regs
    // may be killed and a return register will be defined.
    NatSetDense regs_defd;
    regs_defd += *reg_caller_saves();

    // Generate copy of return value, if necessary.
    bool have_fp_reg_result = false;	// FP result remains in ST(0)
    if (dsts_size(mi)) {
	Opnd d = get_dst(mi);
	TypeId d_type = get_type(d);
	claim(is_scalar(d_type));	// should'a been fixed

	if (is_floating_point(d_type)) {
	    // result returned in ST(0)
	    if (is_var(d)) {
		// pop off ST and place its contents in memory
		mj = fp_stack.pop(d);
		emit(mj);
	    } else if (is_reg(d)) {
		// rename the result value at ST to be this reg's name
		fp_stack.rename_st(get_reg(d));
		have_fp_reg_result = true;
	    }
	    regs_defd.insert(FP0);

	} else {
	    // result returned in EAX
	    mj = new_instr_alm(d, x86::MOV, opnd_reg(EAX, d_type));
	    add_regs_used(mj, AL, AH);
	    emit(mj);
	}
    }
    NatSetNote note;
    note.set_set(&regs_defd);
    set_note(call_mi, k_regs_defd, note);

    // Restore the FP stack, if necessary.
    for (int d = fp_stack_depth - 1; d >= 0; --d) {
	Opnd disp = opnd_immed(8 * d, type_s32);
	Opnd addr = SymDispOpnd(opnd_addr_sym(fp_stack_safe), disp, type_f64);
	mj = new_instr_alm(opnd_reg(FP0, type_f64), FLD, addr);
	set_dst(mj, 1, opnd_reg(FPFLAGS, type_u16));
	emit(mj);
    }
    // If there is a procedure result on the FP stack, it may have been
    // pushed down by restoring saved values.  If so, bring it back to the
    // top of the FP stack.  Use
    //    FLD   st(d),st(0)		#  push ST(d) onto FP stack
    //    FFREE st(d+1)			#  free stack entry just vacated
    // where d is the depth of the pre-call FP stack.

    if (have_fp_reg_result && fp_stack_depth > 0 ) {
	mj = new_instr_alm(opnd_reg(FP0, type_f64), FLD,
			   opnd_reg(FP0 + fp_stack_depth, type_f64));
	set_dst(mj, 1, opnd_reg(FPFLAGS, type_u16));
	emit(mj);
	claim(fp_stack_depth < 7, "FP stack too deep across call");
	mj = new_instr_alm(FFREE, opnd_reg(FP0 + fp_stack_depth + 1, type_f64));
	set_dst(mj, 1, opnd_reg(FPFLAGS, type_u16));
	emit(mj);
    }

    // Update per-procedure variables
    claim((arg_area & 3) == 0);	// better be aligned on byte boundary
    is_leaf = false;
    if (max_arg_area < arg_area)
	max_arg_area = arg_area;

    delete mi;			// done with it
}


/* Translate RET instructions.  Appending of instructions to do
 * restoring of registers is done during the finishing pass. */
void
CodeGenX86::translate_ret(Instr *mi)
{
    Instr *mj;
    TypeId r_type = NULL;

    // If return value exists, then we need to get it in
    // correct hard return register.
    if (srcs_size(mi)) {
	Opnd r = get_src(mi, 0);
	r_type = get_type(r);

	if (is_void(r_type)) {
	    r_type = NULL;

	} else if (is_floating_point(r_type)) {
	    // x86 FP return values are placed in ST(0)
	    if (is_var(r)) {
		mj = fp_stack.push(r);
		emit(mj);
	    } else if (is_reg(r)) {
		mj = fp_stack.exchange(r);
		if (mj) emit(mj);
	    }

	} else {		// integer or ptr or enum type
	    // x86 return values are placed in EAX
	    mj = new_instr_alm(opnd_reg(EAX, r_type), x86::MOV, r);
	    add_regs_defd(mj, AL, AH);
	    emit(mj);
	}
    }

    // fix the return instruction
    set_opcode(mi, x86::RET);
    set_src(mi, 0, BaseDispOpnd(opnd_reg_sp, opnd_immed(0, type_s8)));
    set_dst(mi, 0, opnd_reg_sp);	// return pops EIP off the memory stack

    // remember that this is a return (and not a switch or something else) 
    ListNote<IrObject*> note;		// k_instr_ret
    if (r_type)
	note.set_value(0, r_type);
    set_note(mi, k_instr_ret, note);

    if (r_type) {
	// set regs_used annotation
	NatSetDense regs_used;
	if (is_floating_point(r_type))
	    regs_used.insert(FP0);
	else
	    regs_used.insert(EAX);
	NatSetNote note;
	note.set_set(&regs_used);
	set_note(mi, k_regs_used, note);
    }

    // remember that we need to restore the saved regs and update the
    // stack pointer during the finishing pass
    set_note(mi, k_incomplete_proc_exit, note_flag());

    emit(mi);					// ret $ra
}
