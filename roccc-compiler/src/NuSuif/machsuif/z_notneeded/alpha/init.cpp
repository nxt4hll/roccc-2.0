/* file "alpha/init.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "alpha/init.h"
#endif

#include <machine/machine.h>
#include <suifvm/suifvm.h>

#include <alpha/opcodes.h>
#include <alpha/reg_info.h>
#include <alpha/contexts.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

IdString k_alpha;
IdString k_gprel_init;
IdString k_hint;
IdString k_reloc;
IdString k_next_free_reloc_num;
IdString k_gp_home;

IdString k_OtsDivide64;		// Object-time-system functions..
IdString k_OtsDivide64Unsigned;	// ..for integer divide and remainder
IdString k_OtsDivide32;
IdString k_OtsDivide32Unsigned;
IdString k_OtsRemainder64;
IdString k_OtsRemainder64Unsigned;
IdString k_OtsRemainder32;
IdString k_OtsRemainder32Unsigned;

/*
 * Useful operand definitions.
 */
Opnd opnd_immed_0_u64;
Opnd opnd_immed_1_u64;
Opnd opnd_immed_8_u64;
Opnd opnd_immed_16_u64;
Opnd opnd_immed_24_u64;



Context*
context_creator_alpha()
{
    return new AlphaContext;
}

extern "C" void
init_alpha(SuifEnv* suif_env)
{
    static bool init_done = false;

    if (init_done)
        return;
    init_done = true;

    init_suifvm(suif_env);

    k_alpha = "alpha";
    k_gprel_init = "gprel_init";
    k_hint = "hint";
    k_reloc = "reloc";
    k_next_free_reloc_num = "next_free_reloc_num";
    k_gp_home = "__gp_home__";

    k_OtsDivide64            = "_OtsDivide64";
    k_OtsDivide64Unsigned    = "_OtsDivide64Unsigned";
    k_OtsDivide32            = "_OtsDivide32";
    k_OtsDivide32Unsigned    = "_OtsDivide32Unsigned";
    k_OtsRemainder64         = "_OtsRemainder64";
    k_OtsRemainder64Unsigned = "_OtsRemainder64Unsigned";
    k_OtsRemainder32         = "_OtsRemainder32";
    k_OtsRemainder32Unsigned = "_OtsRemainder32Unsigned";

    nonprinting_notes.insert(k_hint);
    nonprinting_notes.insert(k_reloc);

    // initializations for opcode-indexed tables
    init_alpha_opcode_names();
    init_alpha_opcode_ext_names();
    init_alpha_invert_table();
    init_alpha_reg_tables();

    // initializations for  useful global operand variables
    opnd_reg_const0 = opnd_reg(alpha::CONST0_GPR, type_p64);
    opnd_reg_ra     = opnd_reg(alpha::RA, type_p64);
    opnd_reg_pv     = opnd_reg(alpha::PV, type_p64);
    opnd_reg_sp     = opnd_reg(alpha::SP, type_p64);
    opnd_reg_gp     = opnd_reg(alpha::GP, type_p64);

    opnd_immed_0_u64  = opnd_immed(0, type_u64);
    opnd_immed_1_u64  = opnd_immed(1, type_u64);
    opnd_immed_8_u64  = opnd_immed(8, type_u64);
    opnd_immed_16_u64 = opnd_immed(16, type_u64);
    opnd_immed_24_u64 = opnd_immed(24, type_u64);

    the_context_creator_registry[k_alpha] = context_creator_alpha;
}
