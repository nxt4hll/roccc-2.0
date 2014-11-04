/* file "ia64/instr.h" */

/*
    Copyright (c) 2001 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef IA64_INSTR_H
#define IA64_INSTR_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "ia64/instr.h"
#endif

bool is_ldc_ia64(Instr *mi);
bool is_move_ia64(Instr *mi);
bool is_cmove_ia64(Instr *mi);
bool is_predicated_ia64(Instr *mi);
bool is_line_ia64(Instr *mi);
bool is_ubr_ia64(Instr *mi);
bool is_cbr_ia64(Instr *mi);
bool is_call_ia64(Instr *mi);
bool is_return_ia64(Instr *mi);
bool is_triary_exp_ia64(Instr *mi);
bool is_binary_exp_ia64(Instr *mi);
bool is_unary_exp_ia64(Instr *mi);
bool is_commutative_ia64(Instr *mi);
bool is_two_opnd_ia64(Instr *mi);
bool reads_memory_ia64(Instr *);
bool writes_memory_ia64(Instr *);

bool has_qpred(Instr*);
void set_qpred(Instr*, Opnd);
Opnd get_qpred(Instr*);

#endif /* IA64_INSTR_H */
