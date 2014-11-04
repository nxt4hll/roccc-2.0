/* file "halt/contexts.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "halt/contexts.h"
#endif

#include <machine/machine.h>

#include <halt/recipe.h>
#include <halt/contexts.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

void
halt_begin_unit(OptUnit *unit)
{
    HaltContext *c = dynamic_cast<HaltContext*>(the_context);
    claim (c, "target library not extended with HALT target methods");
    c->halt_begin_unit(unit);
}

void
halt_end_unit(OptUnit *unit)
{
    HaltContext *c = dynamic_cast<HaltContext*>(the_context);
    claim (c, "target library not extended with HALT target methods");
    c->halt_end_unit(unit);
}

HaltRecipe*
halt_recipe(int ik)
{
    HaltContext *c = dynamic_cast<HaltContext*>(the_context);
    claim (c, "target library not extended with HALT target methods");
    return(c->halt_recipe(ik));
}
