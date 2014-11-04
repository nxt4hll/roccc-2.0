/* file "alpha_halt/init.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard University

    All rights reserved.

    This software is provided under the terms described in
    the "suif_copyright.h" include file.
*/

#include <common/suif_copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "alpha_halt/init.h"
#endif

#include <machine/machine.h>
#include <alpha/alpha.h>
#include <halt/halt.h>

#include "init.h"
#include "contexts.h"
#include "recipe.h"

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

IdString k_alpha_halt;

Context*
context_creator_alpha_halt()
{
    return new AlphaHaltContext;
}

extern "C" void
init_alpha_halt(SuifEnv* suif_env)
{
    static bool init_done = false;

    if (init_done)
        return;
    init_done = true;

    init_machine(suif_env);
    init_alpha(suif_env);
    init_halt(suif_env);

    k_alpha_halt = "alpha_halt";

    the_context_creator_registry[k_alpha_halt] = context_creator_alpha_halt;
}
