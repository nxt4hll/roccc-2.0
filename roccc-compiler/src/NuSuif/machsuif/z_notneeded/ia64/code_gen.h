/* file "ia64/code_gen.h" */

/*
    Copyright (c) 2002 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef IA64_CODE_GEN_H
#define IA64_CODE_GEN_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "ia64/code_gen.h"
#endif

#include <machine/machine.h>
#include <suifvm/suifvm.h>

class CodeGenIa64 : public CodeGen {    
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
    void move_param_reg(VarSym*, int reg);
    void spill_param_reg(Opnd v_addr, int r_1st, int r_max);
    void extract_immed_srcs(Instr *mi);

    void emit_ots_call(IdString ots_proc, Instr *div_rem);
    virtual void expand_struct_move(Instr *lr, Instr *sr);
    virtual int expand_struct_move2(Instr *lr, Instr *sr, int next_freeAR,
                                    NatSet *regs_used);
    virtual void do_small_struct_move(Instr *lr, Instr *sr,
                                      int struct_alignment, int struct_size);
    virtual void do_big_struct_move(Instr *lr, Instr *sr,
                                    int struct_alignment, int struct_size);
    virtual void do_remaining_moves(int remaining_bytes, Opnd vrldb_opnd,
				    Opnd vrstb_opnd);

    virtual void cvt_to_fp(Instr *mi);
    virtual void cvt_to_non_fp_scalar(Instr *mi);
 
    // per-unit variables
    List<Instr*> struct_lds;	// Hold list for struct loads

  public:
    CodeGenIa64() { }
    virtual void init(OptUnit *unit);
    virtual void finish(OptUnit *unit);
};

#endif /* IA64_CODE_GEN_H */
