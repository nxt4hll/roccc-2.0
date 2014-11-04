/* file "cfg/node.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "cfg/node.h"
#endif

#include <machine/machine.h>

#include <cfg/cfg_ir.h>
#include <cfg/cfg_ir_factory.h>
#include <cfg/node.h>
#include <cfg/util.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

// Library-internal helpers

extern bool terminate_for_layout(CfgNode *node, CfgNode *layout_succ);
extern void remove_succ(CfgNode*, CfgNode *succ);
extern void add_noted_label(CfgNode*, LabelSym*);


/* local function prototypes */
static void replace_succ(CfgNode *node, int pos, CfgNode *new_succ);
static void remove_node(list<CfgNode*>&, CfgNode*, Integer* = NULL);
static void print_node_list(FILE*, list<CfgNode*>&, CfgNode*, bool);
inline bool no_layout_cycle(CfgNode *top, CfgNode *bot); 

enum { REVERSE = false, FORWARD = true };	// indicators of CFG orientation


/*
 * Basic functions on the instructions in a CFG node
 */
int
instrs_size(CfgNode *node)
{
    return node->instrs().size();
}

InstrHandle
instrs_start(CfgNode *node)
{
    return node->instrs().begin();
}

InstrHandle
instrs_last(CfgNode *node)
{
    return get_last_handle(node->instrs());
}

InstrHandle
instrs_end(CfgNode *node)
{
    return node->instrs().end();
}

InstrHandle
prepend(CfgNode *node, Instr *instr)
{
    if (instr)
	instr->set_parent(node);
    return node->instrs().insert(node->instrs().begin(), instr);
}

InstrHandle
append(CfgNode *node, Instr *instr)
{
    if (instr)
	instr->set_parent(node);
    return node->instrs().insert(node->instrs().end(), instr);
}

void
replace(CfgNode *node, InstrHandle h, Instr *instr)
{
    claim(h != node->instrs().end(), "Invalid list handle");
    if (*h)
	(*h)->set_parent(NULL);
    if (instr)
	instr->set_parent(node);
    *h = instr;
}

InstrHandle
insert_before(CfgNode *node, InstrHandle h, Instr *instr)
{
    if (instr)
	instr->set_parent(node);
    return node->instrs().insert(h, instr);
}

InstrHandle
insert_after(CfgNode *node, InstrHandle h, Instr *instr)
{
    claim(h != node->instrs().end());
    if (instr)
	instr->set_parent(node);
    return node->instrs().insert(++h, instr);
}

Instr*
remove(CfgNode *node, InstrHandle h)
{
    claim(h != node->instrs().end());
    Instr *result = *h;
    node->instrs().erase(h);
    result->set_parent(NULL);
    return result;
}

int
succs_size(CfgNode *node)
{
    return node->succs().size();
}

CfgNodeHandle
succs_start(CfgNode *node)
{
    return node->succs().begin();
}

CfgNodeHandle
succs_end(CfgNode *node)
{
    return node->succs().end();
}

int
preds_size(CfgNode *node)
{
    return node->preds().size();
}

CfgNodeHandle
preds_start(CfgNode *node)
{
    return node->preds().begin();
}

CfgNodeHandle
preds_end(CfgNode *node)
{
    return node->preds().end();
}



/*
 * new_empty_node -- Make and return a new empty, disconnected node in a
 * given CFG.
 */
CfgNode*
new_empty_node(Cfg *cfg)
{
    CfgNode *node =
	create_cfg_node(the_suif_env, size(cfg), NULL, NULL, NULL, 0, 0);
    cfg->append_node(node);
    return node;
}

/*
 * get_cti -- Return the CTI if the node has one, otherwise NULL;
 */
Instr*
get_cti(CfgNode *node)
{
    return node->get_cti();
}

/*
 * get_cti_handle -- Return the CTI's handle if the node has one,
 * otherwise the sentinel handle.
 */
InstrHandle
get_cti_handle(CfgNode *node)
{

    Instr *cti = node->get_cti();
    if (cti != NULL)
    {
	InstrHandle ih = last(node);		// work backwards thru instrs

	for (int i = size(node); i-- > 0; --ih)
	    if (*ih == cti)
		return ih;
    }
    return node->instrs().end();
}


/*
 * set_cti -- Internal routine to update node's CTI pointer.  The public
 * must use reflect_cti().
 */
void
set_cti(CfgNode *node, Instr* cti)
{
    node->set_cti(cti);
}

/*
 * reflect_cti -- Record CTI in node and adjust successor edges to
 * reflect the semantics of the new CTI, which must already have been
 * inserted in the code list of the node.
 *
 * Remove any previous successors, but remember the abnormals ones.
 * Dispatch on the kind of CTI and use the explicit targets to find any
 * non-fall-through successors.  Using those together with the implicit
 * successor (given as second argument), fill in the successor list.  The
 * CTI argument may be NULL, in which case, simply leave the node without a
 * CTI and use the implicit-successor argument as its fall-through
 * successor.
 *
 * Finally, restore any abnormal successors after the normal ones.
 *
 * Make sure that if the CTI can fall through and if the node has a layout
 * successor, the fall-through successor is the same as the layout
 * successor.
 */
void
reflect_cti(CfgNode *node, Instr *cti, CfgNode *implicit_succ)
{
    claim(cti == NULL || find(node->instrs(), cti) != end(node),
	  "reflect_cti() -- non-null cti must already be part of node's code");
    set_cti(node, cti);

    // save the abnormal successor edges
    List<CfgNode*> abnormals;
    List<bool> impossible_flags;
    CfgNodeHandle sh = succs_start(node);
    for (int i = 0; sh != succs_end(node); ++i, ++sh)
	if (*sh != NULL && is_abnormal_succ(node, i)) {
	    abnormals.push_back(*sh);
	    impossible_flags.push_back(is_impossible_succ(node, i));
	}
    // erase successor edges in order to rebuild from scratch
    while (!node->succs().empty())
	remove_succ(node, *succs_start(node));
    node->exc_succs() = 0;
    node->imp_succs() = 0;

    CfgNode *fall_through = NULL;		// remember fall-through, if any

    if (cti == NULL || is_call(cti)) {
	claim(cti == NULL || get_parent(node)->get_break_at_call(),
	      "reflect_cti() -- a call is only a CTI in `break-at-call' mode");
	if (implicit_succ != NULL)
	    set_succ(node, 0, fall_through = implicit_succ);
    } else if (is_ubr(cti)) {
	set_succ(node, 0, label_cfg_node(get_target(cti)));
    } else if (is_cbr(cti)) {
	set_succ(node, 0, fall_through = implicit_succ);
	set_succ(node, 1, label_cfg_node(get_target(cti)));
    } else if (is_return(cti)) {
	set_succ(node, 0, get_exit_node(get_parent(node)));
    } else {
	claim(is_mbr(cti), "reflect_cti() -- instruction isn't a CTI");

	MbrNote note = (const MbrNote&)get_note(cti, k_instr_mbr_tgts);
	claim(!is_null(note));

	int case_count = note.get_case_count();

	set_succ(node, case_count, label_cfg_node(get_target(cti)));

	for (int i = 0; i < case_count; ++i) {
	    LabelSym *target = note.get_case_label(i);
	    set_succ(node, i, label_cfg_node(target));
	}
    }
    claim((fall_through == NULL) || (get_layout_succ(node) == NULL) ||
	  (fall_through == get_layout_succ(node)),
	  "reflect_cti() -- layout successor should be fall-through successor");

    // add any saved abnormal successors after the normal ones
    for (int n = succs_size(node);
	 !abnormals.empty();
	 ++n, abnormals.pop_front(), impossible_flags.pop_front())
	if (impossible_flags.front())
	    set_impossible_succ (node, n, abnormals.front());
	else
	    set_exceptional_succ(node, n, abnormals.front());
}

CfgNode*
get_layout_pred(CfgNode *node)
{
    return node->get_layout_pred();
}

CfgNode*
get_layout_succ(CfgNode *node)
{
    return node->get_layout_succ();
}

/*
 * clear_layout_succ -- Disconnect two nodes in the layout
 * chain.
 */
void
clear_layout_succ(CfgNode *node)
{
    if (get_layout_succ(node)) {
	claim(get_layout_pred(get_layout_succ(node)) == node);
	get_layout_succ(node)->set_layout_pred(NULL);
	node->set_layout_succ(NULL);
    }
}

/*
 * set_layout_succ(label_sym) -- wrapper routine
 * that translates a label successor into the corresponding
 * CfgNode, then calls the other set_layout_succ() routine.
 */
bool
set_layout_succ(CfgNode *node, LabelSym *label)
{
    CfgNode *new_succ = label_cfg_node(label);
    claim(new_succ, "set_layout_succ() -- "
	  "cannot find CFG node for specified label");
    return set_layout_succ(node, new_succ);
}


/*
 *  no_layout_cycle -- Subfunction of set_layout_succ (just below).
 *  Return true unless there's a layout-successor chain from bot to top.
 */
inline bool
no_layout_cycle(CfgNode *top, CfgNode *bot)
{
    do {
	if (bot == top)
	    return false;
    } while ((bot = get_layout_succ(bot)));
    return true;
}

/*
 * set_layout_succ -- Connect two nodes in the layout
 * chain.
 *
 * We really care about the _clump_s that the two nodes represent.  A
 * clump is a maximal set of nodes that are connected by layout
 * pointers. In this method, we connect the top node (this) to the bottom
 * node (bot).  The caller is requesting that bot follow this in layout
 * order.
 *
 * The top node should be the last node in the top clump; the bottom
 * node should be the first node in the bottom clump.  If not, then we
 * are trying to link into the middle of a clump, which is an error.
 *
 * The top and bottom clump should be different.  If they are the same
 * clump, then setting the layout link will create a layout cycle, which is
 * also an error.
 *
 * Use terminate_for_layout to adjust node's CTI for the new layout
 * constraint, and return true if that adjustment inverts a conditional
 * branch.
 */
bool
set_layout_succ(CfgNode *node, CfgNode *bot)
{
    if (bot == NULL) {
	clear_layout_succ(node);
	return false;
    }

    // Verify layout pointers OK, nodes in same cfg
    claim(get_layout_succ(node) == NULL,
	   "set_layout_succ: top node already has a layout successor");
    claim(get_layout_pred(bot) == NULL,
	   "set_layout_succ: bottom node already has a layout predecessor");
    claim(get_parent(node) == get_parent(bot),
	   "set_layout_succ: requested nodes do not belong to the same cfg");
    claim(no_layout_cycle(node, bot),
	   "set_layout_succ: requested link completes a layout cycle");

    node->set_layout_succ(bot);
    get_layout_succ(node)->set_layout_pred(node);

    return terminate_for_layout(node, bot);
}

/*
 * terminate_for_layout -- Subfunction of set_layout_succ and to_instr_list.
 *
 * Adjust the control instruction of a node when its layout successor is
 * set.  If the node has no CTI and its would-be layout successor is not
 * its fall-through successor, it needs an unconditional branch.  On the
 * other hand, if it ends with a ubr to the layout-successor node, its
 * branch can be eliminated.  If it ends with a conditional (2-way) branch
 * and the layout successor node is also a control successor, then we can
 * make the node fall through to the layout successor on some path.  Doing
 * this may require inverting the polarity of the node's conditional
 * branch.  (In this case, terminate_for_layout returns true; otherwise,
 * false.)
 */
bool
terminate_for_layout(CfgNode *node, CfgNode *lsucc)
{
    Instr *cti = get_cti(node);

    if (cti == NULL) {				// append ubr if can't fall thru
	CfgNode *succ0 =
	    (node->succs().empty() ? NULL : node->succs().front());
	if (succ0 && (succ0 != lsucc) && !is_impossible_succ(node, succ0)) {
	    LabelSym *target = get_label(succ0);
	    Instr *mi = new_instr_cti(opcode_ubr(), target);

	    insert_before(node, node->instrs().end(), mi);
	    set_cti(node, mi);
	}
    } else if (is_ubr(cti)) {			// remove ubr if can fall thru
	if (node->succs().front() == lsucc) {
	    node->instrs().erase(get_cti_handle(node));
	    delete cti;
	    set_cti(node, NULL);
	}
    } else if (is_cbr(cti)) {			// invert cbr to allow fall thru
	if (node->succs().front() != lsucc) {
	    claim((node->succs().size() >= 2) &&
		  (*after(node->succs().begin()) == lsucc),
		   "set_layout_succ() -- layout successor of a cbr node "
		   "must be a control successor");
	    invert_branch(node);

	    // swap first and second successors
	    CfgNode *new_2nd = node->succs().front(); node->succs().pop_front();
	    CfgNode *new_1st = node->succs().front(); node->succs().pop_front();
	    node->succs().push_front(new_2nd);
	    node->succs().push_front(new_1st);

	    // set the new branch target
	    set_target(cti, get_label(new_2nd));
	    return true;
	}
    } else
	claim(!is_call(cti) || node->succs().front() == lsucc,
	      "set_layout_succ() -- layout successor of a call node "
	      "must be the fall-through successor");
    return false;
}


CfgNode*
get_pred(CfgNode *node, int pos)
{
    claim(pos <= preds_size(node), "Predecessor node position out of range");
    return node->get_pred(pos);
}

CfgNode*
get_succ(CfgNode *node, int pos)
{
    claim(pos <= succs_size(node), "Successor node position out of range");
    return node->get_succ(pos);
}

/*
 * set_succ -- set the CfgNode succ to be the nth
 * successor of this CfgNode.
 */
void
set_succ(CfgNode *node, int n, CfgNode *succ)
{
    replace_succ(node, n, succ);

    static char *layout_violation =
	"set_succ() -- node has layout successor, but "
	"requested different fall-through successor";

    Instr *cti = get_cti(node);

    if (cti == NULL  || is_call(cti)) {
	claim(n == 0 || is_abnormal_succ(node, n));
	claim(n > 0  ||
	      get_layout_succ(node) == NULL ||	// non-cbr fall-through
	      get_layout_succ(node) == succ,
	      layout_violation);
    }
    else if (is_ubr(cti)) {
	if (n == 0)
	    set_target(cti, get_label(succ));
	else
	    claim(is_abnormal_succ(node, n));
    }
    else if (is_cbr(cti)) {
	if (n == 0) {				// cbr fall-through
	    claim(get_layout_succ(node) == NULL ||
		  get_layout_succ(node) == succ,
		  layout_violation);
	} else if (n == 1) {			// cbr taken
	    set_target(cti, get_label(succ));
	} else {
	    claim(is_abnormal_succ(node, n));
	}
    }
    else if (is_return(cti)) {
	if (n == 0) {
	    claim(succ == get_exit_node(get_parent(node)));
	} else {
	    claim(is_abnormal_succ(node, n));
	}
    }
    else {
	claim(is_mbr(cti));

	MbrNote note = (const MbrNote&)get_note(cti, k_instr_mbr_tgts);
	claim(!is_null(note));

	int case_count = note.get_case_count();
	if (n > case_count) {
	    claim(is_abnormal_succ(node, n));
	} else if (n == case_count) {
	    set_target(cti, get_label(succ));
	} else {
	    note.set_case_label(get_label(succ), n);
	    if (note.get_table_sym() != NULL)
		update_dispatch_table_var(note, n);
	}
    }
}

/*
 * Return an integer representing the bit set of node's successor numbers
 * for which the successor equals succ.  I.e., the (zero-based) "position
 * set" at which succ appears in node's successor sequence.
 */
Integer
succ_pos_set(CfgNode *node, CfgNode *succ)
{
    Integer pos_set = 0;

    CfgNodeHandle h = node->succs().begin();
    for (int i = 0; h != node->succs().end(); ++i, ++h)
	if (*h == succ)
	    pos_set |= (Integer(1) << i);
    return pos_set;
}

bool
is_normal_succ(CfgNode *node, CfgNode *succ)
{
    Integer pos_set = succ_pos_set(node, succ);
    pos_set &= ~node->exc_succs();
    pos_set &= ~node->imp_succs();
    return !!pos_set;
}

bool
is_normal_succ(CfgNode *node, int pos)
{
    Integer pos_mask = Integer(1) << pos;

    return pos < succs_size(node)
	&& (node->exc_succs() & pos_mask) == 0
	&& (node->imp_succs() & pos_mask) == 0;	
}

bool
is_exceptional_succ(CfgNode *node, CfgNode *succ)
{
    return !!(succ_pos_set(node, succ) & node->exc_succs());
}

bool
is_exceptional_succ(CfgNode *node, int pos)
{
    return (node->exc_succs() & (Integer(1) << pos)) != 0;
}

bool
is_possible_succ(CfgNode *node, CfgNode *succ)
{
    return !!(succ_pos_set(node, succ) & ~node->imp_succs());
}

bool
is_possible_succ(CfgNode *node, int pos)
{
    return pos < succs_size(node)
	&& (node->imp_succs() & (Integer(1) << pos)) == 0;	
}

bool
is_impossible_succ(CfgNode *node, CfgNode *succ)
{
    return !!(succ_pos_set(node, succ) & node->imp_succs());
}

bool
is_impossible_succ(CfgNode *node, int pos)
{
    return (node->imp_succs() & (Integer(1) << pos)) != 0;
}

bool
is_abnormal_succ(CfgNode *node, CfgNode *succ)
{
    Integer pos_set = succ_pos_set(node, succ);

    return !!(pos_set & node->imp_succs())
	|| !!(pos_set & node->exc_succs());
}

bool
is_abnormal_succ(CfgNode *node, int pos)
{
    Integer pos_mask = Integer(1) << pos;

    return (node->exc_succs() & pos_mask) != 0
	|| (node->imp_succs() & pos_mask) != 0;	
}

void
set_exceptional_succ(CfgNode *node, int pos, CfgNode *succ)
{
    replace_succ(node, pos, succ);

    Integer pos_mask = Integer(1) << pos;
    node->exc_succs() |=  pos_mask;
    node->imp_succs() &= ~pos_mask;
}

void
set_impossible_succ(CfgNode *node, int pos, CfgNode *succ)
{
    replace_succ(node, pos, succ);

    Integer pos_mask = Integer(1) << pos;
    node->exc_succs() &= ~pos_mask;
    node->imp_succs() |=  pos_mask;
}

void
remove_abnormal_succ(CfgNode *node, CfgNode *succ)
{
    Integer abs = node->exc_succs() | node->imp_succs();
    remove_node(node->succs(), succ, &abs);
    claim(abs != 0, "remove_abnormal_succ: %d isn't "
		    "an abnormal successor of %d",
	  get_number(succ), get_number(node));
}

void
remove_abnormal_succ(CfgNode *node, int pos)
{
    remove_abnormal_succ(node, get_succ(node, pos));
}


/* -------------------  CTI methods  ------------------ */

bool
ends_in_cti(CfgNode *node)
{
    return get_cti(node);
}

bool
ends_in_ubr(CfgNode *node)
{
    Instr *cti = get_cti(node);
    return cti && is_ubr(cti);
}

bool
ends_in_cbr(CfgNode *node)
{
    Instr *cti = get_cti(node);
    return cti && is_cbr(cti);
}

bool
ends_in_mbr(CfgNode *node)
{
    Instr *cti = get_cti(node);
    return cti && is_mbr(cti);
}

bool
ends_in_call(CfgNode *node)
{
    Instr *cti = get_cti(node);
    return cti && is_call(cti);
}

bool
ends_in_return(CfgNode *node)
{
    Instr *cti = get_cti(node);
    return cti && is_return(cti);
}


/*
 * invert_branch -- Invert the branch by changing the opcode.
 * This is usually used in conjunction with set_succ() to
 * swap the targets of a conditional branch while maintaining
 * program correctness.
 * If the branch has edge frequencies, swap them.
 */
void
invert_branch(CfgNode *node)
{
    Instr *cti = get_cti(node);
    claim(cti != NULL && is_cbr(cti));

    set_opcode(cti, opcode_cbr_inverse(get_opcode(cti)));

    static IdString k_branch_edge_weights = "branch_edge_weights";

    if (ListNote<long> note = get_note(node, k_branch_edge_weights)) {
	claim(note.values_size() == 2,
	      "cbr has %d branch edge weights, not 2", note.values_size());
	long temp = note.get_value(0);
	note.set_value(0, note.get_value(1));
	note.set_value(1, temp);
    }
}

/*
 * get_label -- Find the LabelSym that starts a CfgNode.
 * If there isn't one, insert a label instruction at the beginning of
 * the node and return the new label.
 */
LabelSym*
get_label(CfgNode *node)
{
    if (size(node) > 0 && is_label(node->instrs().front()))
	return get_label(node->instrs().front());

    LabelSym *label = new_unique_label(the_suif_env, get_parent(node));
    add_noted_label(node, label);
    insert_before(node, start(node), new_instr_label(label));

    return label;
}

/*
 * add_noted_label -- Library-internal helper to annotate label with the
 * node that it labels, using key "cfg_node".
 */
void
add_noted_label(CfgNode *node, LabelSym *label)
{
    set_note(label, k_cfg_node, OneNote<IrObject*>(node));
    get_parent(node)->noted_labels().push_back(label);
}


/**
 ** -------------  Boundary-instruction methods  ------------
 **/

/*
 * first_active_instr -- Return the first instruction of the node that
 * is not a label or a pseudo-op (such as a line directive).  Return NULL if
 * the node contains only inactive operations of these kinds.  Exclude null
 * instructions from the pseudo-ops, since these instrs should never be
 * discarded.  (FIXME: gag!)
 */
Instr*
first_active_instr(CfgNode *node)
{
    for (InstrHandle h = node->instrs().begin(); h != node->instrs().end(); ++h)
	if (!is_label(*h) && (!is_dot(*h) || is_null(*h)))
	    return *h;

    return NULL;			// node has no active operations
}

/*
 * first_non_label -- Return the first instruction of the node that is
 * not a label.  Return NULL if the node contains only inactive operations
 * of these kinds.
 */
Instr*
first_non_label(CfgNode *node)
{
    
    for (InstrHandle h = node->instrs().begin(); h != node->instrs().end(); ++h)
	if (!is_label(*h))
	    return *h;

    return NULL;			// node has no non-label instructions
}

/*
 * last_non_cti -- Return the last instruction of the node that is not
 * its control-transfer instruction.  Return NULL if the node contains no
 * instructions other than its CTI.
 */
Instr*
last_non_cti(CfgNode *node)
{
    if (node->instrs().empty())
	return NULL;

    InstrHandle h = instrs_last(node);
    do {
	Instr *last = *h;

	if (last != get_cti(node))
	    return last;
	h--;
    } while (h != node->instrs().end());
    
    return NULL;
}

/**
 ** ------------- Node inspection functions -------------
 **/

Cfg*
get_parent(CfgNode *node)
{
    return to<Cfg>(node->get_parent());
}

int
get_number(CfgNode *node)
{
    return node->get_number();
}


/**
 ** ------------- Print functions -------------
 **/

static void
print_node_list(FILE *fp, list<CfgNode*> &l, CfgNode *from, bool direction)
{
    const int buffer_size = 1024;
    static char buffer[buffer_size];
    char *p = buffer;
    *p = '\0';

    for (CfgNodeHandle h = l.begin(); h != l.end(); ++h) {
	CfgNode *to = *h;
	if (to == NULL)
	    p += sprintf(p, "<<null>>");
	else {
	    p += sprintf(p, "%d", get_number(to));

	    CfgNode *head = (direction == FORWARD ? to : from);
	    CfgNode *tail = (direction == REVERSE ? to : from);
	    if (is_exceptional_succ(tail, head))
		strcpy(p++, "x");
	    if (is_impossible_succ(tail, head))
		strcpy(p++, "i");
	}
        if (after(h) != l.end())
            strcpy(p++, " ");
	if (p - buffer >= buffer_size - 12) {
	    warn("print_node_list: Buffer overflow");
	    break;
	}
    }
    fprintf(fp, "%-12s", buffer);
}

void
fprint(FILE *fp, CfgNode *node, bool show_code, bool show_addrs, bool no_header)
{
    if (!no_header) {
	fprintf(fp, "**** Node #%2d: ", get_number(node));
	char *terms;
	Instr *cti = get_cti(node);

	if (cti == NULL)
	    terms = "      ";
	else if (is_ubr(cti))
	    terms = "ubr   ";
	else if (is_cbr(cti))
	    terms = "cbr   ";
	else if (is_mbr(cti))
	    terms = "mbr   ";
	else if (is_call(cti))
	    terms = "call  ";
	else if (is_return(cti))
	    terms = "ret   ";
	else if (is_cti(cti))
	    terms = "CTI?  ";
	fprintf(fp, "%s", terms);

	fprintf(fp, "p ");
	print_node_list(fp, node->preds(), node, REVERSE);
	fprintf(fp, "\ts ");
	print_node_list(fp, node->succs(), node, FORWARD);
	if (get_layout_succ(node))
	    fprintf(fp, "\tl %d", get_number(get_layout_succ(node)));

	putc('\n', fp);
    }
    if (show_code) {
	Printer *printer = target_printer();
	printer->set_file_ptr(fp);

	for (InstrHandle h = start(node); h != end(node); ++h) {
	    if (show_addrs)
		fprintf(fp, "[%lx]\t", (unsigned long)*h);
	    printer->print_instr(*h);
	}
    }
}


/**
 ** ------------- Protected functions -------------
 **/

void
remove_succ(CfgNode *node, CfgNode *succ)
{
    // pos_set is used as both input and output to remove_node.  Passing -1
    // in means remove every matching entry in succs list.  On output, the
    // significant bits of pos_set give the positions of removals.

    Integer pos_set = -1;

    remove_node(node->succs(), succ, &pos_set);
    if (succ != NULL)
	remove_node(succ->preds(), node);
    node->exc_succs() &= ~pos_set;
    node->imp_succs() &= ~pos_set;
}

/*
 * Replace the pos-th successor of node with new_succ.  First find the
 * pos-th slot in node's successor list, creating the list element if
 * necessary.  Save the old successor before putting the new successor in
 * place.  Unless the old successor is still a member of the successors
 * list (i.e., it occurs in other slots than the pos-th), remove node from
 * the predecessors list of the old successor.  Unless new_succ is NULL,
 * make sure that node is in its predecessors list.
 */
void
replace_succ(CfgNode *node, int pos, CfgNode *new_succ)
{
    CfgNodeHandle h = node->succs().begin();
    CfgNodeHandle place;	// place to insert new_succ

    for (int i = 0; i <= pos; ++i, place = h++)
	if (h == node->succs().end())
	    h = node->succs().insert(h, NULL);

    CfgNode *old_succ = *place;
    if (old_succ == new_succ)
	return;

    *place = new_succ;

    if (old_succ != NULL && !contains(node->succs(), old_succ)) {
	h = find(old_succ->preds(), node);
	if (h != old_succ->preds().end())
	    old_succ->preds().erase(h);
    }
    if (new_succ != NULL && !contains(new_succ->preds(), node))
	new_succ->preds().push_back(node);
}


/**
 ** ------------- Static functions -------------
 **/

/*
 * remove_node -- remove mentions of CfgNode node from list seq.
 *
 * If `pos_set' is non-null, it points to the Integer representation of a
 * bit set.  This is both an input and an output variable.  If present, the
 * input bit set is used as a filter: only nodes in the matchin positions
 * will be removed.  The output bit set (if present) is the result of
 * striking out the bits that correspond to non-matching nodes.  So the
 * output bit set represents the positions of the nodes removed.
 *
 */
static void
remove_node(list<CfgNode*> &seq, CfgNode *node, Integer *pos_set)
{
    CfgNodeHandle next = seq.begin();
    for (int i = 0; next != seq.end(); ++i) {
	CfgNodeHandle here = next++;	// can't touch `here' after erasing
	Integer imask;
	if (pos_set)
	    imask = Integer(1) << i;

	if (*here == node)
	{
	    if ((pos_set == NULL) || ((*pos_set & imask) != 0))
		seq.erase(here);
	}
	else if (pos_set)
	{
	    *pos_set &= ~imask;
	}
    }
}
