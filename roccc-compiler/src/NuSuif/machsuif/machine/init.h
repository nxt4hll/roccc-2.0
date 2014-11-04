/* file "machine/init.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef MACHINE_INIT_H
#define MACHINE_INIT_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "machine/init.h"
#endif

#include <machine/substrate.h>

extern "C" void init_machine(SuifEnv* suif_env);

extern IdString k_target_lib;		// see contexts.h.nw
extern IdString k_target_type_ptr;	// see types.h.nw
extern IdString k_generic_types;	// see types.h.nw
extern IdString k_enable_exceptions;	// see codegen.cc
extern IdString k_stack_frame_info;	// see codegen.h.nw

extern IdString k_empty_string;		// I.e. "" (used for defaults)

extern IdString k_history;		// Note listing compilation history
extern IdString k_line;			// Note flagging .line directive

extern IdString k_comment;		// Note containing comment text
extern IdString k_mbr_target_def;	//   "   marking mbr target calculation
extern IdString k_mbr_index_def;	//   "   (deprecated: use preceding key)
extern IdString k_mbr_table_use;	//   "   marking mbr dispatch table use
extern IdString k_instr_mbr_tgts;	//   "   giving mbr case values/labels
extern IdString k_instr_opcode;		//   "   for generic-instr opcode_name
extern IdString k_instr_opcode_exts;	//   "   for opcode extensions
extern IdString k_proc_entry;		//   "   marking procedure entry pt.
extern IdString k_regs_used;		//   "   giving regs used at call
extern IdString k_regs_defd;		//   "   giving regs def'd at call
extern IdString k_instr_ret;		//   "	 marking return instruction
extern IdString k_incomplete_proc_exit;	//   "   marking incomplete proc exit
extern IdString k_header_trailer;	//   "   on instruction added by fin
extern IdString k_builtin_args;		//   "   giving args for builtin "call"
extern IdString k_param_reg;		//   "   giving hard reg for parameter
extern IdString k_vr_count;		//   "   giving unit's virtual-reg count
extern IdString k_stdarg_offset;	//   "   giving unnamed-arg frame offset

extern IdString k_const;                // keyword "const"

extern IdString k_dense;		// NatSetNote tag
extern IdString k_dense_inverse;	//     "       "
extern IdString k_sparse;		//     "       "
extern IdString k_sparse_inverse;	//     "       "

extern IdString k_call_target;		// Note attaching target symbol to call
extern IdString k_param_init;		//   "  marking instr to init proc param

#endif /* MACHINE_INIT_H */
