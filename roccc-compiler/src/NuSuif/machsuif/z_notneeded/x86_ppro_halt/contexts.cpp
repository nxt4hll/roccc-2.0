/* file "x86_ppro_halt/contexts.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "x86_ppro_halt/contexts.h"
#endif

#include <machine/machine.h>
#include <x86_ppro/x86_ppro.h>
#include <x86_halt/x86_halt.h>

#include "contexts.h"

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

// Empty
