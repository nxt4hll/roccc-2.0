/* file "x86_ppro/code_gen.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "x86_ppro/code_gen.h"
#endif

#include <x86_ppro/code_gen.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

/* This constructor fills in the translate_f_table so that the
 * calls in the base class know where to go.
 */
CodeGenX86PPro::CodeGenX86PPro()
{
    // left empty because there's nothing to do
}
