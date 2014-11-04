/* file "x86_ppro/opcodes.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "x86_ppro/opcodes.h"
#endif

#include <machine/machine.h>
#include <x86/x86.h>

#include <x86_ppro/opcodes.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

using namespace x86;

// -------- Opcodes --------

void
init_x86_ppro_opcode_names()
{
    x86_opcode_names.resize(LAST_X86_OPCODE + 1);

    x86_opcode_names[CMOVA]    = "cmova";
    x86_opcode_names[CMOVAE]   = "cmovae";
    x86_opcode_names[CMOVB]    = "cmovb";
    x86_opcode_names[CMOVBE]   = "cmovbe";
    x86_opcode_names[CMOVC]    = "cmovc";
    x86_opcode_names[CMOVE]    = "cmove";
    x86_opcode_names[CMOVG]    = "cmovg";
    x86_opcode_names[CMOVGE]   = "cmovge";
    x86_opcode_names[CMOVL]    = "cmovl";
    x86_opcode_names[CMOVLE]   = "cmovle";
    x86_opcode_names[CMOVNA]   = "cmovna";
    x86_opcode_names[CMOVNAE]  = "cmovnae";
    x86_opcode_names[CMOVNB]   = "cmovnb";
    x86_opcode_names[CMOVNBE]  = "cmovnbe";
    x86_opcode_names[CMOVNC]   = "cmovnc";
    x86_opcode_names[CMOVNE]   = "cmovne";
    x86_opcode_names[CMOVNG]   = "cmovng";
    x86_opcode_names[CMOVNGE]  = "cmovnge";
    x86_opcode_names[CMOVNL]   = "cmovnl";
    x86_opcode_names[CMOVNLE]  = "cmovnle";
    x86_opcode_names[CMOVNO]   = "cmovno";
    x86_opcode_names[CMOVNP]   = "cmovnp";
    x86_opcode_names[CMOVNS]   = "cmovns";
    x86_opcode_names[CMOVNZ]   = "cmovnz";
    x86_opcode_names[CMOVO]    = "cmovo";
    x86_opcode_names[CMOVP]    = "cmovp";
    x86_opcode_names[CMOVPE]   = "cmovpe";
    x86_opcode_names[CMOVPO]   = "cmovpo";
    x86_opcode_names[CMOVS]    = "cmovs";
    x86_opcode_names[CMOVZ]    = "cmovz";
    x86_opcode_names[FCMOVB]   = "fcmovb";
    x86_opcode_names[FCMOVE]   = "fcmove";
    x86_opcode_names[FCMOVBE]  = "fcmovbe";
    x86_opcode_names[FCMOVU]   = "fcmovu";
    x86_opcode_names[FCMOVNB]  = "fcmovnb";
    x86_opcode_names[FCMOVNE]  = "fcmovne";
    x86_opcode_names[FCMOVNBE] = "fcmovnbe";
    x86_opcode_names[FCMOVNU]  = "fcmovnu";
    x86_opcode_names[FCOMI]    = "fcomi";
    x86_opcode_names[FCOMIP]   = "fcomip";
    x86_opcode_names[FUCOMI]   = "fucomi";
    x86_opcode_names[FUCOMIP]  = "fucomip";
    x86_opcode_names[RDPMC]    = "rdpmc";
    x86_opcode_names[UD2]      = "ud2";

    x86_opcode_names[CPUID]    = "cpuid";
    x86_opcode_names[RDMSR]    = "rdmsr";
    x86_opcode_names[WRMSR]    = "wrmsr";
}


// -------- Conditional-branch opcode inversions --------

void
init_x86_ppro_invert_table()
{
    x86_invert_table.resize(LAST_X86_OPCODE + 1);

    for (int i = LAST_X86_OPCODE; i >= CMOVA; i--)
	x86_invert_table[i] = -1;
}; 
