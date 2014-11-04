/* file "alpha/translate.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#include <machine/machine.h>

#include <alpha/init.h>
#include <alpha/opcodes.h>
#include <alpha/reg_info.h>
#include <alpha/code_gen.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

using namespace alpha;

/* Translate function pattern #1 -- Simplest translation.  Just update the
 * opcode.  In this pattern, there is only one target opcode possible. */
#ifdef TRANSLATE_FUN_1
#undef TRANSLATE_FUN_1
#endif
#define TRANSLATE_FUN_1(from_opcode, to_opcode) \
void \
CodeGenAlpha::translate_##from_opcode(Instr *mi) { \
    set_opcode(mi, to_opcode); \
    emit(mi); \
}

TRANSLATE_FUN_1(and,  alpha::AND)
TRANSLATE_FUN_1(asr,  alpha::SRA)
TRANSLATE_FUN_1(ior,  alpha::OR)
TRANSLATE_FUN_1(jmp,  alpha::BR)
TRANSLATE_FUN_1(jmpi, alpha::JMP)
TRANSLATE_FUN_1(not,  alpha::NOT)
TRANSLATE_FUN_1(xor,  alpha::XOR)


/* Translate function pattern #2 -- Here, we also just update the
 * opcode.  In this pattern however, there are multiple target
 * opcodes possible. */
#ifdef TRANSLATE_FUN_2
#undef TRANSLATE_FUN_2
#endif
#define TRANSLATE_FUN_2(from_opcode, to_opcode) \
void \
CodeGenAlpha::translate_##from_opcode(Instr *mi) { \
    TypeId dtype = get_type(get_dst(mi)); \
    int dsize = get_bit_size(dtype); \
    int opcode; \
    if (is_floating_point(dtype)) { \
	if (dsize == 64) \
	    opcode = to_opcode##T; \
	else \
	    opcode = to_opcode##S; \
    } else { \
	if (dsize == 64) \
	    opcode = report_int_overflow ? to_opcode##QV \
					 : to_opcode##Q; \
	else \
	    opcode = report_int_overflow ? to_opcode##LV \
					 : to_opcode##L; \
    } \
    set_opcode(mi, opcode); \
    emit(mi); \
}

TRANSLATE_FUN_2(add, alpha::ADD)
TRANSLATE_FUN_2(mul, alpha::MUL)
TRANSLATE_FUN_2(neg, alpha::NEG)
TRANSLATE_FUN_2(sub, alpha::SUB)



/*
 * translate_null() -- This routine handles opcode_null instructions.  If
 * this is a placeholder for the k_proc_entry note, then do entry-point
 * processing now: maybe save $gp for restoring after calls, and move or
 * spill register-passed parameters.
 */
void
CodeGenAlpha::translate_null(Instr *mi)
{
    if (!has_notes(mi))
	return;			// drop useless null instrs on the floor

    emit(mi);			// put register moves/spills after entry

    if (has_note(mi, k_proc_entry))
	transfer_params(cur_unit, is_varargs);
}


/* translate_nop() -- This routine handles NOP instructions.  We
 * convert nops into null instructions since a nop is useless, except
 * as a placeholder for information, before instruction scheduling. */
void
CodeGenAlpha::translate_nop(Instr *mi)
{
    if (has_notes(mi))		// append to instr list
	emit(mi);
    // else drop instr on the floor
}


/* translate_mrk() -- This routine currently handles "line"
 * annotations only.  The "line" annotations are translated into .loc
 * assembler directives.  The expansion of this single .loc pseudo-op
 * into .loc and .file is postponed until ASCII print time.
 *
 * All other mark instructions are handled by translating them into
 * null instructions. */
void
CodeGenAlpha::translate_mrk(Instr *mi)
{
    if (LineNote note = get_note(mi, k_line)) {
	const char *file = note.get_file().chars();
	claim((file && file[0]), "line note with null file name");
	set_opcode(mi, LOC);
    } else
	set_opcode(mi, opcode_null);
    emit(mi);
}

/* translate_ldc() -- This routine translates the LDC into an
 * LDIx (or its FP equivalent). */
void
CodeGenAlpha::translate_ldc(Instr *mi)
{
    TypeId dtype = get_type(get_dst(mi));

    // just need to determine the new opcode
    if (is_floating_point(dtype)) {
	int dsize = get_bit_size(dtype);
	if (dsize == 32)
	    set_opcode(mi, LDIS);
	else if (dsize == 64)
	    set_opcode(mi, LDIT);
	else
	    claim(false, "translate_ldc() -- FP LDC of size %d bits", dsize);
    } else {
	claim(is_immed_integer(get_src(mi, 0)),
	      "translate_ldc() -- GP LDC of non-integer");
	set_opcode(mi, (get_bit_size(dtype) == 64) ? LDIQ : LDIL);
    }
    emit(mi);
}

/*
 * translate_lda() -- This routine just replaces the suifvm::LDA opcode
 * with alpha::LDA.  However, when the address being loaded refers to a
 * global variable, we also attach a dependence on $gp in the form of a
 * "regs_used" annotation.
 */
void
CodeGenAlpha::translate_lda(Instr *mi)
{
    set_opcode(mi, LDA);
    maybe_use_reg_gp(mi);
    emit(mi);
}

void
CodeGenAlpha::translate_mov(Instr *mi)
{
    TypeId dtype = get_type(get_dst(mi));

    int opcode = alpha::MOV;
    if (is_floating_point(dtype))
	opcode = FMOV;

    set_opcode(mi, opcode);
    emit(mi);
}


void
CodeGenAlpha::translate_abs(Instr *mi)
{
    TypeId dtype = get_type(get_dst(mi));
    int dsize = get_bit_size(dtype);

    int opcode;
    if (is_floating_point(dtype))
	opcode = FABS;
    else {
	if (dsize == 64)
	    opcode = ABSQ;
	else
	    opcode = ABSL;
    }

    set_opcode(mi, opcode);
    emit(mi);
}


/*
 * Integer divide or remainder instructions are implemented as function
 * calls.
 */
void
CodeGenAlpha::emit_ots_call(IdString ots_proc, Instr *mi)
{
    // load each source that's an immed into a register for the call
    for (int i = 0; i <= 1; ++i)
	if (is_immed(get_src(mi, i))) {
	    Opnd vr = opnd_reg(get_type(get_src(mi, i)));
	    translate_ldc(new_instr_alm(vr, suifvm::LDC, get_src(mi, i)));
	    set_src(mi, i, vr);
	}
    Opnd dst  = get_dst(mi);
    Opnd src0 = get_src(mi, 0);
    Opnd src1 = get_src(mi, 1);

    TypeId type  = find_proc_type(type_s64, type_s64, type_s64);
    ProcSym *sym = find_proc_sym(type, ots_proc);

    Instr *call = new_instr_cti(dst, suifvm::CAL, sym, opnd_null(), src0);
    set_src(call, 2, src1);
    translate_cal(call);

    // delete the original div/rem instruction
    delete mi;
}

void
CodeGenAlpha::translate_div(Instr *mi) {
    TypeId dtype = get_type(get_dst(mi));
    int dsize = get_bit_size(dtype);

    if (is_floating_point(dtype)) {
	set_opcode(mi, (dsize == 64) ? DIVT : DIVS);
	emit(mi);
    } else {
	IdString ots_proc =
	  dsize == 64
	    ? (is_signed(dtype) ? k_OtsDivide64 : k_OtsDivide64Unsigned)
	    : (is_signed(dtype) ? k_OtsDivide32 : k_OtsDivide32Unsigned);
	emit_ots_call(ots_proc, mi);
    }
}

void
CodeGenAlpha::translate_rem(Instr *mi) {
    TypeId dtype = get_type(get_dst(mi));
    IdString ots_proc =
      (get_bit_size(dtype) == 64)
	? (is_signed(dtype) ? k_OtsRemainder64 : k_OtsRemainder64Unsigned)
	: (is_signed(dtype) ? k_OtsRemainder32 : k_OtsRemainder32Unsigned);

    emit_ots_call(ots_proc, mi);
}

void
CodeGenAlpha::translate_lsl(Instr *mi)
{
    int dsize = get_bit_size(get_type(get_dst(mi)));

    if (dsize == 64) {
	set_opcode(mi, alpha::SLL);
	emit(mi);

    } else {
	// There's a potential problem here.  Suif may attempt to perform
	// a lsl on a value that is not a quadword.  If for example, suif
	// attempts a lsl on an u.32, we may get the wrong result since
	// the hardware performs a lsl on an u.64 bit value.  We need to
	// guarantee that the upper 32 bits are correct.

	// do shift into a virtual register
	Opnd tmp = opnd_reg(type_s64);
	Instr *mj = new_instr_alm(tmp, alpha::SLL, get_src(mi, 0),
				  get_src(mi, 1));
	copy_notes(mi, mj);
	emit(mj);

	// Sign-extend to correct size--remember that an u.32
	// has a canonical form of s.32 in Alpha.
	claim((dsize == 32), "attempting a lsl on an unexpected type size");
	mj = new_instr_alm(get_dst(mi), ADDL, tmp, opnd_reg_const0);
	emit(mj);

	delete mi;
    }
}

void
CodeGenAlpha::translate_lsr(Instr *mi)
{
    TypeId dtype = get_type(get_dst(mi));
    claim(!is_floating_point(dtype));
    int dsize = get_bit_size(dtype);

    if (dsize == 64) {
	set_opcode(mi, alpha::SRL);
	emit(mi);

    } else {
	// There's a potential problem here.  Suif may attempt to perform
	// a lsr on a value that is not a quadword.  If for example, suif
	// attempts a lsr on an s.32, we may get the wrong result since
	// the hardware performs a lsr on an s.64 bit value.  We need to
	// guarantee that the upper 32 bits are zeros.
	Opnd tmp = opnd_reg(type_u64);

	// zap to correct size
	claim((dsize == 32), "attempting a lsr on an unexpected type size");
	Instr *mj = new_instr_alm(tmp, ZAPNOT, get_src(mi, 0),
				  opnd_immed(15, type_u64));
	emit(mj);

	// do the shift
	mj = new_instr_alm(tmp, alpha::SRL, tmp, get_src(mi, 1));
	copy_notes(mi, mj);
	emit(mj);

	// put back in canonical form
	mj = new_instr_alm(get_dst(mi), ADDL, tmp, opnd_reg_const0);
	emit(mj);

	delete mi;
    }
}

void
CodeGenAlpha::translate_min(Instr *mi)
{
    Opnd d = get_dst(mi), s0 = get_src(mi, 0), s1 = get_src(mi, 1);
    TypeId dtype = get_type(d);
    Opnd vr = opnd_reg(dtype);
    int mov_op, cmp_op, cmov_op;

    // determine opcodes
    if (is_floating_point(dtype)) {
	mov_op = FMOV;
	cmp_op = CMPTLT;
	cmov_op = CMOVNE;
    } else {
	mov_op = alpha::MOV;
	cmp_op = is_signed(dtype) ? CMPLT : CMPULT;
	cmov_op = CMOVNE;
    }

    // generate code sequence
    Instr *mj;
    mj = new_instr_alm(d, mov_op, s1);
    emit(mj);				// (f)mov s1 -> d

    set_opcode(mi, cmp_op);
    set_dst(mi, 0, vr);
    emit(mi);				// (f)cmp(u)lt s0,s1 -> vr

    mj = new_instr_alm(d, cmov_op, vr, s0);
    emit(mj);				// (f)cmovne vr,s0 -> d
}

void
CodeGenAlpha::translate_max(Instr *mi)
{
    Opnd d = get_dst(mi), s0 = get_src(mi, 0), s1 = get_src(mi, 1);
    TypeId dtype = get_type(d);
    Opnd vr = opnd_reg(dtype);
    int mov_op, cmp_op, cmov_op;

    // determine opcodes
    if (is_floating_point(dtype)) {
	mov_op = FMOV;
	cmp_op = CMPTLT;
	cmov_op = CMOVNE;
    } else {
	mov_op = alpha::MOV;
	cmp_op = is_signed(dtype) ? CMPLT : CMPULT;
	cmov_op = CMOVNE;
    }

    // generate code sequence
    Instr *mj;
    mj = new_instr_alm(d, mov_op, s1);
    emit(mj);				// (f)mov s1 -> d

    set_opcode(mi, cmp_op);
    set_dst(mi, 0, vr);
    set_src(mi, 0, s1);
    set_src(mi, 1, s0);
    emit(mi);				// (f)cmp(u)lt s1,s0 -> vr

    mj = new_instr_alm(d, cmov_op, vr, s0);
    emit(mj);				// (f)cmovne vr,s0 -> d
}


/* --------------- conditional branch translations --------------- */

void
CodeGenAlpha::translate_btrue(Instr *mi)
{
    int opcode =
	is_floating_point(get_type(get_src(mi, 0))) ? FBNE : alpha::BNE;
    set_opcode(mi, opcode);
    emit(mi);
}

void
CodeGenAlpha::translate_bfalse(Instr *mi)
{
    int opcode =
	is_floating_point(get_type(get_src(mi, 0))) ? FBEQ : alpha::BEQ;
    set_opcode(mi, opcode);
    emit(mi);
}

/*
 * translate_bcond() -- Common code for translate_b{eq,ne,ge,gt,le,lt} methods.
 * Replace a SUIFvm cbr instruction with a compare followed by an Alpha cbr,
 * which has one source: the result of the compare.
 *
 * Argument flip indicates whether the sources of the SUIFvm cbr should be
 * interchanged when creating the Alpha cbr.
 *
 * Arguments cmp and cbr (fcmp and fcbr) are the appropriate compare and
 * branch opcodes for the signed-integer (floating-point) case.  Argument
 * ucmp is the compare opcode for the unsigned-integer case.
 */
void
CodeGenAlpha::translate_bcond(Instr *mi, bool flip, int cmp, int ucmp,
			      int cbr, int fcmp, int fcbr)
{
    TypeId t = get_type(get_src(mi, 0));
    bool fp = is_floating_point(t);
    bool si = !fp && is_integral(t) && is_signed(t);

    Opnd cond = opnd_reg(fp ? type_f64 : type_u64);
    Opnd src0 = flip ? get_src(mi, 1) : get_src(mi, 0);
    Opnd src1 = flip ? get_src(mi, 0) : get_src(mi, 1);
    Instr *mj;

    // output the compare instruction
    mj = new_instr_alm(cond, (fp ? fcmp : si ? cmp : ucmp), src0, src1);
    emit(mj);

    // output the branch
    mj = new_instr_cti((fp ? fcbr : cbr), get_target(mi), cond);
    copy_notes(mi, mj);
    emit(mj);

    delete mi;
}

enum { KEEP = false, FLIP = true };

void
CodeGenAlpha::translate_beq(Instr *mi)
{
    translate_bcond(mi, KEEP,
		    CMPEQ, CMPEQ, alpha::BNE, CMPTEQ, FBNE);
}

void
CodeGenAlpha::translate_bne(Instr *mi)
{
    translate_bcond(mi, KEEP,
		    CMPEQ, CMPEQ, alpha::BEQ, CMPTEQ, FBEQ);
}

void
CodeGenAlpha::translate_bge(Instr *mi)
{
    translate_bcond(mi, FLIP,
		    CMPLE, CMPULE, alpha::BNE, CMPTLE, FBNE);
}

void
CodeGenAlpha::translate_bgt(Instr *mi)
{
    translate_bcond(mi, FLIP,
		    CMPLT, CMPULT, alpha::BNE, CMPTLT, FBNE);
}

void
CodeGenAlpha::translate_ble(Instr *mi)
{
    translate_bcond(mi, KEEP,
		    CMPLE, CMPULE, alpha::BNE, CMPTLE, FBNE);
}

void
CodeGenAlpha::translate_blt(Instr *mi)
{
    translate_bcond(mi, KEEP,
		    CMPLT, CMPULT, alpha::BNE, CMPTLT, FBNE);
}


/* --------------- set conditional translations --------------- */

/* Notes on sne instructions:
 *
 * There is no direct equivalent of the SUIF SNE instruction in
 * Alpha.  If the use of the SNE is in a branch instruction, then I
 * use an SEQ and reverse the sense of the branch (caught in the
 * BNE case above).  If the SNE is used by any other type of
 * instruction, I generate the set of instructions that produce the
 * desired SUIF semantics.
 *
 * To fix an integer SNE instruction, we use a subtract operation
 * which gives us a zero or not-zero result, and then we use an
 * unsigned less-than comparison with zero to drive not-zero to one. */

/* Notes on floating-point set operations:
 *
 * The size of source values (SP or DP) does not matter under Alpha
 * since all values in FP registers are converted to DP format
 * anyway and cmp instructions only use register operands.
 *
 * The s2m pass creates compare-and-branch instructions whenever
 * obvious, and thus when we encounter a FP set operation now, it
 * probably wants to create a real 0/1 value.  The FP compare
 * instructions in Alpha produce a FP value (0.5,0.0) instead of an
 * integer value.  I thus generate the set of instructions that
 * produce the desired SUIF semantics.
 */

void
CodeGenAlpha::translate_fp_scc(Instr *mi, int cmp_op, int fb_op)
{
    Instr *mj;
    Opnd d = get_dst(mi);
    Opnd ftmp = opnd_reg(type_f64);

    mj = new_instr_alm(d, LDIQ, opnd_immed_0_u64);
    emit(mj);				// ldiq 0 -> d

    set_opcode(mi, cmp_op);
    set_dst(mi, ftmp);
    emit(mi);				// cmp_op s1,s2 -> ftmp

    LabelSym *tgt = new_unique_label(the_suif_env, cur_unit);
    mj = new_instr_cti(fb_op, tgt, ftmp);
    emit(mj);				// fb_op ftmp, tgt

    mj = new_instr_alm(d, LDIQ, opnd_immed_1_u64);
    emit(mj);				// ldiq 1 -> d

    mj = new_instr_label(tgt);
    emit(mj);				// tgt:
}

void
CodeGenAlpha::translate_seq(Instr *mi)
{
    TypeId stype = get_type(get_src(mi, 0));
    if (is_floating_point(stype)) {
	translate_fp_scc(mi, CMPTEQ, FBEQ);
    } else {
	set_opcode(mi, CMPEQ);
	emit(mi);
    }
}

void
CodeGenAlpha::translate_sne(Instr *mi)
{
    TypeId stype = get_type(get_src(mi, 0));
    if (is_floating_point(stype)) {
	translate_fp_scc(mi, CMPTEQ, FBNE);
    } else {				// integer case
	// change SNE to SUB
	if (get_bit_size(stype) == 64)
	    set_opcode(mi, SUBQ);
	else
	    set_opcode(mi, SUBL);
	emit(mi);			// sub s1,s2 -> d

	Opnd d = get_dst(mi);
	Instr *mj;
	mj = new_instr_alm(d, CMPULT, opnd_reg_const0, d);
	emit(mj);			// cmpult 0, d -> d
    }
}

void
CodeGenAlpha::translate_sl(Instr *mi)
{
    TypeId stype = get_type(get_src(mi, 0));
    if (is_floating_point(stype)) {
	translate_fp_scc(mi, CMPTLT, FBEQ);
    } else {
	if (is_integral(stype) && is_signed(stype))
	    set_opcode(mi, CMPLT);
	else
	    set_opcode(mi, CMPULT);
	emit(mi);
    }
}

void
CodeGenAlpha::translate_sle(Instr *mi)
{
    TypeId stype = get_type(get_src(mi, 0));
    if (is_floating_point(stype)) {
	translate_fp_scc(mi, CMPTLE, FBEQ);
    } else {
	if (is_integral(stype) && is_signed(stype))
	    set_opcode(mi, CMPLE);
	else
	    set_opcode(mi, CMPULE);
	emit(mi);
    }
}


/* --------------- no translation --------------- */

void
CodeGenAlpha::translate_mod(Instr *mi)
{
    claim(false);
}

void
CodeGenAlpha::translate_rot(Instr *mi)
{
    claim(false);
}


/*
 * translate_any() -- Translate a generic instruction.
 *
 * Currently, the only expected ANY instructions are __builtin_va_start,
 * __builtin_isfloat, __builtin_va_args, all of which are produced by
 * expansions of the macros in varargs.h/stdarg.h
 */
void
CodeGenAlpha::translate_any(Instr *mi)
{
    ListNote<IrObject*> args_note = take_note(mi, k_builtin_args);
    claim(!is_null(args_note),
	  "Found an ANY instruction that's not a builtin");

    OneNote<IdString> opcode_name_note = take_note(mi, k_instr_opcode);
    claim(!is_null(opcode_name_note));
    const char *opcode_name = opcode_name_note.get_value().chars();

    if (strcmp(opcode_name, "__builtin_va_start") == 0) {
	claim(is_varargs);

	// Expecting three sources: the symbol for the variable that is to
	// hold the non-fixed arg pointer (nicknamed "ap"); an argument
	// from the varargs procedure argument list (va_alist for varargs;
	// the last fixed (named) argument for stdarg); and an immediate
	// integer flag: 0 for varargs.h style and 1 for stdarg.h style.
	// (For Alpha, we ignore the second of these source operands
	// entirely.)

	claim(srcs_size(mi) == 3);
	claim(dsts_size(mi) == 0);
	Opnd src0 = get_src(mi, 0);
	Opnd src2 = get_src(mi, 2);

	claim(is_var(src0));
	VarSym *ap_var = get_var(src0);
	Opnd ap_addr = opnd_addr_sym(ap_var);

	claim(is_immed_integer(src2));
	bool is_stdarg = get_immed_integer(src2) == 1;
	claim(is_stdarg || get_immed_integer(src2) == 0);

	// The EDG-derived C front end gives variable `ap' the type void*.
	// For Compaq UNIX, it should be a struct containing a pointer to
	// the base of the unnamed args plus a running int offset.  If the
	// type of ap_var (the `ap' symbol) is a pointer type, change it to
	// an aggregate type big enough to cover a pointer plus an integer.
	// (But round the size in bytes to a multiple of 8.)  Here we use
	// an array type: it's a bit easier to create than a struct.

	if (is_pointer(get_type(ap_var)))
	    set_type(ap_var, array_type(type_s8, 0, 15)); 	// 16 bytes long

	// Initialize the 1st field in the ap structure as a base address
	// for accessing the unnamed args.  Force the first parameter to
	// spill, and use its address as the base.  One wrinkle: if it's a
	// floating-point parameter, add 48 to point to the start of the
	// GPR arg dump, rather than the one for FPR, which is lower on the
	// stack.

	claim(get_formal_param_count(cur_unit) > 0,
	      "Expected varargs procedure to have a parameter");
	VarSym* p0_sym = get_formal_param(cur_unit, 0);
	set_addr_taken(p0_sym, true);

	Opnd base_addr = opnd_addr_sym(p0_sym);
	if (is_floating_point(get_type(p0_sym)))
	    base_addr = BaseDispOpnd(base_addr, opnd_immed(48, type_s32));

	Opnd ap_base = opnd_reg(type_addr());
	emit(new_instr_alm(ap_base, alpha::LDA, base_addr));
	emit(new_instr_alm(ap_addr, alpha::STQ, ap_base));

	// The 3rd parameter to __builtin_va_start is either zero for
	// varargs.h style or one for stdarg.h style.  In the former case,
	// init the offset field of the ap struct to zero, since the base
	// address points to the first unnamed arg.  In the latter case,
	// set it to the size in bytes of the fixed (named) args skipped to
	// reach the first unnamed arg, remembering to round the size of
	// each arg to an 8-byte multiple.

	int fixed_arg_space = 0;
	if (is_stdarg) {
	    int count = get_formal_param_count(cur_unit);
	    for (int i = 0; i < count; ++i) {
		VarSym *p = get_formal_param(cur_unit, i);
		fixed_arg_space += ((get_bit_size(get_type(p))>>3) + 7) & -8;
	    }
	}
	Opnd ap_offset = opnd_reg(type_s32);
	Opnd to_skip = opnd_immed(fixed_arg_space, type_s16);
	Opnd ap_offset_ea = SymDispOpnd(ap_addr, opnd_immed_8_u64);

	emit(new_instr_alm(ap_offset, LDIL, to_skip));
	emit(new_instr_alm(ap_offset_ea, STL, ap_offset));

	delete mi;
    }
    else if (strcmp(opcode_name, "__builtin_va_arg") == 0) {
	// Expecting one source: the variable (ap) that holds the struct
	// that identifies the current unnamed arg.  Expecting one
	// destination: the register in which to leave the address of the
	// va_arg(...) value.  Unlike most VRs, this opnd has a precise
	// pointer type.  It's referent gives the type of the va_arg value,
	// which is needed to locate the value and to advance ap.

	claim(dsts_size(mi));
	Opnd dst = get_dst(mi);
	TypeId value_type = get_referent_type(get_type(dst));

	// Advance ap.offset by the least multiple of 8 bytes that can
	// contain the current value.

	int advance = ((get_bit_size(value_type) + 56) & -64) / 8;

	Opnd ap_src = get_src(mi, 0);
	VarSym *ap_var = get_var(ap_src);
	Opnd ap_addr = opnd_addr_sym(ap_var);

	// Since va_arg() might be compiled before va_start(), we have to
	// make the same var-type adjustment here as in the handle of
	// __builtin_va_start, above.

	if (is_pointer(get_type(ap_var)))
	    set_type(ap_var, array_type(type_s8, 0, 15)); 	// 16 bytes long

	// The code to advance ap.offset and compute the address identified
	// by the (pre-advance) ap struct is complicated by the case of a
	// floating-point value.  The dump area for register-passed
	// floating args sits 48 bytes lower than that for GPR args.  But
	// the latter dump area is followed conveniently by the stack-
	// passed args.  In the floating-point case, when the offset hasn't
	// reached the size of the GPR dump area (48 bytes), we must shift
	// the resulting value address lower by 48, to put it into the FPR
	// dump area.  Here is the code emitted in this most general case:
	//
	//     ldl    old_offset, ap_var+8
	//     ldq    dst, ap_var+0
	//     ldil   delta, advance
	//     addl   old_offset, delta, new_offset
	//     stl    new_offset, ap_var+8
	//     subl   new_offset, 48, new_offset		) only
	//     mov    0, shift					) for a
	//     cmovle new_offset, 48, shift			) floating-point
	//     subl   old_offset, shift, old_offset		) value
	//     addq   dst, old_offset, dst

	Opnd old_offset = opnd_reg(type_s32);
	Opnd new_offset = opnd_reg(type_s32);
	Opnd delta = opnd_reg(type_s32);
	Opnd shift = opnd_reg(type_s32);
	Opnd immed8  = opnd_immed( 8, type_s16);
	Opnd immed48 = opnd_immed(48, type_u8);
	Opnd ap_var_p_8_1st = SymDispOpnd(ap_addr, immed8);	// can't reuse..
	Opnd ap_var_p_8_2nd = SymDispOpnd(ap_addr, immed8);	// ..an addr_exp

	emit(new_instr_alm(old_offset, LDL, ap_var_p_8_1st));
	emit(new_instr_alm(dst, LDQ, ap_addr));
	emit(new_instr_alm(delta, LDIL, opnd_immed(advance, type_s32)));
	emit(new_instr_alm(new_offset, ADDL, old_offset, delta));
	emit(new_instr_alm(ap_var_p_8_2nd, STL, new_offset));

	if (is_floating_point(value_type)) {
	    emit(new_instr_alm(new_offset, SUBL, new_offset, immed48));
	    emit(new_instr_alm(shift, alpha::MOV, opnd_immed_0_u64));
	    emit(new_instr_alm(shift, CMOVLE, new_offset, immed48));
	    emit(new_instr_alm(old_offset, SUBL, old_offset, shift));
	}
	emit(new_instr_alm(dst, ADDQ, dst, old_offset));

	delete mi;
    }
    else if (strcmp(opcode_name, "__builtin_isfloat") == 0) {

	// Expecting 1 argument (in the annotation only, not in the
	// instruction source list): the type of the varargs argument
	// currently being tested.

	claim(args_note.values_size() == 1 && srcs_size(mi) == 0);
	TypeId arg_type = to<Type>(args_note.get_value(0));

	// Write a '1' into the dst if type is a floating-point type
	// and a '0' otherwise.

	if (is_floating_point(arg_type)) {
	    set_opcode(mi, LDIQ);
 	    set_src(mi, 0, opnd_immed_1_u64);
	} else {
	    set_opcode(mi, alpha::MOV);
	    set_src(mi, 0, opnd_reg_const0);
	}
	emit(mi);
    }
    else if (strcmp(opcode_name, "__builtin_va_end") == 0) {
	// Do nothing at all for va_end(ap).
	delete mi;
    } else {
	claim(false, "translate_any() -- unexpected builtin name");
    }
}

/*
 * translate_mbr() -- Assume that the case constants form an increasing,
 * non-empty sequence of integers.  Assume that the source operand is in
 * range.  (The current SUIF front end only creates a multi-way branch
 * under these circumstances.)
 *
 * Create a dispatch table.
 *
 * Emit code to compute the dispatch-table index in VR `idx', then more
 * code to load the dispatch-table entry address into VR `ptr', adding a
 * k_mbr_target_def note to the last of these instructions for HALT's
 * sake.  Then emit a load-indirect through `ptr' to put the target address
 * in VR `tgt'.
 *
 * Finally, convert the original SUIFvm mbr into an indirect jump via
 * `tgt'.  Update the instr_mbr_tgts note of that instruction by
 * incorporating the dispatch-table symbol as its first element.
 */
void
CodeGenAlpha::translate_mbr(Instr *mi)
{
    MbrNote note = get_note(mi, k_instr_mbr_tgts);
    claim(!is_null(note));

    VarSym *dtsym = new_dispatch_table_var(note);

    Instr *mj;
    Opnd idx;
    int lower = note.get_case_constant(0);

    if (lower == 0)
	idx = get_src(mi, 0);			// let idx be src itself
    else {
	idx = opnd_reg(type_s64);
	Opnd shift = opnd_immed(-lower, type_s32);
	mj = new_instr_alm(idx, ADDL, get_src(mi, 0), shift);
	emit(mj);				// addl src,-lower,idx
    }
    Opnd tab = opnd_reg(type_addr());
    mj = new_instr_alm(tab, alpha::LDA, opnd_addr_sym(dtsym));
    set_note(mj, k_mbr_table_use, note_flag());
    maybe_use_reg_gp(mj);			// add regs_used note for $gp
    emit(mj);					// lda tab,dtsym

    Opnd ptr = opnd_reg(type_addr());
    mj = new_instr_alm(ptr, S8ADDQ, idx, tab);
    set_note(mj, k_mbr_target_def, note_flag());
    emit(mj);					// s8addq idx,tab,ptr

    Opnd tgt = opnd_reg(type_addr());
    Opnd ae  = BaseDispOpnd(ptr, opnd_immed_0_u64, type_ptr);
    emit(new_instr_alm(tgt, LDQ, ae));	// ldq tgt,0(ptr)

    set_opcode(mi, alpha::JMP);
    set_src(mi, 0, tgt);
    set_target(mi, NULL);			// jmp (tgt)
    emit(mi);
}


/* --------------- helpers for translate_null --------------- */

/*
 * move_param_reg() - Emit a move from register `r' to variable `p'.
 */
void
CodeGenAlpha::move_param_reg(VarSym *p, int r)
{
    TypeId ut = get_type(p);
    int move_op;

    if (is_floating_point(ut))
	move_op = FMOV;
    else {
	claim(is_scalar(ut));
	move_op = alpha::MOV;
    }
    Instr *mi = new_instr_alm(opnd_var(p), move_op, opnd_reg(r, ut));
    set_note(mi, k_param_init, note_flag());
    emit(mi);
}

/*
 * spill_param_reg() - Emit stores of some parameter registers.  Note that
 * parameters are always passed in 64-bit wide storage.  `v_addr' is the
 * address-symbol representing the destination stack location for the
 * parameter that needs spilling, at least in part.  `r_1st' and `r_max'
 * are the first and last registers that might need spilling.
 */
void
CodeGenAlpha::spill_param_reg(Opnd v_addr, int r_1st, int r_max)
{
    claim(is_addr_sym(v_addr));
    TypeId v_type = get_type(to<VarSym>(get_sym(v_addr)));
    Instr *mi;

    if (is_floating_point(v_type)) {
	Opnd r_1st_opnd = opnd_reg(r_1st, v_type);

	if (get_bit_size(v_type) == 32)
	    mi = new_instr_alm(v_addr, STS, r_1st_opnd);
	else
	    mi = new_instr_alm(v_addr, STT, r_1st_opnd);
	set_note(mi, k_param_init, note_flag());
	emit(mi);
    } else if (is_record(v_type)) {
	int v_size = get_bit_size(v_type) >> 3;

	int r_i = 0;		/* index into record */
	int r = r_1st;		/* current parameter register */

	while ((r <= r_max) && (r_i < v_size)) {
	    Opnd v_offset = opnd_immed(r_i, type_s32);
	    mi = new_instr_alm(SymDispOpnd(v_addr, v_offset),
			       STQ, opnd_reg(r, type_u64));
	    set_note(mi, k_param_init, note_flag());
	    emit(mi);
	    r += 1;		/* next param register */
	    r_i += 8;		/* next 64-bit quantity in record */
	}

    } else {
	claim(is_scalar(v_type), "bad type for register parameter");
	Opnd r_1st_opnd = opnd_reg(r_1st, v_type);
	mi = new_instr_alm(v_addr, STQ, r_1st_opnd);
	set_note(mi, k_param_init, note_flag());
	emit(mi);
    }
}

/*
 * add_auto_var -- Create and return a new address-taken local variable with
 * name `name' and type `t'.  Give it a stdarg_offset annotation that attaches
 * `os' as its frame offset (negative displacement from the high-address end
 * of the frame).
 */
static VarSym*
add_auto_var(OptUnit *unit, char *name, TypeId t, int os)
{
    VarSym *result = new_named_var(t, name);
    set_note(result, k_stdarg_offset, OneNote<long>(os));
    return result;
}

/*
 * transfer_params() -- Transfer register parameters to the appropriate
 * parameter symbols.  Spill addressed or non-scalar parameter symbols
 * passed in registers to the stack.  (Though the sp_offset calculation is
 * postponed to afin.)
 */
void
CodeGenAlpha::transfer_params(OptUnit *unit, bool is_varargs)
{
    int p_count = get_formal_param_count(unit);
    if (p_count == 0)
	return;

    debug(4, "..transfer register parameters to appropriate symbols (%s)",
	  get_name(get_proc_sym(unit)).chars());

    int num_arg_regs = LAST_ARG_GPR - ARG0_GPR + 1;
    int gp_max = LAST_ARG_GPR;
    int fp_max = LAST_ARG_FPR;
    int argi = 0;
    bool force_spill;
    VarSym *p;
    TypeId p_type;

    for (int i = 0; i < p_count; i++) {
	p = get_formal_param(unit, i);
	p_type = get_type(p);

	/* If this is a varargs proc (rather than a stdarg one),
	 * break when we see a parameter named va_alist.  This
	 * is the start of the varargs parameter spill area, and
	 * we don't want to increment the parameter reg pointer. */
	if (!is_varargs && (strcmp(get_name(p).chars(), "va_alist") == 0)) {
	    claim(i == p_count - 1);	// should be the last formal
	    break;
	}

	/* determine if spill needed or not -- spill any addressed
	 * parameters, etc. independent of regalloc decisions */
	force_spill = is_addr_taken(p) || !is_scalar(p_type);

	/* handle cases where parameter passed in register */
	if (argi < num_arg_regs) {
	    if (is_record(p_type)) {
		/* at least part of record in param regs */
		int r = ARG0_GPR + argi;
		set_param_reg(p, r);
		spill_param_reg(opnd_addr_sym(p), r, gp_max);

		/* update which param regs used by record */
		int r_i = 0;	      /* record index in bytes */
		int p_size = get_bit_size(p_type) >> 3;
		while ((argi < num_arg_regs) && (r_i < p_size)) {
		    argi++;
		    r_i += 8;	      /* increment in bytes */
		}

	    } else if (is_floating_point(p_type)) {
		/* FP parameter is passed in a param_reg */
		int r = ARG0_FPR + argi;
		set_param_reg(p, r);
		if (force_spill)			/* on stack */
		    spill_param_reg(opnd_addr_sym(p), r, fp_max);
		else
		    move_param_reg(p, r);
		argi++;

	    } else {
		claim(is_scalar(p_type), "unexpected parameter type");

		int r = ARG0_GPR + argi;
		set_param_reg(p, r);
		if (force_spill)			/* on stack */
		    spill_param_reg(opnd_addr_sym(p), r, gp_max);
		else
		    move_param_reg(p, r);
		argi++;
	    }
	} else {
	    argi = num_arg_regs;	/* parameter on stack */
	}
    }

    /* varargs overhead -- When we enter this code section, p will
     * contain either the parameter "va_alist" (if varargs) or
     * the last named parameter (if stdarg).  In either case,
     * argi points to the first varargs argument.  We need
     * to start spilling the rest of the argument registers (both
     * integer and FP) starting at this index.
     *
     * To handle the remapping of registers from a simulated larger
     * register file to a real register file, we must be able to
     * go back from a stack offset to some VarSym in the proc's
     * symbol table.  Hence, we name the unnamed vararg/stdarg
     * parameters.  It is critical that the finishing pass understand
     * the ordering of these vararg/stdarg parameters so that it
     * can generate the appropriate stack frame offset corresponding
     * to the architecture's calling convention. */
    if (is_varargs) {
	claim(p != NULL);	/* Note that there must be at least one
				 * named parameter in stdarg. */

	/* Spill only if some of va_alist in parameter registers. */
	if (argi < num_arg_regs) {
	    int i;
	    int os;					// GPR frame offset

	    for (i = argi, os = - 8 * (num_arg_regs - i);
		 i < num_arg_regs;
		 i++, os += 8) {

		int r;
		char cp_nm[15];
		VarSym *cp;	// compiler parameter

		/* spill int arg reg */
		sprintf(cp_nm, "_stdintarg%d", i);
		cp = add_auto_var(unit, cp_nm, type_u64, os);

		r = ARG0_GPR + i;
		spill_param_reg(opnd_addr_sym(cp), r, gp_max);

		/* spill associated fp arg reg */
		sprintf(cp_nm, "_stdfparg%d", i);
		cp = add_auto_var(unit, cp_nm, type_f64, os - 8 * num_arg_regs);

		r = ARG0_FPR + i;
		spill_param_reg(opnd_addr_sym(cp), r, fp_max);
	    }
	}
    }
}



/*
 * DetectGlobalSymbol
 *
 * Helper class for CodeGenAlpha::maybe_use_reg_gp (just below).
 */
class DetectGlobalSymbol : public OpndFilter
{
  public:
    DetectGlobalSymbol() : detected(false) { }
    Opnd operator()(Opnd opnd, InOrOut) {
	if (is_addr_sym(opnd) && is_global(get_sym(opnd)))
	    detected = true;
	return opnd;
    }
    bool detected;
};

/*
 * maybe_use_reg_gp(mi)
 *
 * If the input operands of mi include address symbols based on global
 * symbols, then presume that $gp will need to be used to obtain those
 * addresses.  Add a regs_used note to mi that mentions register GP.
 */
void
CodeGenAlpha::maybe_use_reg_gp(Instr *mi)
{
    DetectGlobalSymbol detector;

    map_src_opnds(mi, detector);

    if (detector.detected) {	// found an addressed global symbol
	NatSetDense regs_used;
	regs_used.insert(GP);
	set_note(mi, k_regs_used, NatSetNote(&regs_used));
    }
}
