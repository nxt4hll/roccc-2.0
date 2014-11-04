/* file "x86_halt/init.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef X86_HALT_INIT_H
#define X86_HALT_INIT_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "x86_halt/init.h"
#endif

#include <machine/machine.h>

extern "C" void init_x86_halt(SuifEnv*);

/* declarations of string constants */
extern IdString k_x86_halt;

#endif /* X86_HALT_INIT_H */
