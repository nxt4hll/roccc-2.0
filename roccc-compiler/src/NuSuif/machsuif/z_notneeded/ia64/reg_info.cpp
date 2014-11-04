/* file "ia64/reg_info.cpp" */

/*
    Copyright (c) 2002 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "ia64/reg_info.h"
#endif

#include <machine/machine.h>
#include <cfg/cfg.h>

#include <ia64/reg_info.h>
#include <ia64/opcodes.h>
#include <ia64/init.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

using namespace ia64;

/*
 * Useful operand constants.
 */
Opnd opnd_reg_const0, opnd_br_ra, opnd_reg_sp, opnd_reg_gp, opnd_reg_tp;
Opnd opnd_pr_const1, opnd_fr_const0, opnd_fr_const1;

static Vector<const char*> reg_name_table;
static Vector<int>	   reg_width_table;
static Vector<NatSetDense> reg_aliases_table;

static Vector<NatSetDense> class_members_table;


static void
define_reg(unsigned reg, const char *name, int width = 0,
	   RegClassId class_id = REG_CLASS_NONE)
{
    reg_name_table[reg] = IdString(name).chars();
    reg_width_table[reg] = width;
    reg_aliases_table[reg] = NatSetDense();
    reg_aliases_table[reg].insert(reg);

    if (class_id != REG_CLASS_NONE)
	class_members_table[class_id].insert(reg);
}

static void
define_reg_ranges(int reg0, int last_reg, int *ranges, const char *prefix,
		  int width = 0, RegClassId class_id = REG_CLASS_NONE)
{
    int index = *ranges++;
    int upper = *ranges++;

    char name[15];

    for (int reg = reg0; reg <= last_reg; ++reg, ++index) {
	if (index > upper) {
	    index = *ranges++;
	    upper = *ranges++;
	}
	sprintf(name, "%s%d", prefix, index);
	define_reg(reg, name, width, class_id);
    }
}

/*
 * This function fills in the arrays indexed by abstract register number.
 */
void
init_ia64_reg_tables()
{
    maybe_expand(reg_name_table,      LAST_IA64_REG,   (const char *)NULL);
    maybe_expand(reg_width_table,     LAST_IA64_REG,   0);
    maybe_expand(reg_aliases_table,   LAST_IA64_REG,   NatSetDense());

    maybe_expand(class_members_table, LAST_IA64_CLASS, NatSetDense());

    define_reg(GR_CONST0, "r0");
    define_reg(PR_CONST1, "p0");
    define_reg(FR_CONST0, "f0");
    define_reg(FR_CONST1, "f1");
    define_reg(BR_RA, 	  "b0");
    define_reg(GP,	  "gp");
    define_reg(SP,	  "sp");
    define_reg(TP,	  "r13");

    int gr_tmp_ranges[] = { 2,3, 14,31 };	// r2-r3, r14-r31
    int gr_ret_ranges[] = { 8,11 };		// r8-r11
    int gr_sav_ranges[] = { 32,119, 4,7 };	// r32-r119, r4-r7

    define_reg_ranges(GR_TMP0, GR_LAST_TMP, gr_tmp_ranges, "r", 64, CLASS_GR);
    define_reg_ranges(GR_RET0, GR_LAST_RET, gr_ret_ranges, "r", 64, CLASS_GR);
    define_reg_ranges(GR_SAV0, GR_LAST_SAV, gr_sav_ranges, "r", 64, CLASS_GR);

    int gr_out_ranges[] = { 0,7 };		// out0-out7

    define_reg_ranges(GR_OUT0, GR_LAST_OUT, gr_out_ranges, "out", 64, CLASS_GR);

    int fr_tmp_ranges[] = { 6,7, 32,127 };	// f6-f7, f32-f127
    int fr_arg_ranges[] = { 8,15 };		// f8-f15
    int fr_sav_ranges[] = { 2,5, 16,31 };	// f2-f5, f16-f31

    define_reg_ranges(FR_TMP0, FR_LAST_TMP, fr_tmp_ranges, "f", 64, CLASS_FR);
    define_reg_ranges(FR_ARG0, FR_LAST_ARG, fr_arg_ranges, "f", 64, CLASS_FR);
    define_reg_ranges(FR_SAV0, FR_LAST_SAV, fr_sav_ranges, "f", 64, CLASS_FR);

    int pr_tmp_ranges[] = { 6,15 };		// p6-p15
    int pr_sav_ranges[] = { 1,5, 16,63 };	// p1-p5, p16-p63

    define_reg_ranges(PR_TMP0, PR_LAST_TMP, pr_tmp_ranges, "p",  1, CLASS_PR);
    define_reg_ranges(PR_SAV0, PR_LAST_SAV, pr_sav_ranges, "p",  1, CLASS_PR);

    int br_tmp_ranges[] = { 6,7 };		// b6-b7
    int br_sav_ranges[] = { 1,5 };		// b1-b5

    define_reg_ranges(BR_TMP0, BR_LAST_TMP, br_tmp_ranges, "b", 64, CLASS_BR);
    define_reg_ranges(BR_SAV0, BR_LAST_SAV, br_sav_ranges, "b", 64, CLASS_BR);

    int ar_kr_ranges[] = { 0,7 };		// kr0-kr7
    define_reg_ranges(AR_KR0,  AR_LAST_KR,  ar_kr_ranges, "ar.k");

    define_reg(AR_RSC,		"ar.rsc");
    define_reg(AR_BSP,		"ar.bsp");
    define_reg(AR_BSPSTORE,	"ar.bspstore");
    define_reg(AR_RNAT,		"ar.rnat");
    define_reg(AR_CCV,		"ar.ccv");
    define_reg(AR_UNAT,		"ar.unat");
    define_reg(AR_FPSR,		"ar.fpsr");
    define_reg(AR_ITC,		"ar.itc");
    define_reg(AR_PFS,		"ar.pfs");
    define_reg(AR_LC,		"ar.lc");
    define_reg(AR_EC,		"ar.ec");

    /* Define miscellaneous registers */
    define_reg(IP,	"ip");
    define_reg(CFM,	"cfm");
    define_reg(UM,	"um");

    /* Define register names for tokens used in directives like .save pr, ... */
    define_reg(RP_TOKEN, "rp");
    define_reg(PR_TOKEN, "pr");
    define_reg(SP_TOKEN, "sp");
}

int
reg_count_ia64()
{
    return LAST_IA64_REG + 1;
}

const char*
reg_name_ia64(int reg)
{
    claim((unsigned)reg < reg_name_table.size());
    return reg_name_table[reg];
}

int
reg_width_ia64(int reg)
{
    claim((unsigned)reg < reg_width_table.size());
    return reg_width_table[reg];
}

const NatSet*
reg_aliases_ia64(int reg)
{
    claim((unsigned)reg < reg_aliases_table.size());
    return &reg_aliases_table[reg];
}

const NatSet*
reg_members_ia64(RegClassId id)
{
    static NatSetDense empty;

    if (id == REG_CLASS_NONE)
	return &empty;

    claim((unsigned)id < class_members_table.size());
    return &class_members_table[id];
}

const NatSet*
reg_allocables_ia64()
{
    static bool ready = false;
    static NatSetDense allocables;

    if (!ready) {
	ready = true;

	for (int i = GR_ALLOCABLE0; i <= GR_LAST_ALLOCABLE; ++i)
	    allocables.insert(i);
	for (int i = FR_ALLOCABLE0; i <= FR_LAST_ALLOCABLE; ++i)
	    allocables.insert(i);
	for (int i = PR_ALLOCABLE0; i <= PR_LAST_ALLOCABLE; ++i)
	    allocables.insert(i);
	for (int i = BR_ALLOCABLE0; i <= BR_LAST_ALLOCABLE; ++i)
	    allocables.insert(i);
    }
    return &allocables;
}

const NatSet*
reg_caller_saves_ia64()
{
    static bool ready = false;
    static NatSetDense caller_saves;

    if (!ready) {
	ready = true;

	for (int i = GR_TMP0; i <= GR_LAST_TMP; ++i)
	    caller_saves.insert(i);
	for (int i = GR_RET0; i <= GR_LAST_RET; ++i)
	    caller_saves.insert(i);
	for (int i = GR_OUT0; i <= GR_LAST_OUT; ++i)
	    caller_saves.insert(i);
	for (int i = FR_TMP0; i <= FR_LAST_TMP; ++i)
	    caller_saves.insert(i);
	for (int i = FR_ARG0; i <= FR_LAST_ARG; ++i)
	    caller_saves.insert(i);
	for (int i = PR_TMP0; i <= PR_LAST_TMP; ++i)
	    caller_saves.insert(i);
	for (int i = BR_TMP0; i <= BR_LAST_TMP; ++i)
	    caller_saves.insert(i);
    }
    return &caller_saves;
}

const NatSet*
reg_callee_saves_ia64()
{
    static bool ready = false;
    static NatSetDense callee_saves;

    if (!ready) {
	ready = true;

	for (int i = GR_SAV0; i <= GR_LAST_SAV; ++i)
	    callee_saves.insert(i);
	for (int i = FR_SAV0; i <= FR_LAST_SAV; ++i)
	    callee_saves.insert(i);
	for (int i = PR_SAV0; i <= PR_LAST_SAV; ++i)
	    callee_saves.insert(i);
	for (int i = BR_SAV0; i <= BR_LAST_SAV; ++i)
	    callee_saves.insert(i);
    }
    return &callee_saves;
}

/*
 * Insert a "fill" (spill load) sequence before `marker' and return the handle
 * of the first inserted instruction.
 *
 * When the memory `src' is an automatic variable, i.e., on the stack, generate
 *
 *    add  tmp = src, sp
 *    ldXX dst = [tmp]
 *
 * where ldXX is the opcode produced by opcode_load(get_type(dst)).
 *
 * But `src' may also be in static memory, in which case generate
 *
 *    addl tmp = src, gp
 *    ld8  tmp = [tmp]
 *    ldXX dst = [tmp]
 *
 * In either case, if the value being reloaded is a subword integer, sign-
 * or zero-extend it after it has been loaded.
 */
InstrHandle
reg_fill_ia64(Opnd dst, Opnd src, InstrHandle marker, bool no_vr)
{
    CfgNode *block = get_parent_node(*marker);
    claim(block != NULL, "reg_fill must be used on a CFG instruction");

    VarSym *var = static_cast<VarSym*>(get_sym(src));
    claim(is_kind_of<VarSym>(var), "Unexpected kind of fill src address");
    bool is_local = is_auto(var);

    Opnd    tmp = no_vr ? opnd_reg(GR_LAST_RET, type_p64) : opnd_reg(type_p64);
    Opnd at_tmp = BaseDispOpnd(tmp, opnd_immed_0_u64);

    Opnd base = is_local ? opnd_reg_sp : opnd_reg_gp;

    int   opcode_add = make_opcode(is_local ? ADD : ADDL, EMPTY,EMPTY,EMPTY);
    Instr *instr_add = new_instr_alm(tmp, opcode_add, src, base);
    InstrHandle first = insert_before(block, marker, instr_add);

    if (!is_local) {
	int   opcode_ld8 = make_opcode(LD, SZ_8,LDTYPE_NONE,LDHINT_NONE);
	Instr *instr_ld8 = new_instr_alm(tmp, opcode_ld8, clone(at_tmp));
	insert_before(block, marker, instr_ld8);
    }
    TypeId type = get_type(dst);
    Instr *filler = new_instr_alm(dst, opcode_load(type), at_tmp);
    insert_before(block, marker, filler);

    if (is_integral(type)) {
	int stem = is_signed(type) ? SXT : ZXT; 
	int size = get_bit_size(type);
	int xsz = (size == 32) ? XSZ_4 :
		  (size == 16) ? XSZ_2 :
		  (size ==  8) ? XSZ_1 :
		  -1;
	if (xsz != -1) {
	    int extend = make_opcode(stem, xsz, EMPTY,EMPTY);
	    insert_before(block, marker, new_instr_alm(dst, extend, dst));
	}
    }
    return first;
}

/*
 * Insert a spill store sequence after `marker' and return the handle of
 * the last inserted instruction.
 *
 * When the memory `dst' is an automatic variable, i.e., on the stack, generate
 *
 *    add  tmp = dst, sp
 *    stXX [tmp] = src
 *
 * where stXX is the opcode produced by opcode_store(get_type(src)).
 *
 * But `dst' may also be in static memory, in which case generate
 *
 *    addl tmp = dst, gp
 *    ld8  tmp = [tmp]
 *    stXX [tmp] = src
 */
InstrHandle
reg_spill_ia64(Opnd dst, Opnd src, InstrHandle marker, bool no_vr)
{
    CfgNode *block = get_parent_node(*marker);
    claim(block != NULL, "reg_spill must be used on a CFG instruction");

    VarSym *var = static_cast<VarSym*>(get_sym(dst));
    claim(is_kind_of<VarSym>(var), "Unexpected kind of spill dst address");
    bool is_local = is_auto(var);

    Opnd    tmp = no_vr ? opnd_reg(GR_LAST_RET, type_p64) : opnd_reg(type_p64);
    Opnd at_tmp = BaseDispOpnd(tmp, opnd_immed_0_u64);

    Opnd base = is_local ? opnd_reg_sp : opnd_reg_gp;

    int   opcode_add = make_opcode(is_local ? ADD : ADDL, EMPTY,EMPTY,EMPTY);
    Instr *instr_add = new_instr_alm(tmp, opcode_add, dst, base);
    marker = insert_after(block, marker, instr_add);

    if (!is_local) {
	int   opcode_ld8 = make_opcode(LD, SZ_8,LDTYPE_NONE,LDHINT_NONE);
	Instr *instr_ld8 = new_instr_alm(tmp, opcode_ld8, clone(at_tmp));
	marker = insert_after(block, marker, instr_ld8);
    }
    TypeId type = get_type(src);
    Instr *spiller = new_instr_alm(at_tmp, opcode_store(type), src);

    return insert_after(block, marker, spiller);
}

int
reg_class_count_ia64()
{
    return LAST_IA64_CLASS + 1;
}

/*
 * Produce the ID of the intersection of two classes.
 */
RegClassId
reg_class_intersection_ia64(RegClassId c1, RegClassId c2)
{
    const int nclasses = LAST_IA64_CLASS + 1;

    if (c1 == REG_CLASS_NONE || c2 == REG_CLASS_NONE)
	return REG_CLASS_NONE;

    if (c1 == REG_CLASS_ANY)
	return c2;
    if (c2 == REG_CLASS_ANY)
	return c1;

    claim(c1 < nclasses && c2 < nclasses);

    return (c1 == c2) ? c1 : REG_CLASS_NONE;
}

/*
 * Helper class for classify_opnd and reg_classify_ia64: classify
 * address-register operands found in an address expression.
 */

class ClassifyFilter : public OpndFilter {
  public:
    ClassifyFilter(OpndCatalog *catalog, RegClassMap *class_map)
	: catalog(catalog), class_map(class_map) { }
    Opnd operator()(Opnd opnd, InOrOut);

  protected:
    OpndCatalog *catalog;
    RegClassMap *class_map;
};

Opnd
ClassifyFilter::operator()(Opnd opnd, InOrOut)
{
    int id = -1;
    catalog->enroll(opnd, &id);
    if (id >= 0) {
	maybe_expand(*class_map, id, REG_CLASS_ANY);

	if (is_hard_reg(opnd)) {
	    int reg = get_reg(opnd);

	    if (GR_ALLOCABLE0 <= reg && reg <= GR_LAST_ALLOCABLE)
		(*class_map)[id] = CLASS_GR;
	    else if (FR_ALLOCABLE0 <= reg && reg <= FR_LAST_ALLOCABLE)
		(*class_map)[id] = CLASS_FR;
	    else if (PR_ALLOCABLE0 <= reg && reg <= PR_LAST_ALLOCABLE)
		(*class_map)[id] = CLASS_PR;
	    else if (BR_ALLOCABLE0 <= reg && reg <= BR_LAST_ALLOCABLE)
		(*class_map)[id] = CLASS_BR;
	    else {
		claim(false, "Unclassifiable register (%d)", reg);
	    }
	} else {
	    TypeId type = get_type(opnd);

	    if (is_floating_point(type))
		(*class_map)[id] = CLASS_FR;
	    else if (is_type_pr(type))
		(*class_map)[id] = CLASS_PR;
	    else if (is_type_br(type))
		(*class_map)[id] = CLASS_BR;
	    else
		(*class_map)[id] = CLASS_GR;
	}
    }
    return opnd;
}

void
reg_classify_ia64(Instr *instr, OpndCatalog *catalog, RegClassMap *class_map)
{
    ClassifyFilter classifier(catalog, class_map);
    map_opnds(instr, classifier);
}

/*****************  Begin implementation of reg_choice_ia64  ******************/

/*
 * We precompute ("memoize") pools of registers to cut down the search for
 * an acceptable register given a constraint on its class and the need for
 * preservation across calls (or not).  Twelve cases are handled: for each
 * of the four classes, there are pools for scratch (TMP), preserved(SAV),
 * and don't care (ANY).
 *
 * Each memo-case index for class `id' is congruent to `id' mod 4,
 * where 4 is the number of classes.
 */
enum { MEMO_ANY_GR = 0,
       MEMO_ANY_FR,
       MEMO_ANY_PR,
       MEMO_ANY_BR,
       MEMO_TMP_GR,
       MEMO_TMP_FR,
       MEMO_TMP_PR,
       MEMO_TMP_BR,
       MEMO_SAV_GR,
       MEMO_SAV_FR,
       MEMO_SAV_PR,
       MEMO_SAV_BR,
       NUMBER_MEMO_CASES };

Vector<int> memo_members[NUMBER_MEMO_CASES];
int         memo_rotor  [NUMBER_MEMO_CASES];

void
init_memos()
{
    for (int i = GR_TMP0; i <= GR_LAST_TMP; ++i) {
	memo_members[MEMO_ANY_GR].push_back(i);
	memo_members[MEMO_TMP_GR].push_back(i);
    }
    for (int i = FR_TMP0; i <= FR_LAST_TMP; ++i) {
	memo_members[MEMO_ANY_FR].push_back(i);
	memo_members[MEMO_TMP_FR].push_back(i);
    }
    for (int i = PR_TMP0; i <= PR_LAST_TMP; ++i) {
	memo_members[MEMO_ANY_PR].push_back(i);
	memo_members[MEMO_TMP_PR].push_back(i);
    }
    for (int i = BR_TMP0; i <= BR_LAST_TMP; ++i) {
	memo_members[MEMO_ANY_BR].push_back(i);
	memo_members[MEMO_TMP_BR].push_back(i);
    }
    for (int i = GR_SAV0; i <= GR_LAST_SAV; ++i) {
	memo_members[MEMO_ANY_GR].push_back(i);
	memo_members[MEMO_SAV_GR].push_back(i);
    }
    for (int i = FR_SAV0; i <= FR_LAST_SAV; ++i) {
	memo_members[MEMO_ANY_FR].push_back(i);
	memo_members[MEMO_SAV_FR].push_back(i);
    }
    for (int i = PR_SAV0; i <= PR_LAST_SAV; ++i) {
	memo_members[MEMO_ANY_PR].push_back(i);
	memo_members[MEMO_SAV_PR].push_back(i);
    }
    for (int i = BR_SAV0; i <= BR_LAST_SAV; ++i) {
	memo_members[MEMO_ANY_BR].push_back(i);
	memo_members[MEMO_SAV_BR].push_back(i);
    }
    for (int i = 0; i < NUMBER_MEMO_CASES; ++i)
	memo_rotor[i] = 0;
}

int
reg_choice_memo(int memo_case, const NatSet *pool, const NatSet *excluded,
		bool rotate)
{
    Vector<int> &members = memo_members[memo_case];
    unsigned size = members.size();

    int start = rotate ? memo_rotor[memo_case] : 0;

    int k = start;
    do {
	int reg = members[k];

	if ((pool == NULL || pool->contains(reg)) && !excluded->contains(reg))
	{
	    if (rotate)
		memo_rotor[memo_case] = k;
	    return reg;
	}
    } while ((k = (k + 1) % size) != start);

    return -1;
}

/*
 * reg_choice_ia64
 *
 * Scan register info cyclically.  If rotate is true, start where last
 * successful search left off.
 */
int
reg_choice_ia64(RegClassId class_id, const NatSet *pool, const NatSet *excluded,
		bool rotate)
{
    static bool memoized = false;

    if (!memoized) {
	memoized = true;
	init_memos();
    }
    int memo_case;
    
    if (pool == reg_caller_saves_ia64()) {
	memo_case = MEMO_TMP_GR;
	pool = NULL;
    }
    else if (pool == reg_callee_saves_ia64()) {
	memo_case = MEMO_SAV_GR;
	pool = NULL;
    }
    else {
	memo_case = MEMO_ANY_GR;
    }

    if ((CLASS_GR <= class_id) && (class_id <= CLASS_BR))
	return reg_choice_memo(memo_case + class_id, pool, excluded, rotate);

    // Handle the unlikely case in which class ID doesn't constrain
    // register choice.

    claim (class_id == REG_CLASS_ANY, "Unexpected register class");

    for (int i = CLASS_GR; i <= CLASS_BR; ++i) {
	int reg = reg_choice_memo(memo_case + i, pool, excluded, rotate);
	if (reg >= 0)
	    return reg;
    }
    return -1;
}
