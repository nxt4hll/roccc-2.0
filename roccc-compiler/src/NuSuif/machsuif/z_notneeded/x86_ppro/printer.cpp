/* file "x86_ppro/printer.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "x86_ppro/printer.h"
#endif

#include <machine/machine.h>

#include <x86_ppro/opcodes.h>
#include <x86_ppro/printer.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

using namespace x86;

PrinterX86PPro::PrinterX86PPro()
{
    int i;
    print_instr_table.resize(LAST_X86_OPCODE + 1);

    for (i = CMOVA; i <= WRMSR; i++)
	print_instr_table[i] = &Printer::print_instr_alm;
}

void
PrinterX86PPro::print_instr_alm(Instr *mi)
{
    int opcode = get_opcode(mi);

    if (opcode < CMOVA) {
	// code in x86 library handles these opcode
	PrinterX86::print_instr_alm(mi);
	return;
    }

    // print out opcode and any extensions
    print_opcode(mi);

    // deal with operands
    if ((opcode == CPUID) || (opcode == RDMSR) || (opcode == WRMSR)
	|| (opcode == RDPMC) || (opcode == UD2)) {
	// nothing to do -- all operands are implicit

    } else if ((opcode >= CMOVA) && (opcode <= FCMOVNU)) {
	// print src1, dst -- src0 is same as dst
	print_opnd(get_src(mi, 1));
	fprintf(out, ",");
	print_opnd(get_dst(mi));

    } else if ((opcode >= FCOMI) && (opcode <= FUCOMIP)) {
	// in AT&T syntax, src1 comes before (left of) src0
	print_opnd(get_src(mi, 1));
	fprintf(out, ",");
	print_opnd(get_src(mi, 0));
    }

    print_notes(mi);
}
