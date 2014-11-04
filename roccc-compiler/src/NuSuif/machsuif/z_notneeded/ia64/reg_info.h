/* file "ia64/reg_info.h" */

/*
    Copyright (c) 2002 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef IA64_REG_INFO_H
#define IA64_REG_INFO_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "ia64/reg_info.h"
#endif

#include <machine/machine.h>

/*
 * Define identifiers for key IA64 registers.
 *
 * To add new registers, add a new enum whose identifier values begin
 * beyond the end of the current enum (specified by LAST_IA64_REG), then
 * redefine LAST_IA64_REG.  Also, enroll new data into the register-
 * indexed data tables.
 *
 * Within a bank, such as the general registers (GR), we clump the
 * allocable ones together in the abstract numbering, and within the
 * allocables, we clump scratch (TMP) registers together and preserved
 * registers together.
 */

namespace ia64 {
enum {
    /* Static general purpose registers */
    GR_CONST0,			        // r0
    GP,					// global data ptr (r1)
    SP,					// stack pointer (r12)
    TP,					// thread pointer (r13)
    GR_ALLOCABLE0,			// first allocable GR register
    GR_TMP0	      = GR_ALLOCABLE0,	// temporary registers (r2-r3, r14-r31)
    GR_LAST_TMP       = GR_TMP0 + 19,	//   (20 of above)
    GR_RET0,				// integer return values (r8-r11)
    GR_LAST_RET       = GR_RET0 + 3,	//   (4 of above)
    // Outgoing argument registers are
    // also scratch registers.
    GR_OUT0,				// callee-argument registers (out0-out7)
    GR_LAST_OUT	      = GR_OUT0 + 7,	//   (8 of above)
    // Preserved registers r120-r127 are
    // omitted to leave room for out0-out7. 
    GR_SAV0,				// preserved registers (r32-r119, r4-r7)
    GR_STACK0	      = GR_SAV0,	// stacked registers (r32-r119)
    GR_LAST_STACK     = GR_STACK0 + 87, //   (88 of above)
    GR_ARG0	      = GR_STACK0,      // incoming argument registers (r32-r39)
    GR_LAST_ARG	      = GR_ARG0 + 7,	//   (8 of above, maximum)
    GR_LAST_SAV       = GR_LAST_STACK+4,// last preserved register (with r4-r7)
    GR_LAST_ALLOCABLE = GR_LAST_SAV,	// last allocable GR register

    /* Static floating-point registers */
    FR_CONST0,			        // f0
    FR_CONST1,			        // f1
    FR_ALLOCABLE0,			// first allocable FR register
    FR_TMP0	      = FR_ALLOCABLE0,	// temporary registers (f6-f7, f32-f127)
    FR_LAST_TMP       = FR_TMP0 + 97,	//   (98 of above)
    FR_ARG0,				// argument/return registers (f8-f15)
    FR_LAST_ARG	      = FR_ARG0 + 7,	//   (8 of above)
    FR_SAV0,				// preserved registers (f2-f5, f16-f31)
    FR_LAST_SAV	      = FR_SAV0 + 19,	//   (20 of above)
    FR_LAST_ALLOCABLE = FR_LAST_SAV,	// last allocable FR register
   
    /* Static predicate registers */
    PR_CONST1,			        // p0
    PR_ALLOCABLE0,			// first allocable PR register
    PR_TMP0	      = PR_ALLOCABLE0,	// static predicates (p6-p15)
    PR_LAST_TMP       = PR_TMP0 + 9,	//   (10 of above)
    PR_SAV0,				// preserved registers (p1-p5, p16-p63)
    PR_LAST_SAV       = PR_SAV0 + 52,	//   (53 of above)
    PR_LAST_ALLOCABLE = PR_LAST_SAV,	// last allocable PR register

    /* Static branch registers */
    BR_RA,			        // b0
    BR_ALLOCABLE0,			// first allocable BR register
    BR_TMP0	      = BR_ALLOCABLE0,	// temporary registers (b6-b7)
    BR_LAST_TMP       = BR_TMP0 + 1,	//   (2 of above)
    BR_SAV0,				// preserved registers (b1-b5)
    BR_LAST_SAV       = BR_SAV0 + 4,	//   (5 of above)
    BR_LAST_ALLOCABLE = BR_LAST_SAV,	// last allocable BR register

    /* Application registers */
    /* IMPORTANT - I do not encode the 100 ignored/reserved registers */
    AR_KR0,			        // kernel registers (kr0-kr7)
    AR_LAST_KR        = AR_KR0 + 7,	//   (8 of above)
    AR_RSC,			        // register stack config reg (ar.rsc)
    AR_BSP,			        // backing store pointer (ar.bsp)
    AR_BSPSTORE,		        // bsp memory stores (ar.bspstore)
    AR_RNAT,			        // RSE NaT collection reg (ar.rnat)
    AR_CCV,				// compare&exch value reg (ar.ccv)
    AR_UNAT,				// user NaT collection reg (ar.unat)
    AR_FPSR,				// floating-point status reg (ar.fpsr)
    AR_ITC,				// interval time counter (ar.itc)
    AR_PFS,				// previous function state (ar.pfs)
    AR_LC,				// loop count reg (ar.lc)
    AR_EC,				// epilog count reg (ar.ec)

    /* Miscellaneous registers */
    IP,					// instruction pointer
    CFM,				// current frame marker
    UM,					// user mask

    /* IMPORTANT - I did not include CPUID and PMD registers */
    /* We should add these if it becomes necessary */

    /* Phony registers for use in assembler directives (e.g., .save) */
    RP_TOKEN,				// return pointer token "rp"
    PR_TOKEN,				// predicate register file token "pr"
    SP_TOKEN,				// stack pointer token "sp"
};
} // namespace ia64

#define LAST_IA64_REG (ia64::SP_TOKEN)
    
/*
 * For IA64, we can allocate from four register classes, which do not overlap:
 *
 *  CLASS_GR:  general-purpose registers
 *  CLASS_FR:  floating-point registers
 *  CLASS_PR:  predicate registers
 *  CLASS_BR:  branch registers
 */
namespace ia64 {
enum { CLASS_GR = 0, CLASS_FR, CLASS_PR, CLASS_BR };
} // namespace ia64

#define LAST_IA64_CLASS (ia64::CLASS_BR)

extern Opnd opnd_reg_const0, opnd_br_ra, opnd_reg_sp, opnd_reg_gp, opnd_reg_tp;
extern Opnd opnd_pr_const1, opnd_fr_const0, opnd_fr_const1;


int reg_count_ia64();
const char* reg_name_ia64(int reg);
int reg_width_ia64(int reg);
const NatSet* reg_aliases_ia64(int reg);

const NatSet* reg_allocables_ia64();
const NatSet* reg_caller_saves_ia64();
const NatSet* reg_callee_saves_ia64();
int reg_maximal_ia64(int reg);

InstrHandle reg_fill_ia64 (Opnd dst, Opnd src, InstrHandle marker,
			   bool post_reg_alloc);
InstrHandle reg_spill_ia64(Opnd dst, Opnd src, InstrHandle marker,
			   bool post_reg_alloc);

int reg_class_count_ia64();
const NatSet* reg_members_ia64(RegClassId);
RegClassId reg_class_intersection_ia64(RegClassId, RegClassId);
void reg_classify_ia64(Instr*, OpndCatalog*, RegClassMap*);
int reg_choice_ia64(RegClassId, const NatSet *pool, const NatSet *excluded,
		    bool rotate);

void init_ia64_reg_tables();

#endif /* IA64_REG_INFO_H */
