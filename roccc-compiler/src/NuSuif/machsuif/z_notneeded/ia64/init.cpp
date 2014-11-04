/* file "ia64/init.cpp" */

/*
    Copyright (c) 2002 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "ia64/init.h"
#endif

#include <machine/machine.h>
#include <suifvm/suifvm.h>

#include <ia64/init.h>
#include <ia64/reg_info.h>
#include <ia64/opcodes.h>
#include <ia64/contexts.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

IdString k_ia64;
IdString k_va_first_unnamed;

IdString k_divdi3;	// Object-time-system functions..
IdString k_divsf3;	// Object-time-system functions..
IdString k_moddi3;	// ..for integer divide and remainder
IdString k_memcpy;	

/*
 * Useful operand definitions.
 */
Opnd opnd_immed_0_u64;
Opnd opnd_immed_1_u64;
Opnd opnd_immed_neg1_u64;
Opnd opnd_immed_8_u64;
Opnd opnd_immed_16_u64;
Opnd opnd_immed_24_u64;


TypeId type_pr;			// Type marking predicate register candidates
TypeId type_br;			//   "     "      branch      "        "
TypeId type_f80;		// 80-bit floating point type

IdString k_type_pr;
IdString k_type_br;

bool
is_type_pr(TypeId type)
{
    return is_a<BooleanType>(type) &&
	static_cast<BooleanType*>(type)->get_name() == k_type_pr;
}

bool
is_type_br(TypeId type)
{
    return is_a<PointerType>(type) &&
	static_cast<BooleanType*>(type)->get_name() == k_type_br;
}

void
init_ia64_special_types()
{
    type_pr = create_boolean_type(the_suif_env,  1,  0, k_type_pr);
    type_br = create_pointer_type(the_suif_env, 64, 64, type_v0, k_type_br);
    type_f80 = create_floating_point_type(the_suif_env, 64, 64); /*FIXME*/

    FileSetBlock *fsb = to<FileSetBlock>(the_file_block->get_parent());
    SymTable *st = fsb->get_external_symbol_table();

    st->append_symbol_table_object(type_pr);
    st->append_symbol_table_object(type_br);
    st->append_symbol_table_object(type_f80);
}

Context*
context_creator_ia64()
{
    return new Ia64Context;
}

extern "C" void
init_ia64(SuifEnv* suif_env)
{
    static bool init_done = false;

    if (init_done)
        return;
    init_done = true;

    init_machine(suif_env);
    init_suifvm(suif_env);

    k_ia64 = "ia64";
    k_va_first_unnamed = "va_first_unnamed";

    k_type_pr = "type_pr";
    k_type_br = "type_br";

    k_divdi3    = "__divdi3";
    k_divsf3    = "__divsf3";
    k_moddi3    = "__moddi3";
    k_memcpy    = "memcpy";

    // Initialize opcode- and register-indexed tables.
    init_ia64_opcode_stem_names();
    init_ia64_opcode_ext_names();
    init_ia64_reg_tables();

    // Initialize special types.
    init_ia64_special_types();

    // Initialize useful global operand variables.
    opnd_reg_const0 = opnd_reg(ia64::GR_CONST0, type_p64);
    opnd_reg_gp = opnd_reg(ia64::GP, type_p64);
    opnd_reg_sp = opnd_reg(ia64::SP, type_p64);
    opnd_reg_tp = opnd_reg(ia64::TP, type_p64);
    opnd_pr_const1 = opnd_reg(ia64::PR_CONST1, type_pr);
    opnd_fr_const0 = opnd_reg(ia64::FR_CONST0, type_f80);
    opnd_fr_const1 = opnd_reg(ia64::FR_CONST1, type_f80);
    opnd_br_ra     = opnd_reg(ia64::BR_RA, type_br);
    opnd_reg_sp     = opnd_reg(ia64::SP, type_p64);
    opnd_reg_gp     = opnd_reg(ia64::GP, type_p64);

    opnd_immed_0_u64  = opnd_immed(0, type_u64);
    opnd_immed_1_u64  = opnd_immed(1, type_u64);
    opnd_immed_neg1_u64  = opnd_immed(-1, type_u64);
    opnd_immed_8_u64  = opnd_immed(8, type_u64);
    opnd_immed_16_u64 = opnd_immed(16, type_u64);
    opnd_immed_24_u64 = opnd_immed(24, type_u64);

    the_context_creator_registry[k_ia64] = context_creator_ia64;
}
