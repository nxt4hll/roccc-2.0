/* file "raga/color.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef METER
#include <time.h>
#include <sys/time.h>
#endif // METER
#include <machine/machine.h>
#include <cfg/cfg.h>

#include <raga/libra.h>
#include <raga/raga.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

/*
 * Names of the status fields of IG nodes and move-instruction records.
 * These are only for printing debugging messages.
 */
const char *state_name[] =
    { "NOSTATE", "INITIAL", "COLORED", "PRECOLORED", "COALESCED", "SPILLED",
      "SIMPLIFY_WORK", "FREEZE_WORK", "SPILL_WORK", "SELECT_STACKED",
      "ACTIVE", "WORKLIST", "CONSTRAINED", "FROZEN" };

inline int
class_placements(RegClassId id)
{
    claim((unsigned)id < (unsigned)reg_class_count());

    return reg_members(id)->size();
}

#ifdef IG_INCLUDES_HR
#define node_placements(n) (class_placements(n->class_id))
#else  // IG_INCLUDES_HR
/*
 * node_placements(n)
 *
 * Number of placements available to IG node n, taking into account not
 * just n's register class, but also its excluded set.  The latter is
 * assumed to be a subset of n's class.
 */
inline int
node_placements(RagaNode *n)
{
    return class_placements(n->class_id) - n->excluded.size();
}
#endif // IG_INCLUDES_HR


/*
 * add_squeeze(u, v)
 *
 * Add the squeeze from v to u->squeeze, and if v is not a hard register,
 * increase v's antisqueeze to reflect its effect on u.
 *
 * However if NEIGHBOR_CENSUS is defined and neighbor_census is true, then only
 * make each of those changes if u is not saturated with neighbors having v's
 * class.  Also in this case, increment u->class_degree[v->class_id].
 */
void
Raga::add_squeeze(RagaNode *u, RagaNode *v)
{
    RegClassId u_class = u->class_id;
    RegClassId v_class = v->class_id;

#ifdef NEIGHBOR_CENSUS
    if (!neighbor_census ||
	(u->class_degree[v_class]++ < class_size[v_class]))
#endif
    {
	int displacement_of_u_by_v = (*displacements)[u_class][v_class];
	u->squeeze += displacement_of_u_by_v;

#ifdef IG_INCLUDES_HR
	if (v->state != PRECOLORED)
#endif
	    v->antisqueeze +=
	        (float)displacement_of_u_by_v / (float)class_size[u_class];
    }
}

/*
 * relax_squeeze(u, v)
 *
 * Not exactly the inverse of add_squeeze(u, v).
 *
 * Subtract the squeeze placed on u by v from u->squeeze, and if v is not a
 * hard register, decrease u's antisqueeze by the amount that reflects its
 * constraint on v.
 *
 * However if NEIGHBOR_CENSUS is defined and neighbor_census is true, then
 * only make each of those changes if saturation (of node u and node v,
 * respectively) doesn't render it pointless.  Also in the neighbor_census
 * case, decrement u->class_degree[v->class_id] to reflect the removal of v.
 */
void
Raga::relax_squeeze(RagaNode *u, RagaNode *v)
{
    RegClassId u_class = u->class_id;
    RegClassId v_class = v->class_id;

#ifdef NEIGHBOR_CENSUS
    if (!neighbor_census ||
	(u->class_degree[v_class] <= class_size[v_class]))
#endif
	u->squeeze -= (*displacements)[u_class][v_class];
#ifdef NEIGHBOR_CENSUS
    if (!neighbor_census ||
	(v->class_degree[u_class] <= class_size[u_class]))
#endif
	u->antisqueeze -=
	    (*displacements)[v_class][u_class] / class_size[v_class];
#ifdef NEIGHBOR_CENSUS
    if (neighbor_census) {
	u->class_degree[v_class]--;
	claim(u->class_degree[v_class] >= 0);
    }
#endif
}

/* assign_registers() -- Perform register allocation for this_bank.
 */
void
Raga::assign_registers()
{
    init_coloring();
    Main();			// Loop until no spills
}

/* init_coloring() -- Prepare for main coloring loop.  The node list
 * `precolored' is implicit; precolored (machine register) nodes
 * are initialized by the caller.
 */
void
Raga::init_coloring()
{
    old_node_count = node_count = node_table.size();
    initial = NULL;
    for (size_t i = 0; i < node_count; ++i) {
	RagaNode *n = node_table[i];

	if (n != NULL) {
	    if (is_hard_reg(n->opnd)) {
		n->state = PRECOLORED;
		n->squeeze = INT_MAX;
		n->color = get_reg(n->opnd);
	    }
	    else {
		push_node(n, initial, INITIAL);
	    }
	}
    }
    simplify_worklist = NULL;
    freeze_worklist = NULL;
    spill_worklist = NULL;
    spilled_nodes = NULL;
    coalesced_nodes = NULL;
    colored_nodes = NULL;
    select_stack = NULL;

    coalesced_moves = NULL;
    constrained_moves = NULL;
    frozen_moves = NULL;
    worklist_moves = NULL;
    active_moves = NULL;

    if_debug(5)
	print_node_table_segment(stderr, 0, node_count,
				 "Registers and would-be registers");

    reinit_coloring();
}

/* reinit_coloring() -- Prepare for each round of coloring.
 * Spilling may have added temps.
 */

#define MAX_NODE_COUNT (1<<16)

void
Raga::reinit_coloring()
{
    clear_current_spillees();			// FIXME: no longer needed

    node_count = node_table.size();

    claim(node_count < MAX_NODE_COUNT, "Too many register candidates: %d >= %d",
	  node_count,  MAX_NODE_COUNT);

    if_debug(5)
	print_node_table_segment(stderr, old_node_count + 1, node_count,
				 "Additional register candidates");

    old_node_count = node_count;
#ifdef METER
    edge_count = 0;
    move_count = 0;
    add_edge_count = 0;
    add_edge_time = 0;
    coalesce_time = 0;
    george_test_count = 0;
    george_test_time = 0;
    briggs_test_count = 0;
    briggs_test_time = 0;
    test_spill_count = 0;
    select_spill_count = 0;
    select_spill_time = 0;
    combine_count = 0;
    combine_time = 0;
    enable_moves_count = 0;
    enable_moves_time = 0;
    visit_count = 0;
#endif // METER
    adj_set.init(node_count);

    // FIXME: resetting the class sets could be done more efficiently
    // at the end of rewrite_program, using the MoveInfo records, just
    // before they are deleted.
    //
    for (size_t i = 0; i < node_count; ++i) {
	RagaNode *n = node_table[i];
	if (n != NULL) {
	    n->class_id = opnd_classes[i];
#ifndef IG_INCLUDES_HR
	    n->excluded.remove_all();
#endif
	}
    }

#ifdef METER
    fprintf(stderr, "METER: Interference graph nodes before coloring:%7d\n", node_count);
#else
    debug(3, "Interference graph nodes before coloring:%7d", node_count);
#endif

    if_debug(1)
	for (int i = 0; i < 2; ++i)
	    for (unsigned j = node_moves[i].size(); j-- > 0; )
		claim(node_moves[i][j] == NULL);

    move_mentioned_count = 0;
    for (int i = 0; i < 2; ++i)
	if ((2 * node_moves[i].size()) < node_count)
	    node_moves[i].resize(node_count / 2);
}



void
Raga::Main()			// G & A main coloring loop
{
    liveness_analysis();
    build();

#ifdef NEIGHBOR_CENSUS
    if_debug(2)
	if (neighbor_census)
	    validate_census();
#endif

    mk_worklist();
    do {
	if (simplify_worklist)
	    simplify();
	else if (worklist_moves)
	    coalesce();
	else if (freeze_worklist)
	    freeze();
	else if (spill_worklist)
	    select_spill();
    } while
	( simplify_worklist || worklist_moves
	 || freeze_worklist || spill_worklist);

#ifdef IG_INCLUDES_HR
    claim(active_moves == NULL);
#else
    // Omitting precolored registers from the graph can occasionally cause moves
    // to be stranded on the active_moves list.  Such moves will only involve
    // precolored registers, however, so it's easy to dispose of them here
    // before going on.

    while (active_moves != NULL) {
	MoveInfo *mv = pop_move(active_moves, ACTIVE);
	claim(mv->node[0]->state == PRECOLORED &&
	      mv->node[1]->state == PRECOLORED);
	if (mv->node[0] == mv->node[1])
	    push_move(mv, coalesced_moves, COALESCED);
	else
	    push_move(mv, constrained_moves, CONSTRAINED);
    }
#endif

    if_debug(5)
	print_coalesced_nodes(stderr);

    if (just_coalesce)
    {
	finish();
	propagate_alias_colors();
	maybe_load_params();
    }
    else {
	assign_colors();
#ifdef METER
	fprintf(stderr, "METER: Number of  actually coalesced   moves:   %7d\n", combine_count);
	fprintf(stderr, "METER: Time spent actually coalescing  moves:   %7.3f sec\n",
	      (double)combine_time / 1.0e6);
	fprintf(stderr, "METER: Interference graph edges after  coloring:%7d\n", edge_count);
	fprintf(stderr, "METER: Number of calls on add_edge:           %9d\n",   add_edge_count);
	fprintf(stderr, "METER: Time spent in      add_edge:             %7.3f sec\n",
	      (double)add_edge_time / 1.0e6);
	fprintf(stderr, "METER: Number of calls on enable_moves_adjacent:%7d\n", enable_moves_count);
	fprintf(stderr, "METER: Time spent in      enable_moves_adjacent:%7.3f sec\n",
	      (double)enable_moves_time / 1.0e6);
	fprintf(stderr, "METER: Time spent in coalescing:                %7.3f sec\n",
	      (double)coalesce_time / 1.0e6);
	fprintf(stderr, "METER: Number of calls on adjacent_conservative:%7d\n", briggs_test_count);
	fprintf(stderr, "METER: Time spent in      adjacent_conservative:%7.3f sec\n",
	      (double)briggs_test_time / 1.0e6);
	fprintf(stderr, "METER: Number of calls on adjacent_ok:          %7d\n", george_test_count);
	fprintf(stderr, "METER: Time spent in      adjacent_ok:          %7.3f sec\n",
	      (double)george_test_time / 1.0e6);
	fprintf(stderr, "METER: Adjacent nodes visited for coalescing: %9d\n",   visit_count);
	fprintf(stderr, "METER: Number of nodes tested by select_spill:%9d\n",   test_spill_count);
	fprintf(stderr, "METER: Number of calls on        select_spill:  %7d\n", select_spill_count);
	fprintf(stderr, "METER: Time spent in             select_spill:  %7.3f sec\n",
	      (double)select_spill_time / 1.0e6);
	if_debug(8)
	    print_coalesce_counts();
#endif // METER
	if (spilled_nodes == NULL) {
	    finish();
	} else {
	    rewrite_program();		// and reinit nodes for new graph
	    reinit_coloring();
	    Main();
	}
    }
}

#ifndef NO_LEUNG_GEORGE
bool
#else
void
#endif
Raga::add_edge(RagaNode *u, RagaNode *v)
{
#ifndef NO_LEUNG_GEORGE
    bool result = false;
#endif

#ifdef METER
    ++add_edge_count;

    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
#endif // METER
    if (u->state == PRECOLORED)
    {
	if (v->state == PRECOLORED)
	    goto exit;

#ifndef IG_INCLUDES_HR
	const NatSet &u_aliases = (*aliases_in_class)[u->color][v->class_id];
	if (v->excluded.contains(u_aliases))
	    goto exit;

	v->excluded += u_aliases;
#else //IG_INCLUDES_HR
	const NatSet *mates = reg_partition->mates(u->color);
	bool subsumed = true;

	for (NatSetIter i = mates->iter(); i.is_valid(); i.next())
	    if (!adj_set(v->id, i.current())) {
		subsumed = false;
		adj_set.insert(v->id, i.current());
	    }
	if (subsumed)
	    goto exit;

	v->adj_list.push_front(u);
	add_squeeze(v, u);
#ifdef METER
	++edge_count;
#endif
#endif // IG_INCLUDES_HR
#ifndef NO_LEUNG_GEORGE
	result = true;
#endif
    }
    else if (v->state == PRECOLORED)
    {
#ifndef IG_INCLUDES_HR
	const NatSet &v_aliases = (*aliases_in_class)[v->color][u->class_id];
	if (u->excluded.contains(v_aliases))
	    return false;

	u->excluded += v_aliases;
#else // IG_INCLUDES_HR
	const NatSet *mates = reg_partition->mates(v->color);
	bool subsumed = true;

	for (NatSetIter i = mates->iter(); i.is_valid(); i.next())
	    if (!adj_set(u->id, i.current())) {
		subsumed = false;
		adj_set.insert(u->id, i.current());
	    }
	if (subsumed)
	    goto exit;
	u->adj_list.push_front(v);
	add_squeeze(u, v);
#ifdef METER
	++edge_count;
#endif
#endif // IG_INCLUDES_HR
#ifndef NO_LEUNG_GEORGE
	result = true;
#endif
    } else {
	if (u == v || adj_set(u->id, v->id))
	    goto exit;

	adj_set.insert(u->id, v->id);

	u->adj_list.push_front(v);
	v->adj_list.push_front(u);

	add_squeeze(u, v);
	add_squeeze(v, u);
#ifdef METER
	++edge_count;
#endif
#ifndef NO_LEUNG_GEORGE
	result = true;
#endif
    }
 exit:
#ifdef METER
    gettimeofday(&end_time, NULL);
    add_edge_time +=
	((long)end_time.tv_sec  - start_time.tv_sec) * 1000000 +
	((long)end_time.tv_usec - start_time.tv_usec);
#endif // METER
#ifndef NO_LEUNG_GEORGE
    return result;
#else
    return;
#endif
}

void
Raga::build()
{
#ifdef METER
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
#endif // METER
    if_debug(6)
	print_basic_blocks(stderr);

    NatSetDense live;

    for (CfgNodeHandle nh = start(unit_cfg); nh != end(unit_cfg); ++nh) {
	CfgNode *cnode = *nh;

	if (cnode == get_entry_node(unit_cfg)) {
	    entry_interferences(cnode);
	    continue;
	}

        live.remove_all();			// set live may be wider than ..
	live += *liveness->out_set(cnode);	// .. cnode's live_out_set

	InstrHandle ih = last(cnode);		// work backwards thru instrs
	for (int i = 0; i < size(cnode); ++i, --ih) {

	    Instr *I = *ih;

	    du_analyzer->analyze(I);	// extract defs/uses for `I'

	    NatSetIter dit = du_analyzer->defs_set()->iter();
	    NatSetIter uit = du_analyzer->uses_set()->iter();

	    // Test for both src and dst of move:  might be wrong
	    // type, and source register might be hardwired to zero.
	    // FIXME: latter case should be handled gracefully.

	    if (is_move(I) && dit.is_valid() && uit.is_valid()) {
		Opnd dopnd(get_dst(I, 0));
		Opnd sopnd(get_src(I, 0));

		if ((!is_hard_reg(dopnd) || !is_hard_reg(sopnd))) {

		    int did = var_or_reg_node_id(dopnd);
		    int sid = var_or_reg_node_id(sopnd);

		    if (did >= 0 && sid >= 0) {

			RagaNode *d = node_table[did];
			RagaNode *s = node_table[sid];
			RegClassId joint_class =
			    (d->class_id != s->class_id)
			      ? reg_class_intersection(d->class_id, s->class_id)
			      : d->class_id;
			if (joint_class != REG_CLASS_NONE) {
			    live -= *du_analyzer->uses_set();
			    MoveInfo *mv = new MoveInfo(ih, d, s);
			    add_node_move(d, mv, 0);
			    add_node_move(s, mv, 1);
			    push_move(mv, worklist_moves, WORKLIST);
#ifdef METER
			    ++move_count;
#endif // METER
			}
		    }
		}
	    }

	    live += *du_analyzer->defs_set(); // be sure multiple dsts interfere

	    for ( ; dit.is_valid(); dit.next()) {
		RagaNode *d = node_table[dit.current()];
		for (NatSetIter it = live.iter(); it.is_valid(); it.next()) {
		    RagaNode *l = node_table[it.current()];
		    claim(l->state != SPILLED);
		    if (l != d)
			add_edge(l, d);
		}
	    }

	    live -= *du_analyzer->defs_set();
	    live += *du_analyzer->uses_set();
	}
    }
#ifdef METER
    gettimeofday(&end_time, NULL);
    long build_time =
	((long)end_time.tv_sec  - start_time.tv_sec) * 1000000 +
	((long)end_time.tv_usec - start_time.tv_usec);

    fprintf(stderr, "METER: Interference graph edges before coloring:%7d\n", edge_count);
    fprintf(stderr, "METER: Number of possibly coalesceable moves:   %7d\n", move_count);
    fprintf(stderr, "METER: Time spent building interference graph:  %7.3f sec\n",
	      (double)build_time / 1.0e6);
#endif // METER
    if_debug(5)
	print_interferences(stderr);
}

/* At the entry node, treat any live, stack-passed parameters that are
 * register candidates as defined, to reflect the fact that they would, if
 * elevated to a register home, be loaded at that point into the register.
 * If their def's aren't reflected in the interference graph, they could
 * all be assigned to the same register.
 */
void
Raga::entry_interferences(CfgNode *entry)
{
    int param_count = cur_unit->get_formal_parameter_count();
    const NatSet &live = *liveness->out_set(entry);

    for (int i = 0; i < param_count; ++i) {
	VarSym *p = cur_unit->get_formal_parameter(i);
	int node_id = var_node_id(p);

	if (is_addr_taken(p)
	 || node_id < 0
	 || !live.contains(node_id)
	 || is_reg_param(p))
	    continue;

	RagaNode *d = node_table[node_id];

	for (NatSetIter lit = live.iter(); lit.is_valid(); lit.next()) {
	    RagaNode *l = node_table[lit.current()];
	    claim(l->state != SPILLED);
	    if (l != d)
		add_edge(l, d);
	}
    }
}

bool
Raga::move_related(RagaNode *n)
{
    if (n->movee_index < 0)
	return false;
    claim(node_moves[0][n->movee_index] != NULL ||
	  node_moves[1][n->movee_index] != NULL);
    return true;
}

void
Raga::mk_worklist()
{
    RagaNode *n;

    while ((n = pop_node(initial, INITIAL))) {
	if (n->squeeze >= node_placements(n))
	    push_node(n, spill_worklist, SPILL_WORK);
	else if (move_related(n))
	    push_node(n, freeze_worklist, FREEZE_WORK);
	else
	    push_node(n, simplify_worklist, SIMPLIFY_WORK);
    }
}

/* map_adjacent() -- Used in simplify, decrement_degree, and combine in
 * lieu of computing the `Adjacent' set of a node. (Cf. [GA96])
 */
void
Raga::map_adjacent(RagaNode *n, void (Raga::*f)(RagaNode*, RagaNode*))
{
    NodeListHandle nit = n->adj_list.begin();
    for ( ; nit != n->adj_list.end(); ++nit) {
	RagaNode *a = *nit;
	if (a->state != SELECT_STACKED && a->state != COALESCED)
	    (this->*f)(a, n);
    }
}

/*
 * Procedure `Simplify' from the G & A pseudocode.
 *
 * Includes this Leung & George extension: call enable_moves on the
 * neighbors of a high-degree node that has just been simplified out of the
 * graph.
 */
void
Raga::simplify()
{
    RagaNode *n = pop_node(simplify_worklist, SIMPLIFY_WORK);
    push_node(n, select_stack, SELECT_STACKED);

    if (n->squeeze >= node_placements(n))			// L & G ..
	enable_moves_adjacent(n);				// .. extension
    map_adjacent(n, &Raga::decrement_degree);
}

/*
 * decrement_degree(m, n)
 *
 * We're pretending to remove node n from the interference graph.  Decrement the
 * degree of its adjacent node m by one, and also remove the squeeze that n has
 * been placing on m.
 *
 * A Leung & George extension: don't enable moves of n itself.  Enable_moves is
 * (now) only to be called on nodes that have just lost a high-degree neighbor.
 */
void
Raga::decrement_degree(RagaNode *m, RagaNode *n)
{
    if (m->state == PRECOLORED)
	return;

    int pre_squeeze = m->squeeze;

    relax_squeeze(m, n);				// lower m->squeeze

    int K = node_placements(m);

    if (pre_squeeze >= K && m->squeeze < K) {
//	enable_moves_once(m);				// L & G extension
	enable_moves_adjacent(m);

	if (m->state != FREEZE_WORK) {
	    remove_node(m, spill_worklist, SPILL_WORK);
	    if (move_related(m))
		push_node(m, freeze_worklist, FREEZE_WORK);
	    else
		push_node(m, simplify_worklist, SIMPLIFY_WORK);
	}
    }
}

/*
 * enable_moves_once(t, hr_moves_only)
 *
 * Does what G & A call `EnableMoves(Adjacent(t))': for all ACTIVE moves that
 * involve neighbors of node t, transfer them from ACTIVE to WORKLIST status so
 * that they will be considered for coalescing.
 *
 * A Leung & George extension is to impose the precondition that the neighbor n
 * must just have lost a high-degree neighbor, and to be more selective about the
 * moves enabled.  Each active move m may have its `hi' field decremented, and it
 * is only put onto the worklist if the `hi' value has reached zero.
 *
 * For our implementation, the `hi' value has one of two meanings.  For a move that
 * involves a precolored node, `hi' is the number of high-degree neighbors of the
 * potential coalescee that don't interfere with the pre-colored node.  Otherwise
 * it is a sum of weight ratios between each high-degree node and the coalescee.
 * In both cases, the move is considered unready to test for coalesceability until
 * its `hi' value reaches zero.  The call on enable_moves_adjacent means that `hi'
 * values should be decremented, either by one for a HR-related move, or by the
 * weight ratio between the original node t and the move-operand node n.
 *
 * In our implementation, this same utility is used when the neighbors of t may
 * _not_ have lost a significant neighbor, but when t may have picked up a new
 * hard-register conflict that could cause HR-related moves of t's neighbors to
 * become coalesceable.  In that case, the flag hr_moves_only will be true, and we
 * consider enabling only the HR-related moves whose HR operand is now in conflict
 * with t.
 */
void
Raga::enable_moves_adjacent(RagaNode *t, bool hr_moves_only)
{
#ifdef METER
    ++enable_moves_count;

    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
#endif // METER

    for (NodeListHandle nit = t->adj_list.begin();
	 nit != t->adj_list.end();
	 ++nit)
    {
	RagaNode *n = *nit;
	if (n->state == SELECT_STACKED || n->state == COALESCED ||
	    n->movee_index < 0)
	    continue;

	for (int link_index = 0; link_index < 2; ++link_index) {
	    for (MoveInfo *m = node_moves[link_index][n->movee_index];
		 m != NULL;
		 m = m->link[link_index].next)
		if (m->state == ACTIVE) {
		    if (m->hi > 0) {			// Begin L & G extension
			int i;
			if (m->node[(i=0)]->state == PRECOLORED ||
			    m->node[(i=1)]->state == PRECOLORED)
			{
			    if (!hr_moves_only || is_in_conflict(m->node[i], t))
				--m->hi;		// not displacements sum
			} else {
			    if (!hr_moves_only)
				m->hi -=
				    (*displacements)[n->class_id][t->class_id];
			}
		    }
		    if (m->hi <= 0) {			// End   L & G extension
			remove_move(m, active_moves, ACTIVE);
			push_move(m, worklist_moves, WORKLIST);
		    }
		} else {
		    claim(m->state == WORKLIST);
		}
	}

    }
#ifdef METER
    gettimeofday(&end_time, NULL);
    enable_moves_time +=
	((long)end_time.tv_sec  - start_time.tv_sec) * 1000000 +
	((long)end_time.tv_usec - start_time.tv_usec);
#endif // METER
}

void
Raga::coalesce()
{
#ifdef METER
    struct timeval start_coalesce, end_coalesce;
    gettimeofday(&start_coalesce, NULL);
#endif // METER
    MoveInfo *m = pop_move(worklist_moves, WORKLIST);
#ifdef METER
    ++m->coalesce_count;
#endif // METER
    RagaNode *x = m->node[0];
    RagaNode *y = m->node[1];
    RagaNode *u, *v;
    if (y->state == PRECOLORED) {
	u = y; v = x;
    } else {
	u = x; v = y;
    }
    claim(u->alias == NULL && v->alias == NULL);

    if (u == v) {
	remove_node_move(u, m);
	push_move(m, coalesced_moves, COALESCED);
	add_work_list(u);
    }
    else if (v->state == PRECOLORED || is_in_conflict(u, v)) {
	remove_node_move(u, m);
	remove_node_move(v, m);
	push_move(m, constrained_moves, CONSTRAINED);
	add_work_list(u);
	add_work_list(v);
    }
    else if (u->state == PRECOLORED && adjacent_ok(v, u, m)
	 ||  u->state != PRECOLORED && adjacent_conservative(u, v, m)) {
	remove_node_move(u, m);
	remove_node_move(v, m);
	push_move(m, coalesced_moves, COALESCED);
	combine(u, v);
	add_work_list(u);
    } else {
	push_move(m, active_moves, ACTIVE);
    }
#ifdef METER
    gettimeofday(&end_coalesce, NULL);
    coalesce_time +=
	((long)end_coalesce.tv_sec  - start_coalesce.tv_sec) * 1000000 +
	((long)end_coalesce.tv_usec - start_coalesce.tv_usec);
#endif // METER
}

void
Raga::add_work_list(RagaNode *u)
{
    if (u->state != PRECOLORED
	&& !move_related(u)
	&& u->squeeze < node_placements(u)) {
	remove_node(u, freeze_worklist, FREEZE_WORK);
	push_node(u, simplify_worklist, SIMPLIFY_WORK);
    }
}

/*
 * Return true iff node u is in conflict with node v, where v cannot be
 * precolored, but u may be.
 *
 * The nodes are in conflict if they are adjacent in the interference
 * graph, or if u is precolored to a register that is excluded by re-
 * source-model constraints from being allocated to v.
 */

bool
Raga::is_in_conflict(RagaNode *u, RagaNode *v)
{
#ifndef IG_INCLUDES_HR
   if (u->state == PRECOLORED)
       return v->excluded.contains(u->color);
#endif
   return adj_set(u->id, v->id);
}

/*
 * adjacent_ok(v, r, m)
 *
 * The coalscing test attributed to George, as extended by Leung & George.  In
 * the original pseudocode it's invoked like this:
 *
 *   (forall t in Adjacent(v), OK(t, r))
 *
 * where r <-> v is the move m under consideration and r is known to be
 * precolored.
 *
 * In the L & G extension, we count the number of v's neighbors (t) that have
 * high degree and do not interfere with r.  As in the original, the test
 * returns false if there are _any_ such neighbors.  But in the extension, we
 * save the count in the `hi' field of the move.
 */

bool
Raga::adjacent_ok(RagaNode *v, RagaNode *r, MoveInfo *m)
{
#ifdef METER
    ++george_test_count;

    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
#endif // METER
    int k = 0;						// L & G extension

    NodeListHandle it = v->adj_list.begin();
    for ( ; it != v->adj_list.end(); ++it) {
	RagaNode *t = *it;
	if (t->state != SELECT_STACKED && t->state != COALESCED) {
#ifdef METER
	    ++visit_count;
#endif // METER
#ifndef IG_INCLUDES_HR
	    if (!(   t->squeeze < node_placements(t)
		  || t->excluded.contains(r->color)))
#else
	    if (!(   t->state == PRECOLORED
		  || t->squeeze < node_placements(t)
		  || adj_set(t->id, r->id)))
#endif
		++k;					// L & G extension
	}
    }
#ifdef METER
    gettimeofday(&end_time, NULL);
    george_test_time +=
	((long)end_time.tv_sec  - start_time.tv_sec) * 1000000 +
	((long)end_time.tv_usec - start_time.tv_usec);
#endif // METER
    if (k > 0) {					// L & G ..
	m->hi = k;					// .. extension
	return false;
    }
    return true;
}

/*
 * adjacent_conservative(u, v, m)
 *
 * Called to test coalesceability of a move m (u <-> v) when both u and v are
 * register candidates (not precolored).  Returns true when removal of the
 * easily-colored neighbors of the coalesced node uv would leave uv
 * "insignificant", i.e. easy to color itself.
 *
 * This is the coalscing test attributed to Briggs, as extended by Leung &
 * George.  In the original pseudocode it's invoked like this:
 *
 *   Conservative(Adjacent(u) union Adjacent(v))
 *
 * since Briggs called this test "conservative".
 *
 * In our version of the original G & A algorithm, we first obtain a placement
 * count (`freedom') for uv, then we begin to sum up weight ratios between
 * potential neighbors of uv and the coalescee uv itself.  We only consider the
 * neighbors that are currently significant, however.  This sum is called
 * `hard_squeeze' because it's the port of the total squeeze on uv that might be
 * hard to eliminate by simplification.  If `hard_squeeze' reaches `freedom',
 * the test returns false, since uv could be hard to color.
 *
 * In the L & G extension, we must compute the full hard_squeeze value (no early
 * termination), so that if the test fails, we can set the `hi' field of move m
 * appropriately for enable_moves to decrement and test.  Since enable_moves
 * tests for zero, we set m->hi to hard_squeeze - (freedom-1), because the
 * Briggs test fails while hard_squeeze >= freedom.
 *
 * We also compute the register class ID of the coalescee uv (the meet of the
 * classes of u and v), and we store it in the class_id field of move m, though
 * this extra field isn't strictly needed and will be removed later.
 *
 * A subtle point about the L & G extension is that when both u and v are
 * initially significant, some of the neighbors of uv will wind up with one less
 * significant neighbor after they are coalesced, and so enable_moves should be
 * called on these nodes.  They are the neighbors that u and v have in common.
 * We compute this intersection set as a by-product of the hard_squeeze
 * calculation, since we need to avoid double-counting the shared neighbors of
 * uv.
 *
 * If Briggs test is to about to return true, so that u and v will be coalesced,
 * we store the meet class in the combined node and we set the cached freedom
 * value accordingly.  We do not adjust the squeeze value of the combined node
 * here; that gets done by transfer_edge().
 *
 * SLEAZE ALERT: we take advantage of the fact that node u is the one retained
 * if this function returns true.
 */
bool
Raga::adjacent_conservative(RagaNode *u, RagaNode *v, MoveInfo *m)
{
#ifdef METER
    ++briggs_test_count;

    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
#endif // METER
    RegClassId joint_class = u->class_id;
    if (joint_class != v->class_id)
	joint_class = reg_class_intersection(joint_class, v->class_id);

#ifndef IG_INCLUDES_HR
    NatSetDense excluded = u->excluded;
    excluded += v->excluded;
    excluded *= *reg_members(joint_class);	// mask to subset of joint_class

    int placements = class_placements(joint_class) - excluded.size();
#else
    int placements = class_placements(joint_class);
#endif

    // visit each neighbor of the potential coalescee to determine whether
    // enough of them have significant weighted degree to keep the coalescee
    // from being easy to color.

    static char visited[MAX_NODE_COUNT];
    memset((void *)visited, 0, node_count);

    int hard_squeeze = 0;

    NodeListHandle it, end;
    int toggle = 0;

    for (it = u->adj_list.begin(), end = u->adj_list.end();
	 toggle++ < 2;
	 it = v->adj_list.begin(), end = v->adj_list.end())
	for ( ; it != end; ++it)
	{
	    RagaNode *a = *it;
	    if (a->state != SELECT_STACKED && a->state != COALESCED)
	    {
		if (visited[a->id] == 0)
		{
		    visited[a->id] = 1;			// mark as visited
#ifdef METER
		    ++visit_count;
#endif // METER
		    if (a->squeeze >= node_placements(a))
			// NB: If the L & G extension is not retained, then the
			// loop over neighbors of uv should be terminated here
			// as soon as hard_squeeze reaches placements.
			hard_squeeze +=
			    (*displacements)[joint_class][a->class_id];
		}
	    }
	}
#ifdef METER
    gettimeofday(&end_time, NULL);
    briggs_test_time +=
	((long)end_time.tv_sec  - start_time.tv_sec) * 1000000 +
	((long)end_time.tv_usec - start_time.tv_usec);
#endif // METER
    if (hard_squeeze >= placements)
    {
	m->hi = hard_squeeze - (placements - 1);	// L & G extension
	return false;
    }
    u->class_id = joint_class;
#ifndef IG_INCLUDES_HR
    u->excluded = excluded;
#endif
    return true;
}

RagaNode*
Raga::get_alias(RagaNode *n)
{
    if (n->state != COALESCED)
	return n;
    return get_alias(n->alias);
}

void
Raga::combine(RagaNode *u, RagaNode *v)
{
#ifdef METER
    ++combine_count;

    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
#endif // METER
    if (v->state == FREEZE_WORK)
	remove_node(v, freeze_worklist, FREEZE_WORK);
    else
	remove_node(v, spill_worklist, SPILL_WORK);
    push_node(v, coalesced_nodes, COALESCED);

    v->alias = u;

    bool u_precolored = u->state == PRECOLORED;

    while (MoveInfo *vm = pop_node_move(v)) {
	if (u_precolored)				// L & G ..
	    if (vm->node[0]->state != PRECOLORED ||	// .. extension
		vm->node[1]->state != PRECOLORED)	// ..
		vm->hi = 0;				// not weight-ratio sum

	// We maintain the invariant that for every move such as vm, the node[i]
	// fields are not coalesced nodes, i.e., their alias fields are null.
	//
	// When precolored registers are being omitted from the interference
	// graph, we don't associate moves with their nodes, since they have no
	// neighbors to take advantage of such move sets.  This can leave
	// certain moves stranded on the active list, but that is easily fixed
	// in function Main, since the stranded moves only involve precolored
	// registers.

	for (int link_index = 0; link_index < 2; ++link_index)
	    if (vm->node[link_index] == v) {
		vm->node[link_index] =  u;
#ifndef IG_INCLUDES_HR
		if (!u_precolored)
#endif
		    add_node_move(u, vm, link_index);
	    }
    }
    bool u_significant = u->squeeze >= node_placements(u);	// L & G ..
    bool v_significant = v->squeeze >= node_placements(v);	// .. extension

    // Difference from L & G pseudocode:
    //
    // If v is significant and u is precolored, some neighbor of v may be losing
    // a significant neighbor, so enable_moves_once must be applied to v's
    // neighbors.  Leung & George postpone that till after edges have been
    // transferred to u.  But that's too late if u is precolored, since u will
    // have an empty adjacency list.  So for significant v and precolored u, try
    // enabling moves for v's neighbors now.

    if (v_significant && u_precolored)			// L & G ..
	enable_moves_adjacent(v);			// .. extension

    map_adjacent(v, &Raga::transfer_edge);

    if (u->squeeze >= node_placements(u) && u->state == FREEZE_WORK) {
	remove_node(u, freeze_worklist, FREEZE_WORK);
	push_node(u, spill_worklist, SPILL_WORK);
    }

    // Difference from L & G pseudocode:
    //
    // Leung & George would try to enable moves for the neighbors of coalescee
    // uv when both its constituents u and v are significant.  But even assuming
    // that u is not precolored (see note above) this could leave an active move
    // stranded.  Suppose u was significant before the merge, but v was not, and
    // suppose u had a neighbor t with an active move t <-> r.  If u had no
    // conflict with r, then it would inhibit enabling t's move.  But if v
    // supplies a conflict with r to the coalescee uv, then t <-> r gets left
    // with an incorrectly large `hi' value.  So we enable uv's neighbors' moves
    // if u was significant, even if v was not.  And since this argument works
    // with u and v reversed, we do the same if v was significant, even if u was
    // not.

    if (!u_precolored && (u_significant || v_significant)) // L & G ..
	enable_moves_adjacent(u);			// .. extension
#ifdef METER
    gettimeofday(&end_time, NULL);
    combine_time +=
	((long)end_time.tv_sec  - start_time.tv_sec) * 1000000 +
	((long)end_time.tv_usec - start_time.tv_usec);
#endif // METER
}

void
Raga::transfer_edge(RagaNode *t, RagaNode *v)
{
    RagaNode *u = v->alias;
#ifndef NO_LEUNG_GEORGE
    // Calling add_edge(t, u) when u is precolored can change t's influence on
    // moves related to the neighbors of t that also involve precolered nodes.
    // If t was already significant before the edge was added, then it may have
    // inhibited such moves under the George test.  However, t's newly acquired
    // conflict with u may relax the constraint on such moves if their hard
    // register operands overlap with u.  To cover this case, we call
    // enable_moves_adjacent, passing a flag that tells it only to check
    // HR-related moves.

    // In the !IG_INCLUDES_HR case, add_edge(t, u) doesn't really add an
    // edge if u is precolored, since u isn't in the graph, but the check
    // is needed anyway.

    bool t_significant = t->squeeze >= node_placements(t);
    if (add_edge(t, u) && u->state == PRECOLORED) {
	if (t_significant)
	    enable_moves_adjacent(t, true);		// L & G extension
    }
#else // NO_LEUNG_GEORGE
    add_edge(t, u);
#endif // NO_LEUNG_GEORGE
    decrement_degree(t, v);
}

void
Raga::freeze()
{
    RagaNode *u = freeze_worklist;
    remove_node(u, freeze_worklist, FREEZE_WORK);

    claim(u->squeeze < node_placements(u));

    push_node(u, simplify_worklist, SIMPLIFY_WORK);
    freeze_moves(u);
}

void
Raga::freeze_moves(RagaNode *u)
{
    while (MoveInfo *m = pop_node_move(u)) {
	remove_move(m, active_moves, ACTIVE);

	RagaNode *w = m->node[0];
	RagaNode *v = (w == u) ? m->node[1] : w;
	claim(u->alias == NULL && v->alias == NULL);

	remove_node_move(v, m);
	push_move(m, frozen_moves, FROZEN);

	if (!move_related(v) && (v->squeeze < node_placements(v))) {
	    remove_node(v, freeze_worklist, FREEZE_WORK);
	    push_node(v, simplify_worklist, SIMPLIFY_WORK);
	}
    }
}

void
Raga::select_spill()
{
#ifdef METER
    ++select_spill_count;

    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
#endif // METER
    double least = INT_MAX;
    RagaNode *m = NULL,  *t = spill_worklist;

    do {
#ifdef METER
	++test_spill_count;
#endif
	float adjusted_spill_cost = t->frequency / t->antisqueeze;
	if (adjusted_spill_cost < least) {
	    least = adjusted_spill_cost;
	    m = t;
	}
    } while ((t = t->next));

    claim(m, "select_spill() -- No acceptable spill candidate");

    remove_node(m, spill_worklist, SPILL_WORK);
    push_node(m, simplify_worklist, SIMPLIFY_WORK);
    freeze_moves(m);
#ifdef METER
    gettimeofday(&end_time, NULL);
    select_spill_time +=
	((long)end_time.tv_sec  - start_time.tv_sec) * 1000000 +
	((long)end_time.tv_usec - start_time.tv_usec);
#endif // METER
}

void
Raga::assign_colors()
{
    if_debug(7)
	fprintf(stderr, "\nColor assignments:\n");

    RagaNode *n;		// current candidate
    NatSetDense excluded;	// registers ruled out by interference

    while ((n = pop_node(select_stack, SELECT_STACKED))) {
#ifndef IG_INCLUDES_HR
	excluded = n->excluded;
#else
	excluded.remove_all();
#endif

	NodeListHandle nit = n->adj_list.begin();
	for ( ; nit != n->adj_list.end(); ++nit) {
	    RagaNode *w = *nit;
	    RagaNode *a = get_alias(w);
	    if (a->state == COLORED || a->state == PRECOLORED)
		excluded += (*aliases_in_class)[a->color][n->class_id];
	}

	RegClassId class_id = n->class_id;			// feasible set
	const NatSet *temp = reg_caller_saves();		// 1st choice
	const NatSet *save = reg_callee_saves();		// fall-back
	bool rotate = !min_caller_saved;			// round robin
	int color;
	if ((color = reg_choice(class_id, temp, &excluded, rotate)) >= 0
	 || (color = reg_choice(class_id, save, &excluded, false )) >= 0)
	{   push_node(n, colored_nodes, COLORED);
	    n->color = color;
	    if_debug(7)
		print_assignment(stderr, n, color);
	} else {
	    push_node(n, spilled_nodes, SPILLED);
	}
    }
    if (!spilled_nodes)
	propagate_alias_colors();
}

/*
 * propagate_alias_colors
 *
 * For every coalesced node n that is now merged into an "alias" node that
 * has a color, color n by propagating the color of its alias.
 *
 * This can occur even in just_coalesce mode, when a node is coalesced into
 * a precolored node.
 */
void
Raga::propagate_alias_colors()
{
    while (RagaNode *n = pop_node(coalesced_nodes, COALESCED)) {
	RagaNode *a = get_alias(n);
	if (a->state == PRECOLORED || a->state == COLORED) {
	    n->color = a->color;
	    push_node(n, colored_nodes, COLORED);
	}
    }
}


/* rewrite_program() -- As in the paper, discard all coalesced moves
 * if there's any spillage.  FIXME: G & A point out that coalesceable
 * moves detected before any spills occur can be eliminated before the
 * next round of coloring.  Get measurements, then implement that and
 * see how much it helps.
 * Note that a spilled symbol here becomes a manifest memory reference,
 * since it goes into an effective address operand.  This prevents its
 * being seen again when the interference graph is rebuilt.
 */
void
Raga::rewrite_program()
{
    RagaNode *v = spilled_nodes;

    do {
	if (!is_var(v->opnd)) {		// Create a symbol for a spilled VR
	    VarSym *s = new_unique_var(get_type(v->opnd));
	    v->opnd = opnd_var(s);
	}
    } while ((v = v->next));

    /* Insert spill code and add the new temp nodes to list `initial'.
     * NB: Leave spilled nodes' state set to SPILLED while doing so.
     */
    scan_edit_code(this, unit_cfg, &Raga::clear_current_spillees, // FIXME
		   &Raga::register_temps_and_classify_after_spill,
		   &Raga::has_reg_candidate_traits,
		   &Raga::replace_and_maybe_spill,
		   &Raga::replace_and_maybe_spill,
		   NULL, NULL);
    clear_current_spillees();			// FIXME: no longer needed

#ifdef NEIGHBOR_CENSUS
    Vector<int> empty(class_count, 0);
#endif

    while ((v = pop_node(spilled_nodes, SPILLED))) {
	v->squeeze = 0;
	v->antisqueeze = 0.0;
#ifdef NEIGHBOR_CENSUS
	if (neighbor_census)
	    v->class_degree = empty;
#endif
	clear(v->adj_list);
    }
    while ((v = pop_node(colored_nodes, COLORED))) {
	push_node(v, initial, INITIAL);
	v->squeeze = 0;
	v->antisqueeze = 0.0;
#ifdef NEIGHBOR_CENSUS
	if (neighbor_census)
	    v->class_degree = empty;
#endif
	clear(v->adj_list);
    }
    while ((v = pop_node(coalesced_nodes, COALESCED))) {
	push_node(v, initial, INITIAL);
	v->squeeze = 0;
	v->antisqueeze = 0.0;
#ifdef NEIGHBOR_CENSUS
	if (neighbor_census)
	    v->class_degree = empty;
#endif
	clear(v->adj_list);
	v->movee_index = -1;
	v->alias = NULL;
    }
    MoveInfo *mv;
    while ((mv = pop_move(coalesced_moves, COALESCED)))
	delete mv;
    while ((mv = pop_move(frozen_moves, FROZEN)))
	delete mv;
    while ((mv = pop_move(constrained_moves, CONSTRAINED)))
	delete mv;
}

/*
 * George and Appel call this routine `finalize'.  It is called to complete
 * the work of the pass.  It eliminates unneeded move instructions, it
 * substitutes final values for register candidate operands and it cleans
 * up various worklists.
 *
 * When the just_coalesce flag is in effect, substitution means replacing a
 * candidate by its "alias" after coalescing.  For normal allocation, the
 * substitution replaces each candidate by its assigned hard register.
 */
void
Raga::finish()
{
    MoveInfo *mv;

    while ((mv = pop_move(coalesced_moves, COALESCED))) {
	eliminate_move_instr(mv);
	delete mv;
    }

    while ((mv = pop_move(frozen_moves, FROZEN))) {
#if !defined(NO_THAW)		/* Delete frozen moves if coloring happened to
				 * make them nop's.  (Not done by G & A.) */
	if ((mv->node[0]->state == PRECOLORED || mv->node[0]->state == COLORED)
	&&  (mv->node[1]->state == PRECOLORED || mv->node[1]->state == COLORED)
	&&  mv->node[0]->color == mv->node[1]->color)
	  eliminate_move_instr(mv);
#endif
	delete mv;
    }

    while ((mv = pop_move(constrained_moves, CONSTRAINED))) {
	assert (mv->node[0]->state != COLORED
	     || mv->node[1]->state != COLORED
	     || mv->node[0]->color != mv->node[1]->color);
	delete mv;
    }

    // Promote any stack-passed formal parameters that have been assigned
    // to registers.
    maybe_load_params();

    Opnd (Raga::* substitute)(Opnd, bool, InstrHandle) =
	just_coalesce ? &Raga::substitute_alias : &Raga::substitute_register;

    scan_edit_code(this, unit_cfg, NULL, NULL, &Raga::has_reg_candidate_traits,
		   substitute, substitute, NULL, NULL);

    // At this point, the only non-empty RagaNode lists are colored_nodes
    // and (if just_coalesce is set) spilled_nodes.

    RagaNode *v;
    while ((v = pop_node(colored_nodes, COLORED))) {
	/*			// FIXME: why was this needed?
	    if (is_var(v->opnd))
		vsym_set_hreg(get_var(v->opnd), color_to_reg[v->color]);
	*/
	clear(v->adj_list);
    }
    while ((v = pop_node(spilled_nodes, SPILLED)))
	clear(v->adj_list);
}

// FIXME: no longer needed.
void
Raga::clear_current_spillees(Instr*)
{
    claim(current_spillees.empty());
}

/*
 * classify_after_spill
 *
 * If instr has undergone one or more spill replacements, reclassify it so the
 * new temps will be covered correctly by the class table
 *
 * Also empty the current_spillees list and clear each temp field of the
 * RagaNodes therein.  The temp field should only remain in effect for one
 * instruction.  This function must be called between instructions during spill
 * code insertion so that a spilled variable gets different point-lifetime temps
 * for different instructions.
 */
void
Raga::classify_after_spill(Instr *instr)
{
    if (!current_spillees.empty()) {
	reg_classify(instr, opnd_catalog, &opnd_classes);

	do {
	    current_spillees.front()->temp = NULL;
	    current_spillees.pop_front();
	} while (!current_spillees.empty());
    }
}

/* replace_and_maybe_spill() -- Spill a register candidate marked SPILLED.
 */
Opnd
Raga::replace_and_maybe_spill(Opnd opnd, bool is_src, InstrHandle h)
{
    int orig_id = var_or_reg_node_id(opnd);
    claim(orig_id >= 0);

    RagaNode *n = node_table[orig_id];

    if (n->state != SPILLED)
	return opnd;		// Spared for now

    return replace_and_spill(n, is_src, h, true);
}

/*
 * Spill operand to memory.  Create a new VR to replace it or reuse one already
 * created for the current instruction.  Insert a fill/spill sequence to
 * load/store this temp VR from/to memory, and classify the new instructions.
 *
 * Arguments:
 * o  Candidate node n represents the var or virtual reg being spilled.
 * o  Boolean is_src is true iff the spillee is an input operand.
 * o  Handle h identifies the instruction containing the spillee.
 * o  Flag true_spill holds for true candidates, but not for pre-spilled vars.
 *
 * Results:
 * o  The temp-VR operand replacing the spillee in the current instruction.
 */
Opnd
Raga::replace_and_spill(RagaNode *n, bool is_src, InstrHandle h,
			bool true_spill)
{
    VarSym *memory_var = get_var(n->opnd);
    Opnd memory_addr = opnd_addr_sym(memory_var);

    // The spilled node's `temp' field is null unless it has already been a
    // spillee in the current instruction.  In that case, we reuse the temp
    // register already generated.  Otherwise, we generate a new point lifetime.

    Opnd reg_opnd = (n->temp ? n->temp->opnd : opnd_reg(get_type(memory_var)));

    // Insert spill/fill instructions and classify them.  (The original
    // instruction gets reclassified after being rewritten with temps.)
    // Optionally count spill events and decorate inserted code with
    // "spill_code" annotations.

    InstrHandle b, e;		// bracket the inserted spill/fill sequence

    if (is_src)
	{ b = reg_fill (reg_opnd, memory_addr, e = h); }
    else
	{ e = reg_spill(memory_addr, reg_opnd, b = h); ++e; }

    for ( ; b != e; ++b) {
	reg_classify(*b, opnd_catalog, &opnd_classes);
	scan_edit_opnds(this, b, &Raga::gather_reg);
	if (true_spill && note_spills)
	    set_note(*b, k_spill_code, note_flag());
    }

    if_debug(2) {
	if (is_src)
	    spill_load_count++;
	else
	    spill_store_count++;
    }

    // If a new, point-lifetime temporary has just been generated, enter it in
    // node_table and give it a register-class set at least as constrained as
    // that of the spillee.  (NB: this can't be done earlier: reg_classify needs
    // to see new operands before they are entered in node_table.)  Per
    // Chaitin's original heuristic: the new temp gets infinite spill cost to
    // avoid letting it be spilled itself.

    if (n->temp == NULL)  {
	n->temp = record_reg(reg_opnd);
	n->temp->frequency = INT_MAX;			// infinite spill cost
	if (true_spill) { // FIXME: no longer needed; reg_classify does this
	    opnd_classes[n->temp->id] =			// constrain reg classes
		reg_class_intersection(opnd_classes[n->temp->id],
				       opnd_classes[n->id]);
	}
	current_spillees.push_front(n);			// current-instr spills
    }
    return reg_opnd;
}

/*
 * register_temps_and_classify_after_spill
 *
 * Point-lifetime temps that have been generated during spilling have to be
 * entered in the `initial' set for the next round of coloring.  Just after an
 * instruction is rewritten, the current_spillees list holds the spilled nodes
 * for the instruction, and their temp fields hold the temps that need to be
 * registered.  Insert each of these in the `initial' set, then call
 * classify_after_spill, which eventually clears current_spillees.
 */
void
Raga::register_temps_and_classify_after_spill(Instr *instr)
{
    if (!current_spillees.empty())
    {
	for (NodeListHandle h = current_spillees.begin();
	     h != current_spillees.end();
	     ++h)
	{
	    RagaNode *temp = (*h)->temp;
	    push_node(temp, initial, INITIAL);
	}

	classify_after_spill(instr);
    }
}

/*
 * Insert loads for parameters that came in on the stack and were assigned
 * to a register.  Put them right after the proc_entry marker in the first
 * code block.  Give each load a param_init annotation.
 */
void
Raga::maybe_load_params()
{
    CfgNode *entry = get_entry_node(unit_cfg);
    CfgNodeHandle sh = succs_start(entry);

    claim(sh != succs_end(entry) && !is_impossible_succ(entry, *sh));

    CfgNode *block = *sh;
    InstrHandle marker = end(block);

    for (InstrHandle ih = start(block); ih != end(block); ++ih)
	if (has_note(*ih, k_proc_entry)) {
	    marker = ih;
	    break;
	}
    claim(marker != end(block), "No proc_entry marker in first code block");

    // reg_fill inserts _before_ the marker given to it.  In the unlikely
    // event that the proc_entry instr ends its block, we insert a dummy
    // for to reg_fill to insert before.

    if (after(marker) == end(block))
	marker = insert_after(block, marker, new_instr_dot(opcode_null));
    else
	++marker;

    for (int i = 0; i < get_formal_param_count(cur_unit); ++i) {

	VarSym *p = get_formal_param(cur_unit, i);
	int pid;

	if ((pid = var_node_id(p)) < 0 ||
	    !liveness->in_set(block)->contains(pid))	// Don't init a ..
	    continue;					// .. DOA parameter

	int reg = var_or_reg_assigned_reg(node_table[pid]->opnd);

	// Parameter p was promoted from memory to register reg.  Use reg_fill
	// to insert a sequence before marker that loads p's value to reg.

	if (reg >= 0 && !is_reg_param(p)) {
	    Opnd dst = opnd_reg(reg, get_type(p));
	    Opnd src = opnd_addr_sym(p);
	    InstrHandle first = reg_fill(dst, src, marker, true);	// no VR
	    if (first != marker)
		set_note(*first, k_param_init, note_flag());
	}
    }
}

Opnd
Raga::substitute_register(Opnd opnd, bool, InstrHandle)
{
    int reg = var_or_reg_assigned_reg(opnd);
    claim(reg >= 0);

    return opnd_reg(reg, get_type(opnd));
}

/*
 * For use when the just_coalesce is in effect: replace a register-
 * candidate operand by one for the register or candidate into which it has
 * been coalesced, if any.  (In G&A jargon, this is its alias.)  Otherwise,
 * return its own operand.
 *
 * When the alias is a hard register, take the type of the returned operand
 * from the original, since hard-register nodes don't record types.
 */
Opnd
Raga::substitute_alias(Opnd opnd, bool, InstrHandle)
{
    int node_id = var_or_reg_node_id(opnd);
    claim(node_id >= 0);

    Opnd alias = get_alias(node_table[node_id])->opnd;
    if (is_hard_reg(alias))
	alias = opnd_reg(get_reg(alias), get_type(opnd));
    return alias;
}

/*
 * eliminate_move_instr() -- Eliminate a move instruction.
 * FIXME:  Take care to preserve a k_cycle_count annotation by
 * moving it to the immediately preceding active instruction
 * within the same block.  But if there is no such active
 * instruction, just discard the note.
 */
void
Raga::eliminate_move_instr(MoveInfo *mv)
{
    CfgNode *cnode = to<CfgNode>((*mv->handle)->get_parent());

    delete remove(cnode, mv->handle);
}

void
Raga::add_node_move(RagaNode *n, MoveInfo *mv, int link_index)
{
#ifndef IG_INCLUDES_HR
    if (n->state == PRECOLORED)
	return;
#endif
    if (n->movee_index < 0) {
	n->movee_index = move_mentioned_count++;
	if (move_mentioned_count > node_moves[0].size()) {
	    node_moves[0].push_back(NULL);
	    node_moves[1].push_back(NULL);
	}
    }
    MoveInfo *next = node_moves[link_index][n->movee_index];
    if (next != NULL)
	next->link[link_index].prev = mv;
    mv->link[link_index].next = next;
    mv->link[link_index].prev = NULL;
    node_moves[link_index][n->movee_index] = mv;
}

MoveInfo*
Raga::pop_node_move(RagaNode *n)
{
    claim(n->state != PRECOLORED);

    if (n->movee_index < 0)
	return NULL;

    claim((size_t)n->movee_index < node_moves[0].size());

    int empty_list_count = 0;
    MoveInfo *result = NULL;

    for (int link_index = 0; link_index < 2; ++link_index) {
	MoveInfo *m = node_moves[link_index][n->movee_index];

	if (m == NULL) {
	    ++empty_list_count;
	} else if (result != NULL) {
	    empty_list_count +=
		(int)(node_moves[link_index][n->movee_index] == NULL);
	} else {
	    claim(m->link[link_index].prev == NULL);
	    MoveInfo *next = m->link[link_index].next;
	    node_moves[link_index][n->movee_index] = next;
	    if (next == NULL)
		++empty_list_count;
	    else
		next->link[link_index].prev = NULL;
	    if_debug(1)
		m->link[link_index].prev = m->link[link_index].next = NULL;
	    result = m;

	    if (m->node[0] == m->node[1])
		unlink_node_move(n, m, (link_index + 1) & 1);
	}
    }
    if (empty_list_count == 2)
	n->movee_index = -1;

    claim(result);
    return result;
}

bool
Raga::unlink_node_move(RagaNode *n, MoveInfo *mv, int link_index)
{
    bool empty;

    MoveInfo *prev = mv->link[link_index].prev;
    MoveInfo *next = mv->link[link_index].next;

    if (prev == NULL) {
	claim(node_moves[link_index][n->movee_index] == mv);
	node_moves[link_index][n->movee_index] = next;
	empty = (int)(next == NULL);
    } else {
	prev->link[link_index].next = next;
	empty = false;
    }
    if (next != NULL)
	next->link[link_index].prev = prev;
    if_debug(1)
	mv->link[link_index].prev = mv->link[link_index].next = NULL;

    return empty;
}

void
Raga::remove_node_move(RagaNode *n, MoveInfo *mv)
{
#ifndef IG_INCLUDES_HR
    if (n->state == PRECOLORED)
	return;
#endif
    claim((size_t)n->movee_index < node_moves[0].size());

    int empty_list_count = 0;

    for (int link_index = 0; link_index < 2; ++link_index)
	empty_list_count +=
	    (n == mv->node[link_index])
		? (int)unlink_node_move(n, mv, link_index)
		: (int)(node_moves[link_index][n->movee_index] == NULL);

    if (empty_list_count == 2)
	n->movee_index = -1;
}

void
Raga::push_move(MoveInfo *mv, MoveInfo *&list, WorkState state)
{
    claim(!mv->next && !mv->prev);

    mv->state = state;
    if (list) {
	claim(!list->prev);
	list->prev = mv;
	mv->next = list;
    }
    list = mv;
}

MoveInfo*
Raga::pop_move(MoveInfo *&list, WorkState state)
{
    if (!list)
	return NULL;

    MoveInfo *mv = list;
    claim(mv->state == state && !list->prev);

    if ((list = list->next))
	list->prev = NULL;

    mv->next = NULL;
    return mv;
}

void
Raga::remove_move(MoveInfo *mv, MoveInfo *&list, WorkState state)
{
    if (mv == list)
	pop_move(list, state);
    else {
	claim(list && mv->state == state);

	mv->prev->next = mv->next;
	if (mv->next)
	    mv->next->prev = mv->prev;
	mv->next = mv->prev = NULL;
    }
}

/* FIXME: push_node, pop_node, and remove_node are too similar to
 *        push_move, pop_move, and remove_move.  Use template functions. */
void
Raga::push_node(RagaNode *n, RagaNode *&list, WorkState state)
{
    claim(!n->next && !n->prev);

    debug(15, "push_node(%d, %s)", n->id, state_name[state]);

    n->state = state;
    if (list) {
	claim(!list->prev);
	list->prev = n;
	n->next = list;
    }
    list = n;
}

RagaNode*
Raga::pop_node(RagaNode *&list, WorkState state)
{
    if (!list)
	return NULL;

    RagaNode *n = list;
    claim(n->state == state && !list->prev);

    debug(15, "pop_node(%d, %s)", n->id, state_name[state]);

    if ((list = list->next))
	list->prev = NULL;

    n->next = NULL;
    return n;
}

void
Raga::remove_node(RagaNode *n, RagaNode *&list, WorkState state)
{
    if (n == list)
	pop_node(list, state);
    else {
	claim(list && n->state == state);

	debug(15, "remove_node(%d, %s)", n->id, state_name[state]);

	n->prev->next = n->next;
	if (n->next)
	    n->next->prev = n->prev;
	n->next = n->prev = NULL;
    }
}

int
Raga::var_or_reg_assigned_reg(Opnd opnd)
{
    int node_id = var_or_reg_node_id(opnd);
    if (node_id < 0)
	return -1;

    RagaNode *e = node_table[node_id];
    return (e->state == COLORED) ? e->color : -1;
}


//********************  Debugging utilities  *********************

/*
 * Print a line for each VR or LV:
 * node_id: squeeze { adjacent node ids } \\ { excluded registers }
 */
void
Raga::print_interferences(FILE *fp)
{
    fprintf(fp, "\nInterferences:\n");
    NatSetDense adjacent;

    for (size_t i = 0; i < node_count; ++i) {
	RagaNode *n = node_table[i];

	if (n == NULL || (n->squeeze == 0 && n->adj_list.empty()))
	    continue;

	fprintf(fp, "%5u:%4d {", (unsigned)i, n->squeeze);

	adjacent.remove_all();

	NodeListHandle nit = n->adj_list.begin();
	for ( ; nit != n->adj_list.end(); ++nit)
	    adjacent.insert((*nit)->id);

	print_nat_set(fp, &adjacent, node_count);

	fprintf(fp, "}");
#ifndef IG_INCLUDES_HR

	if (n->state != PRECOLORED) {
	    fprintf(fp, " \\ ");
	    n->excluded.print(fp);
	}

#endif
	fprintf(fp, "\n");
    }
}

void
Raga::print_node_table_entry(FILE *fp, int i)
{
    fprintf(fp, "%5d: ", i);

    RagaNode *n = node_table[i];

    if (n == NULL) {
	fputs("<null>\n", fp);
    }
    else if (n->state == PRECOLORED) {
	int reg = get_reg(n->opnd);
	fprintf(fp, "$hr%-2d", reg);
	fprintf(fp, " (%s)", reg_name(reg));
	putc('\n', fp);
    } else {
	putc('<', fp);
	fprint(fp, get_type(n->opnd));

	if (is_var(n->opnd))
	    fprintf(fp, ">\t%s\n", get_name(get_var(n->opnd)).chars());
	else
	    fprintf(fp, ">\t$vr%d\n", get_reg(n->opnd));
    }
}

void
Raga::print_node_table_segment(FILE *fp, int begin, int end, char *head)
{
    if (begin < end)
	fprintf(fp, "\n%s:\n", head);
    for (int i = begin; i < end; ++i)
	print_node_table_entry(fp, i);
}

void
Raga::print_basic_blocks(FILE *fp)
{
    fprintf(fp, "\nBasic blocks:\n");

    NatSetDense live_set;
    target_printer()->set_omit_unwanted_notes(true);

    for (CfgNodeHandle h = nodes_start(unit_cfg);
	 h != nodes_end(unit_cfg); ++h) {

	CfgNode *cnode = *h;

	fprintf(fp, "Block %u  -----------------------------------------\n",
		get_number(cnode));

	live_set.remove_all();
	live_set += *liveness->in_set(cnode);

	fprintf(fp, "Live coming in: ");
	print_nat_set(fp, &live_set, node_count);
	putc('\n', fp);

	InstrHandle h = start(cnode);		// move forward thru instrs
	for (int i = 0; i < size(cnode); ++i, ++h) {
	    fprintf(fp, "[%lx]: ", (unsigned long)*h);
	    fprint (fp, *h);
	}

	live_set.remove_all();
	live_set += *liveness->out_set(cnode);

	fprintf(fp, "Live going out: ");
	print_nat_set(fp, &live_set, node_count);
	putc('\n', fp);
   }
}

void
Raga::print_nat_set(FILE *fp, NatSet *ns, int limit, int (*translate)(int))
{
    NatSetIter nsi = ns->iter();
    int e1 = limit, e2;
    char separator = '\0';

    for ( ; nsi.is_valid() && (e2 = nsi.current()) < limit; nsi.next()) {
	if (translate)
	    e2 = translate(e2);
	if ((separator != '-' && e1 < limit)
	 || (separator == '-' && e1 != e2 - 1)) {
	    fprintf(fp, "%.1s%d", &separator, e1);
	    separator = (e1 == e2 - 1) ? '-' : ',';
	}
	e1 = e2;
    }
    if (e1 < limit)
	fprintf(fp, "%.1s%d", &separator, e1);
}

void
Raga::print_assignment(FILE *fp, RagaNode *n, int color)
{
    fprintf(fp, "%2d => ", color);
    print_node_table_entry(fp, n->id);
}

void
Raga::print_coalesced_nodes(FILE *fp)
{
    if (!coalesced_nodes)
	return;

    fprintf(fp, "\nCoalesced nodes:\n");

    NatSetDense coalesced;

    RagaNode *n = coalesced_nodes;
    do
	coalesced.insert(n->id);
    while ((n = n->next));

    size_t i;
    NatSetIter nsi = coalesced.iter();
    for ( ; nsi.is_valid() && (i = nsi.current()) < node_count; nsi.next())
	fprintf(fp, "%5u:%6d\n", (unsigned)i, get_alias(node_table[i])->id);
}

#ifdef METER
void
Raga::print_coalesce_counts()
{
    MoveInfo **roots[] =
	{ &coalesced_moves, &constrained_moves, &frozen_moves, NULL };

    fprintf(stderr, "\nCoalesce attempts for moves:\n");

    for (MoveInfo ***pl = roots; *pl != NULL; ++pl)
	for (MoveInfo *pm = **pl; pm != NULL; pm = pm->next)
	    fprintf(stderr, "%5d  <->  %5d:%8ld\n",
		    pm->node[0]->id, pm->node[1]->id, pm->coalesce_count);
}
#endif

#ifdef NEIGHBOR_CENSUS
void
Raga::validate_census()
{
    for (size_t i = 0; i < node_count; ++i) {
	RagaNode *n = node_table[i];
	Vector<int> class_degree(class_count, 0);

	NodeListHandle it = n->adj_list.begin();
	for ( ; it != n->adj_list.end(); ++it)
	    class_degree[(*it)->class_id]++;
	claim(class_degree == n->class_degree,
	      "Node %d has incorrect class_degree map", n->id);
    }
}
#endif
