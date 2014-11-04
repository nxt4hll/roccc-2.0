/* file "ia64/translate.cpp" */

/*
    Copyright (c) 2002 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#include <machine/machine.h>

#include <ia64/init.h>
#include <ia64/opcodes.h>
#include <ia64/reg_info.h>
#include <ia64/instr.h>
#include <ia64/code_gen.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

using namespace ia64;

/* Translate function pattern #1 -- Simplest translation.  Just update the
 * opcode.  In this pattern, there is only one target opcode possible. */
#ifdef TRANSLATE_FUN_1
#undef TRANSLATE_FUN_1
#endif
#define TRANSLATE_FUN_1(from_opcode, to_opcode) \
void \
CodeGenIa64::translate_##from_opcode(Instr *mi) { \
    set_opcode(mi, make_opcode(to_opcode, EMPTY,EMPTY,EMPTY)); \
    emit(mi); \
}

/* Translate function pattern #2 -- Here, we also just update the
 * opcode. But this time there is a special opcode for flpt ops */
 
#ifdef TRANSLATE_FUN_2
#undef TRANSLATE_FUN_2
#endif
#define TRANSLATE_FUN_2(from_opcode, to_opcode1, to_opcode2) \
void \
CodeGenIa64::translate_##from_opcode(Instr *mi) { \
    TypeId dtype = get_type(get_dst(mi)); \
    int opcode; \
    if (is_floating_point(dtype)) { \
            opcode = make_opcode(to_opcode2, EMPTY,EMPTY,EMPTY); \
    } else { \
            opcode = make_opcode(to_opcode1, EMPTY,EMPTY,EMPTY); \
    } \
    set_opcode(mi, opcode); \
    emit(mi); \
}

//not having the .u completer makes this an arithmetic shift right
TRANSLATE_FUN_1(asr, ia64::SHR)

TRANSLATE_FUN_2(add, ia64::ADD, ia64::FADD)
TRANSLATE_FUN_2(and, ia64::AND, ia64::FAND)
TRANSLATE_FUN_2(ior, ia64::OR,  ia64::FOR)
TRANSLATE_FUN_2(xor, ia64::XOR, ia64::FXOR)
TRANSLATE_FUN_2(sub, ia64::SUB, ia64::FSUB)


/*
 * translate_null() -- This routine handles opcode_null instructions.  If
 * this is a placeholder for the k_proc_entry note, then do entry-point
 * processing now: maybe save $gp for restoring after calls, and move or
 * spill register-passed parameters.
 */
void
CodeGenIa64::translate_null(Instr *mi)
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
CodeGenIa64::translate_nop(Instr *mi)
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
CodeGenIa64::translate_mrk(Instr *mi)
{
    if (LineNote note = get_note(mi, k_line)) {
	const char *file = note.get_file().chars();
	claim((file && file[0]), "line note with null file name");
	set_opcode(mi, make_opcode(LN, EMPTY,EMPTY,EMPTY));
    } else
	set_opcode(mi, opcode_null);
    emit(mi);
}

void
CodeGenIa64::translate_not(Instr *mi)
{
    set_opcode(mi, make_opcode(ANDCM, EMPTY,EMPTY,EMPTY));
    prepend_src(mi, opnd_immed_neg1_u64);
    emit(mi);
}

void
CodeGenIa64::translate_ldc(Instr *mi)
{
    Opnd dst = get_dst(mi);
    TypeId dtype = get_type(dst);
    if (is_floating_point(dtype)) {
	// Create floating-literal var, e.g., _fplit_123.  Then:
	//   addl tmp = @ltoff(_fplit_123), gp
	//   ld8  tmp = [tmp]
	//   ldfd dst = [tmp]

	Opnd literal = opnd_addr_sym(new_unique_var(get_src(mi, 0), "_fplit"));
	Opnd tmp = opnd_reg(type_p64);
	Opnd at_tmp = BaseDispOpnd(tmp, opnd_immed_0_u64);

	int addl = make_opcode(ADDL, EMPTY,EMPTY,EMPTY);
	Instr *mj = new_instr_alm(tmp, addl, literal, opnd_reg_gp);
	emit(mj);

	int ld8 = make_opcode(LD, SZ_8,LDTYPE_NONE,LDHINT_NONE);
	mj = new_instr_alm(tmp, ld8, at_tmp);
	emit(mj);
	
	set_opcode(mi, opcode_load_ia64(dtype));
	set_src(mi, 0, clone(at_tmp));
	emit(mi);
    }
    else {
        set_opcode(mi, make_opcode(MOVL, EMPTY,EMPTY,EMPTY));
        emit(mi);
    }
}

void
CodeGenIa64::translate_lda(Instr *mi)
{
    //Moves the address in the src to the register in the dst
    // NOT the dereferenced value

    // For local address-symbol L, emit:
    //    add dst = L, sp
    // For global address-symbol G, emit:
    //    addl tmp = G, gp
    //    ld8  dst = [tmp]

    int opcode;
    Opnd src = get_src(mi, 0);
    if (is_addr_sym(src)) {
	Sym *s = get_sym(src);

	if (is_kind_of<VarSym>(s) && is_auto(static_cast<VarSym*>(s))) {
	    opcode = make_opcode(ADD, EMPTY,EMPTY,EMPTY);
	    set_src(mi, 1, opnd_reg_sp);
	} else {
	    Opnd tmp = opnd_reg(type_p64);
	    int addl = make_opcode(ADDL, EMPTY,EMPTY,EMPTY);
	    Instr *mj = new_instr_alm(tmp, addl, src, opnd_reg_gp);
	    emit(mj);
	
	    opcode = make_opcode(LD, SZ_8,LDTYPE_NONE,LDHINT_NONE);
	    set_src(mi, 0, BaseDispOpnd(tmp, opnd_immed_0_u64));
	}
    } else if (is_immed(src)) {
	warn("Unexpected LDA case: imm src");
        opcode = make_opcode(MOVL, EMPTY,EMPTY,EMPTY);
    } else {
	warn("Unexpected LDA case: other src");
        opcode = make_opcode(MOV_GR, EMPTY,EMPTY,EMPTY);
    }
    set_opcode(mi, opcode);
    emit(mi);
}

void
CodeGenIa64::translate_mov(Instr *mi)
{
    set_opcode(mi, make_opcode(MOV_GR, EMPTY,EMPTY,EMPTY));
    emit(mi);
}


void
CodeGenIa64::translate_abs(Instr *mi) 
{
    TypeId dtype = get_type(get_dst(mi));

    if (is_floating_point(dtype)) {
	set_opcode(mi, make_opcode(FABS, EMPTY,EMPTY,EMPTY));
        emit(mi);
    }
    else {
        // Cmp the src to zero, set the predicate cmp.gt pT,pF=c0,src
        Opnd pT = opnd_reg(type_pr);
        Opnd pF = opnd_reg(type_pr);
        int opc = make_opcode(ia64::CMP, CTYPE_NONE, CREL_GT, EMPTY);
        Opnd c0 = opnd_reg_const0;
        Opnd src = get_src(mi, 0);
        Opnd dst = get_dst(mi);
        Instr *cmpInst = new_instr_alm(pF, opc, c0, src);
        prepend_dst(cmpInst, pT);
        emit(cmpInst);

        // Then create a predicated subtract from zero (pT) sub dst=r0,src
        opc = make_opcode(ia64::SUB, EMPTY,EMPTY,EMPTY);
        Instr *subInst = new_instr_alm(dst, opc, c0, src);
        set_qpred(subInst, pT); // predicate the instruction
        emit(subInst);
        delete mi;        
    }
}

/*
 * Integer divide or remainder instructions are implemented as function
 * calls.
 */
void
CodeGenIa64::emit_ots_call(IdString ots_proc, Instr *mi)
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
CodeGenIa64::translate_div(Instr *mi) 
{
    TypeId dtype = get_type(get_dst(mi));

    if (is_floating_point(dtype)) {
        IdString ots_proc = k_divsf3;
        emit_ots_call(ots_proc, mi);
    } else {
        IdString ots_proc = k_divdi3;
        emit_ots_call(ots_proc, mi);
    }
}

void
CodeGenIa64::translate_rem(Instr *mi) 
{
    IdString ots_proc = k_moddi3;
    emit_ots_call(ots_proc, mi);
}

void
CodeGenIa64::translate_lsl(Instr *mi)
{
    TypeId dtype = get_type(get_dst(mi));
    claim(!is_floating_point(dtype));

    set_opcode(mi, make_opcode(ia64::SHL, EMPTY,EMPTY,EMPTY));
    emit(mi);
}

void
CodeGenIa64::translate_lsr(Instr *mi)
{
    TypeId dtype = get_type(get_dst(mi));
    claim(!is_floating_point(dtype));

    // The .u completer makes this a logical shift right
    set_opcode(mi, make_opcode(ia64::SHR, _SU_FORM_U,EMPTY,EMPTY));
    emit(mi);
}
void
CodeGenIa64::translate_jmp(Instr *mi)
{
    /* Assumes it's not a long branch */
    set_opcode(mi,make_br_opcode(ia64::BR,BTYPE_NONE,BWH_SPTK,PH_NONE,DH_NONE));
    emit(mi);
}

void
CodeGenIa64::translate_jmpi(Instr *mi)
{
    Opnd src = get_src(mi,0);
    claim((is_reg(src) || is_var(src)), "JmpI: i is not reg/var");
    int opc = make_opcode(ia64::MOV_BR, EMPTY,EMPTY,EMPTY);
    Opnd tgtReg = opnd_reg(type_br);
    Instr *movInstr = new_instr_alm(tgtReg, opc, src);
    emit(movInstr);

    opc = make_br_opcode(ia64::BR,BTYPE_NONE,BWH_SPTK,PH_NONE,DH_NONE);
    Instr *brInstr = new_instr_cti(opc, NULL, tgtReg);
    emit(brInstr);
    delete mi;
}

void
CodeGenIa64::translate_mul(Instr *mi)
{
    TypeId dtype = get_type(get_dst(mi));
    if (is_floating_point(dtype)) {
      set_opcode(mi, make_opcode(ia64::FMPY, EMPTY,EMPTY,EMPTY));
      emit(mi);
    }
    else {
      //Wow, ia64 turns this into four instructions:
      // setf.sig freg1 = src1
      // setf.sig freg2 = src2
      // xma.l freg3 = freg1, freg2, f0
      // getf.sig dst = freg3

      Opnd freg1 = opnd_reg(type_f80);
      Opnd freg2 = opnd_reg(type_f80);
      Opnd freg3 = opnd_reg(type_f80);
      Opnd src1 = get_src(mi, 0);
      Opnd src2 = get_src(mi, 1);
      Opnd dst = get_dst(mi);

      int opc = make_opcode(SETF, _SD_FORM_SIG,EMPTY,EMPTY);
      emit( new_instr_alm(freg1, opc, src1));

      opc = make_opcode(SETF, _SD_FORM_SIG,EMPTY,EMPTY);
      emit( new_instr_alm(freg2, opc, src2));

      opc = make_opcode(XMA, _LHU_FORM_L,EMPTY,EMPTY);
      Instr *mj = new_instr_alm(freg3, opc, freg1, freg2);
      append_src(mj, opnd_fr_const0);
      emit(mj);

      opc = make_opcode(GETF, _SD_FORM_SIG,EMPTY,EMPTY);
      emit( new_instr_alm(dst, opc, freg3));
      delete mi;
    }
}

void
CodeGenIa64::translate_neg(Instr *mi)
{
    TypeId dtype = get_type(get_dst(mi));
    if (is_floating_point(dtype)) {
      set_opcode(mi, make_opcode(ia64::FNEG, EMPTY,EMPTY,EMPTY));
      emit(mi);
    }
    else {
      // (p0) sub dst = r0, src
      set_opcode(mi, make_opcode(ia64::SUB, EMPTY,EMPTY,EMPTY));
      prepend_src(mi, opnd_reg_const0); // src1 = 0
      emit(mi);
    }
}

void
CodeGenIa64::translate_min(Instr *mi)
{
    TypeId dtype = get_type(get_dst(mi));
    if (is_floating_point(dtype)) {
	set_opcode(mi, make_opcode(ia64::FMIN, SF_NONE,EMPTY,EMPTY));
	emit(mi);
    }
    else {
	// (p0) cmp.lt p4,p5 = src1, src2
        // (p4) mov dst = src1
        // (p5) mov dst = src2

        Opnd pT = opnd_reg(type_pr);
        Opnd pF = opnd_reg(type_pr);
        Opnd dst = get_dst(mi);
        Opnd src1 = get_src(mi, 0);
        Opnd src2 = get_src(mi, 1);

        int opc = make_opcode(ia64::CMP, CTYPE_NONE,CREL_LT,EMPTY);
        Instr *cmpInstr = new_instr_alm(pF, opc, src1, src2);
        prepend_dst(cmpInstr, pT);
        emit(cmpInstr);

        opc = make_opcode(ia64::MOV_GR, EMPTY,EMPTY,EMPTY);
        emit(new_instr_alm(dst, opc, pT, src1));
        emit(new_instr_alm(dst, opc, pF, src2));
        delete mi;
    }
}

void
CodeGenIa64::translate_max(Instr *mi)
{
    TypeId dtype = get_type(get_dst(mi));
    if (is_floating_point(dtype)) {
	set_opcode(mi, make_opcode(ia64::FMAX, SF_NONE,EMPTY,EMPTY));
	emit(mi);
    }
    else {
	// (p0) cmp.gt p4,p5 = src1, src2
        // (p4) mov dst = src1
        // (p5) mov dst = src2

        Opnd pT = opnd_reg(type_pr);
        Opnd pF = opnd_reg(type_pr);
        Opnd dst = get_dst(mi);
        Opnd src1 = get_src(mi, 0);
        Opnd src2 = get_src(mi, 1);

        int opc = make_opcode(ia64::CMP, CTYPE_NONE,CREL_GT,EMPTY);
        Instr *cmpInstr = new_instr_alm(pF, opc, src1, src2);
        prepend_dst(cmpInstr, pT);
        emit(cmpInstr);

        opc = make_opcode(ia64::MOV_GR, EMPTY,EMPTY,EMPTY);
        emit(new_instr_alm(dst, opc, pT, src1));
        emit(new_instr_alm(dst, opc, pF, src2));
        delete mi;
    }
}

/*
 * Branch if true. If the source operand contains a true (non-zero) value, 
 * control is transferred to the code at the target label. Otherwise, it 
 * continues with the next instruction in sequential order. The source 
 * operand must have an integer type. 
 */
void
CodeGenIa64::translate_btrue(Instr *mi)
{
    // (p0) cmp.eq pT, pF = r0, src
    // (pF) br.dptk target

    Opnd pT = opnd_reg(type_pr);
    Opnd pF = opnd_reg(type_pr);
    Opnd src = get_src(mi, 0);
    int opc = (is_floating_point(get_type(src))) ? 
       (make_opcode(ia64::FCMP,FCTYPE_NONE,FREL_EQ,EMPTY)) :
       (make_opcode(ia64::CMP,CTYPE_NONE,CREL_EQ,EMPTY));
    Instr * cmpInstr = new_instr_alm(pF, opc, opnd_reg_const0, src);
    prepend_dst(cmpInstr, pT);
    emit(cmpInstr);

    Sym *target = get_target(mi);
    opc = make_br_opcode(ia64::BR,BTYPE_NONE,BWH_DPTK,PH_NONE,DH_NONE);
    Instr * brInstr = new_instr_cti(opc, target);
    set_qpred(brInstr, pF);
    emit(brInstr);

    delete mi;
}

/*
 * Branch if false. If the source operand contains a false (zero) value, 
 * control is transferred to the code at the target label. Otherwise, it 
 * continues with the next instruction in sequential order. The source 
 * operand must have an integer type.
 */
void
CodeGenIa64::translate_bfalse(Instr *mi)
{
    // (p0) cmp.eq pT, pF = r0, src
    // (pT) br.dptk target

    Opnd pT = opnd_reg(type_pr);
    Opnd pF = opnd_reg(type_pr);
    Opnd src = get_src(mi, 0);
    int opc = (is_floating_point(get_type(src))) ? 
       (make_opcode(ia64::FCMP,FCTYPE_NONE,FREL_EQ,EMPTY)) :
       (make_opcode(ia64::CMP,CTYPE_NONE,CREL_EQ,EMPTY));
    Instr * cmpInstr = new_instr_alm(pF, opc, opnd_reg_const0, src);
    prepend_dst(cmpInstr, pT);
    emit(cmpInstr);

    Sym *target = get_target(mi);
    opc = make_br_opcode(ia64::BR,BTYPE_NONE,BWH_DPTK,PH_NONE,DH_NONE);
    Instr * brInstr = new_instr_cti(opc, target);
    set_qpred(brInstr, pT);
    emit(brInstr);

    delete mi;
}


/*
 * BEQ,BNE,BGE,BGT,BLE,BLT. Branch if conditional compare true. The two 
 * source operands are compared according to the indicated condition. 
 * If the result is true, control is transferred to the code at the 
 * target label. Otherwise, it continues with the next instruction in 
 * sequential order. The source operands must either have the same 
 * integer or floating-point type, or else two (possibly different) 
 * pointer types. 
 */
void
CodeGenIa64::translate_beq(Instr *mi)
{
    // (p0) cmp.eq pT, pF = src1, src2
    // (pT) br.dptk target

    Opnd pT = opnd_reg(type_pr);
    Opnd pF = opnd_reg(type_pr);
    Opnd src1 = get_src(mi, 0);
    Opnd src2 = get_src(mi, 1);
    int opc = (is_floating_point(get_type(src2))) ?
       (make_opcode(ia64::FCMP,FCTYPE_NONE,FREL_EQ,EMPTY)) :
       (make_opcode(ia64::CMP,CTYPE_NONE,CREL_EQ,EMPTY));
    Instr * cmpInstr = new_instr_alm(pF, opc, src2, src1);
    prepend_dst(cmpInstr, pT);
    emit(cmpInstr);

    Sym *target = get_target(mi);
    opc = make_br_opcode(ia64::BR,BTYPE_NONE,BWH_DPTK,PH_NONE,DH_NONE);
    Instr * brInstr = new_instr_cti(opc, target);
    set_qpred(brInstr, pT);
    emit(brInstr);

    delete mi;
}

void
CodeGenIa64::translate_bne(Instr *mi)
{
    // (p0) cmp.neq pT, pF = src1, src2
    // (pT) br.dptk target

    Opnd pT = opnd_reg(type_pr);
    Opnd pF = opnd_reg(type_pr);
    Opnd src1 = get_src(mi, 0);
    Opnd src2 = get_src(mi, 1);
    int opc = (is_floating_point(get_type(src2))) ?
       (make_opcode(ia64::FCMP,FCTYPE_NONE,FREL_NEQ,EMPTY)) :
       (make_opcode(ia64::CMP,CTYPE_NONE,CREL_NEQ,EMPTY));
    Instr * cmpInstr = new_instr_alm(pF, opc, src1, src2);
    prepend_dst(cmpInstr, pT);
    emit(cmpInstr);

    Sym *target = get_target(mi);
    opc = make_br_opcode(ia64::BR,BTYPE_NONE,BWH_DPTK,PH_NONE,DH_NONE);
    Instr * brInstr = new_instr_cti(opc, target);
    set_qpred(brInstr, pT);
    emit(brInstr);

    delete mi;
}

void
CodeGenIa64::translate_bge(Instr *mi)
{
    // (p0) cmp.ge pT, pF = src1, src2
    // (pT) br.dptk target

    Opnd pT = opnd_reg(type_pr);
    Opnd pF = opnd_reg(type_pr);
    Opnd src1 = get_src(mi, 0);
    Opnd src2 = get_src(mi, 1);
    int opc = (is_floating_point(get_type(src2))) ?
       (make_opcode(ia64::FCMP,FCTYPE_NONE,FREL_GE,EMPTY)) :
       (make_opcode(ia64::CMP,CTYPE_NONE,CREL_GE,EMPTY));
    Instr * cmpInstr = new_instr_alm(pF, opc, src1, src2);
    prepend_dst(cmpInstr, pT);
    emit(cmpInstr);

    Sym *target = get_target(mi);
    opc = make_br_opcode(ia64::BR,BTYPE_NONE,BWH_DPTK,PH_NONE,DH_NONE);
    Instr * brInstr = new_instr_cti(opc, target);
    set_qpred(brInstr, pT);
    emit(brInstr);

    delete mi;
}

void
CodeGenIa64::translate_bgt(Instr *mi)
{
    // (p0) cmp.gt pT, pF = src1, src2
    // (pT) br.dptk target

    Opnd pT = opnd_reg(type_pr);
    Opnd pF = opnd_reg(type_pr);
    Opnd src1 = get_src(mi, 0);
    Opnd src2 = get_src(mi, 1);
    int opc = (is_floating_point(get_type(src2))) ?
       (make_opcode(ia64::FCMP,FCTYPE_NONE,FREL_GT,EMPTY)) :
       (make_opcode(ia64::CMP,CTYPE_NONE,CREL_GT,EMPTY));
    Instr * cmpInstr = new_instr_alm(pF, opc, src1, src2);
    prepend_dst(cmpInstr, pT);
    emit(cmpInstr);

    Sym *target = get_target(mi);
    opc = make_br_opcode(ia64::BR,BTYPE_NONE,BWH_DPTK,PH_NONE,DH_NONE);
    Instr * brInstr = new_instr_cti(opc, target);
    set_qpred(brInstr, pT);
    emit(brInstr);

    delete mi;
}

void
CodeGenIa64::translate_ble(Instr *mi)
{
    // (p0) cmp.le pT, pF = src1, src2
    // (pT) br.dptk target

    Opnd pT = opnd_reg(type_pr);
    Opnd pF = opnd_reg(type_pr);
    Opnd src1 = get_src(mi, 0);
    Opnd src2 = get_src(mi, 1);
    int opc = (is_floating_point(get_type(src2))) ?
       (make_opcode(ia64::FCMP,FCTYPE_NONE,FREL_LE,EMPTY)) :
       (make_opcode(ia64::CMP,CTYPE_NONE,CREL_LE,EMPTY));
    Instr * cmpInstr = new_instr_alm(pF, opc, src1, src2);
    prepend_dst(cmpInstr, pT);
    emit(cmpInstr);

    Sym *target = get_target(mi);
    opc = make_br_opcode(ia64::BR,BTYPE_NONE,BWH_DPTK,PH_NONE,DH_NONE);
    Instr * brInstr = new_instr_cti(opc, target);
    set_qpred(brInstr, pT);
    emit(brInstr);

    delete mi;
}

void
CodeGenIa64::translate_blt(Instr *mi)
{
    // (p0) cmp.lt pT, pF = src1, src2
    // (pT) br.dptk target

    Opnd pT = opnd_reg(type_pr);
    Opnd pF = opnd_reg(type_pr);
    Opnd src1 = get_src(mi, 0);
    Opnd src2 = get_src(mi, 1);
    int opc = (is_floating_point(get_type(src2))) ?
       (make_opcode(ia64::FCMP,FCTYPE_NONE,FREL_LT,EMPTY)) :
       (make_opcode(ia64::CMP,CTYPE_NONE,CREL_LT,EMPTY));
    Instr * cmpInstr = new_instr_alm(pF, opc, src1, src2);
    prepend_dst(cmpInstr, pT);
    emit(cmpInstr);

    Sym *target = get_target(mi);
    opc = make_br_opcode(ia64::BR,BTYPE_NONE,BWH_DPTK,PH_NONE,DH_NONE);
    Instr * brInstr = new_instr_cti(opc, target);
    set_qpred(brInstr, pT);
    emit(brInstr);

    delete mi;
}

/*
 * SEQ, SNE, SL, SLE. Comparison instructions. If the first source operand 
 * is equal, not equal, less than, or less than or equal, respectively, to 
 * the second operand, assign the integer value one to the destination 
 * operand. Otherwise, set the destination operand to zero. The source 
 * operands must be either the same integer or floating point type, or 
 * two (possibly different) pointer types. 
 */

void
CodeGenIa64::translate_seq(Instr *mi)
{
    // Since the destination of cmp instrs are predicate registers,
    // I turn this into three instructions 
    // (p0) cmp.eq pT, pF = src1, src2
    // (pT) mov dst = 1 
    // (pF) mov dst = 0 

    Opnd src1 = get_src(mi, 0);
    Opnd src2 = get_src(mi, 1);
    Opnd dst = get_dst(mi, 0);
    Opnd pT = opnd_reg(type_pr);
    Opnd pF = opnd_reg(type_pr);

    int opc = (is_floating_point(get_type(src2))) ?
       (make_opcode(ia64::FCMP,FCTYPE_NONE,FREL_EQ,EMPTY)) :
       (make_opcode(ia64::CMP,CTYPE_NONE,CREL_EQ,EMPTY));
    Instr * cmpInstr = new_instr_alm(pF, opc, src2, src1);
    prepend_dst(cmpInstr, pT); // Add first dst
    emit(cmpInstr);

    opc = make_opcode(ia64::MOV_IMM, EMPTY,EMPTY,EMPTY);
    Instr * movInstr = new_instr_alm(dst, opc, opnd_immed_1_u64);
    set_qpred(movInstr, pT); // move if pT
    emit(movInstr);

    movInstr = new_instr_alm(dst, opc, opnd_immed_0_u64);
    set_qpred(movInstr, pF); // move if pF
    emit(movInstr);
    delete mi;
}

void
CodeGenIa64::translate_sne(Instr *mi)
{
    // Since the destination of cmp instrs are predicate registers,
    // I turn this into three instructions 
    // (p0) cmp.neq pT, pF = src1, src2
    // (pT) mov dst = 1 
    // (pF) mov dst = 0 

    Opnd src1 = get_src(mi, 0);
    Opnd src2 = get_src(mi, 1);
    Opnd dst = get_dst(mi, 0);
    Opnd pT = opnd_reg(type_pr);
    Opnd pF = opnd_reg(type_pr);

    int opc = (is_floating_point(get_type(src2))) ?
       (make_opcode(ia64::FCMP,FCTYPE_NONE,FREL_NEQ,EMPTY)) :
       (make_opcode(ia64::CMP,CTYPE_NONE,CREL_NEQ,EMPTY));
    Instr * cmpInstr = new_instr_alm(pF, opc, src1, src2);
    prepend_dst(cmpInstr, pT); // Add first dst
    emit(cmpInstr);

    opc = make_opcode(ia64::MOV_IMM, EMPTY,EMPTY,EMPTY);
    Instr * movInstr = new_instr_alm(dst, opc, opnd_immed_1_u64);
    set_qpred(movInstr, pT); // move if pT
    emit(movInstr);

    movInstr = new_instr_alm(dst, opc, opnd_immed_0_u64);
    set_qpred(movInstr, pF); // move if pF
    emit(movInstr);
    delete mi;
}

void
CodeGenIa64::translate_sl(Instr *mi)
{
    // Since the destination of cmp instrs are predicate registers,
    // I turn this into three instructions 
    // (p0) cmp.lt pT, pF = src1, src2
    // (pT) mov dst = 1 
    // (pF) mov dst = 0 

    Opnd src1 = get_src(mi, 0);
    Opnd src2 = get_src(mi, 1);
    Opnd dst = get_dst(mi, 0);
    Opnd pT = opnd_reg(type_pr);
    Opnd pF = opnd_reg(type_pr);

    int opc = (is_floating_point(get_type(src2))) ?
       (make_opcode(ia64::FCMP,FCTYPE_NONE,FREL_LT,EMPTY)) :
       (make_opcode(ia64::CMP,CTYPE_NONE,CREL_LT,EMPTY));
    Instr * cmpInstr = new_instr_alm(pF, opc, src1, src2);
    prepend_dst(cmpInstr, pT); // Add first dst
    emit(cmpInstr);

    opc = make_opcode(ia64::MOV_IMM, EMPTY,EMPTY,EMPTY);
    Instr * movInstr = new_instr_alm(dst, opc, opnd_immed_1_u64);
    set_qpred(movInstr, pT); // move if pT
    emit(movInstr);

    movInstr = new_instr_alm(dst, opc, opnd_immed_0_u64);
    set_qpred(movInstr, pF); // move if pF
    emit(movInstr);
    delete mi;
}

void
CodeGenIa64::translate_sle(Instr *mi)
{
    // Since the destination of cmp instrs are predicate registers,
    // I turn this into three instructions 
    // (p0) cmp.le pT, pF = src1, src2
    // (pT) mov dst = 1 
    // (pF) mov dst = 0 

    Opnd src1 = get_src(mi, 0);
    Opnd src2 = get_src(mi, 1);
    Opnd dst = get_dst(mi, 0);
    Opnd pT = opnd_reg(type_pr);
    Opnd pF = opnd_reg(type_pr);

    int opc = (is_floating_point(get_type(src2))) ?
       (make_opcode(ia64::FCMP,FCTYPE_NONE,FREL_LE,EMPTY)) :
       (make_opcode(ia64::CMP,CTYPE_NONE,CREL_LE,EMPTY));
    Instr * cmpInstr = new_instr_alm(pF, opc, src1, src2);
    prepend_dst(cmpInstr, pT); // Add first dst
    emit(cmpInstr);

    opc = make_opcode(ia64::MOV_IMM, EMPTY,EMPTY,EMPTY);
    Instr * movInstr = new_instr_alm(dst, opc, opnd_immed_1_u64);
    set_qpred(movInstr, pT); // move if pT
    emit(movInstr);

    movInstr = new_instr_alm(dst, opc, opnd_immed_0_u64);
    set_qpred(movInstr, pF); // move if pF
    emit(movInstr);

    delete mi;
}


/* --------------- no translation --------------- */

void
CodeGenIa64::translate_mod(Instr *mi)
{
    claim(false, "translate_mod() -- MOD not supported in IA-64");
}

void
CodeGenIa64::translate_rot(Instr *mi)
{
    claim(false, "translate_rot() -- ROT not supported in IA-64");
}

/*
 * translate_any() -- Translate a generic instruction.
 *
 * Currently, the only expected ANY instructions are __builtin_va_start,
 * __builtin_va_arg, and __builtin_va_end, all of which are produced by
 * expansions of the macros in varargs.h/stdarg.h
 */
void
CodeGenIa64::translate_any(Instr *mi)
{
    ListNote<IrObject*> args_note = take_note(mi, k_builtin_args);
    claim(!is_null(args_note),
	  "Found an ANY instruction that's not a builtin");

    OneNote<IdString> opcode_name_note = take_note(mi, k_instr_opcode);
    claim(!is_null(opcode_name_note));
    const char *opcode_name = opcode_name_note.get_value().chars();

    if (strcmp(opcode_name, "__builtin_va_start") == 0) {
	claim(is_varargs);

	// Expecting three sources, but only the first is relevant for
	// IA64: the symbol for the variable that is to hold the non-fixed
	// arg pointer (nicknamed "ap").

	claim(srcs_size(mi) == 3);
	claim(dsts_size(mi) == 0);
	Opnd ap_src = get_src(mi, 0);
	claim(is_var(ap_src));

	// Under Linux/IA64, variable `ap' has type void* and it points to
	// the next unnamed arg value in memory.  Any register-passed
	// unnamed args will have been spilled to the stack frame area just
	// below the memory-passed args (if any).  To facilitate initiali-
	// zation of `ap' we generate a unique variable that is bound by
	// the fin pass to the first unnamed-arg memory slot.  This
	// variable's symbol gets a "va_first_unnamed" note so that fin can
	// recognize it.  To init `ap', we just set it to the address of
	// this special variable.

	VarSym *va_first = new_unique_var(type_v0, "__va_first");
	set_note(va_first, k_va_first_unnamed, note_flag());
	int add = make_opcode(ADD, EMPTY,EMPTY,EMPTY);
	emit(new_instr_alm(ap_src, add, opnd_addr_sym(va_first), opnd_reg_sp));

	delete mi;
    }
    else if (strcmp(opcode_name, "__builtin_va_arg") == 0) {
	// Expecting one source: the variable (ap) that holds the pointer
	// identifying the current unnamed arg.  Expecting one destination:
	// the register in which to leave the address of the va_arg(...)
	// value.  Unlike most VRs, this opnd has a precise pointer type.
	// Its referent gives the type of the va_arg value, which is
	// needed to locate the value and to advance ap.

	claim(srcs_size(mi) == 1);
	Opnd ap = get_src(mi, 0);

	claim(dsts_size(mi) == 1);
	Opnd dst = get_dst(mi);

	// First, copy ap to the destination.
	int move = make_opcode(MOV_GR, EMPTY,EMPTY,EMPTY);
	emit(new_instr_alm(dst, move, ap));

	// Then advance ap by the least multiple of 8 bytes that can
	// contain the current value.
	TypeId value_type = get_referent_type(get_type(dst));
	int value_size = ((get_bit_size(value_type) + 63) / 64) * 8;

	int add = make_opcode(ADD, EMPTY,EMPTY,EMPTY);
	emit(new_instr_alm(ap, add, opnd_immed(value_size, type_s32), ap));

	delete mi;
    }
    else if (strcmp(opcode_name, "__builtin_va_end") == 0) {
	// Do nothing at all for va_end(ap).
	delete mi;
    } else {
	claim(false, "translate_any() -- unexpected builtin name");
    }
}

/*
 * Multi-way branch instruction. Transfers control to one of several 
 * target labels depending on the value of the source operand, an integer. 
 * The value of the source operand is compared with a set of constants to 
 * choose the target label. If it matches one of these case constants, the 
 * instruction branches to the corresponding label. Otherwise, it branches 
 * to the default target label, which is the target field of the instruction. 
 *
 * Assume that the case constants form an increasing, non-empty sequence of
 * integers.  Assume that the source operand is in range.  (The current
 * SUIF front end only creates a multi-way branch under these
 * circumstances.)
 *
 * Create a dispatch table.
 *
 * Emit code to compute the dispatch-table index in VR `idx', then more
 * code to put the dispatch-table entry address into VR `ptr', adding a
 * k_mbr_target_def note to the last of these instructions to help HALT
 * find the index value at run time.  Then emit a load-indirect through
 * to pick up the actual target address, and move this into the branch
 * register `tgt'.
 *
 * Finally, convert the original SUIFvm mbr into an indirect jump via
 * `tgt'.  Update the instr_mbr_tgts note of that instruction by
 * incorporating the dispatch-table symbol as its first element.
 */
void
CodeGenIa64::translate_mbr(Instr *mi)
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
        int opc = make_opcode(ADD, EMPTY,EMPTY,EMPTY);
	mj = new_instr_alm(idx, opc, shift, get_src(mi, 0));
	emit(mj);				// add idx = -lower, src
    }
    Opnd tab = opnd_reg(type_addr());
    int opc = make_opcode(ADDL, EMPTY,EMPTY,EMPTY);
    mj = new_instr_alm(tab, opc, opnd_addr_sym(dtsym), opnd_reg_gp);
    set_note(mj, k_mbr_table_use, note_flag());
    emit(mj);					// addl tab = dtsym, gp

    opc = make_opcode(LD, SZ_8,LDTYPE_NONE,LDHINT_NONE);
    mj = new_instr_alm(tab, opc, BaseDispOpnd(tab, opnd_immed_0_u64));
    emit(mj);					// ld8 tab = [tab]

    Opnd ptr = opnd_reg(type_p64);
    Opnd three = opnd_immed(3, type_s64);
    opc = make_opcode(SHLADD, EMPTY,EMPTY,EMPTY);
    mj = new_instr_alm(ptr, opc, idx, three);
    set_src(mj, 2, tab);
    set_note(mj, k_mbr_target_def, note_flag());
    emit(mj);					// shladd ptr = idx,3,tab

    opc = make_opcode(LD, SZ_8,LDTYPE_NONE,LDHINT_NONE);
    mj = new_instr_alm(ptr, opc, BaseDispOpnd(ptr, opnd_immed_0_u64));
    emit(mj);					// ld8 ptr = [ptr]

    Opnd tgt = opnd_reg(type_br);
    opc = make_opcode(MOV_BR, EMPTY,EMPTY,EMPTY);
    emit(new_instr_alm(tgt, opc, ptr));		// mov tgt = ptr

    set_opcode(mi, make_br_opcode(BR, EMPTY,EMPTY,EMPTY,EMPTY));
    set_src(mi, 0, tgt);
    set_target(mi, NULL);			// br tgt
    emit(mi);
}


/* --------------- helpers for translate_null --------------- */

/*
 * move_param_reg() - Emit a move from register `r' to variable `p'.
 */
void
CodeGenIa64::move_param_reg(VarSym *p, int r)
{
    TypeId ut = get_type(p);
    int move_op;

    if (is_floating_point(ut))
	move_op = make_opcode(ia64::MOV_FR, EMPTY,EMPTY,EMPTY);
    else {
	claim(is_scalar(ut));
	move_op = make_opcode(ia64::MOV_GR, EMPTY,EMPTY,EMPTY);
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
CodeGenIa64::spill_param_reg(Opnd v_addr, int r_1st, int r_max)
{
    claim(is_addr_sym(v_addr));
    TypeId v_type = get_type(to<VarSym>(get_sym(v_addr)));
    Instr *mi; int opc;

    opc = make_opcode(ADD, EMPTY,EMPTY,EMPTY);
    Opnd addrReg = opnd_reg(type_p64);
    emit( new_instr_alm(addrReg, opc, v_addr, opnd_reg_sp));
    Opnd dstAddr = BaseDispOpnd(addrReg, opnd_immed_0_u64);

    if (is_floating_point(v_type)) {
	Opnd r_1st_opnd = opnd_reg(r_1st, v_type);
	int v_bits = get_bit_size(v_type);
	int fsz = (v_bits == 64) ? FSZ_D : (v_bits == 32) ? FSZ_S : FSZ_E;

	opc = make_opcode(STF, fsz, STTYPE_NONE,STHINT_NONE);
	mi = new_instr_alm(dstAddr, opc, r_1st_opnd);
	set_note(mi, k_param_init, note_flag());
	emit(mi);
    } else if (is_record(v_type)) {
	int v_bytes = get_bit_size(v_type) >> 3;

	int r_i = 0;		// index into record 
	int r = r_1st;		// current parameter register
        opc = make_opcode(ST, SZ_8, STTYPE_NONE,STHINT_NONE);
	while ((r <= r_max) && (r_i < v_bytes)) {
	    mi = new_instr_alm(dstAddr, opc, opnd_reg(r, type_u64), opnd_immed_8_u64);
	    set_note(mi, k_param_init, note_flag());
	    emit(mi);
	    r += 1;		// next param register
	    r_i += 8;		// next 64-bit quantity in record
	}

    } else {
	claim(is_scalar(v_type), "bad type for register parameter");
	Opnd r_1st_opnd = opnd_reg(r_1st, v_type);
	mi = new_instr_alm(dstAddr, opcode_store_ia64(v_type), r_1st_opnd);
	set_note(mi, k_param_init, note_flag());
	emit(mi);
    }
}

/*
 * transfer_params() -- Transfer register parameters to the appropriate
 * parameter symbols.  Spill addressed or non-scalar parameter symbols
 * passed in registers to the stack.  (Though the sp_offset calculation is
 * postponed to afin.)
 */
void
CodeGenIa64::transfer_params(OptUnit *unit, bool is_varargs)
{
    int p_count = get_formal_param_count(unit);
    if (p_count == 0)
	return;

    debug(4, "..transfer parameters to appropriate symbols (%s)",
	  get_name(get_proc_sym(unit)).chars());

    int num_arg_regs = GR_LAST_ARG - GR_ARG0 + 1;
    int gp_max = GR_LAST_ARG;
    int fp_max = FR_LAST_ARG;
    int argi = 0;
    int fp_used = 0;		// number of FR arg slots used so far
    bool force_spill;
    VarSym *p;
    TypeId p_type;

    for (int i = 0; i < p_count; i++) {
	p = get_formal_param(unit, i);
	p_type = get_type(p);

	// If this is a varargs.h procedure (rather than a stdarg,h one), break
	// when we see a parameter named va_alist.  This is the start of the
	// varargs parameter spill area, and we don't want to increment the
	// parameter reg pointer.
	if (is_varargs && (strcmp(get_name(p).chars(), "va_alist") == 0)) {
	    claim(i == p_count - 1);	// should be the last formal
	    break;
	}

	// determine if spill needed or not -- spill any addressed
	// parameters, etc. independent of regalloc decisions 
	force_spill = is_addr_taken(p) || !is_scalar(p_type);

	// handle cases where parameter passed in register 
	if (argi < num_arg_regs) {
	    if (is_record(p_type)) {
		// at least part of record in param regs
		int r = GR_ARG0 + argi; 
		set_param_reg(p, r);
		spill_param_reg(opnd_addr_sym(p), r, gp_max);

		// update which param regs used by record 
		int r_i = 0;	      // record index in bytes 
		int p_bytes = get_bit_size(p_type) >> 3;
		while ((argi < num_arg_regs) && (r_i < p_bytes)) {
		    argi++;
		    r_i += 8;	      // increment in bytes
		}

	    } else if (is_floating_point(p_type)) {
		// FP parameter is passed in a param_reg 
		int r = FR_ARG0 + fp_used;
		set_param_reg(p, r);
		if (force_spill)			// on stack 
		    spill_param_reg(opnd_addr_sym(p), r, fp_max);
		else
		    move_param_reg(p, r);
		argi++;
		fp_used++;

	    } else {
		claim(is_scalar(p_type), "unexpected parameter type");

		int r = GR_ARG0 + argi;
		set_param_reg(p, r);
		if (force_spill)			// on stack
		    spill_param_reg(opnd_addr_sym(p), r, gp_max);
		else
		    move_param_reg(p, r);
		argi++;
	    }
	} else {
	    argi = num_arg_regs;	// parameter on stack
	}
    }
}
