/* file "exp_alpha/exp_alpha.h" */

/*
    Copyright (c) 1995-99 The President and Fellows of Harvard University

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef EXP_ALPHA_EXP_ALPHA_H
#define EXP_ALPHA_EXP_ALPHA_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "exp_alpha/exp_alpha.h"
#endif

#include <machine/machine.h>
#include <cfg/cfg.h>

/*
 * Expands Alpha macro assembly language instructions into the real
 * sequence of machine instructions for a particular version of the Alpha
 * architecture.
 */

class InstrFilter;

class ExpAlpha {
  public:
    void initialize();
    void do_opt_unit(OptUnit*);
    void finalize();

    // set pass options
    void set_debug_arg(int da)		{ debuglvl = da; }
    void set_no_reloc(bool nr)		{ insert_reloc  = !nr; }
    void set_no_lituse(bool nl)		{ insert_lituse = !nl; }
    void set_use_virtual_regs(bool uv)	{ use_virtual_regs = uv; }
    void set_spill_volatiles(bool sv)	{ spill_volatiles = sv; }
    void set_exp_sublong_ld_st(bool esls) { exp_sublong_ld_st = esls; }

  protected:

    // Pass-option variables
    bool insert_reloc;		// generate relocation operands
    bool insert_lituse;		// generate lituse relocation operands
    bool use_virtual_regs;	// assume register allocation follows this pass
    bool spill_volatiles;	// spill non-local or addressed variables
    bool exp_sublong_ld_st;	// expand LDB/LDW/STB/STW instructions

    // Per-file variables
    int reloc_seq;		// sequence number for relocations in file

    // Per-unit variables
    InstrList *unit_instr_list;	// unit body if in linear (non-CFG) form
    Cfg *unit_cfg;		// unit body when it's a CFG

    bool insert_set_noat;	// true if we may need $at during expansions
    bool noat_is_set;		// true when we have an outstanding .set noat

    // Inter-method communication variables
    CfgNode *cur_cfg_node;

    // Helper methods

    friend class SpillVolatilesOpnd;
    friend class MaybeExpandInstr;

    void map_instrs(InstrFilter&);

    void insert_before(InstrHandle, Instr*);
    void insert_after (InstrHandle, Instr*);
    Instr* remove(InstrHandle);

    void emit(InstrHandle&, Instr*); // insert_after and advance handle
    void maybe_set_noat(InstrHandle&);

    // Worker methods

    void expand_byteword_memory_instr(InstrHandle&, bool is_load);
    void expand_memory_instr(InstrHandle&, bool is_load);
    void expand_ea_calc(InstrHandle&, bool is_load);
    void expand_ldi_immed(InstrHandle&);
    void expand_ldgp_instr(InstrHandle&);
#if 0
    void expand_abs_instr(Instr*, InstrHandle);
    void expand_jsr_instr(Instr*, InstrHandle);
    void expand_mul_instr(Instr*, InstrHandle);
    void expand_immed_opnds(Instr*, InstrHandle);
#endif /* 0 */
};

// Helper class for method map_instrs, which maps operations over instructions

class InstrFilter
{
  public:
    InstrFilter() { }
    virtual ~InstrFilter() { }
    virtual void operator()(InstrHandle) = 0;
};

#endif /* EXP_ALPHA_EXP_ALPHA_H */
