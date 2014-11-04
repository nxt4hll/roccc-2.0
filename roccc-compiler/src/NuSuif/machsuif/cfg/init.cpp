/* file "cfg/init.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "cfg/init.h"
#endif

#include <machine/machine.h>

#include "init.h"
#include "cfg_ir.h"

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

/* string constants */
IdString k_cfg_node;

extern "C" void
init_cfg(SuifEnv* suif_env)
{
    static bool init_done = false;

    if (init_done)
	return;
    init_done = true;

    init_machine(suif_env);
    init_cfg_irnodes(suif_env);

    // initializations for string constants
    k_cfg_node = "cfg_node";
}
