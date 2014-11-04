/* file "suifvm/init.cc" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "suifvm/init.h"
#endif

#include <machine/machine.h>

#include <suifvm/code_gen.h>
#include <suifvm/contexts.h>
#include <suifvm/opcodes.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

IdString k_suifvm;

Context*
context_creator_suifvm()
{
    return new MachineContextSuifVm; 
}

extern "C" void
init_suifvm(SuifEnv *suif_env)
{
    static bool init_done = false;

    if (init_done)
	return;

    init_done = true;

    init_machine(suif_env);

    k_suifvm = "suifvm";

    // initializations for opcode-indexed tables
    init_suifvm_opcode_names();
    init_suifvm_cnames();

    the_context_creator_registry[k_suifvm] = context_creator_suifvm;
}
