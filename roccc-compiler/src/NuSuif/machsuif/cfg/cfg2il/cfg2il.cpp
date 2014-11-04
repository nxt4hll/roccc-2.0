/* file "cfg2il/cfg2il.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "cfg2il/cfg2il.h"
#endif

#include <machine/machine.h>
#include <cfg/cfg.h>

#include <cfg2il/cfg2il.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

void
Cfg2il::do_opt_unit(OptUnit *unit)
{
    IdString name = get_name(get_proc_sym(unit));
    debug(1, "Processing procedure \"%s\"", name.chars());

    // get the body of the OptUnit
    AnyBody *orig_body = get_body(unit);

    if (is_kind_of<InstrList>(orig_body))
        return;		// nothing to do

    claim(is_kind_of<Cfg>(orig_body),
	  "expected OptUnit body in Cfg form");

    // print the CFG if debugging verbosely
    if_debug(5)
      fprint(stderr, static_cast<Cfg*>(orig_body), false, true);

    // convert input body to an InstrList
    InstrList *instr_list = to_instr_list(orig_body);

    // replace original body
    copy_notes(orig_body, instr_list);
    set_body(unit, instr_list);

    // clean up
    delete orig_body;
}
