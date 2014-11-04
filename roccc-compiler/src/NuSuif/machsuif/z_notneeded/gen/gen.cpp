/* file "gen/gen.cc" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "gen/gen.h"
#endif

#include <machine/machine.h>

#include <gen/gen.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif


/* Purpose of program: This program translates a Machine-SUIF file
 * from SUIFvm instructions to code for a real target. */

void
Gen::initialize()
{
    debug(1, "Debug level is %d", debuglvl);

    code_gen = target_code_gen();
}

void
Gen::do_opt_unit(OptUnit *unit)
{
    debug(3, "Processing unit %s", get_name(unit).chars());
    
    // be sure unit body is a linear instruction list
    claim(is_kind_of<InstrList>(get_body(unit)), "Body is not an InstrList");

    // get ready to translate the procedure
    code_gen->init(unit);

    // complete any target-specific massaging of suifvm code
    code_gen->massage_unit(unit);

    // do code generation
    code_gen->translate_list(unit, to<InstrList>(get_body(unit)));

    // complete bookkeeping due to code generation
    code_gen->finish(unit);

    debug(6, "Code generated for unit %s ...", get_name(unit).chars());
    if_debug(6)
	fprint(stderr, to<InstrList>(get_body(unit)));
}
