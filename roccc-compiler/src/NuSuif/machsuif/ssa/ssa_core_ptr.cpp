/* file "ssa/ssa_core_ptr.h" */

/*
    Copyright (c) 2001 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "ssa/ssa_core_ptr.h"
#endif

#include <ssa/ssa_core_ptr.h>
#include <ssa/ssa_core.h>

SsaCorePtr::~SsaCorePtr()
{
    delete underlying_ptr;
}
