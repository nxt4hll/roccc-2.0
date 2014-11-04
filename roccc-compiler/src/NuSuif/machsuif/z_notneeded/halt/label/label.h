/* file "label/label.h" -- HALT label pass */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef LABEL_LABEL_H
#define LABEL_LABEL_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "label/label.h"
#endif

#include <machine/machine.h>
#include <cfg/cfg.h>

class LabelPass {
  public:
    LabelPass();

    void initialize();
    void do_opt_unit(OptUnit*);
    void finalize();

    // set pass options
    void set_unique_id(int n)		{ unique_id = n; } 
    int  get_unique_id(void)		{ return unique_id; }
    void set_debug_proc(IdString name)  { debug_proc = name; }
    void set_setjmp_proc(IdString name) 
    { 
	if (name == empty_id_string) {
	    setjmp_proc = "setjmp";
	} else {
	    setjmp_proc = name; 
	}
    }

    // instrumentation options
    bool label_cbr;
    bool label_mbr;
    bool label_entry;
    bool label_exit;
    bool label_setlongjmp;
    bool label_lod;
    bool label_str;
    bool label_bb;

    // Variables to store the values of setjmp and longjmp symbols
    Sym* setjmp;
    Sym* longjmp;

   protected:
    // option variables
    IdString debug_proc;
    IdString setjmp_proc;

    // per-unit variables
    OptUnit *cur_unit;
    Cfg	*unit_cfg;

    // instrumentation counters.  Each instrumentation point
    // gets a unique number.  If you're instrumenting procedure
    // entries and exits with the same unique number, we use
    // unique_id_entry to remember the entry number
    // while searching for the exits.
    long unique_id;
    long unique_id_entry;

    // labelling routines
    void label_block(CfgNode*, int);
    void label_instr(Instr*, int, long);
};

#endif /* LABEL_LABEL_H */


