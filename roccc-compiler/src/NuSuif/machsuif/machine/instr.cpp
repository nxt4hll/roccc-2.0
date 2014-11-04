/* file "machine/instr.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "machine/instr.h"
#endif

#include <machine/substrate.h>
#include <machine/machine_ir.h>
#include <machine/machine_ir_factory.h>
#include <machine/init.h>
#include <machine/problems.h>
#include <machine/printer.h>
#include <machine/contexts.h>
#include <machine/util.h>
#include <machine/opcodes.h>
#include <machine/opnd.h>
#include <machine/instr.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

// adding predicate, causes the previous to fail
//#define CREATE_INSTR_ALM_INIT create_instr_alm(the_suif_env,opcode,0,0,0,0,false,false,-1,-1,"","") added predicate
//#define CREATE_INSTR_CTI_INIT create_instr_cti(the_suif_env,opcode,0,0,0,0,false,false,-1,-1,"","",target)

#define CREATE_INSTR_ALM_INIT create_instr_alm(the_suif_env,opcode,0,0,0,0,false,false,-1,-1,"","", NULL, false) 
#define CREATE_INSTR_CTI_INIT create_instr_cti(the_suif_env,opcode,0,0,0,0,false,false,-1,-1,"","", NULL, false, target)
#define CREATE_INSTR_LABEL_INIT create_instr_label(the_suif_env,opcode_label,0,0,0,0,false,false,-1,-1,"","",label)
#define CREATE_INSTR_DOT_INIT create_instr_dot(the_suif_env,opcode,0,0,0,0,false,false,-1,-1,"","")

/* Retrieve predicate info.  Added to support predicates for 
 * rewrite of lowcirrfpass 
 */
Opnd
get_predicate(Instr *instr)
{
    claim( has_predicate(instr), "Cannot retrieve instruction that does not have predicate");
    if (has_predicate(instr))
        return ((InstrAlm*)instr)->get_predicate();

    return NULL;
}

bool
has_predicate(Instr *instr)
{
    if (is_kind_of<InstrAlm>(instr))
        return ((InstrAlm*)instr)->get_hasPredicate();
    return false;
}

//####################

static void append_src(InstrAlm*, Opnd); 
static void append_src(InstrDot*, Opnd); 
static void append_dst(InstrAlm*, Opnd); 

InstrList*
new_instr_list()
{
    return create_instr_list(the_suif_env);
}

InstrList*
to_instr_list(AnyBody *body)
{
    return body->to_instr_list();
}

InstrList*
instr_list_to_instr_list(InstrList *list)
{
    InstrList *result = new_instr_list();
    
    for (InstrHandle h = start(list); h != end(list); )
	append(result, remove(list, h++));
    return result;
}

int
instrs_size(InstrList *list)
{
    return list->get_instr_count();
}

InstrHandle
instrs_start(InstrList *list)
{
    return list->instrs().begin();
}

InstrHandle
instrs_last(InstrList *list)
{
    return get_last_handle(list->instrs());
}

InstrHandle
instrs_end(InstrList *list)
{
    return list->instrs().end();
}

void
prepend(InstrList *list, Instr *instr)
{
    list->insert_instr(0, instr);
}

void
append(InstrList *list, Instr *instr)
{
    list->append_instr(instr);
}

void
replace(InstrList *instr_list, InstrHandle handle, Instr *instr)
{
    list<Instr*>& instrs = instr_list->instrs();
    claim(handle != instrs.end(), "Invalid list handle");
    *handle = instr;
    if (instr)
	instr->set_parent(instr_list);
}

void
insert_before(InstrList *list, InstrHandle handle, Instr *instr)
{
    list->instrs().insert(handle, instr);
    if (instr)
	instr->set_parent(list);
}

void
insert_after(InstrList *list, InstrHandle handle, Instr *instr)
{
    list->instrs().insert(after(handle), instr);
    if (instr)
	instr->set_parent(list);
}

Instr*
remove(InstrList *instr_list, InstrHandle handle)
{
    list<Instr*>& instrs = instr_list->instrs();
    claim(handle != instrs.end(), "Invalid list handle");
    Instr *instr = *handle;
    instrs.erase(handle);
    instr->set_parent(NULL);
    return instr;
}

void
fprint(FILE *fp, InstrList *instr_list)
{
    Printer *printer = target_printer();
    printer->set_file_ptr(fp);

    InstrHandle h = start(instr_list);
    for ( ; h != end(instr_list); ++h) {
	fprintf(stderr, "[%lx]  ", (unsigned long)(*h));
	printer->print_instr(*h);
    }
}


// Instr


Instr*
new_instr_alm(int opcode)
{
    return CREATE_INSTR_ALM_INIT; 
}

Instr*
new_instr_alm(int opcode, Opnd src)
{
    InstrAlm *instr = CREATE_INSTR_ALM_INIT; 
    append_src(instr, src);
    return instr;
}

Instr*
new_instr_alm(int opcode, Opnd src1, Opnd src2)
{
    InstrAlm *instr = CREATE_INSTR_ALM_INIT;
    append_src(instr, src1);
    append_src(instr, src2);
    return instr;
}

Instr*
new_instr_alm(Opnd dst, int opcode)
{
    InstrAlm *instr = CREATE_INSTR_ALM_INIT;
    append_dst(instr, dst);
    return instr;
}

Instr*
new_instr_alm(Opnd dst, int opcode, Opnd src)
{
    InstrAlm *instr = CREATE_INSTR_ALM_INIT;
    append_src(instr, src);
    append_dst(instr, dst);
    return instr;
}

Instr*
new_instr_alm(Opnd dst, int opcode, Opnd src1, Opnd src2)
{
    InstrAlm *instr = CREATE_INSTR_ALM_INIT;
    append_src(instr, src1);
    append_src(instr, src2);
    append_dst(instr, dst);
    return instr;
}


Instr*
new_instr_cti(int opcode, Sym *target)
{
    return CREATE_INSTR_CTI_INIT;
}

Instr*
new_instr_cti(int opcode, Sym *target, Opnd src)
{
    InstrCti *instr = CREATE_INSTR_CTI_INIT;
    append_src(instr, src);
    return instr;
}

Instr*
new_instr_cti(int opcode, Sym *target, Opnd src1, Opnd src2)
{
    InstrCti *instr = CREATE_INSTR_CTI_INIT;
    append_src(instr, src1);
    append_src(instr, src2);
    return instr;
}

Instr*
new_instr_cti(Opnd dst, int opcode, Sym *target)
{
    InstrCti *instr = CREATE_INSTR_CTI_INIT;
    append_dst(instr, dst);
    return instr;
}

Instr*
new_instr_cti(Opnd dst, int opcode, Sym *target, Opnd src)
{
    InstrCti *instr = CREATE_INSTR_CTI_INIT;
    append_src(instr, src);
    append_dst(instr, dst);
    return instr;
}

Instr*
new_instr_cti(Opnd dst, int opcode, Sym *target, Opnd src1, Opnd src2)
{
    InstrCti *instr = CREATE_INSTR_CTI_INIT;
    append_src(instr, src1);
    append_src(instr, src2);
    append_dst(instr, dst);
    return instr;
}


Instr*
new_instr_label(LabelSym *label)
{
    return CREATE_INSTR_LABEL_INIT;
}


Instr*
new_instr_dot(int opcode)
{
    return CREATE_INSTR_DOT_INIT;
}

Instr*
new_instr_dot(int opcode, Opnd src)
{
    InstrDot *instr = CREATE_INSTR_DOT_INIT;
    append_src(instr, src);
    return instr;
}

Instr*
new_instr_dot(int opcode, Opnd src1, Opnd src2)
{
    InstrDot *instr = CREATE_INSTR_DOT_INIT;
    append_src(instr, src1);
    append_src(instr, src2);
    return instr;
}

int
get_opcode(Instr *instr)
{
    return instr->get_opcode();
}

void
set_opcode(Instr *instr, int opcode)
{
    instr->set_opcode(opcode);
}

/*
 * Fetch a source operand, extending the srcs vector if necessary.
 * This template function is invoked for either an InstrAlm or
 * InstrDot instruction.
 */
template <class InstrKind>
static Opnd
get_src(InstrKind *instr, int pos)
{
    for (int d = pos - instr->get_src_count(); d >= 0; --d)
	instr->append_src(opnd_null());
    return instr->get_src(pos);
}

Opnd
get_src(Instr *instr, int pos)
{
    if (is_kind_of<InstrAlm>(instr))
	return get_src((InstrAlm*)instr, pos);
    else {
	claim(is_kind_of<InstrDot>(instr), "Instr has no srcs");
	return get_src((InstrDot*)instr, pos);
    }
}

Opnd
get_src(Instr *instr, OpndHandle handle)
{
    claim(handle != srcs_end(instr));
    return *handle;
}

/*
 * Append or replace a source operand.  This template function is
 * invoked for either an InstrAlm or InstrDot instruction.
 */
template <class InstrKind>
static void
set_src(InstrKind *instr, int pos, Opnd src)
{
    int d = pos - instr->get_src_count();

    if (d >= 0) {
	for ( ; d > 0; d--)
	    instr->append_src(opnd_null());
	instr->append_src(src);
    } else {
	instr->replace_src(pos, src);
    }
}

void
set_src(Instr *instr, int pos, Opnd src)
{
    if (is_kind_of<InstrAlm>(instr))
	set_src((InstrAlm*)instr, pos, src);
    else {
	claim(is_kind_of<InstrDot>(instr), "Instr has no srcs");
	set_src((InstrDot*)instr, pos, src);
    }
}

void
set_src(Instr *instr, OpndHandle handle, Opnd src)
{
    if (handle != srcs_end(instr)) {
	*handle = src;
    }
    else if (is_kind_of<InstrAlm>(instr)) {
	((InstrAlm*)instr)->append_src(src);
    }
    else {
	claim(is_kind_of<InstrDot>(instr), "Instr has no srcs");
	((InstrDot*)instr)->append_src(src);
    }
}

void
prepend_src(Instr *instr, Opnd src)
{
    if (is_kind_of<InstrAlm>(instr))
	((InstrAlm*)instr)->insert_src(0, src);
    else {
	claim(is_kind_of<InstrDot>(instr), "Instr has no srcs");
	((InstrDot*)instr)->insert_src(0, src);
    }
}

void
append_src(Instr *instr, Opnd src)
{
    if (is_kind_of<InstrAlm>(instr))
	((InstrAlm*)instr)->append_src(src);
    else {
	claim(is_kind_of<InstrDot>(instr), "Instr has no srcs");
	((InstrDot*)instr)->append_src(src);
    }
}

void
insert_before_src(Instr *instr, int pos, Opnd src)
{
    claim(pos <= srcs_size(instr), "Operand position out of range");

    if (is_kind_of<InstrAlm>(instr))
	((InstrAlm*)instr)->insert_src(pos, src);
    else {
	claim(is_kind_of<InstrDot>(instr), "Instr has no srcs");
	((InstrDot*)instr)->insert_src(pos, src);
    }
}

void
insert_before_src(Instr *instr, OpndHandle handle, Opnd src)
{
    if (is_kind_of<InstrAlm>(instr))
	((InstrAlm*)instr)->srcs().insert(handle, src);
    else {
	claim(is_kind_of<InstrDot>(instr), "Instr has no srcs");
	((InstrDot*)instr)->srcs().insert(handle, src);
    }
}

void
insert_after_src(Instr *instr, int pos, Opnd src)
{
    int size = srcs_size(instr);
    claim(pos < size, "Operand position out of range");

    if (is_kind_of<InstrAlm>(instr))
	((InstrAlm*)instr)->insert_src(pos + 1, src);
    else {
	claim(is_kind_of<InstrDot>(instr), "Instr has no srcs");
	((InstrDot*)instr)->insert_src(pos + 1, src);
    }
}

void
insert_after_src(Instr *instr, OpndHandle handle, Opnd src)
{
    claim(handle != srcs_end(instr),
	  "Can't insert after non-existent operand position");
    if (is_kind_of<InstrAlm>(instr))
	((InstrAlm*)instr)->srcs().insert(++handle, src);
    else {
	claim(is_kind_of<InstrDot>(instr), "Instr has no srcs");
	((InstrDot*)instr)->srcs().insert(++handle, src);
    }
}

Opnd
remove_src(Instr *instr, int pos)
{
    claim(pos < srcs_size(instr), "Operand position out of range");

    Opnd src;
    if (is_kind_of<InstrAlm>(instr)) {
	src = ((InstrAlm*)instr)->get_src(pos);
	((InstrAlm*)instr)->remove_src(pos);
    }
    else {
	claim(is_kind_of<InstrDot>(instr), "Instr has no srcs");
	src = ((InstrDot*)instr)->get_src(pos);
	((InstrDot*)instr)->remove_src(pos);
    }
    return src;
}

Opnd
remove_src(Instr *instr, OpndHandle handle)
{
    claim(handle != srcs_end(instr), "Operand position out of range");
    Opnd src = *handle;

    if (is_kind_of<InstrAlm>(instr))
	((InstrAlm*)instr)->srcs().erase(handle);
    else {
	claim(is_kind_of<InstrDot>(instr), "Instr has no srcs");
	((InstrDot*)instr)->srcs().erase(handle);
    }
    return src;
}

int
srcs_size(Instr *instr)
{
    if (is_kind_of<InstrAlm>(instr))
	return ((InstrAlm*)instr)->srcs().size();
    else if (is_kind_of<InstrDot>(instr))
	return ((InstrDot*)instr)->srcs().size();
    else
	return 0;
}

OpndHandle
srcs_start(Instr *instr)
{
    if (is_kind_of<InstrAlm>(instr))
	return ((InstrAlm*)instr)->srcs().begin();
    else {
	claim(is_kind_of<InstrDot>(instr), "Instr has no srcs");
	return ((InstrDot*)instr)->srcs().begin();
    }
}

OpndHandle
srcs_last(Instr *instr)
{
    OpndHandle h = srcs_end(instr);
    return --h;
}

OpndHandle
srcs_end(Instr *instr)
{
    if (is_kind_of<InstrAlm>(instr))
	return ((InstrAlm*)instr)->srcs().end();
    else {
	claim(is_kind_of<InstrDot>(instr), "Instr has no srcs");
	return ((InstrDot*)instr)->srcs().end();
    }
}

Opnd
get_dst(Instr *instr)
{
    return get_dst(instr, 0);
}

Opnd
get_dst(Instr *instr, int pos)
{
    claim(is_kind_of<InstrAlm>(instr), "Instr has no dsts");
    return ((InstrAlm*)instr)->get_dst(pos);
}

Opnd
get_dst(Instr *instr, OpndHandle handle)
{
    claim(handle != dsts_end(instr));
    return *handle;
}

void
set_dst(Instr *instr, Opnd dst)
{
    set_dst(instr, 0, dst);
}

void
set_dst(Instr *instr, int pos, Opnd dst)
{
    claim(is_kind_of<InstrAlm>(instr), "Instr has no dsts");
    InstrAlm *alm = (InstrAlm*)instr;

    int d = pos - alm->get_dst_count();

    if (d >= 0) {
	for ( ; d > 0; d--)
	    alm->append_dst(opnd_null());
	alm->append_dst(dst);
    } else {
	alm->replace_dst(pos, dst);
    }
}

void
set_dst(Instr *instr, OpndHandle handle, Opnd dst)
{
    claim(is_kind_of<InstrAlm>(instr), "Instr has no dsts");
    InstrAlm *alm = (InstrAlm*)instr;

    if (handle == alm->dsts().end()) {
	alm->append_dst(dst);
    }
    else {
	*handle = dst;
    }
}

void
prepend_dst(Instr *instr, Opnd dst)
{
    claim(is_kind_of<InstrAlm>(instr), "Instr has no dsts");
    ((InstrAlm*)instr)->insert_dst(0, dst);
}

void
append_dst(Instr *instr, Opnd dst)
{
    claim(is_kind_of<InstrAlm>(instr), "Instr has no dsts");
    ((InstrAlm*)instr)->append_dst(dst);
}

void
insert_before_dst(Instr *instr, int pos, Opnd dst)
{
    claim(is_kind_of<InstrAlm>(instr), "Instr has no dsts");
    claim(pos <= dsts_size(instr), "Operand position out of range");

    ((InstrAlm*)instr)->insert_src(pos, dst);
}

void
insert_before_dst(Instr *instr, OpndHandle handle, Opnd dst)
{
    claim(is_kind_of<InstrAlm>(instr), "Instr has no dsts");
    ((InstrAlm*)instr)->dsts().insert(handle, dst);
}

void
insert_after_dst(Instr *instr, int pos, Opnd dst)
{
    claim(is_kind_of<InstrAlm>(instr), "Instr has no dsts");

    claim(pos < dsts_size(instr),
	  "Can't insert after non-existent operand position");
    ((InstrAlm*)instr)->insert_dst(pos + 1, dst);
}

void
insert_after_dst(Instr *instr, OpndHandle handle, Opnd dst)
{
    claim(is_kind_of<InstrAlm>(instr), "Instr has no dsts");
    claim(handle != dsts_end(instr),
	  "Can't insert after non-existent operand position");
    ((InstrAlm*)instr)->dsts().insert(++handle, dst);
}

Opnd
remove_dst(Instr *instr, int pos)
{
    claim(is_kind_of<InstrAlm>(instr), "Instr has no dsts");
    claim(pos < dsts_size(instr), "Operand position out of range");

    Opnd dst = ((InstrAlm*)instr)->get_dst(pos);
    ((InstrAlm*)instr)->remove_dst(pos);
    return dst;
}

Opnd
remove_dst(Instr *instr, OpndHandle handle)
{
    claim(handle != dsts_end(instr), "Operand position out of range");
    claim(is_kind_of<InstrAlm>(instr), "Instr has no dsts");

    Opnd dst = *handle;
    ((InstrAlm*)instr)->dsts().erase(handle);
    return dst;
}

int
dsts_size(Instr *instr)
{
    if (is_kind_of<InstrAlm>(instr))
	return ((InstrAlm*)instr)->dsts().size();
    return 0;
}

/*
 * Use the following in lieu of an empty dsts vector for the kinds of
 * instruction that have no dst vector.
 */

suif_vector<IrOpnd*> empty_opnd_vector;

OpndHandle
dsts_start(Instr *instr)
{
    if (!is_kind_of<InstrAlm>(instr))
	return empty_opnd_vector.end();
    return ((InstrAlm*)instr)->dsts().begin();
}

OpndHandle
dsts_last(Instr *instr)
{
    OpndHandle h = dsts_end(instr);
    return --h;
}

OpndHandle
dsts_end(Instr *instr)
{
    if (!is_kind_of<InstrAlm>(instr))
	return empty_opnd_vector.end();
    return ((InstrAlm*)instr)->dsts().end();
}

Sym*
get_target(Instr *instr)
{
    return to<InstrCti>(instr)->get_target();
}

void
set_target(Instr *instr, Sym *target)
{
    to<InstrCti>(instr)->set_target(target);
}

LabelSym*
get_label(Instr *instr)
{
    return to<InstrLabel>(instr)->get_label();
}

void
set_label(Instr *instr, LabelSym *label)
{
    to<InstrLabel>(instr)->set_label(label);
}



// Instr predicates

bool
is_null(Instr *instr)
{
    return get_opcode(instr) == opcode_null;
}

bool
is_label(Instr *instr)
{
    return get_opcode(instr) == opcode_label;
}

// FIXME: following shouldn't cover null instr's
bool
is_dot(Instr *instr)
{
    return is_kind_of<InstrDot>(instr);
}

bool
is_mbr(Instr *instr)
{
    return has_note(instr, k_instr_mbr_tgts);
}

bool
is_indirect(Instr *instr)
{
    return (get_target(instr) == NULL);
}

bool
is_cti(Instr *instr)
{
    return is_kind_of<InstrCti>(instr);
}

bool
reads_memory(Instr *instr)
{
    return dynamic_cast<MachineContext*>(the_context)->reads_memory(instr);
}

bool
writes_memory(Instr *instr)
{
    return dynamic_cast<MachineContext*>(the_context)->writes_memory(instr);
}

bool
is_builtin(Instr *instr)
{
    return dynamic_cast<MachineContext*>(the_context)->is_builtin(instr);
}

bool
is_ldc(Instr *instr)
{
    return dynamic_cast<MachineContext*>(the_context)->is_ldc(instr);
}

bool
is_move(Instr *instr)
{
    return dynamic_cast<MachineContext*>(the_context)->is_move(instr);
}

bool
is_cmove(Instr *instr)
{
    return dynamic_cast<MachineContext*>(the_context)->is_cmove(instr);
}

bool
is_predicated(Instr *instr)
{
    return dynamic_cast<MachineContext*>(the_context)->is_predicated(instr);
}

bool
is_line(Instr *instr)
{
    return dynamic_cast<MachineContext*>(the_context)->is_line(instr);
}

bool
is_ubr(Instr *instr)
{
    return dynamic_cast<MachineContext*>(the_context)->is_ubr(instr);
}

bool
is_cbr(Instr *instr)
{
    return dynamic_cast<MachineContext*>(the_context)->is_cbr(instr);
}

bool
is_call(Instr *instr)
{
    return dynamic_cast<MachineContext*>(the_context)->is_call(instr);
}

bool
is_return(Instr *instr)
{
    return dynamic_cast<MachineContext*>(the_context)->is_return(instr);
}

bool
is_binary_exp(Instr *instr)
{
    return dynamic_cast<MachineContext*>(the_context)->is_binary_exp(instr);
}

bool
is_unary_exp(Instr *instr)
{
    return dynamic_cast<MachineContext*>(the_context)->is_unary_exp(instr);
}

bool
is_commutative(Instr *instr)
{
    return dynamic_cast<MachineContext*>(the_context)->is_commutative(instr);
}

bool
is_two_opnd(Instr *instr)
{
    return dynamic_cast<MachineContext*>(the_context)->is_two_opnd(instr);
}

bool
is_param_init(Instr *instr)
{
    return has_note(instr, k_param_init);
}

void
fprint(FILE *fp, Instr *mi)
{
    Printer *printer = target_printer();
    claim(printer != NULL);

    printer->set_file_ptr(fp);
    printer->print_instr(mi);
}



//  Helpers

static void
append_src(InstrAlm *instr, Opnd src)
{
    instr->append_src(src);
}

static void
append_src(InstrDot *instr, Opnd src)
{
    instr->append_src(src);
}

static void
append_dst(InstrAlm *instr, Opnd src)
{
    instr->append_dst(src);
}
