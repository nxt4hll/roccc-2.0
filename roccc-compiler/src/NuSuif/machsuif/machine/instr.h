/* file "machine/instr.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef MACHINE_INSTR_H
#define MACHINE_INSTR_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "machine/instr.h"
#endif

#include <machine/substrate.h>
#include <machine/opnd.h>
#include <machine/machine_ir.h>

Opnd        get_predicate(Instr*);
bool        has_predicate(Instr*);

Opnd	    get_src(Instr*, int pos);
Opnd	    get_src(Instr*, OpndHandle);
void	    set_src(Instr*, int pos, Opnd src);
void	    set_src(Instr*, OpndHandle, Opnd src);
void	    prepend_src(Instr*, Opnd src);
void	    append_src(Instr*, Opnd src);
void	    insert_before_src(Instr*, int pos, Opnd src);
void	    insert_before_src(Instr*, OpndHandle, Opnd src);
void	    insert_after_src(Instr*, int pos, Opnd src);
void	    insert_after_src(Instr*, OpndHandle, Opnd src);
Opnd	    remove_src(Instr*, int pos);
Opnd	    remove_src(Instr*, OpndHandle);
int	    srcs_size(Instr* instr);
OpndHandle  srcs_start(Instr*);
OpndHandle  srcs_last(Instr*);
OpndHandle  srcs_end(Instr*);

Opnd	    get_dst(Instr*);
Opnd	    get_dst(Instr*, int pos);
Opnd	    get_dst(Instr*, OpndHandle);
void	    set_dst(Instr*, Opnd dst);
void	    set_dst(Instr*, int pos, Opnd dst);
void	    set_dst(Instr*, OpndHandle, Opnd dst);
void	    prepend_dst(Instr*, Opnd dst);
void	    append_dst(Instr*, Opnd dst);
void	    insert_before_dst(Instr*, int pos, Opnd dst);
void	    insert_before_dst(Instr*, OpndHandle, Opnd dst);
void	    insert_after_dst(Instr*, int pos, Opnd dst);
void	    insert_after_dst(Instr*, OpndHandle, Opnd dst);
Opnd	    remove_dst(Instr*, int pos);
Opnd	    remove_dst(Instr*, OpndHandle);
int	    dsts_size(Instr*);
OpndHandle  dsts_start(Instr*);
OpndHandle  dsts_last(Instr*);
OpndHandle  dsts_end(Instr*);

typedef list<Instr*>::iterator InstrHandle;

Instr* new_instr_alm(int opcode);
Instr* new_instr_alm(int opcode, Opnd src);
Instr* new_instr_alm(int opcode, Opnd src1, Opnd src2);
Instr* new_instr_alm(Opnd dst, int opcode);
Instr* new_instr_alm(Opnd dst, int opcode, Opnd src);
Instr* new_instr_alm(Opnd dst, int opcode, Opnd src1, Opnd src2);

Instr* new_instr_cti(int opcode, Sym *target);
Instr* new_instr_cti(int opcode, Sym *target, Opnd src);
Instr* new_instr_cti(int opcode, Sym *target, Opnd src1, Opnd src2);
Instr* new_instr_cti(Opnd dst, int opcode, Sym *target);
Instr* new_instr_cti(Opnd dst, int opcode, Sym *target, Opnd src);
Instr* new_instr_cti(Opnd dst, int opcode, Sym *target,
		     Opnd src1, Opnd src2);

Instr* new_instr_label(LabelSym *label);

Instr* new_instr_dot(int opcode);
Instr* new_instr_dot(int opcode, Opnd src);
Instr* new_instr_dot(int opcode, Opnd src1, Opnd src2);

int get_opcode(Instr *instr);
void set_opcode(Instr *instr, int opcode);

Sym* get_target(Instr *instr);
void set_target(Instr *instr, Sym *target);

LabelSym* get_label(Instr *instr);
void set_label(Instr *instr, LabelSym *label);

bool is_null(Instr*);
bool is_label(Instr*);
bool is_dot(Instr*);
bool is_mbr(Instr*);
bool is_indirect(Instr*);
bool is_cti(Instr*);
bool reads_memory(Instr*);
bool writes_memory(Instr*);
bool is_builtin(Instr*);

bool is_ldc(Instr*);
bool is_move(Instr*);
bool is_cmove(Instr*);
bool is_predicated(Instr*);
bool is_line(Instr*);
bool is_ubr(Instr*);
bool is_cbr(Instr*);
bool is_call(Instr*);
bool is_return(Instr*);
bool is_binary_exp(Instr*);
bool is_unary_exp(Instr*);
bool is_commutative(Instr*);
bool is_two_opnd(Instr*);
bool is_param_init(Instr*);

void fprint(FILE*, Instr*);

InstrList* new_instr_list();
InstrList* to_instr_list(AnyBody*);
int instrs_size(InstrList *list);
InstrHandle instrs_start(InstrList *list);
InstrHandle instrs_last(InstrList *list);
InstrHandle instrs_end(InstrList *list);
void prepend(InstrList *list, Instr *instr);
void append(InstrList *list, Instr *instr);
void replace(InstrList *list, InstrHandle handle, Instr *instr);
void insert_before(InstrList *list, InstrHandle handle, Instr *instr);
void insert_after(InstrList *list, InstrHandle handle, Instr *instr);
Instr* remove(InstrList *list, InstrHandle handle);

inline int
size(InstrList *instr_list)
{
    return instrs_size(instr_list);
}

inline InstrHandle
start(InstrList *instr_list)
{
    return instrs_start(instr_list);
}

inline InstrHandle
last(InstrList *instr_list)
{
    return instrs_last(instr_list);
}

inline InstrHandle
end(InstrList *instr_list)
{
    return instrs_end(instr_list);
}

void fprint(FILE*, InstrList*);

#endif /* MACHINE_INSTR_H */
