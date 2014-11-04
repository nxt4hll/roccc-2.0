/* file "cfa/loop.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "cfa/loop.h"
#endif

#include <machine/machine.h>
#include <cfg/cfg.h>

#include <cfa/dom.h>
#include <cfa/loop.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

void
NaturalLoopInfo::find_natural_loops()
{
    Cfg *graph = _dom_info->graph();
    int cfg_size = nodes_size(graph);
    int *node_stack = new int[cfg_size];
    int i;

    if (_depth)
	delete [] _depth;
    _depth = new int[cfg_size];

    if (_loop)
	delete [] _loop;
    _loop = new NatSetDense[cfg_size];

    // initialize
    for (i = 0; i < cfg_size; i++)
	_depth[i] = 0;

    // find all the loop entries and loop-back nodes
    for (i = 0; i < cfg_size; i++) {
	CfgNode *cn = get_node(graph, i);
	int top_stack = 0;
	NatSetDense &this_loop = _loop[i];

	// if some predecessor induces a back edge, use it to seed the loop
	for (CfgNodeHandle ph = preds_start(cn); ph != preds_end(cn); ++ph)
	    if (_dom_info->dominates(cn, *ph))
		node_stack[top_stack++] = get_number(*ph);

	// there was some back edge, so this is a header
	if (top_stack) {
	    ++_depth[i];
	    this_loop.insert(i);
	}

	// walk backwards around loop, adding nodes
	while (top_stack) {
	    int top = node_stack[--top_stack];
	    if (!this_loop.contains(top)) {
		this_loop.insert(top);
		++_depth[top];
		CfgNode *top_node = get_node(graph, top);
		for (CfgNodeHandle ph = preds_start(top_node);
		     ph != preds_end(cn); ++ph) {
		    node_stack[top_stack++] = get_number(*ph);
		    claim(top_stack < cfg_size);
		}
	    }
	}
    }
    delete [] node_stack;
}

int
NaturalLoopInfo::loop_depth(int n) const
{
    claim(_depth, "NaturalLoopInfo::loop_depth() -- "
		  "run find_natural_loops() first");
    return _depth[n];
}

int
NaturalLoopInfo::loop_depth(CfgNode *n) const
{
    return loop_depth(get_number(n));
}

const NatSet*
NaturalLoopInfo::loop_at(int n) const
{
    claim(_loop, "NaturalLoopInfo::loop_at() -- "
		  "run find_natural_loops() first");
    return &_loop[n];
}

const NatSet*
NaturalLoopInfo::loop_at(CfgNode *n) const
{
    return loop_at(get_number(n));
}

void
NaturalLoopInfo::set_loop_depth(int n, int d)
{
    claim(_depth, "NaturalLoopInfo::loop_depth() -- "
		  "run find_natural_loops() first");
    _depth[n] = d;
}

void
NaturalLoopInfo::set_loop_depth(CfgNode *n, int d)
{
    set_loop_depth(get_number(n), d);
}

bool
NaturalLoopInfo::is_loop_begin(int n) const
{
    return is_loop_begin(get_node(_dom_info->graph(), n));
}

bool
NaturalLoopInfo::is_loop_begin(CfgNode *cn) const
{
    for (CfgNodeHandle h = preds_start(cn); h != preds_end(cn); ++h)
	if (_dom_info->dominates(cn, *h))
	    return true;
    return false;
}

bool
NaturalLoopInfo::is_loop_end(int n) const
{
    return is_loop_end(get_node(_dom_info->graph(), n));
}

bool
NaturalLoopInfo::is_loop_end(CfgNode *cn) const
{
    for (CfgNodeHandle h = succs_start(cn); h != succs_end(cn); ++h)
	if (_dom_info->dominates(*h, cn))
	    return true;
    return false;
}

bool
NaturalLoopInfo::is_loop_exit(int n) const
{
    return is_loop_exit(get_node(_dom_info->graph(), n));
}

bool
NaturalLoopInfo::is_loop_exit(CfgNode *cn) const
{
    for (CfgNodeHandle h = succs_start(cn); h != succs_end(cn); ++h)
	if (loop_depth(*h) != loop_depth(cn))
	    return true;
    return false;
}

void
NaturalLoopInfo::print(FILE *file) const
{
    if (_depth) {
	int cfg_size = nodes_size(_dom_info->graph());
	fputs("Loop info:\n"
	      "  node depth begin end exit\n", file);
	for (int i = 0; i < cfg_size; i++)
	    fprintf(file, "%5d:%4d     %c     %c    %c\n",
			  i, _depth[i],
			  is_loop_begin(i) ? 'Y' : 'N',
			  is_loop_end(i)   ? 'Y' : 'N',
			  is_loop_exit(i)  ? 'Y' : 'N');
    }
}
