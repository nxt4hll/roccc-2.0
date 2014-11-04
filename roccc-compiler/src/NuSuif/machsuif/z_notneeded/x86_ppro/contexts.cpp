/* file "x86_ppro/contexts.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "x86_ppro/contexts.h"
#endif

#include <machine/machine.h>
#include <suifvm/suifvm.h>

#include <x86_ppro/contexts.h>
#include <x86_ppro/code_gen.h>
#include <x86_ppro/printer.h>
#include <x86_ppro/instr.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

/* --------------------------  SuifVmContextX86PPro  ------------------------ */


CodeGen*
SuifVmContextX86PPro::target_code_gen() const
{
    if (cached_code_gen)
	return cached_code_gen;

    cached_code_gen = new CodeGenX86PPro;
    return cached_code_gen;
}

/* ------------------------  MachineContextX86PPro  ------------------------- */

Printer*
MachineContextX86PPro::target_printer() const
{
    if (cached_printer)
	return cached_printer;

    cached_printer = new PrinterX86PPro;
    return cached_printer;
}


bool
MachineContextX86PPro::is_cmove(Instr *mi) const
{
    return (is_cmove_x86_ppro(mi) || is_cmove_x86(mi));
}
