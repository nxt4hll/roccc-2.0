/* file "cfg2ssa/cfg2ssa.cpp" */

/*
    Copyright (c) 2001 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "cfg2ssa/cfg2ssa.h"
#endif

#include <machine/machine.h>
#include <cfg/cfg.h>
#include <ssa/ssa.h>

#include <cfg2ssa/cfg2ssa.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

void
Cfg2ssa::do_opt_unit(OptUnit *unit)
{
    IdString name = get_name(get_proc_sym(unit));
    debug(1, "Processing procedure \"%s\"", name.chars());

    // get the body of the OptUnit
    AnyBody *orig_body = get_body(unit);

    claim(is_kind_of<Cfg>(orig_body),
	  "Expected OptUnit body in Cfg form");
    Cfg *unit_cfg = static_cast<Cfg*>(orig_body);

    // Print the CFG if debugging verbosely.
    if_debug(5)
	fprint(stderr, unit_cfg, false, true);

    // Convert input body to SSA form.
    SsaCfg *unit_ssa = new_ssa_cfg(unit_cfg, unit, build_flags);
    set_body(unit, unit_ssa);

    // Replace the original body, the CFG, with the SSA form, which now
    // embeds the CFG.
    move_notes(orig_body, unit_ssa);
    set_body(unit, unit_ssa);
}
