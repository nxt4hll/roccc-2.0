/* file "x86/fpstack.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "x86/fpstack.h"
#endif

#include <machine/machine.h>

#include <x86/fpstack.h>
#include <x86/opcodes.h>
#include <x86/reg_info.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

using namespace x86;

/* constructor */
FpStack::FpStack(int sz)
{
    int i;
    st_sz = sz;
    stack = new int[sz];
    for (i = 0; i < sz; i++)
	stack[i] = STLOC_UNUSED;
    st_p = sz;		// stack grows downward, points to current top
}

/* Marks the next stack location as occupied.  Stores bk_value into stack
 * location for bookkeeping purposes. */
void
FpStack::bk_push(int bk_value)
{
    st_p--;
    claim(st_p >= 0, "FpStack::bk_push() -- stack overflow");
    stack[st_p] = bk_value;
}

/* Records the actions caused by a FXCH instruction.  It expects
 * the ST-relative index to exchange with as a parameter.  It
 * does minimal error checking. */
void
FpStack::bk_exchange(int st_i)
{
    int t = stack[st_p];
    claim(st_i > 0);
    int i = st_p + st_i;	// index into stack array
    claim(i < st_sz);
    stack[st_p] = stack[i];
    stack[i] = t;
}


/* Starting from top-of-stack, this routine finds the first stack location
 * that contains a value matching the formal parameter.  The top-of-stack
 * relative position of this match is returned.  If value not found, -1
 * is returned. */
int
FpStack::find(int v)
{
    int i;
    for (i = st_p; i < st_sz; i++) {
	if (stack[i] == v)
	    return (i - st_p);
    }
    return -1;	// v not found in stack
}

/*
 * Replace the contents of top-of-stack.  This allows us to rename a
 * FpStack location due to a VR-to-VR copy operation.
 */
void
FpStack::rename_st(int v)
{
    claim(st_p < st_sz, "FpStack::rename_st() -- empty stack");
    stack[st_p] = v;
}

/*
 * At a call instruction, if the destination exists and has floating type,
 * add a bookkeeping entry to represent the returned value in ST.
 */
void
FpStack::reflect_call(Instr *call)
{
    if (dsts_size(call) > 0 && is_floating_point(get_type(get_dst(call))))
      bk_push(STLOC_CONTAINS_CALL_RESULT);
}

/* Create an instruction that pushes the operand s onto the
 * x86 floating-point stack.  If the operand is a virtual register,
 * it should already exist as a location in the FpStack.  If s is
 * an effective address calculation, this routine pushes the value
 * pointed to by the effective address calculation.  If you
 * attempt to push a NULL operand, this routine returns a NULL
 * Instr pointer. */
Instr*
FpStack::push(Opnd s)
{
    Instr *mi;

    if (is_var(s)) {
	// push symbol onto the top-of-stack
	bk_push(STLOC_CONTAINS_SYMBOL);
	mi = new_instr_alm(opnd_reg(FP0, get_type(s)), 
			   FLD, opnd_addr_sym(get_var(s)));
	set_dst(mi, 1, opnd_reg(FPFLAGS, type_u16));

    } else if (is_reg(s)) {
	// Push value in some other stack location onto the top-of-stack.
	// Realize that this makes a copy!  And hence we don't usually 
	// do this--see translate.cpp/translate_fp().
	int vr = get_reg(s);
	int st_i = find(vr);	// get index before st_p decremented
	claim(st_i >= 0, "FpStack::push() -- vr%d not on FpStack", vr);
	bk_push(vr);
	mi = new_instr_alm(opnd_reg(FP0, get_type(s)), FLD,
			   opnd_reg(FP0 + st_i, get_type(s)));
	set_dst(mi, 1, opnd_reg(FPFLAGS, type_u16));

    } else if (is_addr(s)) {
	// push value pointed to by EA onto the top-of-stack
	bk_push(STLOC_CONTAINS_SYMBOL);
	mi = new_instr_alm(opnd_reg(FP0, get_type(s)), FLD, s);
	set_dst(mi, 1, opnd_reg(FPFLAGS, type_u16));

    } else if (is_null(s)) {
	mi = NULL;

    } else {
	claim(false, "FpStack::push() -- unexpected operand kind %d",
	      get_kind(s));
    }

    return mi;
}

/* Create an instruction that pushes a FP constant onto ST using
 * one of the x86 FLD<const> instructions. */
Instr*
FpStack::push_const(int op)
{
    Instr *mi;
    bk_push(STLOC_CONTAINS_SYMBOL);
    mi = new_instr_alm(opnd_reg(FP0, type_f32), op);
    set_dst(mi, 1, opnd_reg(FPFLAGS, type_u16));
    return mi;
}

/* Create an instruction that pushes a signed integer onto ST using the
 * x86 FILD instruction.  Converts the integer into an extended real. */
Instr*
FpStack::push_int(Opnd s)
{
    Instr *mi;
    claim(is_var(s));		// int must come from memory
    bk_push(STLOC_CONTAINS_SYMBOL);
    mi = new_instr_alm(opnd_reg(FP0, type_f32), FILD,
		       opnd_addr_sym(get_var(s)));
    set_dst(mi, 1, opnd_reg(FPFLAGS, type_u16));
    return mi;
}

/* Create an instruction that exchanges the SUIF VR operand s with
 * the ST value.  Both values are initially on the x86 FP stack. 
 * No instruction is emitted if the operand is already at the top
 * of stack. */
Instr*
FpStack::exchange(Opnd s)
{
    Instr *mi;
    claim(is_reg(s), "FpStack::exchange() -- unexpected operand kind %d",
	  get_kind(s));
    int vr = get_reg(s);
    int st_i = find(vr);
    claim(st_i >= 0, "FpStack::exchange() -- vr%d not on FpStack", vr);

    // do nothing if symbol is already at ST
    if (st_i == 0) 
	return NULL;

    // otherwise create and return an exchange operation
    bk_exchange(st_i);
    mi = new_instr_alm(opnd_reg(FP0, get_type(s)),
		       FXCH, 
		       opnd_reg(FP0 + st_i, get_type(s)),
		       opnd_reg(FP0, type_f32));
    set_dst(mi, 1, opnd_reg(FP0 + st_i, get_type(s)));
    set_dst(mi, 2, opnd_reg(FPFLAGS, type_u16));
    return mi;
}


/* Pop off ST and send the contents to memory.  Assumes that the
 * parameter operand is a symbol or an effective address (where to
 * place the contents of ST).  If the parameter is ST(0), this routine
 * returns an instruction that effectively pops the fp stack.  If the
 * parameter is a NULL operand, this routine pops the contents of ST
 * into the bit-bucket and returns NULL. */
Instr*
FpStack::pop(Opnd d)
{
    Instr *mi;

    claim(st_p < st_sz, "FpStack::pop() -- popping an empty stack");

    // place ST in memory
    if (is_var(d)) {
	mi = new_instr_alm(opnd_addr_sym(get_var(d)), FSTP,
			   opnd_reg(FP0, get_type(d)));
	set_dst(mi, 1, opnd_reg(FPFLAGS, type_u16));

    } else if (is_addr(d)) {
	mi = new_instr_alm(d, FSTP, 
			   opnd_reg(FP0, get_type(d)));
	set_dst(mi, 1, opnd_reg(FPFLAGS, type_u16));

    } else if (is_reg(d) && (get_reg(d) == FP0)) {
	mi = new_instr_alm(d, FSTP, d);	// mov fp0 <- fp0 and pop fp0
	set_dst(mi, 1, opnd_reg(FPFLAGS, type_u16));

    } else if (is_null(d)) {
	mi = NULL;

    } else {
	claim(false);
    }

    // do bookkeeping
    stack[st_p] = STLOC_UNUSED;
    st_p++;

    return mi;
}


/* Pop off ST, convert the contents to an integer, and write the result
 * to the memory location indicated by the parameter.
 */
Instr*
FpStack::pop_int(Opnd d)
{
    Instr *mi;

    claim(is_addr_sym(d));
    claim(st_p < st_sz, "FpStack::pop() -- popping an empty stack");

    // place ST in memory as an integer
    mi = new_instr_alm(d, FISTP, opnd_reg(FP0, get_deref_type(d)));
    set_dst(mi, 1, opnd_reg(FPFLAGS, type_u16));
    stack[st_p] = STLOC_UNUSED;
    st_p++;

    return mi;
}
