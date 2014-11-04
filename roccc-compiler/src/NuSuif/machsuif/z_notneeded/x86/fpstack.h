/* file "x86/fpstack.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef X86_FPSTACK_H
#define X86_FPSTACK_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "x86/fpstack.h"
#endif

/* This class manages the floating-point stack in the x86 architecture. */

/* Bookkeeping values for our FP stack.  Implicitly, STLOC_CONTAINS_VREG
 * if its contents >= 0.  The contents are the virtual register number
 * of the value at that stack location.  Otherwise, ... */
#define STLOC_UNUSED -1
#define STLOC_CONTAINS_SYMBOL -2
#define STLOC_CONTAINS_CALL_RESULT -3

/* There are no x86 FP registers.  FP operations are handled by an
 * 8-entry, FP stack.  Stack indices in the REG structure are relative
 * to the current top-of-stack!  They do not represent the actual index
 * into the actual hardware structure.
 * FP0 is always the top of the stack! */

class FpStack {
  private:
    int st_p;		// top-of-stack pointer
    int st_sz;		// size of stack
    int *stack;		// stack array

  protected:
    // internal bookkeeping routines
    void bk_push(int bk_value);
    void bk_exchange(int st_i);

  public:
    FpStack(int sz);
    ~FpStack() { delete[] stack; }
    void clear() { st_p = st_sz; }
    int count() { return st_sz - st_p; }

    int find(int v);			// return ST-relative location of v
    void rename_st(int v);		// replace ST contents with v
    void reflect_call(Instr *call);	// reflect stack state change at call

    Instr* push(Opnd s);
    Instr* push_const(int);
    Instr* push_int(Opnd s);
    Instr* exchange(Opnd s);
    Instr* pop(Opnd d = opnd_null());	// pop with store to dst d
    Instr* pop_int(Opnd d);
};

#endif /* X86_FPSTACK_H */
