/* file "machine/init.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "machine/init.h"
#endif

#include <machine/substrate.h>
#include <machine/machine_ir.h>
#include <machine/util.h>
#include <machine/init.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

/* string constants */
IdString k_target_lib;
IdString k_generic_types;
IdString k_stack_frame_info;
IdString k_enable_exceptions;
IdString k_empty_string;
IdString k_history;
IdString k_line;
IdString k_comment;
IdString k_mbr_target_def;
IdString k_mbr_index_def;
IdString k_mbr_table_use;
IdString k_instr_mbr_tgts;
IdString k_instr_opcode;
IdString k_instr_opcode_exts;
IdString k_proc_entry;
IdString k_regs_used;
IdString k_regs_defd;
IdString k_instr_ret;
IdString k_incomplete_proc_exit;
IdString k_header_trailer;
IdString k_builtin_args;
IdString k_param_reg;
IdString k_vr_count;
IdString k_stdarg_offset;
IdString k_const;
IdString k_dense;
IdString k_dense_inverse;
IdString k_sparse;
IdString k_sparse_inverse;
IdString k_call_target;
IdString k_param_init;


extern "C" void
init_machine(SuifEnv* suif_env)
{
    static bool init_done = false;

    if (init_done)
	return;
    init_done = true;

    init_typebuilder(suif_env);
    init_utils(suif_env);
    init_bit_vector(suif_env);
    init_suifcloning(suif_env);
    init_machine_irnodes(suif_env);

    // initializations for string constants
    k_target_lib = "target_lib";
    k_generic_types = "generic_types";
    k_stack_frame_info = "stack_frame_info";
    k_enable_exceptions = "enable_exceptions";
    k_empty_string = "";
    k_history = "history";
    k_line = "line";
    k_comment = "comment";
    k_mbr_target_def = "mbr_target_def";
    k_mbr_index_def = "mbr_index_def";
    k_mbr_table_use = "mbr_table_use";
    k_instr_mbr_tgts = "instr_mbr_tgts";
    k_instr_opcode = "instr_opcode";
    k_instr_opcode_exts = "instr_opcode_exts";
    k_proc_entry = "proc_entry";
    k_regs_used = "regs_used";
    k_regs_defd = "regs_defd";
    k_instr_ret = "instr_ret";
    k_incomplete_proc_exit = "incomplete_proc_exit";
    k_header_trailer = "header/trailer";
    k_builtin_args = "builtin args";	// Comes from front end
    k_param_reg = "param_reg";
    k_vr_count = "vr_count";
    k_stdarg_offset = "stdarg_offset";
    k_const = "const";
    k_dense = "dense";
    k_dense_inverse = "dense_inverse";
    k_sparse = "sparse";
    k_sparse_inverse = "sparse_inverse";
    k_call_target = "call_target";
    k_param_init = "param_init";

    // keys of notes not normally printed
    nonprinting_notes.insert(k_comment);
    nonprinting_notes.insert(k_instr_opcode_exts);
    nonprinting_notes.insert(k_line);
    nonprinting_notes.insert(k_proc_entry);
}
