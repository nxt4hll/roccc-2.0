/* file "bvd/reaching_defs.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "bvd/reaching_defs.h"
#endif

#include <machine/machine.h>
#include <cfg/cfg.h>

#include <bvd/solve.h>
#include <bvd/reaching_defs.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

/* ---------------------------  ReachingDefs  ------------------------------ */

ReachingDefs::ReachingDefs(Cfg *graph, DefTeller *teller)
    : Bvd(graph, forward, any_path)
{
    def_map = new DefMap;
    def_teller = teller;

    // Scan the whole CFG, filling in def_map and def_point_sets.

    for (int i = 0; i < size(graph); ++i) {
	CfgNode *node = get_node(graph, i);

	int def_point = def_map->num_def_points();

	for (InstrHandle ih = start(node); ih != end(node); ++ih) {
	    Instr *instr = *ih;

	    NatSetSparse definees;
	    def_teller->insert_definees(instr, &definees);
	    def_map->enter(instr, definees.size());

	    for (NatSetIter nit = definees.iter(); nit.is_valid(); nit.next()) {
		unsigned definee = nit.current();
		if (definee >= def_point_sets.size())
		    def_point_sets.resize(definee + 1);
                def_point_sets[definee].insert(def_point++);
	    }
	}
    }
    // Let the underlying machinery in class Bvd compute the data flow.
    solve();
}

const NatSet*
ReachingDefs::def_points_for(int definee) const
{
    if ((unsigned)definee < def_point_sets.size())
	return &def_point_sets[definee];

    // definee could be uninitialized
    return &empty_set;
}

void
ReachingDefs::find_kill_and_gen(Instr *instr)
{
    instr_kill_set.remove_all();
    instr_gen_set.remove_all();

    NatSetSparse definees;
    def_teller->insert_definees(instr, &definees);

    claim(def_map->point_count(instr) == definees.size());

    int first_point = def_map->first_point(instr);

    for (NatSetIter nit = definees.iter(); nit.is_valid(); nit.next()) {
	instr_kill_set += def_point_sets[nit.current()];
	instr_gen_set.insert(first_point++);
    }
}


/* ------------------------------  DefMap  -------------------------------- */

DefMap::DefMap()
    : def_point_first(), def_point_count()
{
    next_def_point = 0;
}

int
DefMap::num_def_points() const
{
    return next_def_point;
}

int
DefMap::enter(Instr *mi, int count)
{
    if (!count)
        return -1;

    int first = first_point(mi);
    if (first >= 0)
        return first;

    first = next_def_point;

    def_point_first.enter_value(mi, first);
    def_point_count.enter_value(mi, count);

    claim(table.size() == (unsigned)next_def_point);

    do {
        table.push_back(mi);
	++next_def_point;
    } while (--count);

    return first;
}

Instr *
DefMap::lookup(int def_point) const
{
    if ((unsigned)def_point >= (unsigned)num_def_points())
	return NULL;
    else
	return table[def_point];
}

int
DefMap::first_point(Instr *mi) const
{
    HashMap<Instr*, int>::iterator h = def_point_first.find(mi);

    return (h != def_point_first.end()) ? (*h).second : -1;
}

int
DefMap::point_count(Instr *mi) const
{
    HashMap<Instr*, int>::iterator h = def_point_count.find(mi);

    return (h != def_point_count.end()) ? (*h).second : 0;
}


/* ----------------------------  DefTeller  ------------------------------- */

static void def_operand(Opnd, OpndCatalog*, NatSet*);
static void def_on_entry(OptUnit*, OpndCatalog*, NatSet*);

void
VarDefTeller::insert_definees(Instr *mi, NatSet *definees) const
{
    for (int i = 0; i < dsts_size(mi); i++)
        def_operand(get_dst(mi, i), the_catalog, definees);

    if (mi->peek_annote(k_proc_entry))
        def_on_entry(the_opt_unit, the_catalog, definees);
}

int
VarDefTeller::num_definees() const
{
    return the_catalog->num_slots();
}

extern void
    defs_uses_from_note(Instr*, NoteKey, const RegPartition*, NatSet *result);

void
RegDefTeller::insert_definees(Instr *mi, NatSet *definees) const
{
    for (int i = 0; i < dsts_size(mi); i++)
        def_operand(get_dst(mi, i), the_catalog, definees);

    defs_uses_from_note(mi, k_regs_defd, the_partition, definees);

    if (!is_null(get_note(mi, k_proc_entry)))
        def_on_entry(the_opt_unit, the_catalog, definees);
}

int
RegDefTeller::num_definees() const
{
    return the_catalog->num_slots();
}


/* def_operand:  Append an operand's definee index to the list of definees,
 * provided the operand is acceptable to the operand catalog.
 */
static void
def_operand(Opnd opnd, OpndCatalog *the_catalog, NatSet *definees)
{
    int opnd_id = -1;
    the_catalog->enroll(opnd, &opnd_id);
    if (opnd_id >= 0)
	definees->insert(opnd_id);
}


/*
 * def_on_entry: At a procedure entry point, make implicit defini-
 * tions for any parameter registers used, and also for the symbols of
 * parameters that are passed on the stack, since generated code
 * doesn't set them explicitly, as it does for register-passed
 * parameters.  
 */
static void
def_on_entry(OptUnit *unit, OpndCatalog *the_catalog, NatSet *definees)
{
    int num_formals = get_formal_param_count(unit);

    for (int i = 0; i < num_formals; i++) {
        VarSym *param = get_formal_param(unit, i);
        Opnd opnd;

        if (is_reg_param(param))
            opnd = opnd_reg(get_param_reg(param), get_type(param));
        else
	    opnd = opnd_var(param);          

	def_operand(opnd, the_catalog, definees);
    }
}
