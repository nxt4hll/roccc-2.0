/* file "halt/init.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "halt/init.h"
#endif

#include <machine/machine.h>
#include <cfg/cfg.h>
#include <halt/kinds.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

IdString k_halt;

extern "C" void
init_halt(SuifEnv* suif_env)
{
    static bool init_done = false;

    if (init_done)
	return;
    init_done = true;
    init_cfg(suif_env);

    k_halt = "halt";

    init_halt_proc_names();
    init_halt_proc_syms();
}
