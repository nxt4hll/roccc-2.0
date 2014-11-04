/* file "x86_halt_blpp/init.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "x86_halt_blpp/init.h"
#endif

#include <machine/machine.h>
#include <cfg/cfg.h>
#include <halt/halt.h>
#include <halt_blpp/halt_blpp.h>
#include <x86/x86.h>
#include <x86_halt/x86_halt.h>

#include <x86_halt_blpp/recipe.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

IdString k_x86_halt_blpp;

/*
 * The target-specific context for this library is the same as that for
 * the x86_halt library.  The only difference is that we extend the
 * tables of kinds and kind-specific analysis routine names/symbols.
 */
Context*
context_creator_x86_halt_blpp()
{
    // create a new X86HaltContext instance and init x86_halt recipes
    Context *c = the_context_creator_registry[k_x86_halt]();

    init_halt_blpp_recipes_x86();
    return c;
}

/*
 * This library has no direct reason to initialize the halt_blpp library,
 * but if it doesn't, then the extra blpp kinds and the extra analysis
 * routine symbol won't be installed.
 */
extern "C" void
init_x86_halt_blpp(SuifEnv* suif_env)
{
    static bool init_done = false;

    if (init_done)
        return;
    init_done = true;

    init_machine(suif_env);
    init_cfg(suif_env);
    init_halt(suif_env);
    init_halt_blpp(suif_env);
    init_x86(suif_env);
    init_x86_halt(suif_env);

    k_x86_halt_blpp = "x86_halt_blpp";

    the_context_creator_registry[k_x86_halt_blpp] =
	context_creator_x86_halt_blpp;
}
