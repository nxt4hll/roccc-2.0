/* file "x86/code_gen.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "x86/code_gen.h"
#endif

#include <machine/machine.h>

#include <x86/code_gen.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif


/* CodeGenX86::init() -- This routine completes the initialization of
 * the variables needed during translation of the current procedure.
 *
 * No code is produced; pseudo ops for the beginning of a procedure text
 * segment are produced only when we generate assembly/binary code. */
void
CodeGenX86::init(OptUnit *unit)
{
    CodeGen::init(unit);
    fp_stack.clear();

    TypeId safe_type = array_type(type_f64, 0, 7);		// 8 elements
    fp_stack_safe = new_unique_var(safe_type, "_fp_stack_safe");
}

/*
 * CodeGenX86::finish() -- Finalize code generation:
 *
 * o  Make sure the struct_lds list has been consumed by corresponding
 *    stores.
 *
 * o  Record any information gathered during code generation that needs to
 *    be kept for later passes by placing it in annotations.
 */
void
CodeGenX86::finish(OptUnit *unit)
{
    claim(struct_lds.empty());

    CodeGen::finish(unit);
}
