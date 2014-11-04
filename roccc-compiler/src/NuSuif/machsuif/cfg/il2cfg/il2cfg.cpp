/* file "il2cfg/il2cfg.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "il2cfg/il2cfg.h"
#endif

#include <machine/machine.h>
#include <cfg/cfg.h>

#include <il2cfg/il2cfg.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

void
Il2cfg::do_opt_unit(OptUnit *unit)
{
    IdString name = get_name(get_proc_sym(unit));
    debug(1, "Processing procedure \"%s\"", name.chars());

    // get the body of the OptUnit
    AnyBody *orig_body = get_body(unit);

    if (is_kind_of<Cfg>(orig_body)) {
	// just make sure that the options are correct
	canonicalize((Cfg*)orig_body, keep_layout,
		     break_at_call, break_at_instr);
        return;
    }

    claim(is_kind_of<InstrList>(orig_body),
	  "expected OptUnit body in InstrList form");

    // convert input body to a Cfg
    Cfg *cfg = new_cfg((InstrList*)orig_body, keep_layout,
		       break_at_call, break_at_instr);
        
    // replace original body
    copy_notes(orig_body, cfg);
    set_body(unit, cfg);

    if_debug(5)
	fprint(stderr, cfg, false, true);		// no layout, just code

    // clean up
    delete orig_body;
}
