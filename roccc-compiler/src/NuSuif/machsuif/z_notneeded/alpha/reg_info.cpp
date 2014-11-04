/* file "alpha/reg_info.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "alpha/reg_info.h"
#endif

#include <machine/machine.h>
#include <cfg/cfg.h>

#include <alpha/reg_info.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

using namespace alpha;

/*
 * Useful operand constants.
 */
Opnd opnd_reg_const0, opnd_reg_ra, opnd_reg_sp, opnd_reg_gp, opnd_reg_pv;

static Vector<const char*>  reg_name_table;
static Vector<int>	    reg_width_table;
static Vector<NatSetDense> reg_aliases_table;

static Vector<NatSetDense> class_members_table;


static void
define_reg(unsigned reg, const char *name,
	   RegClassId class_id = REG_CLASS_NONE)
{
    claim(reg == reg_name_table.size());
    claim(reg == reg_width_table.size());
    claim(reg == reg_aliases_table.size());

    reg_name_table   .push_back(IdString(name).chars());
    reg_width_table  .push_back(64);
    reg_aliases_table.push_back(NatSetDense());
    reg_aliases_table[reg].insert(reg);

    if (class_id != REG_CLASS_NONE)
	class_members_table[class_id].insert(reg);
}

/*
 * This function fills in the arrays indexed by abstract register number.
 */
void
init_alpha_reg_tables()
{
    maybe_expand(class_members_table, LAST_ALPHA_CLASS, NatSetDense());

    define_reg(CONST0_GPR,	"31");
    define_reg(RA,		"26");
    define_reg(SP,		"sp");
    define_reg(GP,		"gp");

    define_reg(TMP0_GPR,	 "1", CLASS_GPR);
    define_reg(TMP0_GPR + 1,	 "2", CLASS_GPR);
    define_reg(TMP0_GPR + 2,	 "3", CLASS_GPR);
    define_reg(TMP0_GPR + 3,	 "4", CLASS_GPR);
    define_reg(TMP0_GPR + 4,	 "5", CLASS_GPR);
    define_reg(TMP0_GPR + 5,	 "6", CLASS_GPR);
    define_reg(TMP0_GPR + 6,	 "7", CLASS_GPR);
    define_reg(TMP0_GPR + 7,	 "8", CLASS_GPR);
    define_reg(TMP0_GPR + 8,	"22", CLASS_GPR);

    define_reg(ARG0_GPR,	"16", CLASS_GPR);
    define_reg(ARG0_GPR + 1,	"17", CLASS_GPR);
    define_reg(ARG0_GPR + 2,	"18", CLASS_GPR);
    define_reg(ARG0_GPR + 3,	"19", CLASS_GPR);
    define_reg(ARG0_GPR + 4,	"20", CLASS_GPR);
    define_reg(ARG0_GPR + 5,	"21", CLASS_GPR);

    define_reg(RET_GPR,		 "0", CLASS_GPR);

    define_reg(SAV0_GPR,	 "9", CLASS_GPR);
    define_reg(SAV0_GPR + 1,	"10", CLASS_GPR);
    define_reg(SAV0_GPR + 2,	"11", CLASS_GPR);
    define_reg(SAV0_GPR + 3,	"12", CLASS_GPR);
    define_reg(SAV0_GPR + 4,	"13", CLASS_GPR);
    define_reg(SAV0_GPR + 5,	"14", CLASS_GPR);
    define_reg(SAV0_GPR + 6,	"15", CLASS_GPR);

    define_reg(ASM_TMP0,	"at");
    define_reg(ASM_TMP0 + 1,	"23");
    define_reg(ASM_TMP0 + 2,	"24");
    define_reg(ASM_TMP0 + 3,	"25");
    define_reg(PV,		"27");

    define_reg(CONST0_FPR,     "f31");

    define_reg(ARG0_FPR,       "f16", CLASS_FPR);
    define_reg(ARG0_FPR + 1,   "f17", CLASS_FPR);
    define_reg(ARG0_FPR + 2,   "f18", CLASS_FPR);
    define_reg(ARG0_FPR + 3,   "f19", CLASS_FPR);
    define_reg(ARG0_FPR + 4,   "f20", CLASS_FPR);
    define_reg(ARG0_FPR + 5,   "f21", CLASS_FPR);

    define_reg(RET0_FPR,	"f0", CLASS_FPR);
    define_reg(RET0_FPR + 1,	"f1", CLASS_FPR);

    define_reg(SAV0_FPR,	"f2", CLASS_FPR);
    define_reg(SAV0_FPR + 1,	"f3", CLASS_FPR);
    define_reg(SAV0_FPR + 2,	"f4", CLASS_FPR);
    define_reg(SAV0_FPR + 3,	"f5", CLASS_FPR);
    define_reg(SAV0_FPR + 4,	"f6", CLASS_FPR);
    define_reg(SAV0_FPR + 5,	"f7", CLASS_FPR);
    define_reg(SAV0_FPR + 6,	"f8", CLASS_FPR);
    define_reg(SAV0_FPR + 7,	"f9", CLASS_FPR);

    define_reg(TMP0_FPR,       "f10", CLASS_FPR);
    define_reg(TMP0_FPR + 1,   "f11", CLASS_FPR);
    define_reg(TMP0_FPR + 2,   "f12", CLASS_FPR);
    define_reg(TMP0_FPR + 3,   "f13", CLASS_FPR);
    define_reg(TMP0_FPR + 4,   "f14", CLASS_FPR);
    define_reg(TMP0_FPR + 5,   "f15", CLASS_FPR);
    define_reg(TMP0_FPR + 6,   "f22", CLASS_FPR);
    define_reg(TMP0_FPR + 7,   "f23", CLASS_FPR);
    define_reg(TMP0_FPR + 8,   "f24", CLASS_FPR);
    define_reg(TMP0_FPR + 9,   "f25", CLASS_FPR);
    define_reg(TMP0_FPR + 10,  "f26", CLASS_FPR);
    define_reg(TMP0_FPR + 11,  "f27", CLASS_FPR);
    define_reg(TMP0_FPR + 12,  "f28", CLASS_FPR);
    define_reg(TMP0_FPR + 13,  "f29", CLASS_FPR);
    define_reg(TMP0_FPR + 14,  "f30", CLASS_FPR);
}

int
reg_count_alpha()
{
    return LAST_ALPHA_REG + 1;
}

const char*
reg_name_alpha(int reg)
{
    claim((unsigned)reg < reg_name_table.size());
    return reg_name_table[reg];
}

int
reg_width_alpha(int reg)
{
    claim((unsigned)reg < reg_width_table.size());
    return reg_width_table[reg];
}

const NatSet*
reg_aliases_alpha(int reg)
{
    claim((unsigned)reg < reg_aliases_table.size());
    return &reg_aliases_table[reg];
}

const NatSet*
reg_members_alpha(RegClassId id)
{
    static NatSetDense empty;

    if (id == REG_CLASS_NONE)
	return &empty;

    claim((unsigned)id < class_members_table.size());
    return &class_members_table[id];
}

const NatSet*
reg_allocables_alpha()
{
    static bool ready = false;
    static NatSetDense allocables;

    if (!ready) {
	ready = true;

	for (int i = TMP0_GPR; i <= LAST_TMP_GPR; ++i)
	    allocables.insert(i);
	for (int i = ARG0_GPR; i <= LAST_ARG_GPR; ++i)
	    allocables.insert(i);
	allocables.insert(RET_GPR);
	for (int i = SAV0_GPR; i <= LAST_SAV_GPR; ++i)
	    allocables.insert(i);
	for (int i = TMP0_FPR; i <= LAST_TMP_FPR; ++i)
	    allocables.insert(i);
	for (int i = ARG0_FPR; i <= LAST_ARG_FPR; ++i)
	    allocables.insert(i);
	for (int i = RET0_FPR; i <= LAST_RET_FPR; ++i)
	    allocables.insert(i);
	for (int i = SAV0_FPR; i <= LAST_SAV_FPR; ++i)
	    allocables.insert(i);
    }
    return &allocables;
}

const NatSet*
reg_caller_saves_alpha()
{
    static bool ready = false;
    static NatSetDense caller_saves;

    if (!ready) {
	ready = true;

	for (int i = TMP0_GPR; i <= LAST_TMP_GPR; ++i)
	    caller_saves.insert(i);
	for (int i = ARG0_GPR; i <= LAST_ARG_GPR; ++i)
	    caller_saves.insert(i);
	caller_saves.insert(RET_GPR);
	for (int i = TMP0_FPR; i <= LAST_TMP_FPR; ++i)
	    caller_saves.insert(i);
	for (int i = ARG0_FPR; i <= LAST_ARG_FPR; ++i)
	    caller_saves.insert(i);
	for (int i = RET0_FPR; i <= LAST_RET_FPR; ++i)
	    caller_saves.insert(i);
    }
    return &caller_saves;
}

const NatSet*
reg_callee_saves_alpha()
{
    static bool ready = false;
    static NatSetDense callee_saves;

    if (!ready) {
	ready = true;

	for (int i = SAV0_GPR; i <= LAST_SAV_GPR; ++i)
	    callee_saves.insert(i);
	for (int i = SAV0_FPR; i <= LAST_SAV_FPR; ++i)
	    callee_saves.insert(i);
    }
    return &callee_saves;
}

InstrHandle
reg_fill_alpha(Opnd dst, Opnd src, InstrHandle marker)
{
    Instr  *filler = new_instr_alm(dst, opcode_load(get_type(dst)), src);
    CfgNode *block = get_parent_node(*marker);

    claim(block != NULL, "reg_fill must be used on a CFG instruction");

    return insert_before(block, marker, filler);
}

InstrHandle
reg_spill_alpha(Opnd dst, Opnd src, InstrHandle marker)
{
    Instr  *spiller = new_instr_alm(dst, opcode_store(get_type(src)), src);
    CfgNode *block  = get_parent_node(*marker);

    claim(block != NULL, "reg_spill must be used on a CFG instruction");

    return insert_after(block, marker, spiller);
}

int
reg_class_count_alpha()
{
    return LAST_ALPHA_CLASS + 1;
}

/*
 * Produce the ID of the intersection of two classes.
 *
 * FIXME: this should be the default implementation.
 */
RegClassId
reg_class_intersection_alpha(RegClassId c1, RegClassId c2)
{
    const int nclasses = LAST_ALPHA_CLASS + 1;

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
 * Helper class for classify_opnd and reg_classify_alpha: classify
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

	    if ((TMP0_GPR <= reg && reg <= LAST_TMP_GPR) ||
		(ARG0_GPR <= reg && reg <= LAST_ARG_GPR) ||
		(SAV0_GPR <= reg && reg <= LAST_SAV_GPR) ||
		(reg == RET_GPR)) {
		(*class_map)[id] = CLASS_GPR;
	    } else if
	       ((TMP0_FPR <= reg && reg <= LAST_TMP_FPR) ||
		(ARG0_FPR <= reg && reg <= LAST_ARG_FPR) ||
		(RET0_FPR <= reg && reg <= LAST_RET_FPR) ||
		(SAV0_FPR <= reg && reg <= LAST_SAV_FPR)) {
		(*class_map)[id] = CLASS_FPR;
	    } else {
		claim(false, "Unclassifiable register (%d)", reg);
	    }
	} else {
	    if (is_floating_point(get_type(opnd)))
		(*class_map)[id] = CLASS_FPR;
	    else
		(*class_map)[id] = CLASS_GPR;
	}
    }
    return opnd;
}

void
reg_classify_alpha(Instr *instr, OpndCatalog *catalog, RegClassMap *class_map)
{
    ClassifyFilter classifier(catalog, class_map);
    map_opnds(instr, classifier);
}

/*****************  Begin implementation of reg_choice_alpha  ******************/

/*
 * We precompute ("memoize") pools of registers to cut down the search for
 * an acceptable register given a constraint on its class and the need for
 * preservation across calls (or not).  Six cases are handled: for each
 * of the two classes, there are pools for scratch (TMP), preserved(SAV),
 * and don't care (ANY).
 *
 * Each memo-case index for class `id' is congruent to `id' mod 2,
 * where 2 is the number of classes.
 */
/*
 * The GPR memo cases are even numbers; the FPR cases, odd.
 */
enum { MEMO_ANY_GPR = 0,
       MEMO_ANY_FPR,
       MEMO_TMP_GPR,
       MEMO_TMP_FPR,
       MEMO_SAV_GPR,
       MEMO_SAV_FPR,
       NUMBER_MEMO_CASES };

Vector<int> memo_members[NUMBER_MEMO_CASES];
int         memo_rotor  [NUMBER_MEMO_CASES];

void
init_memos()
{
    NatSetDense done;

    for (int i = TMP0_GPR; i <= LAST_TMP_GPR; ++i)
	{ memo_members[MEMO_ANY_GPR].push_back(i); done.insert(i); }
    for (int i = ARG0_GPR; i <= LAST_ARG_GPR; ++i)
	{ memo_members[MEMO_ANY_GPR].push_back(i); done.insert(i); }
    for (int i = RET_GPR; i <= RET_GPR; ++i)
	{ memo_members[MEMO_ANY_GPR].push_back(i); done.insert(i); }
    for (int i = SAV0_GPR; i <= LAST_SAV_GPR; ++i)
	{ memo_members[MEMO_ANY_GPR].push_back(i); done.insert(i); }
    for (int i = 0; i <= LAST_GPR; ++i)
	if (!done.contains(i))
	    memo_members[MEMO_ANY_GPR].push_back(i);
    done.remove_all();

    for (int i = TMP0_FPR; i <= LAST_TMP_FPR; ++i)
	{ memo_members[MEMO_ANY_FPR].push_back(i); done.insert(i); }
    for (int i = ARG0_FPR; i <= LAST_ARG_FPR; ++i)
	{ memo_members[MEMO_ANY_FPR].push_back(i); done.insert(i); }
    for (int i = RET0_FPR; i <= LAST_RET_FPR; ++i)
	{ memo_members[MEMO_ANY_FPR].push_back(i); done.insert(i); }
    for (int i = SAV0_FPR; i <= LAST_SAV_FPR; ++i)
	{ memo_members[MEMO_ANY_FPR].push_back(i); done.insert(i); }
    for (int i = LAST_GPR + 1; i <= LAST_FPR; ++i)
	if (!done.contains(i))
	    memo_members[MEMO_ANY_FPR].push_back(i);

    for (NatSetIter it = reg_caller_saves_alpha()->iter();
	 it.is_valid(); it.next())
    {
	int reg = it.current();
	int memo_case = (reg <= LAST_GPR) ? MEMO_TMP_GPR : MEMO_TMP_FPR;
	memo_members[memo_case].push_back(reg);
    }

    for (NatSetIter it = reg_callee_saves_alpha()->iter();
	 it.is_valid(); it.next())
    {
	int reg = it.current();
	int memo_case = (reg <= LAST_GPR) ? MEMO_SAV_GPR : MEMO_SAV_FPR;
	memo_members[memo_case].push_back(reg);
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
 * reg_choice_alpha
 *
 * Scan register info cyclically.  If rotate is true, start where last
 * successful search left off.  For Alpha, value width is not a criterion.
 */
int
reg_choice_alpha(RegClassId class_id, const NatSet *pool,
		 const NatSet *excluded, bool rotate)
{
    static bool memoized = false;

    if (!memoized) {
	memoized = true;
	init_memos();
    }
    int memo_case;
    
    if (pool == reg_caller_saves_alpha()) {
	memo_case = MEMO_TMP_GPR;
	pool = NULL;
    }
    else if (pool == reg_callee_saves_alpha()) {
	memo_case = MEMO_SAV_GPR;
	pool = NULL;
    }
    else {
	memo_case = MEMO_ANY_GPR;
    }

    for (int i = CLASS_GPR; i <= CLASS_FPR; ++i) {
	if (class_id == i || class_id == REG_CLASS_ANY) {
	    int reg = reg_choice_memo(memo_case + i, pool, excluded, rotate);
	    if (reg >= 0 || class_id == i)
		return reg;
	}
    }
    claim (false, "Unexpected register class");
    return -1;
}
