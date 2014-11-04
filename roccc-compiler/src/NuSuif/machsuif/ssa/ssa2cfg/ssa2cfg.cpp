/* file "ssa2cfg/ssa2cfg.cpp" */

/*
    Copyright (c) 2001 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "ssa2cfg/ssa2cfg.h"
#endif

#include <machine/machine.h>
#include <cfg/cfg.h>
#include <ssa/ssa.h>

#include <ssa2cfg/ssa2cfg.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

void
Ssa2cfg::do_opt_unit(OptUnit *unit)
{
    IdString name = get_name(get_proc_sym(unit));
    debug(1, "Processing procedure \"%s\"", name.chars());

    // get the body of the OptUnit
    AnyBody *orig_body = get_body(unit);

    claim(is_kind_of<SsaCfg>(orig_body),
	  "Expected OptUnit body in Cfg form");
    SsaCfg *unit_ssa = static_cast<SsaCfg*>(orig_body);

    // Print the SSA CFG if debugging verbosely.
    if_debug(5)
	fprint(stderr, unit_ssa);

    Cfg *unit_cfg = restore(unit_ssa, !restore_orig_names);
    move_notes(orig_body, unit_cfg);
    set_body(unit, unit_cfg);

    delete unit_ssa;
}
