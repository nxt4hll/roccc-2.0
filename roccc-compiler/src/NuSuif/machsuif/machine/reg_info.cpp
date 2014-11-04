/* file "machine/reg_info.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "machine/reg_info.h"
#endif

#include <stdarg.h>
#include <machine/substrate.h>
#include <machine/machine_ir.h>
#include <machine/problems.h>
#include <machine/contexts.h>
#include <machine/reg_info.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

/*
 * reg_lookup()
 *
 * Find the abstract number for the register named name.
 *
 * Assume that each element of the target-specific reg_names() vector has
 * been entered in the IdString uniquifying map.
 */
int
reg_lookup(const char *name)
{
    int nregs = reg_count();
    const char* unique = IdString(name).chars();

    for (int r = 0; r < nregs; ++r)
	if (unique == reg_name(r))
	    return r;

    claim(false, "Couldn't find a register named '%s'", name);
    return -1;
}

int
reg_count()
{
    return dynamic_cast<MachineContext*>(the_context)->reg_count();
}

const char*
reg_name(int reg)
{
    return dynamic_cast<MachineContext*>(the_context)->reg_name(reg);
}

int
reg_width(int reg)
{
    return dynamic_cast<MachineContext*>(the_context)->reg_width(reg);
}

const NatSet*
reg_aliases(int reg)
{
    return dynamic_cast<MachineContext*>(the_context)->reg_aliases(reg);
}

const NatSet*
reg_allocables(bool maximals)
{
    return dynamic_cast<MachineContext*>(the_context)->
	reg_allocables(maximals);
}

const NatSet*
reg_caller_saves(bool maximals)
{
    return dynamic_cast<MachineContext*>(the_context)->
	reg_caller_saves(maximals);
}

const NatSet*
reg_callee_saves(bool maximals)
{
    return dynamic_cast<MachineContext*>(the_context)->
	reg_callee_saves(maximals);
}

int
reg_maximal(int reg)
{
    return dynamic_cast<MachineContext*>(the_context)->reg_maximal(reg);
}

InstrHandle
reg_fill(Opnd dst, Opnd src, InstrHandle marker, bool post_reg_alloc)
{
    return dynamic_cast<MachineContext*>(the_context)->
	reg_fill(dst, src, marker, post_reg_alloc);
}

InstrHandle
reg_spill(Opnd dst, Opnd src, InstrHandle marker, bool post_reg_alloc)
{
    return dynamic_cast<MachineContext*>(the_context)->
	reg_spill(dst, src, marker, post_reg_alloc);
}

int
reg_class_count()
{
    return
	dynamic_cast<MachineContext*>(the_context)->reg_class_count();
}

const NatSet*
reg_members(RegClassId class_id)
{
    return dynamic_cast<MachineContext*>(the_context)->reg_members(class_id);
}

RegClassId
reg_class_intersection(RegClassId class_id1, RegClassId class_id2)
{
    return dynamic_cast<MachineContext*>(the_context)->
	reg_class_intersection(class_id1, class_id2);
}

void
reg_classify(Instr *instr, OpndCatalog *catalog, RegClassMap *class_map)
{
    return dynamic_cast<MachineContext*>(the_context)->
	reg_classify(instr, catalog, class_map);
}

int
reg_choice(RegClassId class_id, const NatSet *pool, const NatSet *excluded,
	   bool rotate = false)
{
    return dynamic_cast<MachineContext*>(the_context)->
	reg_choice(class_id, pool, excluded, rotate);
}

static int col;
static int
distance(int to)
{
    int delta = to - col;
    return delta > 0 ? delta : 1;
}

void
reg_info_print(FILE *out)
{
    const int colA = 0, colN = 8, colW = 16, colM = 24;

    col = 0;
    col += fprintf(out, "%*s", distance(colA + 7), "Abst reg");
    col += fprintf(out, "%*s", distance(colN + 6), "Name");
    col += fprintf(out, "%*s", distance(colW + 6), "Width");
    col += fprintf(out, "%*s", distance(colM + 6), "Aliases");
    fprintf(out, "\n");

    unsigned nregs = reg_count();

    for (unsigned r = 0; r < nregs; ++r) {
	col = 0;
	col += fprintf(out, "%*d", distance(colA + 5), r);
	col += fprintf(out, "%*s", distance(colN + 5), reg_name(r));
	col += fprintf(out, "%*d", distance(colW + 5), reg_width(r));
	col += fprintf(out, "%*s", distance(colM + 1), " ");
	reg_aliases(r)->print(out);
	fprintf(out, "\n");
    }
}
