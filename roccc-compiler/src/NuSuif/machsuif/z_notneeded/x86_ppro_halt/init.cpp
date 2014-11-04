/* file "x86_ppro_halt/init.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "x86_ppro_halt/init.h"
#endif

#include <machine/machine.h>
#include <x86_ppro/x86_ppro.h>
#include <x86_halt/x86_halt.h>

#include "init.h"
#include "contexts.h"

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

IdString k_x86_ppro_halt;

Context*
context_creator_x86_ppro_halt()
{
    return new X86PProHaltContext;
}

extern "C" void
init_x86_ppro_halt(SuifEnv* suif_env)
{
    static bool init_done = false;

    if (init_done)
        return;
    init_done = true;

    init_machine(suif_env);
    init_x86_ppro(suif_env);
    init_x86_halt(suif_env);

    k_x86_ppro_halt = "x86_ppro_halt";

    the_context_creator_registry[k_x86_ppro_halt] = context_creator_x86_ppro_halt;
}
