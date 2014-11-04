/* file "x86/instr.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef X86_INSTR_H
#define X86_INSTR_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "x86/instr.h"
#endif

bool is_ldc_x86(Instr*);
bool is_move_x86(Instr*);
bool is_cmove_x86(Instr*);
bool is_line_x86(Instr*);
bool is_ubr_x86(Instr*);
bool is_cbr_x86(Instr*);
bool is_call_x86(Instr*);
bool is_return_x86(Instr*);
bool is_binary_exp_x86(Instr*);
bool is_unary_exp_x86(Instr*);
bool is_commutative_x86(Instr*);
bool is_two_opnd_x86(Instr*);
bool reads_memory_x86(Instr*);
bool writes_memory_x86(Instr*);

#endif /* X86_INSTR_H */
