/* file "suifvm/init.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef SUIFVM_INIT_H
#define SUIFVM_INIT_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "suifvm/init.h"
#endif

#include <machine/machine.h>

extern "C" void init_suifvm(SuifEnv* suif_env);

extern IdString k_suifvm;

#endif /* SUIFVM_INIT_H */
