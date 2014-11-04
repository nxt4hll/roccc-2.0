/* file "cfg/util.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "cfg/util.h"
#endif

#include <machine/machine.h>

#include <cfg/cfg_ir.h>
#include <cfg/graph.h>
#include <cfg/node.h>
#include <cfg/util.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

// Library-internal helpers

Integer succ_pos_set(CfgNode *node, CfgNode *succ);

// Constants

enum { REVERSE = false, FORWARD = true };	// indicators of CFG orientation


/*
 *  depth_first_walk -- Traverse nodes in depth-first order, beginning with
 *  a given `node', using `direction' to choose the forward or reverse
 *  graph, and avoiding nodes whose numbers are already in the set
 *  `visited'.  After all successors of the given node have been visited
 *  perform a given `action' on that node.
 *  
 *  Flag not_impossible means ignore impossible edges.  In the forward
 *  direction, this just means that we check whether an edge is impossible
 *  before traversing it.  In the backward direction, we have to be sure
 *  that there is at least one possible edge from the predecessor to the
 *  current node before proceeding to the predecessor.
 */
void
depth_first_walk(CfgNode *node, bool direction, NatSet *visited,
		 DepthFirstWalkAction &action, bool not_impossible)
{
    visited->insert(node->get_number());

    list<CfgNode*> &adjacent =
	(direction == FORWARD) ? node->succs() : node->preds();
    int count = adjacent.size();
    CfgNodeHandle h = adjacent.begin();

    for (int pos = 0; pos < count; ++h, ++pos)
	if (!visited->contains((*h)->get_number())) {
	    if (not_impossible &&
		 (direction == FORWARD
		     ? (node->imp_succs() & (Integer(1) << pos)) != 0
		     : (succ_pos_set(*h, node) & ~(*h)->imp_succs()) == 0))
		continue;
	    depth_first_walk(*h, direction, visited, action, not_impossible);
	}
    action(node);
}


/*
 *  CfgNodeListRpo::CfgNodeListRpo -- Build a list of flow graph nodes in
 *  reverse postorder, using either the forward or reverse flow graph.
 *
 *  First, define a simple postorder action to be taken after each node is
 *  visited in a depth-first walk: the node is pushed on the front of the
 *  developing rpo list.  This has the effect of reversing the order.
 */
class RpoAction : public DepthFirstWalkAction
{
  public:
    RpoAction(list<CfgNode*> &nodes) : nodes(nodes) { }
    virtual void operator()(CfgNode *node) { nodes.push_front(node); }

  private:
    list<CfgNode*> &nodes;
};

CfgNodeListRpo::CfgNodeListRpo(Cfg *cfg, bool direction)
{
    CfgNode *start =
	(direction == FORWARD ? get_entry_node(cfg) : get_exit_node(cfg));
    NatSetDense visited;
    RpoAction action(nodes);
    depth_first_walk(start, direction, &visited, action);
}

CfgNodeListRpo::~CfgNodeListRpo() { }

int
CfgNodeListRpo::size()
{
    return nodes.size();
}

CfgNodeHandle
CfgNodeListRpo::start()
{
    return nodes.begin();
}

CfgNodeHandle
CfgNodeListRpo::end()
{
    return nodes.end();
}

void
CfgNodeListRpo::prepend(CfgNode *node)
{
    nodes.push_front(node);
}

void
CfgNodeListRpo::append(CfgNode *node)
{
    nodes.push_back(node);
}

void
CfgNodeListRpo::replace(CfgNodeHandle h, CfgNode *node)
{
    *h = node;
}

void
CfgNodeListRpo::insert_before(CfgNodeHandle h, CfgNode *node)
{
    nodes.insert(h, node);
}

void
CfgNodeListRpo::insert_after(CfgNodeHandle h, CfgNode *node)
{
    if (h == nodes.end() || ++h == nodes.end())
	nodes.push_back(node);
    else
	nodes.insert(h, node);	
}

CfgNode*
CfgNodeListRpo::remove(CfgNodeHandle h)
{
    CfgNode *result = *h;
    nodes.erase(h);
    return result;
}


/*
 *  Retrieve the node corresponding to the position of a label in a control
 *  flow graph.  Return NULL if the label is not linked to a node.
 */
CfgNode*
label_cfg_node(Sym *label)
{
    if (OneNote<IrObject*> note = get_note(label, k_cfg_node))
	return to<CfgNode>(note.get_value());
    return NULL;
}

CfgNode*
get_parent_node(Instr *instr)
{
    IrObject *parent = (IrObject*)instr->get_parent();
    if (is_kind_of<CfgNode>(parent))
	return (CfgNode*)parent;
    return NULL;
}

bool
has_pred(CfgNode *node, CfgNode *pred)
{
    for (CfgNodeHandle h = preds_start(node);
	 h != preds_end(node); ++h)
	if (*h == pred)
	    return true;
    return false;
}

/*
 * Generate vcg format file for viewing with xvcg
 */
void
generate_vcg(FILE *f, Cfg *cfg)
{
    /* Use minbackward layout algorithm in an attempt to make all backedges  */
    /* true backedges. Doesn't always work, but the back algorithm for this. */
    /* Change these parameters as needed, but they have worked fine so far.  */
    fprintf(f,"graph: { title: \"CFG_GRAPH\"\n");
    fprintf(f,"\n");
    fprintf(f,"x: 30\n");
    fprintf(f,"y: 30\n");
    fprintf(f,"height: 800\n");
    fprintf(f,"width: 500\n");
    fprintf(f,"stretch: 60\n");
    fprintf(f,"shrink: 100\n");
    fprintf(f,"layoutalgorithm: minbackward\n");
    fprintf(f,"node.borderwidth: 3\n");
    fprintf(f,"node.color: white\n");
    fprintf(f,"node.textcolor: black\n");
    fprintf(f,"node.bordercolor: black\n");
    fprintf(f,"edge.color: black\n");
    fprintf(f,"\n");

    /* Put down nodes */
    int num = size(cfg);
    int i;
    for (i=0; i<num; i++)
	fprintf(f,"node: { title:\"%d\" label:\"%d\" }\n", \
		get_number(get_node(cfg, i)), get_number(get_node(cfg, i)));
    fprintf(f,"\n");

    /* Put down edges and backedges */
    for (i=0; i<num; i++) {
	CfgNode *node = get_node(cfg, i);
	for (CfgNodeHandle p = preds_start(node); p != preds_end(node); ++p) {
	    CfgNode *itnd = *p;
	    if (get_number(itnd)<get_number(get_node(cfg,i)))
		fprintf(f,"edge: {sourcename:\"%d\" targetname:\"%d\" }\n",
			get_number(itnd), get_number(get_node(cfg,i)));
	    else
		fprintf(f,"backedge: {sourcename:\"%d\" targetname:\"%d\" }\n",
			get_number(itnd), get_number(get_node(cfg,i)));
	}
    }

    /* Wrap up at the end */
    fprintf(f,"\n");
    fprintf(f, "}\n");
}
