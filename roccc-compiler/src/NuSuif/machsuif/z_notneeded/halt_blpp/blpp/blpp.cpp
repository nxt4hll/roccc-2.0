/* file "blpp/blpp.cpp" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "blpp/blpp.h"
#endif

#include <machine/machine.h>
#include <cfg/cfg.h>
#include <cfa/cfa.h>
#include <halt/halt.h>
#include <halt_blpp/halt_blpp.h>

#include "blpp.h"

#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

#define STAT_FILE "stat_info.txt"

BlppPass::BlppPass() {
}

void
BlppPass::initialize(void) 
{
    // no initialization needed
}

void
BlppPass::do_opt_unit(OptUnit *unit)
{
    cur_unit = unit;

    // this pass requires a unit's body to be a CFG
    claim(is_kind_of<Cfg>(get_body(cur_unit)), "Body is not in CFG form");
    unit_cfg = static_cast<Cfg*>(get_body(cur_unit));

    
    create_graph();

    // Run BL path profiling algorithm 
    assign_edge_vals();
    choose_st();
    calc_incs();
    // print_graph();
    gen_static_info(); 	// (# procs & for each path in each proc its length in instrs)
    annotate();
}

void 
BlppPass::finalize(void) 
{
    // no finalization needed
}

/************************************************************
   Supporting routines for BL path profiling algorithm 
   Thomas Ball and James Larus, "Efficient Path Profiling"
   (all references to sections and figures refer to this
    paper)
*************************************************************/

/*
 * This routine generates a representation of the CFG for the BL algorithm
 * to use in its analysis.  (A different representation is required bc. 
 * CFGs in Suif have no "physical" edges; they consist of nodes with lists of 
 * preds and succs.  The BL algorithm requires some notion of an edge to which
 * it can attach an increment value, so we use an a new CFG representation from 
 * it is easy to refer back to the original Suif CFG, the one to which this pass
 * attaches its annotations.)
 *
 * There are two fundamental structures that are tied together to build the CFG:
 * nodes and edges.  A Node has two vectors of pointers to edges, one containing 
 * pointers to the edges that lead to the predecessors of the node and one 
 * containing pointers to the edges that lead to the successors.  Each edge 
 * contains a pointer to the node which is its head and a pointer the node which is
 * its tail.
 *
 * Nodes are identified by id number (the same id as the equivalent node in the Suif 
 * CFG).  Edges are identified by the nodes they connect.
 * 
 * All this routine does is walk over the Suif CFG and generate a CFG of the kind 
 * described above.  
 *
 * NOTE: If a backedge is encountered, two dummy edges are put into the generated
 * graph (and flagged as such) according to the rule: backedge t->h becomes dummy 
 * edges entry->h, t->exit.
 */
void
BlppPass::create_graph(void) {
    int entry, exit, curr, head, i, j, d;
    BlppEdge e, *e_addr;
    CfgNode *curr_node, *head_node;

    num_nodes = nodes_size(unit_cfg);
    for (i = 0; i < num_nodes; i++) {
	new_node(i);
    }

    DominanceInfo di(unit_cfg);
    di.find_dominators();
    d = 0;

    entry = get_number(get_entry_node(unit_cfg));
    exit = get_number(get_exit_node(unit_cfg));

    for (i = 0; i < num_nodes; i++) {

	curr_node = get_node(unit_cfg, i);
	curr = get_number(curr_node);

	for (j = 0; j < succs_size(curr_node); j++) {
	    head_node = get_succ(curr_node, j);
	    head = get_number(head_node);

	    if (di.dominates(head_node, curr_node)) {
		d++;
		// backedge
		// edge from entry to i
		e = new_edge(entry, head);
		e.d_id = d;
		edges.push_front(e);
		e_addr = find_edge(entry, head);
		nodes[entry].succs.push_back(e_addr);
		nodes[head].preds.push_back(e_addr);
		// edge from j to exit
		e = new_edge(curr, exit);
		e.d_id = d;
		edges.push_front(e);
		e_addr = find_edge(curr, exit);
		nodes[curr].succs.push_front(e_addr);
		nodes[exit].preds.push_front(e_addr);
	    } else {
		// regular edge
		e = new_edge(curr, head);
		edges.push_front(e);
		e_addr = find_edge(curr, head);
		nodes[curr].succs.push_front(e_addr);
		nodes[head].preds.push_front(e_addr);
	    }
	}
    }

}    

/*
 * Returns a pointer to the edge connecting 'tail' to 'head'.
 */
BlppEdge*
BlppPass::find_edge(int tail, int head) {
    BlppEdge e;

    for (List<BlppEdge>::iterator it = edges.begin(); it != edges.end(); it++) {
	e = *it;
	if ((e.head->id == head) && (e.tail->id == tail)) {
	    return(&(*it));
	}
    }

    claim(false, "Edge %d->%d not found in list!\n", tail, head);
    return(NULL);
}

/*
 * Adds a new node with the given id number to the list of nodes.
 */
void
BlppPass::new_node(int i) {
    BlppNode n;

    claim(i <= num_nodes, "BlppNode does not exist: id num too high.\n");

    n.id = i;
    nodes.push_back(n);
}    

/*
 * Adds a new edge between the specified nodes, returning the edge itself.
 */
BlppEdge
BlppPass::new_edge(int t, int h) {
    BlppEdge e;	
    
    e.head = &nodes[h];
    e.tail = &nodes[t];
    e.chord = false;
    e.events = 0;	
    e.d_id = 0; // non-zero if not a dummy edge
    e.inc = 0;
    e.reset = 0;
    e.inst = false;

    return e;
}    

/*
 * This routine assigns integer values to each edge such that no two
 * paths through the graph (known to be a DAG thanks to dummy edges)
 * have the same sum.  This is an implementation of the pseudocode
 * in Figure 5 of the paper.
 */
void
BlppPass::assign_edge_vals(void) {

    int *num_paths, *rpo;
    int i, v, w;
    BlppEdge e, *e_addr;

    num_paths = new int[num_nodes];
    claim(num_paths, "Unable to create space for temporary arrays.\n");
    
    for (i = 0; i < num_nodes; i++) {
	num_paths[i] = 0;
    }
    rpo = get_rpo();

    // for each vertex v in reverse postorder
    for (i = 0; i < num_nodes; i++) {
	v = rpo[i];

	// if v is a leaf
	if (nodes[v].succs.size() == 0) {
	    num_paths[v] = 1;
	} else {
	    num_paths[v] = 0;
	    // for each edge e v->w
	    for (List<BlppEdge*>::iterator it = nodes[v].succs.begin(); it != nodes[v].succs.end(); it++) {
		e = **it;
		w = e.head->id;
		e_addr = find_edge(v, w);
		e_addr->events = num_paths[v];
		num_paths[v] = num_paths[v] + num_paths[w];
	    }
	}
    }

    free(num_paths);
}
    
/*
 * A helper routine which fills in an array of integers with the node
 * numbers in reverse postorder.
 */
int*
BlppPass::get_rpo(void) {
    // reverse graph and return reverse top. order of nodes
    int* lst = new int[num_nodes];
    int* marked = new int[num_nodes];
    int exit;

    for (int i = 0; i < num_nodes; i++) {
	lst[i] = 0;
	marked[i] = false;
    }

    exit = get_number(get_exit_node(unit_cfg));
    
    tmp = num_nodes-1;
    dfs(lst, marked, nodes[exit]);
    claim(tmp == -1, "Didn't do dfs properly! [tmp=%d]", tmp);

    return lst;
}

/* 
 * Performs a depth first search on the graph starting at node n, 
 * travelling backwards along the edges, searching a node's 
 * predecessors instead of successors. 
 */
void
BlppPass::dfs(int* lst, int* marked, BlppNode n) {
    int p;
    
    for (List<BlppEdge*>::iterator it = nodes[n.id].preds.begin(); 
	 it != nodes[n.id].preds.end(); it++) {
	p = (**it).tail->id;
	if (!marked[p]) {
	    dfs(lst, marked, nodes[p]);
	}
    }

    lst[tmp] = n.id;
    tmp--;
    marked[n.id] = true;
}

/*
 * For debugging purposes, this routine prints out the CFG's structure
 * and all of the information contained in it.
 */
void 
BlppPass::print_graph(void) {
    BlppEdge e;

    printf("NODE INFO:\n");
    for (int i = 0; i < num_nodes; i++) {
	printf("%d --", i);
	printf("\t preds:");
	// for each pred of node i 
	for (List<BlppEdge*>::iterator it = nodes[i].preds.begin(); it != nodes[i].preds.end(); it++) {
	    e = **it;
	    printf("%d ", e.tail->id);
	} 
	printf("\n\t succs:");
	// for each succ of node i 
	for (List<BlppEdge*>::iterator it = nodes[i].succs.begin(); it != nodes[i].succs.end(); it++) {
	    e = **it;
	    printf("%d ", e.head->id);
	}
	printf("\n");
    }

    printf("EDGE INFO:\n");
    printf("tail head events chord d_id inc reset\n");
    printf("---- ---- ------ ----- ---- --- -----\n");

    // for each member e of the edge list
    for (List<BlppEdge>::iterator it = edges.begin(); it != edges.end(); it++) {
	e = *it;
	printf("%4d %4d %6d %5d %4d %3d %5d\n", e.tail->id, e.head->id,
	       e.events, e.chord, e.d_id, e.inc, e.reset);
    }
}

/*
 * This function generates the static output from this pass:
 * A list of procedures in the source file and for each procedure
 * the length (in instrs) of each path through it.
 */
void
BlppPass::gen_static_info(void) {
    FILE* out;
    if ((out = fopen(STAT_FILE, "w")) == NULL) {
	printf("Cannot open %s.\n", STAT_FILE);
	exit(0);
    }

    /* Better way of doing this?  One that doesn't estimate. . . 
       any way to figure out max # paths in advance? */
    int s = nodes.size();
    int max_paths = s * s * s;			// avg. degree of nodes = 3
    PathInfo* path_info = new PathInfo[max_paths];
    if (path_info == NULL) {
	printf("Cannot find space for array of path information.\n");
	exit(0);
    }
    
    for (int i = 0; i < max_paths; i++) {
	path_info[i].instr_len = 0;
    }

    // find entry node
    int entry = get_number(get_entry_node(unit_cfg));
    
    // calculate path lengths
    BlppNode* n = &nodes[entry];
    NatSetDense ns;
    add_lines_to_set(entry, &ns);
    for (unsigned int i = 0; i < n->succs.size(); i++) {
	path_info = update(n->succs[i], path_info, 0, 
			   instrs_size(get_node(unit_cfg, entry)),
			   &ns);
    }

    int total_paths = 0;
    for (int i = 0; i < max_paths; i++) {
	if (path_info[i].instr_len == 0) {
	    break;
	} 
	total_paths++;
    }
    
    LineNote note = get_note(cur_unit, k_line);
    char* src = (char*) note.get_file().chars();
    char* routine = (char*) get_name(get_proc_sym(cur_unit)).chars();
    // write out data
    fprintf(out, "* %d %d %s %s\n", proc_def, total_paths, routine, src);
    for (int i = 0; i < total_paths; i++) {
	fprintf(out, "\t%d:%d ", i, path_info[i].instr_len);
	ns = path_info[i].src_lines;
	ns.print(out);
	fprintf(out, "\n");
    }
    
    // close file
    fclose(out);
}

/* 
 * The following two routines are helper routines which help to
 * generate the static info required by the Hot Path Browser.
 * This info consists of: for each procedure the procedure name and
 * the number of paths in the proc, as well as each path id and the 
 * number of instructions in each path and the lines numbers of the
 * source code executed along the that path.
 *
 * update() recursively searches the graph, calculating and recording
 * each path sum as well as the number of instructions on that path 
 * and the corresponding lines in the source code (which is where 
 * add_lines_to_set() comes in).
 */

PathInfo*
BlppPass::update(BlppEdge* e, PathInfo* cts, int ps, int i_count, NatSet* lines) {
    PathInfo* res = cts;

    int instrs = instrs_size(get_node(unit_cfg, e->head->id));
    i_count += instrs;
    add_lines_to_set(e->head->id, lines);
    
    if (e->head->id == get_number(get_exit_node(unit_cfg))) {    // if you've reached the exit node
	res[ps + e->inc].instr_len = i_count;
	res[ps + e->inc].src_lines = *lines;
	//printf("End of path %d: ", ps+e->inc);
	//lines->print();
	//printf("\n");
	lines->remove_all();
    } else {	// otherwise recurse
	for (unsigned int i = 0; i < e->head->succs.size(); i++) {
	    res = update(e->head->succs[i], res, ps + e->inc, i_count, lines);
	} 
    }
    return res;
}

void
BlppPass::add_lines_to_set(int n, NatSet* s) {
    CfgNode* b = get_node(unit_cfg, n);
    LineNote note;
    int ln;
    
    //    printf("NODE = %d, lines:", n);
    
    for (InstrHandle h = start(b); h != end(b); ++h) {
	Instr* in = *h;
	if (has_note(in, k_line)) {
	    // add line number to set
	    note = get_note(in, k_line);
	    ln = note.get_line();
	    s->insert(ln);
	    //    printf("%d ", ln);
	}
    }  
    //    printf("\n");
}

/*
 * This routine chooses edges to include in the spanning tree.  Chords
 * (edges left out of the tree) will be assigned increments and ultimately
 * instrumented.  
 * If one had knowledge of how frequently each edge was generally executed, 
 * one could modify this routine to calculate a maximal spanning tree, therefore 
 * minimizing the frequency that the chords (and thus instrumentation) is executed.
 */
void
BlppPass::choose_st(void) {
    NatSetDense tree;
    BlppEdge e;

    tree.remove_all();

    // for each edge e in edges 
    for (List<BlppEdge>::iterator it = edges.begin(); it != edges.end(); it++) {
	e = *it;
	if (!tree.contains(e.head->id) || !tree.contains(e.tail->id)) {
	    tree.insert(e.head->id);
	    tree.insert(e.tail->id);
	} else {
	    (*it).chord = true;
	}
    }

}

/*
 * This routine places the annotations on nodes of the original 
 * (Suif) CFG according to the pseudocode in Figure 8.
 */
void
BlppPass::annotate(void) {
    List<int> ws;
    int v, w;
    BlppEdge* e;
    int unique_id = 0;

    // Label procedure (entry and exit)
    label_proc(unique_id++);

    // Calculate path_sum register initialization values which should always 
    // be zero execpt in the case of dummy edges (in which case the increment
    // over the dummy edge leaving the entry node becomes the 'reset' value for 
    // its matching dummy edge going to the exit node).
    // (corresponds to "Register initialization code" pseudocode)
    ws.push_back(get_number(get_entry_node(unit_cfg)));
    while (ws.size() != 0) {
	v = ws.back();
	ws.pop_back();

	// for each edge e out of v
	for (List<BlppEdge*>::iterator it = nodes[v].succs.begin(); it != nodes[v].succs.end(); it++) {
	    e = &(**it);
	    w = e->head->id;
	    if (e->chord && e->d_id) {
		// set reset in match of e to be e.inc
		(dummy_match(e))->reset = e->inc;
	    } else if (nodes[w].preds.size() == 1) {
		ws.push_back(w);
	    }
	}
    }

    // Places heavyweight (memory increment) annotations.  When a dummy edge
    // with an increment is encountered label_heavy() is left to find where 
    // place the label on the "real" (Suif) CFG.
    // (corresponds to "Memory increment code" pseudocode)
    ws.push_back(get_number(get_exit_node(unit_cfg)));
    while (ws.size() != 0) {
	w = ws.back();
	ws.pop_back();
	// for each edge e = v->w (into w) 
	for (List<BlppEdge*>::iterator it = nodes[w].preds.begin(); it != nodes[w].preds.end(); it++) {
	    e = &(**it);
	    if (e->chord && e->reset) {
		// instrument count[r+e->inc]++, reset to e->reset
		label_heavy(e, ++unique_id, e->inc, e->reset);
	    } else if (e->chord) {
		// instrument count[r+e->inc]++, reset to 0
		label_heavy(e, ++unique_id, e->inc, 0);
	    } else if (e->reset) {
		// instrument count[r]++, reset to e->reset
		label_heavy(e, ++unique_id, 0, e->reset);
	    } else if (e->tail->succs.size() == 1) {
		ws.push_back(e->tail->id);
	    } else {
		// instrument count[r]++, reset to 0
		label_heavy(e, ++unique_id, 0, 0);
	    }
	}
    }

    // Finally, the lightweight (path sum register increments) annotations are
    // placed in the Suif CFG.
    for (List<BlppEdge>::iterator it = edges.begin(); it != edges.end(); it++) {
	e = &(*it);
	if (e->chord && !e->inst) {
	    // instrument e s.t. r+=e.inc
	    label_light(e, ++unique_id, e->inc);
	}
    }
}

/*
 * Given a tail and head node (t and h), this routine calculates the index 
 * h in the successor vector of t.
 */
int
BlppPass::get_succ_index(CfgNode* t, CfgNode* h) {
    int i;
    
    for (i = 0; i < succs_size(t); i++) {
	if (get_succ(t, i) == h) {
	    return i;
	}
    }
    return 0;
}

/*
 * Given a dummy edge e, this routine returns a pointer to the 
 * other dummy edge in the pair.
 */
BlppEdge*
BlppPass::dummy_match(BlppEdge* e) {
    BlppEdge* m;

    // for each edge m in the edge list
    for (List<BlppEdge>::iterator it = edges.begin(); it != edges.end(); it++) {
	m = &(*it);
	if ((e->head != m->head) && (e->tail != m->tail) && 
	    (e->d_id == m->d_id)) {
	    return m;
	}
    }

    claim(false, "Unable to find matching dummy edge; graph poorly formed.\n");

    return m;
}

/*
 * Once the edge weights have been calculated, and the chords chosen,
 * this routine walks over the graph and calculates the increment for each 
 * chord to preserve the unique path sum for each path.
 */
void
BlppPass::calc_incs(void) {

    BlppEdge e, *e_addr;

    int entry = get_number(get_entry_node(unit_cfg));
    int exit = get_number(get_exit_node(unit_cfg));

    // add backedge from exit -> entry
    e = new_edge(exit, entry);
    edges.push_front(e);
    e_addr = find_edge(exit, entry);
    nodes[exit].succs.push_front(e_addr);
    nodes[entry].preds.push_front(e_addr);

    dfs_st(0, entry, NULL);

    // for each edge e 
    for (List<BlppEdge>::iterator it = edges.begin(); it != edges.end(); it++) {
	e_addr = &(*it);
	if (e_addr->chord) {
	    e_addr->inc = e_addr->inc + e_addr->events;
	}
    }
}

/*
 * Performs a depth first search on the spanning tree.
 * (Figure out which paper the pseudocode for this came from.)
 */
void
BlppPass::dfs_st(int events, int v, BlppEdge* e){
    BlppEdge* f;

    // for each edge f
    for (List<BlppEdge>::iterator it = edges.begin(); it != edges.end(); it++) {
	f = &(*it);
	if (!f->chord && f != e && f->head->id == v) {
	    dfs_st((dir(e, f)*events + f->events), f->tail->id, f);
	}
    }

    for (List<BlppEdge>::iterator it = edges.begin(); it != edges.end(); it++) {
	f = &(*it);
	if (!f->chord && f != e && f->tail->id == v) {
	    dfs_st((dir(e, f)*events + f->events), f->head->id, f);
	}
    }

    for (List<BlppEdge>::iterator it = edges.begin(); it != edges.end(); it++) {
	f = &(*it);
	if (f->chord && (v == f->tail->id || v == f->head->id)) {
	    f->inc = f->inc + (dir(e, f) * events);
	}
    }
}

/*
 * Given two edges that share atleast one endpoint, this routine 
 * returns 1 if they are pointed in the same direction and -1 if
 * they are pointed in opposite directions.
 */
int
BlppPass::dir(BlppEdge* e, BlppEdge* f) {

    if (!e) {
	return 1;
    } 

    // routine requires that e and f share atleast one endpt.
    claim(((f->head->id == e->head->id) ||
	   (f->tail->id == e->tail->id) ||
	   (f->head->id == e->tail->id) ||
	   (f->tail->id == e->head->id)), 
	  "BlppPass::dir expects BlppEdge operands to share an endpoint.\n");

    if ((f->tail->id == e->head->id) || (f->head->id == e->tail->id)) {
	return 1;
    } else {
	return -1;
    }
}

/*
 * The following three functions place HALT annotations on the Suif
 * CFG.  The placement is determined by an edge, given as a parameter,
 * and the annotation id, increment and reset value are also parameters.
 * label_light() labels path sum register increments (and thus has no 
 * 'reset' parameter).  label_heavy() labels points where the path counts
 * in memory must be updated.  label_proc handles the special case at the
 * procedure entry when the path sum register must be initialized to zero.
 */
void
BlppPass::label_light(BlppEdge* e, int id, int inc) {
    CfgNode* empty;

    CfgNode* tail = get_node(unit_cfg, e->tail->id);
    CfgNode* head = get_node(unit_cfg, e->head->id);    
    Instr* i;

    if (preds_size(head) > 1) {
	// head has multiple preds -> must insert new node on the edge
	empty = insert_empty_node(unit_cfg, tail, head);
	i = new_instr_dot(opcode_null);
	append(empty, i);
    } else {
	// head has only one pred and you can therefore safely put
	// instrumentation at the top of head
	i = first_active_instr(head);
	if (i == NULL) {
	    i = new_instr_dot(opcode_null);
	    append(head, i);
	}
    }

    if_debug(4) {
	fprintf(stderr, "labelling: ");
	fprint(stderr, i);
    }
    long l_id = (long) id;

    HaltLabelNote note(halt::PATH_SUM_INCR, l_id);
    set_note(i, k_halt, note);	     

    note.set_static_arg(0, inc);
    
    e->inst = true;
}

void
BlppPass::label_heavy(BlppEdge* e, int id, int inc, int reset) {
    CfgNode *tail, *head, *empty;
    
    if (e->d_id) {
	tail = get_node(unit_cfg, e->tail->id);
	head = get_node(unit_cfg, (dummy_match(e)->head->id));
    } else {
	tail = get_node(unit_cfg, e->tail->id);
	head = get_node(unit_cfg, e->head->id);    
    }

    // insert empty node and place null instr to label in it
    empty =  insert_empty_node(unit_cfg, tail, head);
    Instr *holder = new_instr_dot(opcode_null);
    append(empty, holder);

    if_debug(4) {
	fprintf(stderr, "labelling: ");
	fprint(stderr, holder);
    }
    long l_id = (long) id;

    HaltLabelNote note(halt::PATH_SUM_READ, l_id);
    set_note(holder, k_halt, note);	     

    note.set_static_arg(0, inc);
    note.set_static_arg(1, reset);
    note.set_static_arg(2, proc_def);

    e->inst = true;
}

void
BlppPass::label_proc(int uid)
{

    // get first successor of entry node, the real procedure entry point
    CfgNode *b = get_entry_node(unit_cfg);
    b = *succs_start(b);

    // create note and instruction to hold entry note
    Instr *holder = new_instr_dot(opcode_null);
    HaltLabelNote entry(halt::PATH_SUM_INIT, uid);
    set_note(holder, k_halt, entry);
    // for static info generation
    proc_def = uid;

    Instr *first = first_non_label(b);
    if (first == NULL) {
	// no non-labels in block so append holder to end of block
	append(b, holder);
    } else {
	// get handle of first
	for (InstrHandle h = start(b); h != end(b); ++h) {
	    if (*h == first) {
		if (has_note(*h, k_proc_entry))
		    insert_after(b, h, holder);
		else
		    insert_before(b, h, holder);
		return;
	    }
	}
	claim(false, "oops, couldn't find first");
    }
}



