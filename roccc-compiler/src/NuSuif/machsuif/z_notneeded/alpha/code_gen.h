/* file "alpha/code_gen.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef ALPHA_CODE_GEN_H
#define ALPHA_CODE_GEN_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "alpha/code_gen.h"
#endif

#include <machine/machine.h>
#include <suifvm/suifvm.h>

class CodeGenAlpha : public CodeGen {    
  protected:
    virtual void translate_null(Instr*);
    virtual void translate_nop(Instr*);
    virtual void translate_cvt(Instr*);
    virtual void translate_lda(Instr*);
    virtual void translate_ldc(Instr*);
    virtual void translate_add(Instr*);
    virtual void translate_sub(Instr*);
    virtual void translate_neg(Instr*);
    virtual void translate_mul(Instr*);
    virtual void translate_div(Instr*);
    virtual void translate_rem(Instr*);
    virtual void translate_mod(Instr*);
    virtual void translate_abs(Instr*);
    virtual void translate_min(Instr*);
    virtual void translate_max(Instr*);
    virtual void translate_not(Instr*);
    virtual void translate_and(Instr*);
    virtual void translate_ior(Instr*);
    virtual void translate_xor(Instr*);
    virtual void translate_asr(Instr*);
    virtual void translate_lsl(Instr*);
    virtual void translate_lsr(Instr*);
    virtual void translate_rot(Instr*);
    virtual void translate_mov(Instr*);
    virtual void translate_lod(Instr*);
    virtual void translate_str(Instr*);
    virtual void translate_memcpy(Instr*);
    virtual void translate_seq(Instr*);
    virtual void translate_sne(Instr*);
    virtual void translate_sl(Instr*);
    virtual void translate_sle(Instr*);
    virtual void translate_btrue(Instr*);
    virtual void translate_bfalse(Instr*);
    virtual void translate_beq(Instr*);
    virtual void translate_bne(Instr*);
    virtual void translate_bge(Instr*);
    virtual void translate_bgt(Instr*);
    virtual void translate_ble(Instr*);
    virtual void translate_blt(Instr*);
    virtual void translate_jmp(Instr*);
    virtual void translate_jmpi(Instr*);
    virtual void translate_mbr(Instr*);
    virtual void translate_cal(Instr*);
    virtual void translate_ret(Instr*);
    virtual void translate_any(Instr*);
    virtual void translate_mrk(Instr*);

    virtual void transfer_params(OptUnit*, bool is_varargs);

    // helper routines
    Instr* translate_cvt_zap(TypeId, Opnd d, Opnd s);
    Instr* translate_cvt_shf(int d_sz, Opnd d, int s_sz, Opnd s);
    void translate_bcond(Instr*, bool flip, int cmp, int ucmp,
			 int cbr, int fcmp, int fcbr);
    void translate_fp_scc(Instr *mi, int cmp_op, int fb_op);

    void move_param_reg(VarSym*, int reg);
    void spill_param_reg(Opnd v_addr, int r_1st, int r_max);
    void extract_immed_srcs(Instr *mi);
    void maybe_use_reg_gp(Instr*);

    void emit_ots_call(IdString ots_proc, Instr *div_rem);

    virtual void expand_struct_move(Instr *lr, Instr *sr);
    virtual int expand_struct_move2(Instr *lr, Instr *sr, int next_freeAR,
				    NatSet *regs_used);
    virtual void do_small_struct_move(Instr *lr, Instr *sr,
				      int struct_alignment, int struct_size);
    virtual void do_big_struct_move(Instr *lr, Instr *sr,
				    int struct_alignment, int struct_size);
    
    // per-unit variables
    List<Instr*> struct_lds;	// Hold list for struct loads

  public:
    CodeGenAlpha() { }
    virtual void init(OptUnit *unit);
    virtual void finish(OptUnit *unit);
};

/*
 * RelocNote is a target-specific annotation carrying relocation info
 * Its value fields for the Alpha are a reloc name, a sequence number,
 * and index of the relevant operand in the annotated instruction.
 */

class RelocNote : public Note {
 public:
    RelocNote() : Note(note_list_any()) { }
    RelocNote(IdString name, int seq, int opnd_pos)
	: Note(note_list_any())
    {
	set_name(name);
	set_seq(seq);
	set_opnd_pos(opnd_pos);
    }
    RelocNote(const RelocNote &other) : Note(other) { }
    RelocNote(const Note &note) : Note(note) { }

    IdString get_name() const	{ return _get_string(0); }
    void set_name(IdString name){ _replace(0, name); }
    int get_seq() const		{ return _get_c_long(1); }
    void set_seq(int seq)	{ _replace(1, seq); }
    int get_opnd_pos() const	{ return _get_c_long(2); }
    void set_opnd_pos(int pos)	{ _replace(2, pos); }
};

#endif /* ALPHA_CODE_GEN_H */
