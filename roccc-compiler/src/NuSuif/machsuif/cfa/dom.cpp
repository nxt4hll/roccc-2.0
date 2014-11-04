/* file "cfa/dom.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "cfa/dom.h"
#endif

#include <machine/machine.h>
#include <cfg/cfg.h>

#include <cfa/dom.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

enum { REVERSE = false, FORWARD = true };	// indicators of CFG orientation


DominanceInfo::DominanceInfo(Cfg *graph)
{
    _graph = graph;
    _doms = _pdoms = _df = _rdf = NULL;
    _idom = _ipdom = NULL;
}

DominanceInfo::~DominanceInfo()
{
    delete [] _doms;
    delete [] _pdoms;
    delete [] _idom;
    delete [] _ipdom;
    delete [] _df;
    delete [] _rdf;
}

/*
 *  Find the dominators of each node.  remove_unreachable_nodes must be
 *  applied to the CFG prior to this or the results may be incorrect.
 */

void
DominanceInfo::find_dominators ()
{
    // delete any old information
    if (_doms) delete [] _doms;
    if (_idom) delete [] _idom;

    CfgNode *entry = get_entry_node(_graph);

    int nsuccs = succs_size(entry);

    set_impossible_succ(entry, nsuccs, get_exit_node(_graph));
    _doms = do_dominators(FORWARD);
    _idom = do_immed_dominators(FORWARD);
    set_impossible_succ(entry, nsuccs, NULL);
    reflect_cti(entry, NULL, *succs_start(entry));
}


/*
 *  Find the postdominators of each node.  A legal graph should never have
 *  unreachable nodes in the reverse graph.  If that is not true, this will
 *  not work correctly.
 */

void
DominanceInfo::find_postdominators()
{
    // delete any old information
    if (_pdoms) delete [] _pdoms;
    if (_ipdom) delete [] _ipdom;

    CfgNode *entry = get_entry_node(_graph);

    int nsuccs = succs_size(entry);

    set_impossible_succ(entry, nsuccs, get_exit_node(_graph));
    _pdoms = do_dominators(REVERSE);
    _ipdom = do_immed_dominators(REVERSE);
    set_impossible_succ(entry, nsuccs, NULL);
    reflect_cti(entry, NULL, *succs_start(entry));
}


/*
 *  Compute (post)dominators.  This is a protected method that is used
 *  internally to do the actual work of finding dominators.
 */

NatSetDense*
DominanceInfo::do_dominators(bool direction) const
{
    CfgNodeListRpo rpo(_graph, direction);
    bool changed = false;
    int cfg_size = nodes_size(_graph);

    // allocate and initialize the new bit vectors
    NatSetDense *d = new NatSetDense[cfg_size];
    for (int i = 0; i < cfg_size; i++)
	d[i].insert_all();

    // set up the first node
    CfgNode *start =
	(direction == FORWARD ? get_entry_node(_graph) : get_exit_node(_graph));
    d[get_number(start)].remove_all();
    d[get_number(start)].insert(get_number(start));

    NatSetDense t;

    // iterate until no changes
    do {
	changed = false;
	for (CfgNodeHandle nh = rpo.start(); nh != rpo.end(); ++nh) {
	    CfgNode *n = *nh;
	    if (n == start) continue;

	    // get intersection of predecessors of n
	    t.insert_all();

	    CfgNodeHandle h;
	    int count;
	    if (direction == FORWARD)
		{ h = preds_start(n); count = preds_size(n); }
	    else
		{ h = succs_start(n); count = succs_size(n); }
	    for (int i = 0; i < count; ++i, ++h)
		t *= d[get_number(*h)];

	    // include itself in dominator set
	    t.insert(get_number(n));

	    // check if there were any changes
	    if (d[get_number(n)] != t) {
		d[get_number(n)] = t;
		changed = true;
	    }
	}
    } while (changed);

    return d;
}


/*
 *  Find the immediate (post)dominators.  This is a protected method that
 *  is only used internally to do the actual work of finding immediate
 *  dominators.
 */

CfgNode**
DominanceInfo::do_immed_dominators(bool direction) const
{
    int cfg_size = nodes_size(_graph);
    CfgNode *start =
	(direction == FORWARD ? get_entry_node(_graph) : get_exit_node(_graph));
    int root = get_number(start);
    NatSetDense *d = (direction == FORWARD ? _doms : _pdoms);

    CfgNode **im = new CfgNode*[cfg_size];
    im[root] = NULL;

    for (int n = 0; n < cfg_size; n++) {

	// the root node has no dominators
	if (n == root) continue;

	// remove self for comparison
	d[n].remove(n);

	// check nodes in dominator set
	im[n] = NULL;

	for (NatSetIter dnseq(d[n].iter()); dnseq.is_valid(); dnseq.next()) {
	    int i = dnseq.current();

	    claim(i < cfg_size, "do_immed_dominators - "
				"can't find immediate dominator of node %u", n);
	    if (d[n] == d[i]) {
		im[n] = get_node(_graph, i);
		break;
	    }
	}

	// re-add self when done
	d[n].insert(n);
    }

    return im;
}


/*
 *  Find the dominance frontier of each node.  The dominators must have
 *  already been computed.
 */

void
DominanceInfo::find_dom_frontier()
{
    claim(_doms, "DominanceInfo::find_dom_frontier - "
	         "need to compute dominators first");

    // delete any old information
    if (_df) delete [] _df;

    CfgNode *entry = get_entry_node(_graph);

    _df = new NatSetDense[nodes_size(_graph)];

    int nsuccs = succs_size(entry);
    set_impossible_succ(entry, nsuccs, get_exit_node(_graph));

    do_dom_frontiers(entry, FORWARD);

    set_impossible_succ(entry, nsuccs, NULL);
    reflect_cti(entry, NULL, *succs_start(entry));
}


/*
 *  Find the reverse dominance frontier of each node.  The postdominators
 *  must have already been computed.
 */

void
DominanceInfo::find_reverse_dom_frontier()
{
    claim(_pdoms, "DominanceInfo::find_reverse_dom_frontier - "
		  "need postdominators first");

    // delete any old information
    if (_rdf) delete [] _rdf;

    CfgNode *entry = get_entry_node(_graph);

    _rdf = new NatSetDense[nodes_size(_graph)];

    int nsuccs = succs_size(entry);
    set_impossible_succ(entry, nsuccs, get_exit_node(_graph));

    do_dom_frontiers(get_exit_node(_graph), REVERSE);

    set_impossible_succ(entry, nsuccs, NULL);
    reflect_cti(entry, NULL, *succs_start(entry));
}


/*
 *  Compute dominance frontiers using a postorder traversal of the dominator
 *  tree.  This is a protected method that is only used internally.
 */

void
DominanceInfo::do_dom_frontiers(CfgNode *x, bool direction)
{
    int n, cfg_size = nodes_size(_graph);
    CfgNode **xidom;
    NatSetDense *dom_front;
    CfgNodeHandle h;
    int count;

    if (direction == FORWARD) {
	xidom = _idom;
	dom_front = _df;
	h = succs_start(x);
	count = succs_size(x);
    } else {
	xidom = _ipdom;
	dom_front = _rdf;
	h = preds_start(x);
	count = preds_size(x);
    }

    // visit all children (i.e. immediate dominatees) first
    for (n = 0; n < cfg_size; n++) {
	if (xidom[n] == x) {
	    do_dom_frontiers(get_node(_graph, n), direction);
	}
    }

    // calculate dominance frontier, from paper RCytron_89

    // local loop, uses CFG
    for (int i = 0; i < count; ++i, ++h) {
	if (x != xidom[get_number(*h)]) {
	    dom_front[get_number(x)].insert(get_number(*h));
	}
    }

    // up loop, uses dominator tree
    for (n = 0; n < cfg_size; n++) {
	if ((xidom[n] == x) && !dom_front[n].is_empty()) {
	    for (int y = 0; y < cfg_size; y++) {
		if (dom_front[n].contains(y) && (x != xidom[y])) {
		    dom_front[get_number(x)].insert(y);
		}
	    }
	}
    }
}


// Access routines for dominators and dominance frontiers.

bool
DominanceInfo::dominates(int n_dominator, int n_dominatee) const
{
    claim(_doms, "DominanceInfo::dominates() -- "
		 "run find_dominators() first");
    return _doms[n_dominatee].contains(n_dominator);
}

bool
DominanceInfo::dominates(CfgNode *dominator, CfgNode *dominatee) const
{
    return dominates(get_number(dominator), get_number(dominatee));
}

const NatSet*
DominanceInfo::dominators(int n) const
{
    claim(_doms, "DominanceInfo::dominators() -- run find_dominators first");
    return &_doms[n];
}

const NatSet*
DominanceInfo::dominators(CfgNode *n) const
{
    return dominators(get_number(n));
}

bool
DominanceInfo::postdominates(int n_dominator, int n_dominatee) const
{
    claim(_pdoms, "DominanceInfo::postdominates() -- "
		  "run find_postdominators() first");
    return _pdoms[n_dominatee].contains(n_dominator);
}

bool
DominanceInfo::postdominates(CfgNode *dominator, CfgNode *dominatee) const
{
    return postdominates(get_number(dominator), get_number(dominatee));
}

const NatSet*
DominanceInfo::postdominators(int n) const
{
    claim(_pdoms, "DominanceInfo::postdominators() -- "
	  "run find_postdominators() first");
    return &_pdoms[n];
}

const NatSet*
DominanceInfo::postdominators(CfgNode *n) const
{
    return postdominators(get_number(n));
}

CfgNode*
DominanceInfo::immed_dom(int n) const
{
    claim(_idom, "DominanceInfo::immed_dom() -- run find_dominators() first");
    return _idom[n];
}

CfgNode*
DominanceInfo::immed_dom(CfgNode *n) const
{
    return immed_dom(get_number(n));
}

CfgNode*
DominanceInfo::immed_postdom(int n) const
{
    claim(_ipdom, "DominanceInfo::immed_postdom() -- "
	  "run find_postdominators() first");
    return _ipdom[n];
}

CfgNode*
DominanceInfo::immed_postdom(CfgNode *n) const
{
    return immed_postdom(get_number(n));
}

const NatSet*
DominanceInfo::dom_frontier(int n) const
{
    claim(_df, "DominanceInfo::dom_frontier() -- run find_dom_frontier first");
    return &_df[n];
}

const NatSet*
DominanceInfo::dom_frontier(CfgNode *n) const
{
    return dom_frontier(get_number(n));
}

const NatSet*
DominanceInfo::reverse_dom_frontier(int n) const
{
    claim(_rdf, "DominanceInfo::reverse_dom_frontier() -- "
	  "run find_reverse_dom_frontier() first");
    return &_rdf[n];
}

const NatSet*
DominanceInfo::reverse_dom_frontier(CfgNode *n) const
{
    return reverse_dom_frontier(get_number(n));
}

void
DominanceInfo::print(FILE *file) const
{
    int cfg_size = nodes_size(_graph);
    if (_doms) {
	fputs("Dominator sets:\n", file);
	for (int i = 0; i < cfg_size; i++) {
	    fprintf(file, "%5d: ", i);
	    _doms[i].print(file, cfg_size);
	    fputc('\n', file);
	}
	fputs("Immediate dominators:\n", file);
	for (int i = 0; i < cfg_size; i++) {
	    fprintf(file, "%5d: ", i);
	    if (_idom[i])
		fprintf(file, "%5d\n", get_number(_idom[i]));
	    else
		fputs("(none)\n", file);
	}
    }
    if (_pdoms) {
	fputs("Postdominator sets:\n", file);
	for (int i = 0; i < cfg_size; i++) {
	    fprintf(file, "%5d: ", i);
	    _pdoms[i].print(file, cfg_size);
	    fputc('\n', file);
	}
	fputs("Immediate postdominators:\n", file);
	for (int i = 0; i < cfg_size; i++) {
	    fprintf(file, "%5d: ", i);
	    if (_ipdom[i])
		fprintf(file, "%5d\n", get_number(_ipdom[i]));
	    else
		fputs("(none)\n", file);
	}
    }
    if (_df) {
	fputs("Dominance frontiers:\n", file);
	for (int i = 0; i < cfg_size; i++) {
	    fprintf(file, "%5d: ", i);
	    _df[i].print(file, cfg_size);
	    fputc('\n', file);
	}
    }
    if (_rdf) {
	fputs("Reverse dominance frontiers:\n", file);
	for (int i = 0; i < cfg_size; i++) {
	    fprintf(file, "%5d: ", i);
	    _rdf[i].print(file, cfg_size);
	    fputc('\n', file);
	}
    }
}
