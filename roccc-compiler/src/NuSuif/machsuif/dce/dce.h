/* file "dce/dce.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef DCE_DCE_H
#define DCE_DCE_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "dce/dce.h"
#endif

/*
 * dce is a dead code elimiation pass based on the algorithm
 * in Robert Morgan's book "Building an Optimizing Compiler",
 * section 8.9.
 */

#include <machine/machine.h>
#include <cfg/cfg.h>
#include <bvd/bvd.h>

class Dce {
  public:
    Dce() { }

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
    Cfg			*unit_cfg;
    DominanceInfo	*dominance_info;
    RegSymCatalog	*opnd_catalog;
    DefUseAnalyzer	*du_analyzer;
    ReachingDefs	*reaching_defs;

    List<Instr*>	worklist;
    HashMap<Instr*,NatSetSparse> dependences;

    // Submethods of do_opt_unit
    void find_basic_necessities();
    void process_worklist();
    void discard_unnecessities();

    void mark_necessary(Instr*);
    void print_numbered_instr(char *mess, Instr*);
    bool writes_volatile(Instr*);
};

#endif /* DCE_DCE_H */
