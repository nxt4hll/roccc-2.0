/* file "cfg/graph.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "cfg/graph.h"
#endif

#include <machine/machine.h>

#include <cfg/cfg_ir.h>
#include <cfg/cfg_ir_factory.h>
#include <cfg/graph.h>
#include <cfg/node.h>
#include <cfg/util.h>
#include <cfg/init.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

// Library-internal helpers

extern bool terminate_for_layout(CfgNode *node, CfgNode *layout_succ);
extern void remove_succ(CfgNode*, CfgNode *succ);
extern void set_cti(CfgNode*, Instr*);
extern void add_noted_label(CfgNode*, LabelSym*);

// Local helpers

void add_branch_edges(Cfg*, CfgNode*, CfgNode *exit);
void add_succ(CfgNode*, CfgNode *succ);

// Local constants

enum { REVERSE = false, FORWARD = true };	// indicators of CFG orientation
const bool COMPLEMENT = true;			// iterate over NatSet's inverse

/*
 * Build a new flow graph.  Flow graphs represent entire procedures.
 *
 * Deal separately with the maximal-basic-block and single-instruction-block
 * cases.  Set layout links if keep_layout is true.
 *
 * Assume entry node is initially the only node that exists.  Make sure that
 * a proc_entry note is found, so that the entry node has a successor.
 */

void add_impossible_edges(Cfg*, bool direction);	// helper
Cfg*
    fill_cfg(Cfg*, InstrList*,
	     bool keep_layout, bool break_at_call, bool break_at_instr);

Cfg*
new_cfg(InstrList* body,
	bool keep_layout, bool break_at_call, bool break_at_instr)
{
    Cfg *cfg = create_cfg(the_suif_env, NULL, NULL, break_at_instr,
			  break_at_call, keep_layout);
    return fill_cfg(cfg, body, keep_layout, break_at_call, break_at_instr);
}

/*
 * fill_cfg -- Fill an existing CFG (which should be empty initially) from
 * a linear instruction list and return the CFG.
 */
Cfg*
fill_cfg(Cfg *cfg, InstrList* body,
	 bool keep_layout, bool break_at_call, bool break_at_instr)
{
    set_entry_node(cfg, new_empty_node(cfg));

    // First pass -- create nodes and fallthrough edges.  (Assumes
    // entry node is the only graph node so far.)

    CfgNode *prev = NULL;

    while (size(body) > 0) {
	Instr *mi = remove(body, start(body));

	// first handle the easy case of nodes for individual instructions
	if (break_at_instr) {

	    // create a new node for the instruction
	    CfgNode *inode = new_empty_node(cfg);
	    append(inode, mi);

	    // record entry-point nodes as entry-node successors
	    if (has_note(mi, k_proc_entry))
		add_succ(get_entry_node(cfg), inode);

	    // set layout links to previously-created node if keeping layout
	    if (keep_layout) {
		CfgNode *lpred = cfg->get_node(get_number(inode) - 1);
		lpred->set_layout_succ(inode);
		inode->set_layout_pred(lpred);
	    }

	    // record the positions of labels
	    if (is_label(mi))
		add_noted_label(inode, get_label(mi));

	    // create in-edge if previous node falls through
	    if (prev)
		add_succ(prev, inode);

	    // normally, this node falls thru to the next
	    prev = inode;

	    // note a CTI and NULLify prev if fall-thru can't happen
	    if (is_cti(mi) && (break_at_call || !is_call(mi))) {
		set_cti(inode, mi);

		if (!is_cbr(mi) && !is_call(mi))	// can't fall through
		    prev = NULL;
	    }
	    continue;
	}

	// now comes the harder task of building maximal basic blocks ...

	bool end_block = false;

	// create a new node for the block
	CfgNode *bnode = new_empty_node(cfg);

	// set layout links to previously-created node if keeping layout
	if (keep_layout) {
	    CfgNode *lpred = cfg->get_node(get_number(bnode) - 1);
	    lpred->set_layout_succ(bnode);
	    bnode->set_layout_pred(lpred);
	}

	// create an in-edge if previous node falls through
	if (prev)
	    add_succ(prev, bnode);

	prev = bnode;

	// check for label instructions
	while (is_label(mi)) {

	    // record the label position
	    add_noted_label(bnode, get_label(mi));

	    append(bnode, mi);

	    // record entry-point nodes as entry-node successors
	    if (!is_null(get_note(mi, k_proc_entry)))
		add_succ(get_entry_node(cfg), bnode);

	    // read the next instruction
	    if (size(body) == 0) {
		end_block = true;
		break;
	    }
	    mi = remove(body, start(body));
	}

	// if there's nothing but labels then this is the end
	if (end_block) continue;

	// assimilate instructions until something terminates basic block
	while (true) {
	    append(bnode, mi);

	    // record entry-point nodes as entry-node successors
	    if (!is_null(get_note(mi, k_proc_entry)))
		add_succ(get_entry_node(cfg), bnode);

	    // check for block-terminating cti
	    if (is_cti(mi) && (break_at_call || !is_call(mi))) {
		set_cti(bnode, mi);

		if (!is_cbr(mi) && !is_call(mi))
		    prev = NULL;		// can't fall through
		break;
	    }

	    // break if there's no non-label instruction after this
	    if ((size(body) == 0) || (is_label(*start(body))))
		break;

	    mi = remove(body, start(body));
	}
    }
    claim(succs_size(get_entry_node(cfg)) > 0,
	  "new_cfg -- missing proc_entry note");

    set_exit_node(cfg, new_empty_node(cfg));

    // Second pass -- add the remaining edges.  (Assumes entry node is
    // first and exit node is last.)
    for (int i = 1; i < size(cfg) - 1; i++) {
	CfgNode *nd = get_node(cfg, i);
	if (get_cti(nd))
	    add_branch_edges(cfg, nd, get_exit_node(cfg));
    }

    // Third pass -- add impossible edges as necessary to ensure every
    // node has a path from the entry node and to the exit node.
    add_impossible_edges(cfg, FORWARD);
    add_impossible_edges(cfg, REVERSE);

    return cfg;
}

/*
 * clear_cfg -- Helper for cfg_to_instr_list() and canonicalize().  Release all
 * nodes and remove "cfg_node" notes from all LabelSyms.
 */
void
clear_cfg(Cfg *cfg)
{
    while (!cfg->nodes().empty()) {
	CfgNode *nd = cfg->nodes().front();
	cfg->nodes().pop_front();
	delete nd;
    }

    while (!cfg->noted_labels().empty()) {
	LabelSym *label = cfg->noted_labels().front();
	take_note(label, k_cfg_node);
	cfg->noted_labels().pop_front();
    }
}

/*
 * cfg_to_instr_list -- Combine the node instruction lists into a single
 * InstrList, obeying any layout constraints.  Leave the input CFG
 * completely empty.
 *
 * Give each node a layout successor if it doesn't already have one.  Then
 * move each node's instructions into the result list.
 *
 * Note that set_layout_succ() may introduce a label for a previously
 * unlabeled node, so it's important to establish layout before combining
 * the instructions into a single list.  
 */
InstrList*
cfg_to_instr_list(Cfg *cfg)
{
    fix_layout(cfg);

    InstrList *result = new_instr_list();

    for (CfgNode *n = get_entry_node(cfg); n != NULL; n = get_layout_succ(n)) {
	while (size(n) > 0)
	    append(result, remove(n, start(n)));
	reflect_cti(n, NULL, get_layout_succ(n));	// just clear CTI handle
    }
    clear_cfg(cfg);
    return result;
}

/*
 * fix_layout -- Impose layout constraints where they don't already exist.
 * Insert UBR instructions where necessary to reflect a control-flow edge
 * that might have corresponded to fall-through, but which can't be so
 * implemented because of conflicting layout.  Likewise, eliminate a UBR
 * if it turns out to be equivalent to fall-through in the newly imposed
 * layout.
 */
void
fix_layout(Cfg *cfg)
{
    int i = 0;					// number of node to try next
    CfgNode *nd;				// current node to process
    CfgNode *ls = get_entry_node(cfg);		// next node to process
    CfgNode *end = ls;				// last node added to layout
    NatSetDense visited;			// nodes already processed

    while ((nd = ls)) {

	visited.insert(get_number(nd));
	if (get_layout_succ(nd))
	    ls = get_layout_succ(nd);
	else {
	    Instr *cti = get_cti(nd);

	    // Try for fall-through node as layout successor, but be sure
	    // that an apparent fall-through successor is really there.
	    // A call- or cbr-terminated node MUST have a control success-
	    // or as its layout successor.  If necessary, create an empty
	    // node as fall-thru for those cases, so that a ubr can be
	    // inserted by set_layout_succ.

	    if (!cti) {
		if (succs_size(nd) == 0
		    || get_layout_pred(ls = *succs_start(nd)))
		    ls = NULL;
	    }
	    else if (is_call(cti)) {
		if (get_layout_pred(ls = *succs_start(nd)))
		    ls = insert_empty_node(cfg, nd, 0);
	    } else if (is_cbr(cti)) {
		CfgNodeHandle h = succs_start(nd);
		if ((get_layout_pred(ls = *h))
		 && (get_layout_pred(ls = *++h)))
		    ls = insert_empty_node(cfg, nd, 0);
	    } else
		ls = NULL;

	    // Try next unconstrained, unvisited node in numerical order as
	    // layout successor.  If that node is constrained, look instead
	    // for a suitable node earlier in its layout chain.

	    for ( ; ls == NULL && i < size(cfg); i++) {
		ls = cfg->get_node(i);
		if (visited.contains(get_number(ls)))
		    ls = NULL;
		else {
		    while (CfgNode *lp = get_layout_pred(ls))
			ls = lp;
		    claim(nd != ls && !visited.contains(get_number(ls)));
		}
	    }
	    set_layout_succ(nd, ls);
	    end = nd;
	}
    }
    claim(visited.size() == size(cfg));

    // The last node added to the layout may have a successor but no CTI.
    // Check for that case and terminate it with an unconditional branch.
    terminate_for_layout(end, NULL);
}


/*
 * canonicalize -- If the argument flags match those of the current CFG, do
 * nothing; otherwise, rebuild the CFG using the new flags.
 */
void
canonicalize(Cfg *cfg, bool keep_layout, bool break_at_call,
	     bool break_at_instr)
{
    if (   keep_layout    == cfg->get_keep_layout()
	&& break_at_call  == cfg->get_break_at_call()
	&& break_at_instr == cfg->get_break_at_instr())
	return;
    InstrList *instr_list = cfg_to_instr_list(cfg);
    clear_cfg(cfg);
    fill_cfg(cfg, instr_list, keep_layout, break_at_call, break_at_instr);
    cfg->set_keep_layout(keep_layout);
    cfg->set_break_at_call(break_at_call);
    cfg->set_break_at_instr(break_at_instr);
}

CfgNode*
get_node(Cfg *cfg, int pos)
{
    claim(pos < nodes_size(cfg), "Node position out of range");
    return cfg->get_node(pos);
}

CfgNode*
get_node(Cfg *cfg, CfgNodeHandle handle)
{
    claim(handle != nodes_end(cfg), "Node beyond last node");
    return *handle;
}

CfgNode*
node_at(Cfg *cfg, LabelSym *label)
{
    if (OneNote<IrObject*> note = get_note(label, k_cfg_node))
	return to<CfgNode>(note.get_value());
    return NULL;
}

/*
 * Return the number of nodes currently in the CFG.
 */
int
nodes_size(Cfg *cfg)
{
    return cfg->nodes().size();
}

CfgNodeHandle
nodes_start(Cfg *cfg)
{
    return cfg->nodes().begin();
}

CfgNodeHandle
nodes_last(Cfg *cfg)
{
    return get_last_handle(cfg->nodes());
}

CfgNodeHandle
nodes_end(Cfg *cfg)
{
    return cfg->nodes().end();
}

CfgNode*
get_entry_node(Cfg *cfg)
{
    return cfg->get_entry_node();
}

CfgNode*
get_exit_node(Cfg *cfg)
{
    return cfg->get_exit_node();
}

void
set_entry_node(Cfg *cfg, CfgNode *node)
{
    cfg->set_entry_node(node);
}

void
set_exit_node(Cfg *cfg, CfgNode *node)
{
    cfg->set_exit_node(node);
}

#if 0				// FIXME
/*
 * Return the number of edges or multiedges currently in the CFG.
 */
int
num_edges(Cfg *cfg)
{
    int ne = 0;
    cfg_edge_iter edges(this, false);
    while (!edges.is_empty()) {
	edges.increment();
	if (!edges.is_empty())
	    ++ne;
    }
    return ne;
}

int
num_multi_edges(Cfg *cfg)
{
    int ne = 0;
    cfg_edge_iter edges(this, true);
    while (!edges.is_empty()) {
	edges.increment();
	if (!edges.is_empty())
	    ++ne;
    }
    return ne;
}
#endif


/*
 * clone_node -- Make a copy of a node, link it into
 * this cfg, leaving it disconnected, and return it.
 *
 * If the node has a CTI with one or more target labels,
 * substitute NULL for each such target.  The client
 * replaces these target slots when connecting the new
 * node via set_succ().
 */

// First a helper class ...

class AddrSymReplacer : public OpndFilter {
public:
  AddrSymReplacer(VarSym *old_dtsym, VarSym *new_dtsym)
    : old_dtsym(old_dtsym), new_dtsym(new_dtsym) { }
  
  Opnd operator()(Opnd opnd, InOrOut) {
    if (is_addr_sym(opnd) && get_sym(opnd) == old_dtsym)
      return opnd_addr_sym(new_dtsym);
    
    return 0;
  }
protected:
  VarSym *old_dtsym;
  VarSym *new_dtsym;
};

CfgNode *
clone_node(Cfg *cfg, CfgNode *orig)
{
    Instr *orig_cti = get_cti(orig);
    Instr *copy_cti = NULL;
    InstrHandle copy_cti_h;
    CfgNode *copy = new_empty_node(cfg);

    for (InstrHandle oh = start(orig), ch = end(copy);
	 oh != end(orig);
	 ++oh, ++ch)
    {
	Instr *oi = *oh,  *ci;
	
	if (is_label(oi)) {
	    LabelSym* label = new_unique_label(get_name(get_label(oi)).c_str());
	    add_noted_label(copy, label);

	    ci = new_instr_label(label);
	} else {
	    ci = (Instr*)(*oh)->deep_clone();
	}
	ch = insert_before(copy, ch, ci);
	if (*oh == orig_cti) {
	    copy_cti = ci;
	    copy_cti_h = ch;
	    set_cti(copy, ci);
	}
    }
    claim((int)(orig_cti == NULL) == (int)(copy_cti == NULL));

    if (copy_cti) {
	if (is_ubr(copy_cti) || is_cbr(copy_cti))
	    set_target(copy_cti, NULL);
	else if (is_mbr(copy_cti)) {
	    set_target(copy_cti, NULL);
	    MbrNote note = get_note(copy_cti, k_instr_mbr_tgts);
	    claim(!is_null(note));

	    int count = note.get_case_count();
	    for (int i = 0; i < count; ++i)
		note.set_case_label(NULL, i);

	    // Find the use of the the old dispatch table in the code and
	    // replace it with the new one.  The use is marked with a
	    // k_mbr_table_use note.

	    VarSym *old_dtsym = note.get_table_sym();
	    VarSym *new_dtsym = new_dispatch_table_var(note);
	    AddrSymReplacer replacer(old_dtsym, new_dtsym);

	    InstrHandle ch = copy_cti_h;
	    while (ch-- != start(copy))
		if (has_note(*ch, k_mbr_table_use)) {
		    map_src_opnds(*ch, replacer);
		    break;
		}
	}
    }
    return copy;
}

/*
 *  Remove unreachable nodes from a flow graph and renumber the rest.
 *  Return true if any nodes are removed.  Before deleting each removed
 *  node, delete the instructions it contains.
 */

void discard_mbr_table(CfgNode*);		// used in next 2 methods

bool
remove_unreachable_nodes(Cfg *cfg)
{
    const bool POSSIBLE = true;			// ignore impossible edges

    // find the reachable blocks
    NatSetDense mark;
    DepthFirstWalkAction inaction;
    depth_first_walk(get_entry_node(cfg), FORWARD, &mark, inaction, POSSIBLE);

    // Keep the exit node, which may only be reachable by impossible edges.
    mark.insert(get_number(get_exit_node(cfg)));

    if (mark.size() == size(cfg))
	return false;

    // When removing nodes, be sure that any references to them from the
    // cfg_node annotations of labels are also removed.  Don't count on
    // finding label instructions to indicate which labels have such notes.
    // Some label instructions may have been excised already.

    list<LabelSym*> &noted = cfg->noted_labels();
    for (list<LabelSym*>::iterator nit = noted.begin(); nit != noted.end(); ) {
	list<LabelSym*>::iterator this_nit = nit++;

	LabelSym *label = *this_nit;
	OneNote<IrObject*> note = get_note(label, k_cfg_node);
	claim(!is_null(note), "CFG label has no associated CFG node");
	CfgNode *node = to<CfgNode>(note.get_value());
	if (!mark.contains(get_number(node))) {
	    take_note(label, k_cfg_node);
	    noted.erase(this_nit);
	}
    }

    int new_number = 0;
    list<CfgNode*> &nodes = cfg->nodes();

    // Handle h scans the node list in search of unreachables.  When one of
    // these is found, h must be advanced before the list element is excised.

    for (CfgNodeHandle h = nodes.begin(); h != nodes.end(); ) {
	if (mark.contains(get_number(*h))) {
	    (*h++)->set_number(new_number++);
	} else {
	    // Detach the unreachable node from its successors.
	    while (!(*h)->succs().empty())
		remove_succ(*h, *succs_start(*h));

	    // Detach the unreachable node from its predecessors
            // (needed when there's a back-edge from a higher numbered node
	    // to a lower numbered node).
	    while (!(*h)->preds().empty())
		remove_succ(*preds_start(*h), *h);

	    claim(succs_size(*h) == 0 && preds_size(*h) == 0);

	    // Clean up before deleting node: forward layout links around it,
	    // nullify its mbr table if it's mbr-terminated.

	    CfgNode *olsucc = get_layout_succ(*h);
	    CfgNode *olpred = get_layout_pred(*h);
	    if (get_layout_succ(*h))
		get_layout_succ(*h)->set_layout_pred(olpred);
	    if (get_layout_pred(*h))
		get_layout_pred(*h)->set_layout_succ(olsucc);

	    if (ends_in_mbr(*h))
		discard_mbr_table(*h);

	    delete *h;
	    nodes.erase(h++);
	}
    }
    return true;
}


/* merge_node_sequences -- For each sequence p_1,...,p_k of
 * control equivalent cfg_block's such that p_i immediately
 * precedes p_i+1, move the instructions of p_2,...,p_k
 * into p1 and transfer the successors of pk to p1, leaving
 * p_2,...,p_k isolated and content-free, except for labels.
 * If p_k has a layout successor, make it the layout successor
 * of the merged block.
 *
 * Return true iff any sequences are merged.
 *
 * If the CFG's _break_at_call flag is true, break a sequence
 * at the first block that ends with a call instruction.
 *
 * Ignore sequences not reachable from the entry node (so that
 * the implementor doesn't need to worry about cycles).
 *
 * As a side effect, eliminate conditional or multiway branches
 * terminating blocks that have unique successors.
 */
bool
merge_node_sequences(Cfg *cfg)
{
    bool did_merge = false;
    CfgNodeListRpo rpo(cfg);
    CfgNodeHandle h = rpo.start();

    do {
	CfgNode *pn = *h;
	if (pn == get_entry_node(cfg)) {
	    ++h;
	    continue;
	}
	CfgNode *sn = NULL;		// unique successor of pn or else null

	for (CfgNodeHandle sh = succs_start(pn); sh != succs_end(pn); ++sh)
	    if (sn == NULL)
		sn = *sh;
	    else if (sn != *sh) {
		sn = NULL;
	        break;			// no unique successor
	    }

	if (sn == NULL
	    || sn == get_exit_node(cfg)
	    || (cfg->get_break_at_call() && ends_in_call(pn))) {
	    ++h;
	    continue;
	}

	/* The current predecessor node (pn) has a unique successor (sn).
	 * If pn ends with a cbr or mbr, that instruction must not be
	 * needed.  Take it out, even if the current merge doesn't succeed.
	 * Substitute a ubr unless sn is in fall-through position. */

	if (ends_in_cbr(pn) || ends_in_mbr(pn)) {

	    if (ends_in_mbr(pn))		// nullify mbr table, if any
		discard_mbr_table(pn);

	    Instr *cti = get_cti(pn);
	    remove(pn, get_cti_handle(pn));
	    delete cti;
	    set_cti(pn, NULL);

	    if (pn->get_layout_succ() != sn) {	// can't just fall through
		Instr *mi = new_instr_cti(opcode_ubr(), get_label(sn));
		append(pn, mi);
		set_cti(pn, mi);
	    }
	    remove_succ(pn, sn);		// remove all successor edges
	    set_succ(pn, 0, sn);		// put one back
	}

	if (preds_size(sn) != 1) {
	    ++h;
	    continue;
	}

	CfgNode *sn_los = sn->get_layout_succ();

	if (pn->get_layout_succ())
	    clear_layout_succ(pn);
	if (sn_los)
	    clear_layout_succ(sn);

	remove_succ(pn, sn);

	if (get_cti(pn)) {
	    delete remove(pn, get_cti_handle(pn));
	    set_cti(pn, NULL);
	}

	for (InstrHandle ih = instrs_start(sn); ih != instrs_end(sn); ) {
	    Instr *mi = *ih;
	    InstrHandle mih = ih++;
	    if (!is_label(mi)) {
		bool was_cti = (mi == get_cti(sn));
		remove(sn, mih);
		mih = append(pn, mi);
		if (was_cti) {
		    set_cti(sn, NULL);
		    set_cti(pn, mi);
		}
	    }
	}

	for (int j = 0; succs_size(sn) > 0; j++) {
	    CfgNode *ss = *succs_start(sn);
	    if (is_exceptional_succ(sn, j))
		set_exceptional_succ(pn, j, ss);
	    else if (is_impossible_succ (sn, j))
		set_impossible_succ (pn, j, ss);
	    else
		set_succ(pn, j, ss);
	    remove_succ(sn, ss);
	}

	if (sn_los)
	    set_layout_succ(pn, sn_los);

	did_merge = true;

    } while (h != rpo.end());

    return did_merge;
}

/*
 * discard_mbr_table -- The argument node is known to end with an mbr
 * that's about to be rubbed out.  Find and neutralize its jump table
 * definition (if any), so that the entries don't mention non-existent
 * labels in the assembly file.  The table definition needs to remain,
 * because it might be used in setup computations that have been scheduled
 * out of the current block.  Delete all table entries but one, however,
 * and replace the label in that one by zero.
 */
void
discard_mbr_table(CfgNode *node) {
    Instr *mbr = get_cti(node); 

    MbrNote note = take_note(mbr, k_instr_mbr_tgts);
    claim(!is_null(note));

    strip_dispatch_table_var(note);

    note = MbrNote();			// lose the old note
    note.set_table_sym(NULL);
    set_note(mbr, k_instr_mbr_tgts, note);
}


/*
 * optimize_jumps -- Replace jumps to jumps.  Return true
 * if some jump is changed, else false.
 *
 * Repeat the following passes until no change occurs:
 *
 * (1) Identify nodes that end with an unconditional jump,
 *     but are otherwise vacuous.  (Vacuous means: contains
 *     only labels, pseudo-ops, and null instructions.)
 * (2) Replace jumps to such nodes by jumps to their
 *     unique successors.
 */
bool
optimize_jumps(Cfg *cfg)
{
    bool ever_changed, changed = false;
    int nn = size(cfg);
    CfgNode **redirect = new CfgNode *[nn];

    for (int i = 0; i < nn; ++i) {
	CfgNode *node = cfg->get_node(i);
	Instr *mi;

	if ((succs_size(node) == 1) &&
	    (((mi = first_active_instr(node)) == NULL) ||
	     is_ubr(mi)))
	    redirect[i] = *succs_start(node);
	else
	    redirect[i] = NULL;
    }
    do {
	ever_changed = changed;
	changed = false;

	for (int i = 0; i < nn; ++i) {
	    CfgNode *node = cfg->get_node(i);

	    for (int j = 0; j < succs_size(node); ++j)
	    {
		// Don't redirect an impossible edge.  The successor of an
		// unreachable node may not be unreachable.

		if (is_impossible_succ(node, j))
		    continue;

		CfgNode *succ = node->succs()[j];
		int succ_num = get_number(succ);
		CfgNode *new_succ = redirect[succ_num];

		// Don't set changed unless a successor actually changes.
		// Termination depends on real progress here.

		if (!new_succ || new_succ == succ)
		    continue;

		// If not a fall-through case, just use set_succ() to alter
		// the branch target.  If succ is node's (empty) fall-through
		// successor, first clear any layout link between them.

		CfgNode *fall_thru_los = NULL;

		if (j == 0 && !ends_in_ubr(node) && !ends_in_mbr(node))
		    fall_thru_los = node->get_layout_succ();

		if (fall_thru_los)	// node falls thru to empty layout succ
		    clear_layout_succ(node);

		set_succ(node, j, new_succ);

		// If there was a layout chain from node falling through succ to
		// new_succ, close the chain after excising succ.

		if (fall_thru_los && succ->get_layout_succ() == new_succ) {
		    clear_layout_succ(succ);
		    set_layout_succ(node, new_succ);
		}

		// Shorten any cycles of vacuous nodes by updating redirect[n]
		// when the successor of vacuous node n is itself redirected.

		if (redirect[get_number(node)])
		    redirect[get_number(node)] = new_succ;

		changed = true;
	    }
	}
    } while (changed);

    delete [] redirect;

    return ever_changed;
}


/*
 * remove_layout_info -- Clear all the layout links between
 * cfg_node's.
 */
void
remove_layout_info(Cfg *cfg)
{
    for (CfgNodeHandle h = start(cfg); h != end(cfg);  ++h)
	clear_layout_succ(*h);
}


/**
 ** ------------- Protected methods -------------
 **/



/*
 *  target_to_node -- Subfunction of add_branch_edges (just below).
 *  Return node for target label, after making sure that one exists.
 */
inline CfgNode *
target_to_node(Sym *target)
{
    CfgNode *target_node = label_cfg_node(target);
    claim(target_node, "add_branch_edges -- cannot find node for "
	  "label `%s'", get_name(target).chars());
    return target_node;
}


/*
 * add_branch_edges -- Add the branch edges of a cti-terminated node after
 * initial flow-graph node creation.  The third parameter is the exit node
 * of the graph.
 *
 * Fall-through edges will already have been done by new_cfg.
 *
 * Unlike set_succ(), we don't have to bother to remove old predecessor
 * pointers.  We're setting them up for the first time.
 */
void
add_branch_edges(Cfg *cfg, CfgNode *node, CfgNode *exit)
{
    Instr *cti = get_cti(node);
    claim(cti);

    if (is_cbr(cti)) {
	add_succ(node, target_to_node(get_target(cti)));
	claim(succs_size(node) == 2,
	      "add_branch_edges -- cbr has wrong number of successors");
    }
    else if (is_ubr(cti)) {
	add_succ(node, target_to_node(get_target(cti)));
	claim(succs_size(node) == 1,
	      "add_branch_edges -- ubr has wrong number of successors");
    }
    else if (is_mbr(cti)) {
	claim(succs_size(node) == 0,
	      "add_branch_edges -- mbr already has some successors");
	MbrNote note = (const MbrNote&)get_note(cti, k_instr_mbr_tgts);
	claim(!is_null(note), "add_branch_edges -- no mbr targets");

	int case_count = note.get_case_count();

	for (int i = 0; i < case_count; ++i) {
	    LabelSym *target = note.get_case_label(i);
	    add_succ(node, target_to_node(target));
	}

	Sym *default_target = get_target(cti);
	if (default_target != NULL)
	    add_succ(node, target_to_node(default_target));
    }
    else if (is_return(cti)) {
	add_succ(node, exit);
	claim(succs_size(node) == 1,
	      "add_branch_edges -- return has wrong number of successors");
    } else
	claim(is_call(cti));
}


/*
 *  add_impossible_edges -- Depending on flag direction, ensure that a path
 *  from the entry exists to every node (direction == FORWARD) or that a
 *  path to the exit exists for every node (direction == REVERSE).  If
 *  necessary, add impossible edges to make it so.
 *
 *  Repeatedly perform a depth-first walk in the CFG, moving forward or in
 *  reverse according to flag `direction'.  Initially start the walk at the
 *  entry node if FORWARD, else at the exit.  After each walk completes,
 *  look for a node that hasn't been visited.  If such exists, give it an
 *  impossible edge to or from the chosen boundary node, and use it to
 *  begin the next depth-first walk.  Quit when all nodes have been
 *  visited.
 */
void
add_impossible_edges(Cfg* cfg, bool direction)
{
    NatSetDense visited;			// empty initially
    CfgNode *start =
	(direction == FORWARD ? get_entry_node(cfg) : get_exit_node(cfg));
    int num = size(cfg);
    DepthFirstWalkAction inaction;		// takes no action at all
    CfgNode *current = start;
    for ( ; ; ) {
	depth_first_walk(current, direction, &visited, inaction);
	int unseen = num - visited.size();
	if (unseen == 0)
	    break;

	// Get the number of a node not yet visited.  When going forward,
	// make it the one with the smallest number, hoping that larger-
	// numbered nodes will be reachable from that one.  When in reverse,
	// pick the largest-numbered node.
	NatSetIter iter = visited.iter(COMPLEMENT);
	if (direction == REVERSE)
	    for ( ; unseen > 1; unseen--)
		iter.next();
	int n = iter.current();
	claim(n < num);

	// Connect the chosen node to the start node, then walk again
	// from chosen one.
	current = get_node(cfg, n);
	CfgNode *head = (direction == FORWARD ? current : start);
	CfgNode *tail = (direction == REVERSE ? current : start);
	set_impossible_succ(tail, succs_size(tail), head);
    }
}


/*
 *  cfg::insert_empty_node(tail, succ_num) -- Create a node and insert it
 *  along the succ_num-th out edge of node tail.
 */

CfgNode *
insert_empty_node(Cfg *cfg, CfgNode *tail, int succ_num)
{
    claim(succ_num < succs_size(tail));
    CfgNode *head = tail->succs()[succ_num];

    // Create and attach the node
    CfgNode *node = new_empty_node(cfg);

    // Link to the node
    bool preserve_layout = (tail->get_layout_succ() == head);
    if (preserve_layout)
	clear_layout_succ(tail);

    set_succ(tail, succ_num, node);
    set_succ(node, 0, head);

    if (preserve_layout) {
	tail->set_layout_succ(node);
	node->set_layout_succ(head);
    }
    return node;
}

/*
 *  cfg::insert_empty_node(tail, head) -- Same as preceding variant, but allow
 *  for multiple edges from tail to head and redirect all of them through new
 *  node.
 */
CfgNode *
insert_empty_node(Cfg *cfg, CfgNode *tail, CfgNode *head)
{
    claim(contains(tail->succs(), head));

    CfgNode *node = new_empty_node(cfg);

    // Link to the node
    bool preserve_layout = (get_layout_succ(tail) == head);
    if (preserve_layout)
	clear_layout_succ(tail);

    int i = 0;
    CfgNodeHandle h = succs_start(tail);

    for ( ; h != succs_end(tail); ++h, ++i)
	if (*h == head)
	    set_succ(tail, i, node);
    set_succ(node, 0, head);

    if (preserve_layout) {
	set_layout_succ(tail, node);
	set_layout_succ(node, head);
    }
    return node;
}


/*
 * Helper for fprint (below).  Return true iff node can fall through to its
 * first successor (not via an existing explicit branch).
 */
bool
can_fall_through(CfgNode *node)
{
    return !ends_in_cti(node) || ends_in_cbr(node) || ends_in_call(node);
}

/*
 * Helper for fprint (below).  If the last_printed node might appear to
 * fall through incorrectly to the current node, print a ubr whose target
 * is the current node.  If the current node has any predecessors other
 * than the node last_printed, make sure that it has an explicit label.
 */
void
fprint_glue(FILE *fp, CfgNode *node, CfgNode *last_printed)
{
    if ((last_printed != NULL) &&
	(succs_size(last_printed) > 0) &&
	(node != *succs_start(last_printed)) &&
	can_fall_through(last_printed))
    {
	CfgNode *target = *succs_start(last_printed);
	Instr *ubr = new_instr_cti(opcode_ubr(), get_label(target));
	Printer *printer = target_printer();
	printer->set_file_ptr(fp);
	
	printer->print_instr(ubr);
	delete ubr;
    }
    for (CfgNodeHandle h = preds_start(node); h != preds_end(node); ++h)
	if (*h != last_printed)
	    get_label(node);		// force node to have a label
}

/*
 *  fprint -- Print simple ASCII representation of CFG.  If follow_layout
 *  is true, honor layout constraints in ordering the list.  If show_code
 *  is true, print a debugging representation of the instructions in each
 *  node.
 */
void
fprint(FILE *fp, Cfg *cfg, bool follow_layout, bool show_code)
{
    fprintf(fp, "**** CFG contains %d blocks\n", size(cfg));

    CfgNode *last_printed = NULL;

    if (follow_layout) {
	BitVector done;

	for (CfgNodeHandle h = start(cfg); h != end(cfg); ++h) {
	    CfgNode *node = *h;

	    if (done.get_bit(get_number(node)))
		continue;
	    while (node->get_layout_pred())
		node = node->get_layout_pred();
	    while (node) {
		if (show_code)
		    fprint_glue(fp, node, last_printed);
		fprint(fp, node, show_code);
		last_printed = node;
		done.set_bit(get_number(node), true);
		node = get_layout_succ(node);
	    }
	}
    } else {
	for (CfgNodeHandle h = start(cfg); h != end(cfg); ++h) {
	    CfgNode *node = *h;
	    if (show_code)
		fprint_glue(fp, node, last_printed);
	    fprint(fp, node, show_code);
	    last_printed = node;
	}
    }
}

/*
 * This separate overloading is necessary to resolve ambiguity with
 * fprint(FILE *fp, IrObject *obj) in the machine library.
 */
void
fprint(FILE *fp, Cfg *cfg)
{
    fprint(fp, cfg, false, false);
}

void
add_succ(CfgNode *node, CfgNode *succ)
{
    node->succs().push_back(succ);
    if (!has_pred(succ, node))
	succ->preds().push_back(node);
}

#if 0
/* ------------  Implementation of cfg_edge_iter  ----------- */

cfg_edge_iter::cfg_edge_iter(Cfg *base, bool multigraph)
{
    this->base = base;
    this->multigraph = multigraph;
    reset();
}

void
cfg_edge_iter::reset()
{
    this->curr_node = 0;
    this->curr_index = -1;
}

bool
cfg_edge_iter::is_empty() const
{
    return curr_node >= base->num_nodes();
}

void
cfg_edge_iter::increment()
{
    ++curr_index;
    CfgNode *node = base->node(curr_node);
    while ((multigraph && curr_index >= succs_size(node)) ||
	   (!multigraph && curr_index >= preds_size(node))) {
	++curr_node;
	if (curr_node >= base->num_nodes())
	    break;
	curr_index = 0;
	node = base->node(curr_node);
    }
}

CfgNode *
cfg_edge_iter::get_head() const
{
    if (multigraph)
	return base->node(curr_node);
    else
	return base->node(curr_node)->preds()[curr_index];
}

CfgNode *
cfg_edge_iter::get_tail() const
{
    if (multigraph)
	return base->node(curr_node)->succs()[curr_index];
    else
	return base->node(curr_node);
}
#endif
