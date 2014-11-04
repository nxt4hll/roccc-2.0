/* file "suifvm/code_gen.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef SUIFVM_CODE_GEN_H
#define SUIFVM_CODE_GEN_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "suifvm/code_gen.h"
#endif

#include <machine/machine.h>

class CodeGen {
  public:
    virtual ~CodeGen() { }
    virtual void init(OptUnit *unit);	// must be called 1st!
    virtual void massage_unit(OptUnit *unit) { }
    virtual void finish(OptUnit *unit);	// must be called last!

    void translate_instr(OptUnit *unit, Instr*, InstrList *receiver);
    void translate_list(OptUnit *unit, InstrList *in_place);

    void emit(Instr*);

  private:
    void translate_mi(Instr*);

  protected:
    virtual void translate_null(Instr*) = 0;
    virtual void translate_nop(Instr*) = 0;
    virtual void translate_cvt(Instr*) = 0;
    virtual void translate_lda(Instr*) = 0;
    virtual void translate_ldc(Instr*) = 0;
    virtual void translate_add(Instr*) = 0;
    virtual void translate_sub(Instr*) = 0;
    virtual void translate_neg(Instr*) = 0;
    virtual void translate_mul(Instr*) = 0;
    virtual void translate_div(Instr*) = 0;
    virtual void translate_rem(Instr*) = 0;
    virtual void translate_mod(Instr*) = 0;
    virtual void translate_abs(Instr*) = 0;
    virtual void translate_min(Instr*) = 0;
    virtual void translate_max(Instr*) = 0;
    virtual void translate_not(Instr*) = 0;
    virtual void translate_and(Instr*) = 0;
    virtual void translate_ior(Instr*) = 0;
    virtual void translate_xor(Instr*) = 0;
    virtual void translate_asr(Instr*) = 0;
    virtual void translate_lsl(Instr*) = 0;
    virtual void translate_lsr(Instr*) = 0;
    virtual void translate_rot(Instr*) = 0;
    virtual void translate_mov(Instr*) = 0;
    virtual void translate_lod(Instr*) = 0;
    virtual void translate_str(Instr*) = 0;
    virtual void translate_memcpy(Instr*) = 0;
    virtual void translate_seq(Instr*) = 0;
    virtual void translate_sne(Instr*) = 0;
    virtual void translate_sl(Instr*) = 0;
    virtual void translate_sle(Instr*) = 0;
    virtual void translate_btrue(Instr*) = 0;
    virtual void translate_bfalse(Instr*) = 0;
    virtual void translate_beq(Instr*) = 0;
    virtual void translate_bne(Instr*) = 0;
    virtual void translate_bge(Instr*) = 0;
    virtual void translate_bgt(Instr*) = 0;
    virtual void translate_ble(Instr*) = 0;
    virtual void translate_blt(Instr*) = 0;
    virtual void translate_jmp(Instr*) = 0;
    virtual void translate_jmpi(Instr*) = 0;
    virtual void translate_mbr(Instr*) = 0;
    virtual void translate_cal(Instr*) = 0;
    virtual void translate_ret(Instr*) = 0;
    virtual void translate_any(Instr*) = 0;
    virtual void translate_mrk(Instr*) = 0;

    InstrList *receiver;		// client-provided output list
    Note line_note;			// prevailing source-line annotation

    // convenient variables
    OptUnit *cur_unit;			// defined by init()
    const char *cur_unit_name;		// name of current procedure
    CProcedureType *cur_unit_type;	// type of current procedure

    // variables that direct CodeGen
    bool report_int_overflow, report_int_divideby0;

    // working variables from cur_unit's stack_frame_info annotation
    bool is_leaf;
    bool is_varargs;
    int max_arg_area;

    virtual void transfer_params(OptUnit*, bool is_varargs) = 0;
    
    CodeGen() { }
};

CodeGen* target_code_gen();

#endif /* SUIFVM_CODE_GEN_H */
