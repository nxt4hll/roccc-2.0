/* file "x86_halt/init.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "x86_halt/init.h"
#endif

#include <machine/machine.h>
#include <x86/x86.h>
#include <halt/halt.h>

#include <x86_halt/contexts.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

IdString k_x86_halt;

Context*
context_creator_x86_halt()
{
    return new X86HaltContext;
}

extern "C" void
init_x86_halt(SuifEnv* suif_env)
{
    static bool init_done = false;

    if (init_done)
        return;
    init_done = true;

    init_machine(suif_env);
    init_x86(suif_env);
    init_halt(suif_env);

    k_x86_halt = "x86_halt";

    the_context_creator_registry[k_x86_halt] = context_creator_x86_halt;
}
