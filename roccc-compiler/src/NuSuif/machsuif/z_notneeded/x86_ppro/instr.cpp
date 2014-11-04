/* file "x86_ppro/instr.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "x86_ppro/instr.h"
#endif

#include <machine/machine.h>

#include <x86_ppro/opcodes.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif


/* ---------------- is_* helper routines ------------------- */

bool
is_cmove_x86_ppro(Instr *mi)
{
    int o = get_opcode(mi);
    return ((o >= x86::CMOVA) && (o <= x86::FCMOVNU));
}
