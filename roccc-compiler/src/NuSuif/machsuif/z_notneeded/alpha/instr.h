/* file "alpha/instr.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef ALPHA_INSTR_H
#define ALPHA_INSTR_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "alpha/instr.h"
#endif

bool is_ldc_alpha(Instr *mi);
bool is_move_alpha(Instr *mi);
bool is_cmove_alpha(Instr *mi);
bool is_line_alpha(Instr *mi);
bool is_ubr_alpha(Instr *mi);
bool is_cbr_alpha(Instr *mi);
bool is_call_alpha(Instr *mi);
bool is_return_alpha(Instr *mi);
bool is_binary_exp_alpha(Instr *mi);
bool is_unary_exp_alpha(Instr *mi);
bool is_commutative_alpha(Instr *mi);
bool reads_memory_alpha(Instr *);
bool writes_memory_alpha(Instr *);

#endif /* ALPHA_INSTR_H */
