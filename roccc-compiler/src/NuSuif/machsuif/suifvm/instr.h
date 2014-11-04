/* file "suifvm/instr.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef SUIFVM_INSTR_H
#define SUIFVM_INSTR_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "suifvm/instr.h"
#endif

#include <machine/machine.h>

bool is_ldc_suifvm(Instr*);
bool is_move_suifvm(Instr*);
bool is_cmove_suifvm(Instr*);
bool is_line_suifvm(Instr*);

bool is_ubr_suifvm(Instr*);
bool is_cbr_suifvm(Instr*);
bool is_call_suifvm(Instr*);
bool is_return_suifvm(Instr*);
bool is_binary_exp_suifvm(Instr*);
bool is_unary_exp_suifvm(Instr*);
bool is_commutative_suifvm(Instr*);
bool reads_memory_suifvm(Instr*);
bool writes_memory_suifvm(Instr*);
bool is_builtin_suifvm(Instr*);

#endif /* SUIFVM_INSTR_H */
