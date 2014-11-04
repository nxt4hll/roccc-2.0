/* file "peep/peep.h" */

/*
    Copyright (c) 1995-99 The President and Fellows of Harvard University

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef PEEP_PEEP_H
#define PEEP_PEEP_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "peep/peep.h"
#endif

#include <machine/machine.h>
#include <cfg/cfg.h>
#include <bvd/bvd.h>

/*
 * peep is a simple pass that scans each procedure's instruction list
 * trying to eliminate unneeded register-move instructions.  Like a
 * peephole optimizer, peep looks only at a small "window" of consecutive
 * instructions at one time.  In general, it can combine moves only with
 * immediately adjacent instructions.  It is capable of ignoring
 * intervening "line" directives, however.  (A line directive is a
 * pseudo-op that connects points in the instruction list back to the
 * original source line from whence they came.)
 *
 * Unlike the typical peephole optimizer, peep takes advantage of data-flow
 * analysis.  It invokes a liveness analyzer on registers and local
 * register variables.  This allows it to prove some move instructions
 * useless using facts not visible through the peephole alone.
 */

class Peep {
  public:
    Peep() { }

    void initialize();
    void do_opt_unit(OptUnit*);
    void finalize();

    // set pass options
    void set_debug_arg(int da)		{ debuglvl = da; }
    void set_const_prop(int cp)		{ const_prop = cp; }

  protected:

    // Per-pass variables
    bool const_prop;			// push constants forward

					// Statistics -- mnemonics based on
					// Alpha opcodes, though peep works
					// for any target architecture.
    int no_of_remaining_moves;
    int no_of_mov_rx_to_rx, no_of_fmov_rx_to_rx;
    int no_of_mov_after_op, no_of_fmov_after_op;
    int no_of_mov_before_str, no_of_fmov_before_str;
    int no_of_ldc_before_exp;

    void update_counts(bool is_fp, int &i_cnt, int &f_cnt) 
	{ if (is_fp) f_cnt++; else i_cnt++; }

    // handlers for particular instruction kinds
    bool process_mov_op(Instr*, Instr*, CfgNode*, NatSet*);	// moves
    bool process_str_op(Instr*, Instr*, CfgNode*, NatSet*);	// stores
    bool process_exp_op(Instr*, Instr*, CfgNode*, NatSet*);	// expressions
    
    // Per-unit variables
    OptUnit *cur_unit;
    AnyBody *cur_body;
    Cfg *unit_cfg;
    RegSymCatalog *opnd_catalog;
    DefUseAnalyzer *du_analyzer;
    Liveness *liveness;
};

#endif /* PEEP_PEEP_H */
