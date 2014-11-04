/* file "ex1/ex1.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "ex1/ex1.h"
#endif

#include <machine/machine.h>

#include "ex1.h"

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

void
Ex1::do_opt_unit(OptUnit *unit)
{
    IdString name = get_name(get_proc_sym(unit));
    printf("Processing procedure \"%s\"\n", name.chars());
}
