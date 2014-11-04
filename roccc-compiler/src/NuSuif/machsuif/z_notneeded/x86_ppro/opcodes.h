/* file "x86_ppro/opcodes.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef X86_PPRO_OPCODES_H
#define X86_PPRO_OPCODES_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "x86_ppro/opcodes.h"
#endif

#include <x86/x86.h>

/* Extend the x86 opcode enumeration to include the opcodes added to
 * the Pentium Pro and later compatible models.
 *
 * To modify the contents of the opcode enumeration, you add new enum
 * constants to current end of the enum (specified by
 * LAST_X86_OPCODE), redefine LAST_X86_OPCODE, and you enroll new data
 * into the opcode-indexed data tables.
 */

namespace x86 {

// Conditional move instructions check the state of the EFLAGS bits
const int CMOVA    = LAST_X86_OPCODE + 1;
const int CMOVAE   = LAST_X86_OPCODE + 2;
const int CMOVB    = LAST_X86_OPCODE + 3;
const int CMOVBE   = LAST_X86_OPCODE + 4;
const int CMOVC    = LAST_X86_OPCODE + 5;
const int CMOVE    = LAST_X86_OPCODE + 6;
const int CMOVG    = LAST_X86_OPCODE + 7;
const int CMOVGE   = LAST_X86_OPCODE + 8;
const int CMOVL    = LAST_X86_OPCODE + 9;
const int CMOVLE   = LAST_X86_OPCODE + 10;
const int CMOVNA   = LAST_X86_OPCODE + 11;
const int CMOVNAE  = LAST_X86_OPCODE + 12;
const int CMOVNB   = LAST_X86_OPCODE + 13;
const int CMOVNBE  = LAST_X86_OPCODE + 14;
const int CMOVNC   = LAST_X86_OPCODE + 15;
const int CMOVNE   = LAST_X86_OPCODE + 16;
const int CMOVNG   = LAST_X86_OPCODE + 17;
const int CMOVNGE  = LAST_X86_OPCODE + 18;
const int CMOVNL   = LAST_X86_OPCODE + 19;
const int CMOVNLE  = LAST_X86_OPCODE + 20;
const int CMOVNO   = LAST_X86_OPCODE + 21;
const int CMOVNP   = LAST_X86_OPCODE + 22;
const int CMOVNS   = LAST_X86_OPCODE + 23;
const int CMOVNZ   = LAST_X86_OPCODE + 24;
const int CMOVO    = LAST_X86_OPCODE + 25;
const int CMOVP    = LAST_X86_OPCODE + 26;
const int CMOVPE   = LAST_X86_OPCODE + 27;
const int CMOVPO   = LAST_X86_OPCODE + 28;
const int CMOVS    = LAST_X86_OPCODE + 29;
const int CMOVZ    = LAST_X86_OPCODE + 30;

// Conditional FP move instructions check the state of the EFLAGS bits
const int FCMOVB   = LAST_X86_OPCODE + 31;
const int FCMOVE   = LAST_X86_OPCODE + 32;
const int FCMOVBE  = LAST_X86_OPCODE + 33;
const int FCMOVU   = LAST_X86_OPCODE + 34;
const int FCMOVNB  = LAST_X86_OPCODE + 35;
const int FCMOVNE  = LAST_X86_OPCODE + 36;
const int FCMOVNBE = LAST_X86_OPCODE + 37;
const int FCMOVNU  = LAST_X86_OPCODE + 38;

// Compare real and set EFLAGS
const int FCOMI    = LAST_X86_OPCODE + 39;
const int FCOMIP   = LAST_X86_OPCODE + 40;
const int FUCOMI   = LAST_X86_OPCODE + 41;
const int FUCOMIP  = LAST_X86_OPCODE + 42;

// Read performance-monitoring counters
const int RDPMC    = LAST_X86_OPCODE + 43;

// Undefined instruction -- raise invalid opcode exception
const int UD2      = LAST_X86_OPCODE + 44;

// The following instructions were added before the Pentium Pro, but after
// the time that we generated our first x86 opcode file.

// CPU identification
const int CPUID    = LAST_X86_OPCODE + 45;

// Read or write a model-specific register
const int RDMSR    = LAST_X86_OPCODE + 46;
const int WRMSR    = LAST_X86_OPCODE + 47;

#undef LAST_X86_OPCODE
#define LAST_X86_OPCODE x86::WRMSR

} // namespace x86


// following used only in x86_ppro/init.cc
void init_x86_ppro_opcode_names();
void init_x86_ppro_invert_table();

#endif /* X86_PPRO_OPCODES_H */
