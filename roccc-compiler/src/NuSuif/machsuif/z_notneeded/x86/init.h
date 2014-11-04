/* file "x86/init.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef X86_INIT_H
#define X86_INIT_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "x86/init.h"
#endif

extern "C" void init_x86(SuifEnv*);

/* declarations of string constants */
extern IdString k_x86;

// common operands
extern Opnd opnd_reg_const0, opnd_reg_sp, opnd_reg_fp, opnd_reg_eflags;

#endif /* X86_INIT_H */
