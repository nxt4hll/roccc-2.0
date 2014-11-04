/* file "dce/dce.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "dce/dce.h"
#endif

#include <machine/machine.h>
#include <cfg/cfg.h>
#include <cfa/cfa.h>
#include <bvd/bvd.h>

#include <dce/dce.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

// Constants

NoteKey k_necessary_instr;			// for note marking needed instr

// Per-unit variables

int eliminated_instr_count;			// success statistic

// Mask containing catalog numbers for the non-allocable hard registers.
// Hide it in a static function to avoid initialization problems (can't be
// certain of the order of initialization of the modules comprising a C++
// program).

static NatSetDense&
non_allocables_mask()
{
  static NatSetDense _non_allocables_mask;
  return _non_allocables_mask;
}

void
Dce::initialize()
{
    debug(1, "Debug level is %d", debuglvl);

    k_necessary_instr = "necessary_instr";
}

void
Dce::do_opt_unit(OptUnit *unit)
{
    cur_unit = unit;

    IdString name = get_name(get_proc_sym(cur_unit));
    debug(2, "Processing procedure \"%s\"", name.chars());

    eliminated_instr_count = 0;				// success statistic

    // This pass requires a unit's body to be a CFG.  Be sure that a
    // preceding pass has left it in that form.

    claim(is_kind_of<Cfg>(get_body(cur_unit)), "Body is not in CFG form");
    unit_cfg = static_cast<Cfg*>(get_body(cur_unit));
    if_debug(5)
	fprint(stderr, unit_cfg, false, true);		// no layout, just code

    // Find dominance frontier in reverse graph for control dependence info.
    // Find postdominators to use in control simplification.
    dominance_info = new DominanceInfo(unit_cfg);
    dominance_info->find_postdominators();
    dominance_info->find_reverse_dom_frontier();

    // Compute reaching_defs information for the current procedure's local
    // variables and registers.  Here
    //
    //   opnd_catalog	maps operands of interest to bit-vector slots,
    //   def_teller	identifies "interesting" definitions,
    //   reaching_defs	holds per-basic-block reaching_defs results.
    //
    // The argument to the catalog constructor means: save opnds for
    // printing only when debugging.

    opnd_catalog = new RegSymCatalog(debuglvl > 0);
    DefTeller *def_teller = new RegDefTeller(cur_unit, opnd_catalog);
    reaching_defs = new ReachingDefs(unit_cfg, def_teller);

    if_debug(6) {
	fprintf(stderr, "Operand catalog:\n");
	opnd_catalog->print(stderr);
	fprintf(stderr, "\n");
    }

    // Set up a mask containing the catalog numbers of the non-allocable
    // hard registers.
       
    non_allocables_mask().remove_all();
    const NatSet &allocables = *reg_allocables();

    for (int reg = reg_count() - 1; reg >= 0; --reg)
	if (!allocables.contains(reg)) {
	    int index;
	    opnd_catalog->enroll(opnd_reg(reg, type_v0), &index);
	    non_allocables_mask().insert(index);
	}

    // A DefUseAnalyzer identifies the defs/uses of interest in a single
    // instruction.  See the section on liveness in the BVD document for
    // the interface of this class.

    du_analyzer = new DefUseAnalyzer(opnd_catalog);

    // execute the three stages of Morgan's algorithm
    find_basic_necessities();
    process_worklist();
    discard_unnecessities();

    delete opnd_catalog;
    delete du_analyzer;
    delete reaching_defs;

    if_debug(5) {
	fprintf(stderr, "\nFinal CFG:\n");
	fprint(stderr, unit_cfg, false, true);		// no layout, just code
    }
}

void
Dce::find_basic_necessities()
{
    clear(worklist);

    // all def points reaching current instruction
    NatSetDense reaching;

    // all def points current instruction might depend on
    NatSetDense def_points;

    // visit each block b in procedure
    for (int i = 0; i < nodes_size(unit_cfg); ++i) {
        CfgNode *b = get_node(unit_cfg, i);

	// all defs reaching the 1st instruction of b
	reaching = *reaching_defs->in_set(b);

        // visit each instruction in b
	for (InstrHandle h = start(b); h != end(b); ++h) {
	    Instr *instr = *h;

            if_debug(6)
                print_numbered_instr("", instr);

            // mark an "obviously" necessary instruction
            if (is_call(instr)		||
		is_return(instr)	||
		writes_memory(instr)	||
		writes_volatile(instr)	||
		is_builtin(instr)       ||
		has_note(instr, k_header_trailer))
	      mark_necessary(instr);

            // determine items defined and used by instr
            du_analyzer->analyze(instr);

	    // Certain registers used for data access (e.g., stack pointer,
	    // frame pointer, or global-object pointer) must be treated
	    // specially because the importance of assignments to them isn't
	    // apparent from purely local inspection.  When an instruction
	    // defines a non-allocable register, assume that it's setting a
	    // data-access register and mark it as necessary.

	    NatSetDense defs = *du_analyzer->defs_set();
	    defs *= non_allocables_mask();
	    if (!defs.is_empty())
		mark_necessary(instr);
	    
            // determine def points on which instr depends: combine
	    // the def points of every item it uses, then mask out
	    // those that don't reach it
            def_points.remove_all();

	    for (NatSetIter uit = du_analyzer->uses_iter();
		 uit.is_valid(); uit.next())
                def_points += *reaching_defs->def_points_for(uit.current());
            def_points *= reaching;

            // if non-empty, attach def-point set to instr as its dependences
            if (!def_points.is_empty())
		dependences.enter_value(instr, def_points);
#if 0
	    // the compact way to advance reaching defs info past instr...
	    reaching_defs->find_kill_and_gen(instr);
	    reaching -= *reaching_defs->kill_set();
	    reaching += *reaching_defs->gen_set();
#else
	    // The slightly faster way is to avoid rescanning instr, and
	    // make use of the defs info provided by du_analyzer.

	    NatSetIter dit = du_analyzer->defs_iter();
	    if (!dit.is_valid())
		continue;		// no def points killed or genned

            // determine def points possibly killed by current instruction
            def_points.remove_all();

	    do {
                def_points += *reaching_defs->def_points_for(dit.current());
		dit.next();
	    } while (dit.is_valid());

            // killed definitions reach no further
            reaching -= def_points;

            // new definitions start reaching here
            int point = reaching_defs->map()->first_point(instr);
            int count = reaching_defs->map()->point_count(instr);
            while (count-- > 0)
                reaching.insert(point++);
#endif
        }
    }
}


/* Process_worklist() -- Middle phase of Morgan's algorithm.
 * For each worklist entry, follow control and data dependences
 * of a known-necessary instruction and add any newly-discovered
 * necessities to the worklist.  The nodes on which block B is
 * control dependent are given by the reverse dominance frontier
 * of B.
 *
 * Owing to "impossible" or "abnormal" edges (cf. Morgan), we
 * don't assume that a block in a dominance frontier always ends
 * with a branch.
 */
void
Dce::process_worklist()
{
    while (!worklist.empty()) {
	Instr *instr = worklist.front();
	worklist.pop_front();
        CfgNode *b = get_parent_node(instr);
        
        // follow control dependences: if a block on which instr depends has
	// a CTI, mark is as necessary
        const NatSet *reverse_df = dominance_info->reverse_dom_frontier(b);
        for (NatSetIter fit = reverse_df->iter(); fit.is_valid(); fit.next())
            if (Instr *cti = get_cti(get_node(unit_cfg, fit.current())))
                mark_necessary(cti);
        
        // follow data dependences: for each def point that instr depends
        // on, mark the instruction holding that def point as necessary
	HashMap<Instr*,NatSetSparse>::iterator handle = dependences.find(instr);
	if (handle == dependences.end())
	    continue;
        const NatSet &dependees = (*handle).second;
	for (NatSetIter dit = dependees.iter(); dit.is_valid(); dit.next()) {
	    int def_point = dit.current();
	    mark_necessary(reaching_defs->map()->lookup(def_point));
	}
    }
}

/* Discard_unnecessities() -- Morgan's third phase.
 * Delete instructions not marked necessary, with the following
 * exceptions:
 *
 *   Labels, null instructions, and line pseudo-ops.  The first
 *   two of these may have good reason to survive, and line
 *   markers can be useful for debugging.  None of these inhibit
 *   removal of unreachable or empty blocks.
 *
 *   Unnecessary branches.  Though messy to remove directly,
 *   they're handled indirectly by redirecting all successor
 *   edges, so that the branch is replaced during graph cleanup.              
 */
void
Dce::discard_unnecessities()
{
    // visit each block b in procedure
    for (int i = 0; i < nodes_size(unit_cfg); ++i) {
        CfgNode *b = get_node(unit_cfg, i);

        // visit each instruction in b
	for (InstrHandle h = start(b); h != end(b); ) {
	    InstrHandle instr_h = h++; // advance before possible instr removal
	    Instr *instr = *instr_h;

	    // Check for and remove the "necessary_instr" annotation.
	    // Don't discard labels, pseudo-ops, or null instructions.
	    // (The latter might be a holding an important annotation.)
	    if (!is_null(take_note(instr, k_necessary_instr))
                || is_label(instr)
		|| is_dot(instr)
		|| is_null(instr))
                continue;

            // handle an unnecessary branch CTI (can't be a call or return)
            // by diverting all successor edges to the immediate postdominator
            if (instr == get_cti(b)) {
		CfgNode *new_succ = dominance_info->immed_postdom(b);

		// An infinite loop can cause the immediate postdominator
		// of b to be the exit node.  In that case, use the first
		// successor instead.  It's a real successor, not the exit.

		if (new_succ == get_exit_node(unit_cfg)) {
		    new_succ = *succs_start(b);
		    claim(new_succ != get_exit_node(unit_cfg));
		}
                for (int j = 0; j < succs_size(b); j++)
                    set_succ(b, j, new_succ);
	    }
            // otherwise, delete the unnecessary non-branch
            else {
		if_debug(4)
		    print_numbered_instr("  deleting: ", instr);
		++eliminated_instr_count;
		delete remove(b, instr_h);
            }
        }
    }

    // simplify the CFG if possible
    while(optimize_jumps(unit_cfg)		||
	  merge_node_sequences(unit_cfg)	||
	  remove_unreachable_nodes(unit_cfg))
      { }

    debug(1, "Eliminated %d dead instructions", eliminated_instr_count);
}

/* mark_necessary() -- Add an instruction to Morgan's `Necessary' set and
 * to the worklist.
 *
 * Membership in `Necessary' is indicated by presence of a "necessary_instr"
 * annotation.  (The annotation itself is empty.)  The algorithm removes the
 * note when it tests for membership.
 */
void
Dce::mark_necessary(Instr *instr)
{
    if (has_note(instr, k_necessary_instr))
        return;

    set_note(instr, k_necessary_instr, note_flag());
    worklist.push_back(instr);
}

/* print_numbered_instr() -- For debugging.
 */
void
Dce::print_numbered_instr(char *mess, Instr *instr)
{
    fprintf(stderr, "%s[%lx]: ", mess, (unsigned long)instr);
    fprint (stderr, instr);
}

/* writes_volatile() -- Return true for an instruction with a
 * destination that's a non-local or address-taken symbol.
 */
bool
Dce::writes_volatile(Instr *instr)
{
    if (dsts_size(instr) == 0)
	return false;
    for (OpndHandle h = dsts_start(instr); h != dsts_end(instr); ++h)
	if (is_var(*h)) {
	    VarSym *v = get_var(*h);

	    if (!is_auto(v) || is_addr_taken(v))
		return true;
	}
    return false;
}
