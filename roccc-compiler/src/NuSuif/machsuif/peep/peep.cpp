/* file "peep/peep.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "peep/peep.h"
#endif

#include <machine/machine.h>
#include <cfg/cfg.h>
#include <bvd/bvd.h>

#include <peep/peep.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

void
Peep::initialize()
{
    debug(1, "Debug level is %d", debuglvl);

    // Initialize progress counters for the current run.
    //
    no_of_remaining_moves = 0;
    no_of_mov_rx_to_rx = 0;
    no_of_fmov_rx_to_rx = 0;
    no_of_mov_after_op = 0;
    no_of_fmov_after_op = 0;
    no_of_mov_before_str = 0;
    no_of_fmov_before_str = 0;
    no_of_ldc_before_exp = 0;
}


static bool is_mortal(Opnd);			// forward declaration

void
Peep::do_opt_unit(OptUnit *unit)
{
    cur_unit = unit;

    IdString name = get_name(get_proc_sym(unit));
    debug(2, "Processing procedure \"%s\"", name.chars());

    // This pass requires a unit's body to be a CFG.  Be sure that a
    // preceding pass has left it in that form.

    claim(is_kind_of<Cfg>(get_body(cur_unit)), "Body is not in CFG form");
    unit_cfg = static_cast<Cfg*>(get_body(cur_unit));
    if_debug(4)
	fprint(stderr, unit_cfg, false, true);		// no layout, just code

    // Compute liveness information for the current procedure's registers
    // and potentially register-resident variables.  Here
    //
    //   opnd_catalog	maps operands of interest to bit-vector slots,
    //   du_analyzer	identifies the defs/uses of interest in a single
    //			instruction, and
    //   liveness	holds per-basic-block liveness results.

    opnd_catalog = new RegSymCatalog(debuglvl > 0, is_mortal);
    du_analyzer = new DefUseAnalyzer(opnd_catalog);
    liveness = new Liveness(unit_cfg, opnd_catalog, du_analyzer);

    if_debug(4) {
	fprintf(stderr, "Operand catalog:\n");
	opnd_catalog->print(stderr);
	fprintf(stderr, "\n");
    }

    // As we scan the instructions of a block, we want liveness info at the
    // current point of scan.  Variable `live' is the set of slot numbers
    // representing live entities at each point.  It is initialized at the
    // bottom of each block and updated for each instruction in the
    // backward scan over the block.
    NatSetDense live;

    // Visit each block in the procedure, in an arbitrary order.
    for (int i = 0; i < nodes_size(unit_cfg); ++i) {

	CfgNode *b = get_node(unit_cfg, i);		// current block

	if_debug(4) {
	    fprintf(stderr, "\n");
	    fprint(stderr, b);

	    int bound = liveness->num_slots();

	    fprintf(stderr, "Live in:  ");
	    liveness->in_set (b)->print(stderr, bound);
	    fprintf(stderr, "\n");

	    for (InstrHandle h = start(b); h != end(b); ++h)
		fprint(stderr, *h);

	    fprintf(stderr, "Live out: ");
	    liveness->out_set(b)->print(stderr, bound);
	    fprintf(stderr, "\n");
	}


	// Scan the instructions of block b in reverse.
	//
	// Variable `mi' holds the current instruction, and `next_mi' holds
	// the one to be reached next in the reverse scan.

	Instr *mi, *next_mi;
	bool need_to_delete_next_mi = false;

	live = *liveness->out_set(b);	// live set at block exit

	InstrHandle ih = last(b);
	int bsize = size(b);
	for (int i = 0; i < bsize; ) {
	    InstrHandle mi_h = ih;
	    mi = *mi_h;				// current instruction
	    if_debug(6)
		fprint(stderr, mi);

	    // Find next instruction (in reverse scan) that's not a "line
	    // directive" (source-code location pseudo-op), if there is one.
	    do {
		next_mi = NULL;
		++i, --ih;
	    } while (i < bsize && is_line(next_mi = *ih));

	    bool delete_mi = false;

	    // Handle two kinds of instructions specially: register-move
	    // instructions and instructions that write memory.  Analysis
	    // of the former can doom the current instruction.  Analysis
	    // of the latter can doom the next one to be scanned.

	    if (is_move(mi)) {
		delete_mi = process_mov_op(mi, next_mi, b, &live);
	    } else if (writes_memory(mi)) {
		need_to_delete_next_mi = process_str_op(mi, next_mi, b, &live);
	    } else if (const_prop && (is_binary_exp(mi) || is_unary_exp(mi))) {
		need_to_delete_next_mi = process_exp_op(mi, next_mi, b, &live);
	    }

	    // When deleting mi (the current instruction) or next_mi (its flow
	    // predecessor), don't change the `live' set, because we're not
	    // moving across an instruction that affects liveness.

	    if (need_to_delete_next_mi) {	// (next_mi == *ih)
		delete remove(b, ih);
		need_to_delete_next_mi = false;
		ih = mi_h;			// retry current instruction
	    }
	    else if (delete_mi) {		// (mi == *mi_h)
		delete remove(b, mi_h);
	    }
	    else {				// update `live' set for `mi'
		du_analyzer->analyze(mi);	// extract defs/uses for `mi'

		// Subsequent instructions in the scan (which are earlier in
		// flow order) should see def'd guys as dead now, and used
		// guys as live now, the latter taking precedence.
		live -= *du_analyzer->defs_set();
		live += *du_analyzer->uses_set();
	    }
	}
    }

    delete opnd_catalog;
    delete du_analyzer;
    delete liveness;

    if_debug(5) {
	fprintf(stderr, "\nFinal CFG:\n");
	fprint(stderr, unit_cfg, false, true);		// no layout, just code
    }
}

void
Peep::finalize()
{
    if_debug(1) {

        fputs("Statistics: \n\n", stderr);

	fprintf(stderr, "%d\tmov $x,$x instructions eliminated\n",
		no_of_mov_rx_to_rx);
	fprintf(stderr, "%d\tfmov $x,$x instructions eliminated\n\n",
		no_of_fmov_rx_to_rx);
	fprintf(stderr, "%d\tmov's after operation eliminated\n",
		no_of_mov_after_op);
	fprintf(stderr, "%d\tfmov's after operation eliminated\n\n",
		no_of_fmov_after_op);
	fprintf(stderr, "%d\tmov's before stores eliminated\n",
		no_of_mov_before_str);
	fprintf(stderr, "%d\tfmov's before stores eliminated\n\n",
		no_of_fmov_before_str);
	fprintf(stderr, "%d\tldc's before expressions eliminated\n\n",
		no_of_ldc_before_exp);
	fprintf(stderr, "%d\tmoves remain\n", no_of_remaining_moves);
    }
}

/*
 * process_mov_op() -- Look for opportunities to eliminate the current
 * move instruction.
 *
 * mi		Current instruction, a register-move.
 * next_mi	Instruction preceding mi in flow order, or else NULL.
 * b		CFG node for current basic block.
 * live		live slots just _after_ execution of instruction `mi'.
 */
bool
Peep::process_mov_op(Instr *mi, Instr *next_mi, CfgNode *b, NatSet *live)
{
    Opnd s = get_src(mi, 0);		// source of the move
    Opnd d = get_dst(mi, 0);		// destination

    // catch: mov $x, $x
    if (s == d) {
	update_counts(is_floating_point(get_type(d)),
		      no_of_mov_rx_to_rx, no_of_fmov_rx_to_rx);
	return true;		// delete this move
    }

    // catch:	op  ...,$x
    //		mov $x,$y	==> op ...,$y if $x dead

    // Obtain the liveness slot number for source operand `s'.  If it has
    // no slot, treat it as being live.
    int slot = -1;
    opnd_catalog->lookup(s, &slot);

    if (slot >= 0) {	
	if (next_mi				// If there's a preceding non-
	    && (!is_two_opnd(next_mi))		// .. 2-opnd instruction with a
	    && (dsts_size(next_mi))		// .. destination equal to the
	    && (s == get_dst(next_mi, 0))	// .. move's source, and it's
	    && !live->contains(slot)) {		// .. dead, then alter that
	    set_dst(next_mi, 0, d);		// .. destination and ..
	    update_counts(is_floating_point(get_type(d)),
			  no_of_mov_after_op, no_of_fmov_after_op);
	    return true;			// .. delete this move
	}
    }
    no_of_remaining_moves++;
    return false;
}


/*
 * process_str_op() -- Look for opportunities to eliminate useless
 * moves occurring immediately before a (RISC-like) store.
 *
 * mi		Current instruction, which writes memory.
 * next_mi	Instruction preceding `mi' in flow order, or else NULL.
 * b		CFG node for current basic block.
 * live		live slots just _after_ execution of instruction `mi'.
 */
bool
Peep::process_str_op(Instr *mi, Instr *next_mi, CfgNode *b, NatSet *live)
{
    // There must be a `next_mi' and it must be a move.
    if ((next_mi == NULL) || !is_move(next_mi))
	return false;
    // Also, the pattern match works only for simple, RISC-like stores.
    if ((srcs_size(mi) > 1) || (dsts_size(mi) > 1))
	return false;

    Opnd str_d = get_dst(mi, 0);
    Opnd str_s = get_src(mi, 0);

    Opnd mov_d = get_dst(next_mi, 0);
    Opnd mov_s = get_src(next_mi, 0);

    // catch:	mov $x,$y
    //		str $y,EA	==> str $x,EA' if $y dead
    //
    // where EA' is effective address EA after replacement of $y by $x

    if (str_s != mov_d)
	return false;
    if (mov_s == mov_d)				// handled by process_mov_op
	return false;

    // Obtain the liveness slot number for the store's source operand
    // `str_s'.  If it has no slot, treat it as being live.
    int slot = -1;
    opnd_catalog->lookup(str_s, &slot);

    if (slot < 0)
	return false;				// not covered by liveness

    if (live->contains(slot))
	return false;				// store source is live

    // Found pattern match, so update appropriate input operands
    if (AddrExpOpnd str_d_ae = str_d) {
	for (int i = 0; i < str_d_ae.srcs_size(); ++i)
	    if (str_d_ae.get_src(i) == mov_d)	// update addr exp suboperands
		((AddrExpOpnd&)str_d).set_src(i, mov_s);

	set_dst(mi, 0, str_d);			// update store dst operand
    }
    set_src(mi, 0, mov_s);			// update store src operand

    update_counts(is_floating_point(get_type(str_s)),
		  no_of_mov_before_str, no_of_fmov_before_str);

    // remember to delete next_mi
    return true;
}


/*
 * process_exp_op() -- Look for opportunities to eliminate load-constant
 * instructions by folding the constant into a unary or binary expression.
 *
 * mi		Current instruction, which writes memory.
 * next_mi	Instruction preceding `mi' in flow order, or else NULL.
 * b		CFG node for current basic block.
 * live		live slots just _after_ execution of instruction `mi'.
 */
bool
Peep::process_exp_op(Instr *mi, Instr *next_mi, CfgNode *b, NatSet *live)
{
    // There must be a `next_mi' and it must be a load-constant instruction.
    if ((next_mi == NULL) || !is_ldc(next_mi))
	return false;

    Opnd exp_d  = get_dst(mi);
    Opnd exp_s0 = get_src(mi, 0);
    Opnd exp_s1 = (srcs_size(mi) > 0) ? get_src(mi, 1) : opnd_null();

    Opnd ldc_d  = get_dst(next_mi);
    Opnd ldc_s  = get_src(next_mi, 0);

    // catch:	ldc $x,const
    //		exp $x,$y	==> exp const,$y if $x dead
    //    or:	ldc $y,const
    //		exp $x,$y	==> exp $x,const if $y dead

    if (ldc_d != exp_s0 && ldc_d != exp_s1)
	return false;

    // Obtain the liveness slot number for the ldc's destination operand
    // `ldc_d'.  If it has no slot, treat it as being live.
    int slot = -1;
    opnd_catalog->lookup(ldc_d, &slot);

    if (slot < 0)
	return false;				// not covered by liveness
    if (live->contains(slot))
	return false;				// ldc's dst is live after mi

    bool delete_ldc = false;

    for (int i = 0; i < srcs_size(mi); ++i)
	if (get_src(mi, i) == ldc_d) {
	    set_src(mi, i, ldc_s);
	    delete_ldc = true;
	}

    no_of_ldc_before_exp++;

    return delete_ldc;
}

/*
 * is_mortal -- Return true iff the argument operand is allowed to die
 * within the current procedure, i.e., if it's a register or a local
 * variable whose address is not taken.  A non-local or address-taken
 * variable is treated as "immortal" to be sure that its value is correct
 * when the current procedure calls others, and after the current
 * procedure returns.
 *
 * This predicate is used to filter out immortal items when doing liveness
 * analysis so that they'll never be treated as potentially dead.
 */

bool
is_mortal(Opnd opnd)
{
    if (is_reg(opnd))
	return true;
    if (is_var(opnd)) {
	VarSym *vs = get_var(opnd);
	return (!is_addr_taken(vs) && is_auto(vs));
    }
    return false;
}
