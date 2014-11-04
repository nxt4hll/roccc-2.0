/* file "x86/code_gen.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef X86_CODE_GEN_H
#define X86_CODE_GEN_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "x86/code_gen.h"
#endif

#include <machine/machine.h>
#include <suifvm/suifvm.h>

#include <x86/fpstack.h>


class CodeGenX86 : public CodeGen {    
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

    virtual void transfer_params(OptUnit*, bool is_varargs) { }

    // helper routines
    Instr* make_2opnd(Instr*);
    void widen_index_opnd(Instr*);
#ifndef CFE_CONVERSION_FIXED
    Opnd widen_opnds(Instr*, bool is_binary);
#endif
    Instr* create_fld_const(Instr*);
    void create_div_sequence(Instr*);

    virtual void translate_fp(int opc, Instr*);
    virtual void translate_bcc(int signed_opcode, int unsigned_opcode, Instr*);
    virtual void translate_iscc(int cmp_op, int set_op, int uset_op, Instr*);

    virtual void expand_struct_move(Instr *lr, Instr *sr);
    virtual void do_small_struct_move(Instr *lr, Instr *sr,
				      int struct_alignment, int struct_size);
    virtual void do_big_struct_move(Instr *lr, Instr *sr,
				    int struct_alignment, int struct_size);
    virtual void do_remaining_moves(int remaining_bytes, Opnd vrldb_opd,
				    Opnd vrstb_opd, int struct_offset);

    // scratch variables
    list<Instr *> struct_lds;

    FpStack fp_stack;
    VarSym *fp_stack_safe;		// place to save FP stack across calls

  public:
    CodeGenX86() : fp_stack(8) { }
    virtual void init(OptUnit *unit);
    virtual void finish(OptUnit *unit);
};

#endif /* X86_CODE_GEN_H */
