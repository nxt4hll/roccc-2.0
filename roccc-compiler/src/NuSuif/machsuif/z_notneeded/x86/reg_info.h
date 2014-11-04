/* file "x86/reg_info.h" */

/*
    Copyright (c) 2000-2002 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef X86_REG_INFO_H
#define X86_REG_INFO_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "x86/reg_info.h"
#endif

/*
 * Define identifiers for x86 registers.
 *
 * To add new registers, add a new enum whose identifier values begin
 * beyond the end of the current enum (specified by LAST_X86_REG), then
 * redefine LAST_X86_REG.  Also, enroll new data into the register-
 * indexed data tables.
 */
namespace x86 {
enum {
    EAX, EBX, ECX, EDX,
     AX,  BX,  CX,  DX,
     AL,  BL,  CL,  DL,
     AH,  BH,  CH,  DH,

    ESI, EDI,
     SI,  DI,

    CONST0,             // fictitious CONST0 reg
    ESP,
    EBP,

    ES,                 // segment registers
    CS,
    SS,
    DS,
    FS,
    GS,

    EFLAGS,
    EIP,                // instruction pointer
    FPCR,               // FP control register
    FPFLAGS,            // FP status  register
    FPTW,               // FP tag word

    FP0,                // FP stack: %st, %st(1) - %st(7)
    FP7 = FP0 + 7,
};
} // namespace x86

#define LAST_X86_REG (x86::FP7)


/*
 * Register classes for x86:
 *
 *  EX: { EAX, EBX, ECX, EDX }
 *   X: {  AX,  BX,  CX,  DX }
 *  LH: {  AL,  AH,  BL,  BH,  CL,  CH, DL,  DH }
 *  EI: { ESI, EDI }
 *   I: {  SI,  DI }
 *
 * EXI = EX  U  EI
 *  XI =  X  U   I
 *
 * We only need EI and I in case we want to ascribe classes to hardware
 * registers.  These are never the classes of register candidates
 */

namespace x86 {
enum {
    CLASS_EXI, CLASS_XI, CLASS_EX, CLASS_X, CLASS_LH, CLASS_EI, CLASS_I
};
} // namespace x86

#define LAST_X86_CLASS (x86::CLASS_I)

int reg_count_x86();
const char* reg_name_x86(int reg);
int reg_width_x86(int reg);
const NatSet* reg_aliases_x86(int reg);

const NatSet* reg_allocables_x86(bool maximals = false);
const NatSet* reg_caller_saves_x86(bool maximals = false);
const NatSet* reg_callee_saves_x86(bool maximals = false);
int reg_maximal_x86(int reg);

InstrHandle reg_fill_x86 (Opnd dst, Opnd src, InstrHandle marker);
InstrHandle reg_spill_x86(Opnd dst, Opnd src, InstrHandle marker);

int reg_class_count_x86();
const NatSet* reg_members_x86(RegClassId);
RegClassId reg_class_intersection_x86(RegClassId, RegClassId);
void reg_classify_x86(Instr*, OpndCatalog*, RegClassMap*);
int reg_choice_x86(RegClassId, const NatSet *pool, const NatSet *excluded,
		   bool rotate);

void init_x86_reg_tables();

#endif /* X86_REG_INFO_H */
