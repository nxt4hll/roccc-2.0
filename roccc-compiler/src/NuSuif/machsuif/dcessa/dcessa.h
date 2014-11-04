/* file "dcessa/dcessa.h" */

/*
    Copyright (c) 2001 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef DCESSA_DCE_H
#define DCESSA_DCE_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "dcessa/dcessa.h"
#endif

/*
 * dce is a dead code elimiation pass based on the algorithm
 * in Robert Morgan's book "Building an Optimizing Compiler",
 * section 8.9.
 */

#include <machine/machine.h>
#include <cfg/cfg.h>
#include <cfa/cfa.h>
#include <bvd/bvd.h>
#include <ssa/ssa.h>

class Dcessa {
  public:
    Dcessa() { }

    void initialize();
    void do_opt_unit(OptUnit*);
    void finalize() { }

    // set pass options
    void set_debug_arg(int da)		{ debuglvl = da; }

  protected:

    // Per-pass variables
    List<IdString> files;		// Names of source files in file set

    // Per-unit variables
    OptUnit		*cur_unit;
    AnyBody		*cur_body;
    SsaCfg		*unit_ssa;
    Cfg			*unit_cfg;
    DominanceInfo	*dominance_info;
    RegSymCatalog	*opnd_catalog;
    DefUseAnalyzer	*du_analyzer;
    NatSetDense		necessary_phi_nodes;
					// phi-nodes needed by necessary instrs
    NatSetSparse	unneeded_cti_blocks;
					// ids of blocks whose CTIs are unneeded

    List<Operation>	worklist;

    // Submethods of do_opt_unit
    void find_basic_necessities();
    void process_worklist();
    void discard_unnecessities();
    void retarget_unneeded_ctis();

    void mark_controlling_ctis(CfgNode*);
    void mark_necessary(Operation);
    void print_numbered_instr(char *mess, Instr*);
    bool writes_volatile(Instr*);
    bool writes_state_reg(Instr*);

    friend class MarkDefNecessaryFilter;
};

#endif /* DCESSA_DCE_H */
