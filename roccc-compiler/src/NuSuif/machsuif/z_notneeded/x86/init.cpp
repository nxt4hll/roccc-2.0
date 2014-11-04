/* file "x86/init.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "x86/init.h"
#endif

#include <machine/machine.h>
#include <suifvm/suifvm.h>

#include <x86/init.h>
#include <x86/reg_info.h>
#include <x86/opcodes.h>
#include <x86/contexts.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

using namespace x86;

IdString k_x86;

// Useful global definitions.  We can define a single operand for each
// of the following registers because they always hold values of a
// single type.  These are initialized in init_x86().
Opnd opnd_reg_const0;
Opnd opnd_reg_sp;
Opnd opnd_reg_fp;
Opnd opnd_reg_eflags;


Context*
context_creator_x86()
{
    return new X86Context;
}

extern "C" void
init_x86(SuifEnv* suif_env)
{
    static bool init_done = false;

    if (init_done)
        return;
    init_done = true;

    init_machine(suif_env);
    init_suifvm(suif_env);

    k_x86 = "x86";

    // Initializations for opcode-indexed tables
    init_x86_opcode_names();
    init_x86_opcode_ext_names();
    init_x86_invert_table();
    init_x86_reg_tables();

    // Initialize useful global operand variables.
    opnd_reg_const0 = opnd_reg(CONST0, type_u32);
    opnd_reg_sp     = opnd_reg(ESP,    type_p32);
    opnd_reg_fp     = opnd_reg(EBP,    type_p32);
    opnd_reg_eflags = opnd_reg(EFLAGS, type_u32);

    the_context_creator_registry[k_x86] = context_creator_x86;
}
