/* file "alpha/reg_info.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef ALPHA_REG_INFO_H
#define ALPHA_REG_INFO_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "alpha/reg_info.h"
#endif

#include <machine/machine.h>

/*
 * Define identifiers for key Alpha registers.
 *
 * To add new registers, add a new enum whose identifier values begin
 * beyond the end of the current enum (specified by LAST_ALPHA_REG), then
 * redefine LAST_ALPHA_REG.  Also, enroll new data into the register-
 * indexed data tables.
 */

namespace alpha {
enum {
    CONST0_GPR,				       // $31
    RA,					       // $26
    SP,					       // $sp (= $30)
    GP,					       // $gp (= $29)
    ALLOCABLE0_GPR,
    TMP0_GPR = ALLOCABLE0_GPR,		       // $1 - $8, $22
    LAST_TMP_GPR       = TMP0_GPR + 8,	       // = nine gp TMP registers
    ARG0_GPR	       = LAST_TMP_GPR + 1,     // $16 - $21
    LAST_ARG_GPR       = ARG0_GPR + 5,	       // = six	 gp ARG registers
    RET_GPR	       = LAST_ARG_GPR + 1,     // $0
    SAV0_GPR,				       // $9 - $15
    LAST_SAV_GPR       = SAV0_GPR + 6,	       // = seven gp SAV registers
    LAST_ALLOCABLE_GPR = LAST_SAV_GPR,
    ASM_TMP0	       = LAST_SAV_GPR + 1,     // $at (= $28), $23 - $25
    PV		       = ASM_TMP0+ 4,	       // $pv (= $27)
    LAST_ASM_TMP       = PV,		       // = five ASM_TMP registers
    LAST_GPR	       = LAST_ASM_TMP,	   
    CONST0_FPR,				       // $f31
    ALLOCABLE0_FPR,
    ARG0_FPR = ALLOCABLE0_FPR,		       // $f16 - $f21
    LAST_ARG_FPR       = ARG0_FPR + 5,	       // = six fp ARG registers
    RET0_FPR	       = LAST_ARG_FPR + 1,     // $f0 - $f1
    LAST_RET_FPR       = RET0_FPR + 1,	       // = two fp RET registers
    SAV0_FPR	       = LAST_RET_FPR + 1,     // $f2 - $f9
    LAST_SAV_FPR       = SAV0_FPR + 7,	       // = eight fp SAV registers
    TMP0_FPR	       = LAST_SAV_FPR + 1,     // $f10 - $f15, $f22 - $f30
    LAST_TMP_FPR       = TMP0_FPR + 14,	       // = fifteen fp TMP registers
    LAST_ALLOCABLE_FPR = LAST_TMP_FPR,
    LAST_FPR	       = LAST_TMP_FPR
};
} // namespace alpha

#define LAST_ALPHA_REG (alpha::LAST_TMP_FPR)
    
/*
 * For Alpha, we have two register classes that do not overlap: the general-purpose
 * registers (GPR) and the floating-point registers (FPR).
 */
namespace alpha {
enum { CLASS_GPR = 0, CLASS_FPR };
} // namespace alpha

#define LAST_ALPHA_CLASS (alpha::CLASS_FPR)

extern Opnd opnd_reg_const0, opnd_reg_ra, opnd_reg_sp, opnd_reg_gp, opnd_reg_pv;

int reg_count_alpha();
const char* reg_name_alpha(int reg);
int reg_width_alpha(int reg);
const NatSet* reg_aliases_alpha(int reg);

const NatSet* reg_allocables_alpha();
const NatSet* reg_caller_saves_alpha();
const NatSet* reg_callee_saves_alpha();

InstrHandle reg_fill_alpha (Opnd dst, Opnd src, InstrHandle marker);
InstrHandle reg_spill_alpha(Opnd dst, Opnd src, InstrHandle marker);

int reg_class_count_alpha();
const NatSet* reg_members_alpha(RegClassId);
RegClassId reg_class_intersection_alpha(RegClassId, RegClassId);
void reg_classify_alpha(Instr*, OpndCatalog*, RegClassMap*);
int reg_choice_alpha(RegClassId, const NatSet *pool, const NatSet *excluded,
		     bool rotate);

void init_alpha_reg_tables();

#endif /* ALPHA_REG_INFO_H */
