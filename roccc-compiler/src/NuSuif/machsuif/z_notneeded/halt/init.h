/* file "halt/init.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef HALT_INIT_H
#define HALT_INIT_H

#include <machine/copyright.h>

#include <machine/machine.h>

extern IdString k_halt;

extern "C" void init_halt(SuifEnv*);

#endif /* HALT_INIT_H */
