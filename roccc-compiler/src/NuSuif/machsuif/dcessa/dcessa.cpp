/* file "dcessa/dcessa.cpp" */

/*
    Copyright (c) 2001 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "dcessa/dcessa.h"
#endif

#include <machine/machine.h>
#include <cfg/cfg.h>
#include <cfa/cfa.h>
#include <bvd/bvd.h>
#include <ssa/ssa.h>

#include <dcessa/dcessa.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

// Constants

NoteKey k_necessary_operation;			// note key for needed operation

// Per-unit variables

int eliminated_instr_count;			// success statistic



// ------------------------------  class Dcessa  -------------------------------

void
Dcessa::initialize()
{
    debug(1, "Debug level is %d", debuglvl);

    k_necessary_operation = "necessary_operation";
}

void
Dcessa::do_opt_unit(OptUnit *unit)
{
    cur_unit = unit;

    IdString name = get_name(get_proc_sym(cur_unit));
    debug(2, "Processing procedure \"%s\"", name.chars());

    eliminated_instr_count = 0;				// success statistic

    // This pass requires a unit's body to be a CFG.  Be sure that a
    // preceding pass has left it in that form.  Also, remove any
    // unreachable nodes because SSA conversion mingles RPO scans with
    // unordered scans of the node set.

    claim(is_kind_of<Cfg>(get_body(cur_unit)), "Body is not in CFG form");
    unit_cfg = static_cast<Cfg*>(get_body(cur_unit));
    remove_unreachable_nodes(unit_cfg);
    if_debug(5)
	fprint(stderr, unit_cfg, true, true);		// show layout and code

    // Find dominance frontier in reverse graph for control dependence info.
    // Find postdominators to use in control simplification.
    dominance_info = new DominanceInfo(unit_cfg);
    dominance_info->find_postdominators();
    dominance_info->find_reverse_dom_frontier();

    // Translate to SSA form.

    using namespace ssa;

    unit_ssa = new_ssa_cfg(unit_cfg, cur_unit,
			   BUILD_SEMI_PRUNED_FORM | BUILD_DEF_USE_CHAINS |
			   PRINT_WARNINGS);
    if_debug(5) {
	fprintf(stderr, "\nInitial SSA CFG:\n");
	fprint (stderr, unit_ssa);
    }

    // Execute the three stages of Morgan's algorithm.

    find_basic_necessities();
    process_worklist();
    discard_unnecessities();

    // Revert from SSA to ordinary form before neutralizing unneeded CTIs
    // because modifying the CFG could invalidate SSA form.

    unit_cfg = restore(unit_ssa);
    set_body(unit, unit_cfg);

    delete unit_ssa;

    retarget_unneeded_ctis();

    // Simplify the CFG if possible.

    while (optimize_jumps(unit_cfg)		||
	   merge_node_sequences(unit_cfg)	||
	   remove_unreachable_nodes(unit_cfg))
      { }

    if_debug(5) {
	fprintf(stderr, "\nFinal CFG:\n");
	fprint(stderr, unit_cfg, true, true);		// show layout and code
    }
}

// Filter for scanning instructions or phi-node sources to mark defs on
// which they depend as necessary

class MarkDefNecessaryFilter : public OpndFilter
{
  public:
    MarkDefNecessaryFilter(Dcessa *dce, SsaCfg *ssa)
	: dce(dce), ssa(ssa)
	{ }
    Opnd operator()(Opnd, InOrOut);

  protected:
    Dcessa *dce;
    SsaCfg *ssa;
};

Opnd
MarkDefNecessaryFilter::operator()(Opnd opnd, InOrOut in_or_out)
{
    if (in_or_out == IN)
    {
	Operation def = get_unique_def(ssa, opnd);
	if (!def.is_null())
	    dce->mark_necessary(def);
    }
    return opnd;
}

void
Dcessa::find_basic_necessities()
{
    clear(worklist);

    // visit each block block in procedure
    for (int i = 0; i < nodes_size(unit_cfg); ++i)
    {
        CfgNode *block = get_node(unit_cfg, i);

        // visit each instruction in block
	for (InstrHandle instr_h = start(block);
	     instr_h != end(block);
	     ++instr_h)
	{
	    Instr *instr = *instr_h;

            if_debug(6)
                print_numbered_instr("", instr);

            // mark an "obviously" necessary instruction
            if (is_call(instr)		||
		is_return(instr)	||
		writes_memory(instr)	||
		writes_volatile(instr)	||
		writes_state_reg(instr)	||
		is_builtin(instr)       ||
		has_note(instr, k_header_trailer))
	      mark_necessary(instr_h);
	}
    }
}


/* Process_worklist() -- Middle phase of Morgan's algorithm.
 * For each worklist entry, follow control and data dependences
 * of a known-necessary operation and add any newly-discovered
 * necessities to the worklist.
 */
void
Dcessa::process_worklist()
{
    while (!worklist.empty()) {
	Operation operation = worklist.front();
	worklist.pop_front();

        // Follow control dependences: mark as necessary the CTIs of each
        // block on which operation depends.

	mark_controlling_ctis(get_parent_node(operation));

        // Follow data dependences: for each definition that instr depends
        // on, mark the corresponding instruction or phi-node necessary.

	MarkDefNecessaryFilter mark_def_necessary(this, unit_ssa);
	map_opnds(operation, mark_def_necessary);
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
 *   Unnecessary branches.  Collect the ids of blocks terminated by
 *   unneeded CTIs for later processing.
 */
void
Dcessa::discard_unnecessities()
{
    // prepare to collect ID's of blocks with useless CTIs

    unneeded_cti_blocks.remove_all();

    // visit each block in procedure
    for (int i = 0; i < nodes_size(unit_cfg); ++i) {
        CfgNode *block = get_node(unit_cfg, i);

        // visit each instruction in block
	for (InstrHandle h = start(block); h != end(block); ) {
	    InstrHandle instr_h = h++; // advance before possible instr removal
	    Instr *instr = *instr_h;

	    // Check for and remove the "necessary_instr" annotation.
	    // Don't discard labels, pseudo-ops, or null instructions.
	    // (The latter might be a holding an important annotation.)
	    if (!is_null(take_note(instr, k_necessary_operation))
                || is_label(instr)
		|| is_dot(instr)
		|| is_null(instr))
                continue;

            // handle an unnecessary branch CTI by inserting current
            // block's ID in the `unneeded_cti_blocks' set for later
            if (instr == get_cti(block)) {
		unneeded_cti_blocks.insert(get_number(block));
	    }
            // otherwise, delete the unnecessary non-branch
            else {
		if_debug(4)
		    print_numbered_instr("  deleting: ", instr);
		++eliminated_instr_count;
		delete remove(block, instr_h);
            }
        }
    }
    debug(1, "Eliminated %d dead instructions", eliminated_instr_count);
}

/*
 * retarget_unneeded_ctis() -- Modify unneeded CTIs so that CFG utilities
 * will later eliminate them. 
 *
 * Since a CTI is messy to remove directly, redirect all successor edges to
 * a common successor, normally the immediate postdominator.  When the
 * merge_node_sequences utility runs later, it eliminates the branch.
 */
void
Dcessa::retarget_unneeded_ctis()
{
    for (NatSetIter it = unneeded_cti_blocks.iter(); it.is_valid(); it.next())
    {
	CfgNode *block = get_node(unit_cfg, it.current());
	CfgNode *new_succ = dominance_info->immed_postdom(block);

	// An infinite loop can cause the immediate postdominator
	// of block to be the exit node.  In that case, use the first
	// successor instead.  It's a real successor, not the exit.

	if (new_succ == get_exit_node(unit_cfg)) {
	    new_succ = *succs_start(block);
	    claim(new_succ != get_exit_node(unit_cfg));
	}
	for (int j = 0; j < succs_size(block); j++)
	    set_succ(block, j, new_succ);
    }
}

/*
 * mark_controlling_ctis() -- If a node on which `block' is control
 * dependent has a CTI, mark that instruction as necessary.  The nodes on
 * which `block' is control dependent are given by its reverse dominance
 * frontier.
 *
 * Owing to "impossible" or "abnormal" edges (cf. Morgan), we
 * don't assume that a block in a dominance frontier always ends
 * with a branch.
 */
void
Dcessa::mark_controlling_ctis(CfgNode *block)
{
    claim(block != NULL, "Operation (phi or instr) has no containing block");

    const NatSet *reverse_df = dominance_info->reverse_dom_frontier(block);
    for (NatSetIter fit = reverse_df->iter(); fit.is_valid(); fit.next())
    {
	CfgNode *controller = get_node(unit_cfg, fit.current());
	if (get_cti(controller) != NULL)
	    mark_necessary(get_cti_handle(controller));
    }
}


/* mark_necessary() -- Add an operation to Morgan's `Necessary' set and
 * to the worklist.
 *
 * Membership in `Necessary' is indicated by presence of a "necessary_instr"
 * annotation.  (The annotation itself is empty.)  The algorithm removes the
 * note when it tests for membership.
 */
void
Dcessa::mark_necessary(Operation operation)
{
    if (has_note(operation, k_necessary_operation))
	return;
    set_note(operation, k_necessary_operation, note_flag());
    worklist.push_back(operation);
}


/* print_numbered_instr() -- For debugging.
 */
void
Dcessa::print_numbered_instr(char *mess, Instr *instr)
{
    fprintf(stderr, "%s[%lx]: ", mess, (unsigned long)instr);
    fprint (stderr, instr);
}

/* writes_volatile() -- Return true for an instruction with a
 * destination that's a non-local or address-taken symbol.
 */
bool
Dcessa::writes_volatile(Instr *instr)
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

/*
 * writes_state_reg() -- For the time being, assume that when an
 * instruction defines a hard register, it is an essential instruction.
 * For example, the destination may be a status register or stack pointer
 * whose role is not fully apparent by inspection of the local code.
 *
 * Start with a helper class that finds hard-register operands.
 */

class HardRegDetector : public OpndFilter
{
  public:
    HardRegDetector() : found(false) { }
    Opnd operator()(Opnd, InOrOut);
    bool found_hard_regs() const { return found; }

  protected:
    bool found;
};

Opnd
HardRegDetector::operator()(Opnd opnd, InOrOut in_or_out)
{
    if (in_or_out == OUT && is_hard_reg(opnd))
	found = true;
    return opnd;
}

bool
Dcessa::writes_state_reg(Instr *instr)
{
    HardRegDetector detector;
    map_dst_opnds(instr, detector);
    return detector.found_hard_regs();
}
