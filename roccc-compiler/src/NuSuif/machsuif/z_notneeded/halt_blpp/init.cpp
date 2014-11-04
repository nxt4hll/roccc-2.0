/* file "halt_blpp/init.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "halt_blpp/init.h"
#endif

#include <machine/machine.h>
#include <halt/halt.h>

#include <halt_blpp/kinds.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

extern "C" void
init_halt_blpp(SuifEnv* suif_env)
{
    static bool init_done = false;

    if (init_done)
	return;
    init_done = true;

    init_machine(suif_env);
    init_halt(suif_env);

    init_halt_blpp_proc_names();    
    init_halt_blpp_proc_syms();
}
