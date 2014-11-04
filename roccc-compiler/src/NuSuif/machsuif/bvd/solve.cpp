/* file "bvd/solve.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "bvd/solve.h"
#endif

#include <machine/machine.h>
#include <cfg/cfg.h>

#include <bvd/flow_fun.h>
#include <bvd/solve.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

Bvd::Bvd(Cfg *graph, direction dir, confluence_rule rule,
	 FlowFun *entry_flow, FlowFun *exit_flow,
	 int num_slots_hint)
{
    _graph = graph;
    _dir = dir;
    _rule = rule;
    _entry_flow = entry_flow;
    _exit_flow = exit_flow;
    _num_slots_hint = num_slots_hint;

    _before = (_dir == forward) ? &_in  : &_out;
    _after  = (_dir == forward) ? &_out : &_in;
}

/*
 * "Combine" the data-flow on the "after" side of the "data-flow
 * predecessors" of node n.  The meanings of "combine" and "after", and the
 * identification of "data-flow predecessors", depend on the problem
 * direction and the confluence rule.
 */
void 
Bvd::combine_flow(CfgNode *n)
{
    int nnum = get_number(n);
    CfgNodeHandle dfph   = (_dir == forward) ? preds_start(n) : succs_start(n);
    CfgNodeHandle dfpend = (_dir == forward) ? preds_end(n)   : succs_end(n);

    NatSetDense &before_n = (*_before)[nnum];

    switch (_rule) {
      case all_paths:			// intersection
	before_n.insert_all();
	for ( ; dfph != dfpend; ++dfph)
	    before_n *= (*_after)[get_number(*dfph)];
	break;

      case any_path:			// union
	before_n.remove_all();
	for ( ; dfph != dfpend; ++dfph)
	    before_n += (*_after)[get_number(*dfph)];
	break;
    }
}

/*
 * Bvd::build_local_info -- Build flow function for a CFG node.
 *
 * For entry and exit nodes, use a special flow if one has been provided.
 * Otherwise use the identity function.
 */
void  
Bvd::compute_local_flow(int i)
{
    CfgNode *node = get_node(_graph, i);

    if      (node == get_entry_node(_graph) && _entry_flow != NULL)
	_flow[i] = *_entry_flow;
    else if (node == get_exit_node(_graph) && _exit_flow  != NULL)
	_flow[i] = *_exit_flow;
    else
	_flow[i] = FlowFun(_num_slots_hint);	// identity function

    // Adjust _flow[i] to reflect kill's and gen's in the node's instructions

    InstrHandle h = (_dir == forward) ? start(node) : last(node);
    for (int c = size(node); c-- > 0; (_dir == forward) ? ++h : --h) {
	Instr *instr = *h;

	find_kill_and_gen(instr);

	// remove kills
	for (NatSetIter kit(kill_set()->iter()); kit.is_valid(); kit.next())
	    _flow[i].set_to_bottom(kit.current());

	// add gens
	for (NatSetIter git(gen_set()->iter()); git.is_valid(); git.next())
	    _flow[i].set_to_top(git.current());
    }

    // Initialize _before[i] to empty or universe, depending on confluence.
    // Initialize _after [i] to the result of applying _flow[i] to _before[i]

    bool init_to_universe = (_rule == all_paths);
    (*_before)[i] = NatSetDense(init_to_universe, _num_slots_hint);
    (*_after) [i] = (*_before)[i];
    _flow[i].apply((*_after)[i]);
}


bool 
Bvd::solve(int iteration_limit)
{
    int iterations = 0;

    int node_count = nodes_size(_graph);
    _flow.resize(node_count);
    _in.resize(node_count);
    _out.resize(node_count);

    // build local flow information and allocate bit vectors
    for (int i = 0; i < node_count; i++)
	compute_local_flow(i);
    
    /* Initialize to solve bit vector equations by iteration. */
    bool changes = true;
    
    /*
     * Initialize traversal list in reverse postorder, using the forward
     * CFG for forward problems, and the reverse graph for backward ones.
     * (See the dragon book [ASU], section 10.10.)
     */


    CfgNodeListRpo rpo(_graph, _dir == forward);
    /*
     * Iterate to fixed-point or until we exceed iteration_limit.
     *
     * Should use work list here rather than looping over all nodes all the
     * time.  Work list is an ordered set of rpo numbers, so that we always
     * pick the earliest ready node in rpo order.
     */

    while (changes && (iterations < iteration_limit)) {
	iterations++;
	changes = false;

	for (CfgNodeHandle h = rpo.start(); h != rpo.end(); ++h) {
	    CfgNode *n = *h;
	    int nnum = get_number(n);
	    
	    combine_flow(n);			// create new "before" value
	    NatSetDense bv = (*_before)[nnum];	// copy new "before" value
	    _flow[nnum].apply(bv);		// transform it to reflect node
	    
	    if (bv != (*_after)[nnum]) {
		(*_after)[nnum] = bv;		// changed: update "after" bv
		changes = true;
	    }
	}
    }
    _flow.resize(0);				// discard flow functions
    return (iterations < iteration_limit);
}

/*
 * Public methods for testing the results of a bit vector analysis.
 */

const NatSet*
Bvd::in_set(CfgNode *n) const
{
    return &_in[get_number(n)];
}

const NatSet*
Bvd::out_set(CfgNode *n) const
{
    return &_out[get_number(n)];
}

void
Bvd::print(CfgNode *n, FILE *fp) const
{
    fputs("in:   ", fp);
    _in[get_number(n)].print(fp, num_slots());

    fputs("out:  ", fp);
    _out[get_number(n)].print(fp, num_slots());
}
