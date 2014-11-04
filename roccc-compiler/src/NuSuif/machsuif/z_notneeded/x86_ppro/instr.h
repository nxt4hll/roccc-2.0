/* file "x86_ppro/instr.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef X86_PPRO_INSTR_H
#define X86_PPRO_INSTR_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "x86_ppro/instr.h"
#endif

#include <machine/machine.h>

bool is_cmove_x86_ppro(Instr *mi);

#endif /* X86_PPRO_INSTR_H */
