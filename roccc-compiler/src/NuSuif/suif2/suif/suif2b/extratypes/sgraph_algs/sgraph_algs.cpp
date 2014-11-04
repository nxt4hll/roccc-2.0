/*  Graph algorithms */

/*  Copyright (c) 1994,1998 Stanford University

    All rights reserved.

    This software is provided under the terms described in
    the "suif_copyright.h" include file. */

#include "common/suif_copyright.h"


#include "sgraph/sgraph.h"
#include "sgraph_algs.h"
#include "sgraph/sgraph_iter.h"
#include "bit_vector/bit_vector.h"
#include "suifkernel/suifkernel_messages.h"
#include "suifkernel/suif_env.h"

/*
 * The ordered graph visitor 
 */

ordered_sgraph_visitor::ordered_sgraph_visitor() :
  _the_sgraph(NULL),
  _visited(NULL),
  _do_forward(true)
{}

ordered_sgraph_visitor::ordered_sgraph_visitor(const SGraph *the_sgraph,
					       bool do_forward) :
  _the_sgraph(the_sgraph),
  _visited(NULL),
  _do_forward(do_forward)
{
  reset(the_sgraph, do_forward);
}
ordered_sgraph_visitor::~ordered_sgraph_visitor() {
  if (_visited != NULL) delete _visited; }

void ordered_sgraph_visitor::reset(const SGraph *the_sgraph,
				   bool do_forward) {
  _the_sgraph = the_sgraph; 
  _do_forward = do_forward;
  if (_visited != NULL) delete _visited;
  _visited = new BitVector;//_the_sgraph->new_node_set();
}

// depth-first postorder
void ordered_sgraph_visitor::visit_postorder(SGraphNode node) {
  // I think this was a bug from the old code.
  _visited->set_bit(node, true);
  for (SNodeIter iter = 
	 _the_sgraph->get_node_successor_iterator(node, _do_forward);
       !iter.done(); iter.increment()) {
    SGraphNode next_node = iter.get();
    /* check if the node has already been visited */
    if (_visited->get_bit(next_node)) continue;
    // Here was the bug.  The entry node might be visited
    // more than once.
    //  _visited->add_member(next_node);
    visit_postorder(next_node);
  }
  do_node_action(node);
}
  
  // depth-first preorder
void ordered_sgraph_visitor::visit_preorder(SGraphNode node) {
  do_node_action(node);
  _visited->set_bit(node, true);
  for (SNodeIter iter(_the_sgraph->get_node_successor_iterator(node, _do_forward));
       !iter.done(); iter.increment()) {
    SGraphNode next_node = iter.get();
    /* check if the node has already been visited */
    if (_visited->get_bit(next_node)) continue;
    visit_preorder(next_node);
  }
}

/*
 * building lists
 */
ordered_sgraph_list_builder::
ordered_sgraph_list_builder(const SGraph *the_sgraph,
			    bool do_forward,
			    SGraphNodeList *lst,
			    bool reverse_list) :
  _list(NULL),
  _reverse_list(reverse_list)
  
{
  reset_builder(the_sgraph, do_forward, lst, reverse_list);
}
    
ordered_sgraph_list_builder::~ordered_sgraph_list_builder() {
  if (_list != NULL) delete _list; }

void ordered_sgraph_list_builder::reset_builder(const SGraph *the_sgraph,
						bool reverse_graph,
						SGraphNodeList *lst,
						bool reverse_list) {
  reset(the_sgraph, reverse_graph);
  if (_list != NULL) delete _list;
  _list = lst;
  _reverse_list = reverse_list;
}

  // Here is the action.  Change this in the subclass
void ordered_sgraph_list_builder::do_node_action(SGraphNode node) {
  if (_reverse_list) {
    _list->push_front(node);
  } else {
    _list->push_back(node);
  }
}



// Build a subgraph with the subset of nodes reachable from
// the entry subset.
BitVector *build_reachable(const SGraph *the_sgraph,
			    SGraphNode node,
			    bool do_forward) {
  
  ordered_sgraph_visitor vis(the_sgraph, do_forward);
  vis.visit_preorder(node);
  return(vis.take_visited());
}

BitVector *build_reachable(const SGraph *the_sgraph,
			   SGraphNodeList *entries,
			    bool do_forward) {
  ordered_sgraph_visitor vis(the_sgraph, do_forward);
  for (SGraphNodeList::iterator iter = entries->begin();
       iter != entries->end(); iter++) {
    vis.visit_preorder(*iter);
  }
  return(vis.take_visited());
}

SGraphNodeList *
build_reverse_postorder_list(const SGraph *the_sgraph,
			     SGraphNode node,
			     bool do_forward) {
  SGraphNodeList *the_list = new SGraphNodeList;
  ordered_sgraph_list_builder vis(the_sgraph, do_forward, the_list, true);
  vis.visit_postorder(node);
  return(vis.take_list());
}

SGraphNodeList *
build_reverse_postorder_list(const SGraph *the_sgraph,
			     const SGraphNodeList *entries,
			     bool do_forward) {
  SGraphNodeList *the_list = new SGraphNodeList;
  ordered_sgraph_list_builder vis(the_sgraph, do_forward, 0, true);
  for (SGraphNodeList::iterator iter = entries->begin();
       iter != entries->end(); iter++) {
    vis.reset_builder(the_sgraph, do_forward, the_list, true);
    SGraphNode node = *iter;
    vis.visit_postorder(node);
    the_list = vis.take_list();
  }
  return the_list;
}

// Fill in domination.
void build_dominators(SGraphBit *domination,
		      const SGraph *reachable,
		      SGraphNodeList *rev_postord_list,
		      SGraphNode start,
		      bool do_forward) {
  SGraphNodeList *node_list = rev_postord_list;
  //    build_reverse_postorder_list(reachable, node, do_forward);

  //size_t max_num_nodes = reachable->max_num_nodes();

  bool changed = false;

  //  SGraphBit *domination = new SGraphBit;
  domination->add_nodes(reachable);
  SGraph *dom = domination;
  // Set its size to the same as the other

  //  domination->set_fully_connected();
  domination->add_all_edges();
    
  /* set up the first node */
  // initialize for no outgoing edges.
  //  domination->remove_edges_from(start);
  dom->remove_node_successor_edges(start, do_forward);
  
  // a node dominates itself.
  domination->add_edge(SGraphEdge(start, start));
  

  BitVector *node_set = reachable->new_node_set();

  BitVector t;

  /* iterate until no changes */
  do {
    changed = false;
    for (SGraphNodeList::iterator node_iter = node_list->begin();
	 node_iter != node_list->end(); node_iter++) {
      SGraphNode current_node = *node_iter;

      if (current_node == start) continue;
      BitVector *local_val = 
	domination->new_node_successors(current_node, do_forward);
      
      /* get intersection of predecessors of n */
      // Set all
      //t.universal();
      t = (*node_set);

      // @@@ is this successors or preds?
      for (SNodeIter pred_iter(reachable->get_node_predecessor_iterator(current_node, do_forward));
	   !pred_iter.done(); pred_iter.increment()) {
	SGraphNode pred_node = pred_iter.get();
	
	//t *= d[p->number()];
	// Intersect t with the set of successor nodes of this predecessor.
	//	size_t num_bits = t.num_significant_bits();
	for (BitVectorIter bv_iter(&t); bv_iter.is_valid(); bv_iter.next()) {
	  //	for (size_t this_node = 0; this_node < num_bits; this_node++) {
	  SGraphNode this_node = bv_iter.current();
	  if (this_node == current_node) continue; // keep it.
	  if (!dom->is_edge_member(SGraphEdge(pred_node, this_node),
				   do_forward)) {
	    t.set_bit(this_node, false);
	    //	    local_changed = true;
	  }
	}
      }
	
      /* include itself in dominator set */
      //    t.add(n->number());
      if (!t.get_bit(current_node)) {
	//	local_changed = true;
	t.set_bit(current_node, true); }

      if (!(t == (*local_val))) {
	/* check if there were any local changes */
	//      if (local_changed) {
	changed = true;
	// copy the temp into the domination
	// clear the dominator
	// set_node_sucessors (or preds) a subset.
	
	domination->set_node_successors(current_node, &t, do_forward);
      }
      delete local_val;
    }
  } while (changed);
  delete node_set;
  
}

/*
 *  Find the immediate (post)dominators.
 */
/*
#define NO_IMMED_DOM ((size_t)-1)
array_tos<SGraphNode> *build_immediate_dominators(
			    const SGraph *reachable, 
			    const SGraph *domination, 
			    tos<SGraphNode> *rev_postord_list,
			    SGraphNode root,
			    bool do_forward) {
			    */
void build_immediate_dominators(SGraph *immed_doms,
				const SGraph *reachable, 
				const SGraph *domination, 
				SGraphNodeList *rev_postord_list,
				SGraphNode root,
				bool do_forward) {
  //unsigned num = reachable->max_num_nodes();
  //  unsigned root = (forward ? entry_node()->number() : exit_node()->number());
  //  Bit_set *d = (forward ? doms : pdoms);
  /*
  array_tos<SGraphNode> *immed_doms = 
    new array_tos<SGraphNode>;
  for (unsigned i = 0; i < num; i++) {
    //  cfgraph_node **im = new cfgraph_node*[num];
    immed_doms->set_elem(i, NO_IMMED_DOM);

    //  im[root] = NULL;
  } 
  */
  // Iterate over all of the nodes
  for (SNodeIter iter(domination->get_node_iterator());
       !iter.done(); iter.increment()) {

    //  for (unsigned n = 0; n < num; n++) {
    SGraphNode current_node = iter.get();
    
    /* the root node has no dominators */
    if (current_node == root) continue;
    
    /* remove self for comparison */
    //    domination->remove_edge(n, n);
    //    d[n].remove(n);
    
    /* check nodes in dominator set */
    bool found = false;

    // iterate over all of the possible predecessors
    for (SNodeIter iter2(domination->get_node_iterator());
	 !iter2.done(); iter2.increment()) {
    
      //    bit_set_iter diter(&d[n]);
      //    while (!diter.is_empty()) {
      SGraphNode check_node = iter2.get();
      if (check_node == current_node) continue;

      // Now iterate over the successors of current_node and check_node
      // at the same time.  Ignore self.
      SNodeIter current_iter(domination->get_node_successor_iterator(current_node, do_forward));
      SNodeIter check_iter(domination->get_node_successor_iterator(check_node, do_forward));

      bool not_found = false;
      // The easier way:
      // get the first, remove it
      // then compare.
      for (; !current_iter.done() && !check_iter.done();
	   current_iter.increment(), check_iter.increment()) {
		  
	//      while (!current_iter.done() &&
	//	     !check_iter.done()) {
	//	SGraphNode current_edge = current_iter.get();
	if (current_node == current_iter.get()) {
	  current_iter.increment();
	  if (current_iter.done()) break;
	}
	//	SGraphNode check_edge = check_iter.get();
	if (current_iter.get() != check_iter.get()) {
	  not_found = true;
	  break;
	}
      } 
      if (!not_found &&
	  !current_iter.done() &&
	  check_iter.done()) {
	if (current_iter.get() == current_node) {
	  current_iter.increment();
	}
      }

      if (!not_found &&
	  current_iter.done() &&
	  check_iter.done()) {
	found = true;
	immed_doms->add_edge(SGraphEdge(current_node,check_node));
	break;
      }
    }
	    
    if (!found) {
      suif_warning("can't find idom of node %u", current_node);
    }
    
  }
}



/*
 *  Compute dominance frontiers using a postorder traversal of the dominator
 *  tree.  This is a protected method that is only used internally.
 */

/*
 * This could also be called
 * get_the_only successor
 */
static SGraphNode get_idom(const SGraph *immed_dom,
			    SGraphNode node) {
  // This feels backwards.
  //@@@  
  SNodeIter iter(immed_dom->get_node_successor_iterator(node));
  assert(!iter.done());
  SGraphNode idom = iter.get();
  iter.increment();
  assert(iter.done());
  return(idom);
}

static bool has_idom(const SGraph *immed_dom,
		     SGraphNode node) {
  // This feels backwards.
  //@@@  
  SNodeIter iter(immed_dom->get_node_successor_iterator(node));
  bool val = !iter.done();
  return(val);
}
/* The graph of dominance frontiers is place into df_graph. */
/* It should be initialized to the size of the SGraph */
void
build_dominance_frontiers (SGraphBit *df_graph,
			   const SGraph *the_sgraph,
			   const SGraph *immediate_dominators,
			   SGraphNode x,
			   bool do_forward) {
  
  unsigned n;
  size_t num = df_graph->max_num_nodes();

  /*
    graph_edge_list *nodes;

    if (forward) {
	nodes = entry->successors();
    } else {
	df_graph = rdf;
	nodes = entry->predecessors();
    }
    */
  /* visit all children (i.e. immediate dominatees) first */
  for (n = 0; n < num; n++) {
    if (n == x) continue;
    if (!has_idom(immediate_dominators, n)) continue;
    //    if (x == get_top_region()->>is_exit()) continue;
    if (get_idom(immediate_dominators, n) == x) {
      build_dominance_frontiers(df_graph, 
				the_sgraph,
				immediate_dominators,
				n, do_forward);
    }
  }

  /* calculate dominance frontier, from paper RCytron_89 */

  //  df_graph[x->number()].expand(0, num);

  /* local loop, uses CFG */
  for (SNodeIter succ_iter(the_sgraph->get_node_successor_iterator(x, do_forward));
       !succ_iter.done(); succ_iter.increment()) {
    SGraphNode s = succ_iter.get();
    
    if (x != get_idom(immediate_dominators, s)) {
      df_graph->add_edge(SGraphEdge(x, s));
    }
  }
  
  /* up loop, uses dominator tree */
  for (n = 0; n < num; n++) {
    if (has_idom(immediate_dominators, n) &&
	(get_idom(immediate_dominators, n) == x) && 
	df_graph->node_has_successors(n)) {
      for (unsigned y = 0; y < num; y++) {
	if (df_graph->is_edge_member(SGraphEdge(n, y)) && 
	    (x != get_idom(immediate_dominators, y))) {
	  df_graph->add_edge(SGraphEdge(x, y));
	}
      }
    }
  }
}

// return the relation
// {(x,y)} forall y in the iterated_dom_frontier of x
// in the df_graph.
// This algorithm was lifted from wilbyr/setup.cc
void
build_iterated_dominance_frontiers (SGraph *idf_graph,
				    const SGraph *the_sgraph,
				    const SGraph *dominators)
{
  // Put the iterated dominance frontier
  // into the idf_graph as a relation.

  BitVector visited;
  SGraphNodeList work_list;
  idf_graph->add_nodes_from_sgraph(the_sgraph);

  /*  compute the iterated dominance frontier */
  for (SNodeIter iter = the_sgraph->get_node_iterator();
       iter.is_valid(); iter.next()) {
    SGraphNode n = iter.current();

    visited.set_to_zero();
    visited.set_bit(n, true);
    work_list.push_back(n);

    while (!work_list.empty()) {
      SGraphNode x = work_list.back();
      work_list.pop_back();
      for (SNodeIter dom_iter = dominators->get_node_successor_iterator(x);
	   dom_iter.is_valid(); dom_iter.next()) {
	SGraphNode y = dom_iter.current();
	SGraphEdge edge(n, y);
	if (!idf_graph->is_edge_member(edge)) {
	  idf_graph->add_edge(edge);
	  if (!visited.get_bit(y)) {
	    visited.set_bit(y, true);
	    work_list.push_back(y);
	  }
	}
      }
    }
  }
    
}
  


static void do_scc_dfs(const SGraph *graph,
		       SGraphNode src, 
		       suif_vector<int> &node_id, 
		       suif_vector<int> &dfsnum, 
		       int &starttime, 
		       list<int> &SC, 
		       BitVector &touched, 
		       suif_vector<int> &low,
		       list<int> &dfslist) {

  dfslist.push_back(src);
  //int srcid = node_id[src]; // get an id number
  starttime++;
  int dfsid = starttime;
  dfsnum[src] = dfsid;
  low[src] = dfsid;
  
//#  print "Dfs->$src $dfsid\n";
  touched.set_bit(src, 0);

  for (SNodeIter iter(graph->get_node_successor_iterator(src));
       !iter.done(); iter.increment()) {
    SGraphNode dst = iter.get();
    
    //int dstid = node_id[dst];
    if (dfsnum[dst] == 0) {
      do_scc_dfs(graph, dst, node_id, dfsnum, 
		 starttime, SC, touched, low, dfslist);
      
      if (low[dst] < low[src]) {
	low[src] = low[dst];
      }
    } else {
      if (! touched.get_bit(dst) ) {
	if (dfsnum[dst] < low[src]) {
	  low[src] = dfsnum[dst];
	}
      }
    }
  }
  if (low[src] == dfsnum[src]) {
    touched.set_bit(src, true);
    while (SC.length() != 0 &&
	   dfsnum[SC.front()] > dfsnum[src]) {
      touched.set_bit(SC.front(), true);
      SC.pop_front();
    }
  } else {
    SC.push_front(src);
  }
}


suif_vector<int> *build_scc(const SGraph *the_sgraph) {
  int starttime = 0; // global val
  int size = the_sgraph->max_num_nodes();
  list<int> SC;
  BitVector touched;
  suif_vector<int> node_id(size);
  suif_vector<int> *dfsnum = new suif_vector<int>(size);
  suif_vector<int> low(size);
  int id = 1;

  list<int> dfslist;

  for (int i = 0; i < size; i++) {
    (*dfsnum)[i] = 0;
    node_id[i] = 0;
  }
  /*
  for(SNodeIter iter(the_sgraph->get_node_iterator());
      !iter.done(); iter.increment()) {
    dfsnum[*iter] = 0;
  }
  */

  for(SNodeIter iter(the_sgraph->get_node_iterator());
      !iter.done(); iter.increment()) {
    SGraphNode src = iter.get();
    if (node_id[src] != 0) {
      node_id[src] = id;
      id++;
    }
    //SGraphNode srcid = node_id[src];
    if ((*dfsnum)[src] == 0) {
      do_scc_dfs(the_sgraph, src, node_id, *dfsnum, 
		 starttime, SC, touched, low,
		 dfslist);
    }
  }
  return(dfsnum);
}









/** The following functions implements the Stringly_Connected_Components
  * algorithm as described in "Algorithms" by Thomas Cormen.
  */

/** First phase of SCC.  Find the f() mapping in a dfs traversal starting
  * from a node.
  * @param src starts dfs from this node.
  * @param the_sgraph IN the sgraph
  * @param finished OUT contains the end result, a list of node ids.
  *        Node ids are appended in post order fashion.  Almost like
  *        the f() in "Algorithm" except that in reverse mapping and no
  *        d().
  * @param visited IN/OUT used internally for stopping recursion.
  *        A node is in visited iff it is visited by this function; i.e.
  *        either colored grey or black.
  */
static void find_dfs_finished(SGraphNode src, const SGraph *the_sgraph,
			      suif_vector<SGraphNode> *finished,
			      suif_vector<SGraphNode> *visited)
{
  if (visited->is_member(src)) return;
  visited->push_back(src);
  for (SNodeIter iter(the_sgraph->get_node_successor_iterator(src));
       !iter.done(); iter.increment()) {
    SGraphNode dst = iter.get();
    find_dfs_finished(dst, the_sgraph, finished, visited);
  }
  finished->push_back(src);
}
   
/** Second phase of SCC.  Each call to this function will get one SCC
  * component.
  * @param src        IN the forefather node of this new SCC component.
  * @param the_sgraph IN the sgraph.
  * @param sccid      IN the scc id for this new scc component.
  * @param nodeid_sccid OUT mapping nodeid to sccid.  If nodeid is not in
  *        \a the_sgraph, nodeid_sccid[nodeid] == -1.
  */
static void get_scc_subgraph(SGraphNode srcnode,
			     const SGraph *the_sgraph,
			     int sccid,
			     suif_vector<int>* nodeid_sccid)
{
  if ((*nodeid_sccid)[srcnode] != -1) return;
  (*nodeid_sccid)[srcnode] = sccid;
  for (SNodeIter iter(the_sgraph->get_node_predecessor_iterator(srcnode));
       !iter.done(); iter.increment()) {
    SGraphNode prednode = iter.get();
    get_scc_subgraph(prednode, the_sgraph, sccid, nodeid_sccid);
  }
}



unsigned build_nodeid_to_sccid(const SGraph *the_sgraph,
			       suif_vector<int>* nodeid_sccid)
{
  suif_vector<SGraphNode> finished;
  suif_vector<SGraphNode> fvisited;
  for(SNodeIter iter(the_sgraph->get_node_iterator());
      !iter.done();
      iter.increment()) {
    SGraphNode src = iter.get();
    if (!finished.is_member(src))
      find_dfs_finished(src, the_sgraph, &finished, &fvisited);
  }
  int sccid = 0;
  {for (SGraphNode i=0; i<nodeid_sccid->length(); i++) {
    (*nodeid_sccid)[i] = -1;
  }}

  {for (SGraphNode i=nodeid_sccid->length();
       i<the_sgraph->max_num_nodes();
       i++) {
    nodeid_sccid->push_back(-1);
  }}

  for (SGraphNode i1=finished.length(); i1>0; i1--) {
    unsigned i = i1-1;
    SGraphNode src = finished[i];
    if ((*nodeid_sccid)[src] == -1) {
      get_scc_subgraph(src, the_sgraph, sccid, nodeid_sccid);
      sccid++;
    }
  }
  return sccid;
}
    


unsigned build_scc_graph(const SGraph* the_sgraph,
			 suif_vector<int>* nodeid_sccid,
			 SGraph* scc_graph)
{
  unsigned new_node_cnt = 0;
  for (SGraphNode srcnode = 0; srcnode<nodeid_sccid->length(); srcnode++) {
    int srcscc = (*nodeid_sccid)[srcnode];
    if (srcscc == -1) continue;
    scc_graph->add_node(srcscc);
    for (SNodeIter iter(the_sgraph->get_node_successor_iterator(srcnode));
	 !iter.done();
	 iter.increment()) {
      int dstscc = (*nodeid_sccid)[iter.get()];
      if (dstscc==-1) continue;
      if (dstscc == srcscc) continue;
      new_node_cnt++;
      scc_graph->add_node(dstscc);
      scc_graph->add_edge(SGraphEdge(srcscc, dstscc));
    }
  }
  return (new_node_cnt);
}


unsigned build_scc_subgraph(const SGraph* the_sgraph,
			    suif_vector<int>* nodeid_sccid,
			    int sccid,
			    SGraph* scc_subgraph)
{
  unsigned new_node_cnt = 0;
  for (SGraphNode srcnode=0; srcnode<nodeid_sccid->length(); srcnode++) {
    if (nodeid_sccid->at(srcnode) != sccid) continue;
    if (!scc_subgraph->is_node_member(srcnode)) {
      scc_subgraph->add_node(srcnode);
      new_node_cnt++;
    }
    for (SNodeIter iter(the_sgraph->get_node_successor_iterator(srcnode));
	 !iter.done();
	 iter.increment()) {
      SGraphNode dstnode = iter.get();
      if (nodeid_sccid->at(dstnode) != sccid) continue;
      if (!scc_subgraph->is_node_member(dstnode)) {
	scc_subgraph->add_node(dstnode);
	new_node_cnt++;
      }
      if (!scc_subgraph->is_edge_member(SGraphEdge(srcnode, dstnode)))
	scc_subgraph->add_edge(SGraphEdge(srcnode, dstnode));
    }
  }
  return (new_node_cnt);
}
  







static void node_print(SGraphNode node, 
		       const SGraph *the_sgraph, ion *out,
		       suif_vector<String> *name_map) {
  if (name_map &&
      name_map->size() > node) {
    String s = (*name_map)[node];
    if (s != emptyString) {
      out->printf("%s", s.c_str());
      return;
    }
  }
  the_sgraph->print_node(out, node);
}  

void export_dot(const SGraph *the_sgraph, ion *out){
    out->printf("digraph foo {\nsize = \"8,10\";\n");

    SNodeIter iter(the_sgraph->get_node_iterator());
    // Print out nodes names and their labels
    for (; !iter.done(); iter.increment()) {
      SGraphNode node = iter.get();
      the_sgraph->print_node(out, node);
      out->printf(" [");
      out->printf("shape=box, label=\"");
      the_sgraph->print_node(out, node);
      out->printf("\"];\n");
    }

    iter = the_sgraph->get_node_iterator();
    // Print out each connecting edge
    for (; !iter.done(); iter.increment()) {
      SGraphNode from_node = iter.get();
      for (SNodeIter iter2(the_sgraph->get_node_successor_iterator(from_node));
	   !iter2.done(); iter2.increment()) {
	SGraphNode to_node = iter2.get();
	the_sgraph->print_node(out, from_node);
	out->printf(" -> ");
	the_sgraph->print_node(out, to_node);
	out->printf("\n");
      }
    }
    out->printf("}\n");
}

void export_named_dot(const SGraph *the_sgraph, ion *out,
                      suif_vector<String>* name_map){
    out->printf("digraph foo {\nsize = \"8,10\";\n");

    SNodeIter iter(the_sgraph->get_node_iterator());
    // Print out nodes names and their labels
    for (; !iter.done(); iter.increment()) {
      SGraphNode node = iter.get();
      the_sgraph->print_node(out, node);
      out->printf(" [");
      out->printf("shape=box, label=\"");
      node_print(node, the_sgraph, out, name_map);
      out->printf("\"];\n");
    }

    iter = the_sgraph->get_node_iterator();
    // Print out each connecting edge
    for (; !iter.done(); iter.increment()) {
      SGraphNode from_node = iter.get();
      for (SNodeIter iter2(the_sgraph->get_node_successor_iterator(from_node));
	   !iter2.done(); iter2.increment()) {
	SGraphNode to_node = iter2.get();
    the_sgraph->print_node(out, from_node);
	out->printf(" -> ");
    the_sgraph->print_node(out, to_node);
    //out->printf("%s", (*name_map)[to_node].c_str());
	out->printf("\n");
      }
    }
    out->printf("}\n");
}

void print_named(const SGraph *the_sgraph, ion *out,
		 suif_vector<String> *name_map)
{
  //    out->printf("digraph foo {\nsize = \"8,10\";\n");
  out->printf("BEGIN GRAPH\n");
  // Print out nodes names and their labels
  {for (SNodeIter iter(the_sgraph->get_node_iterator());
       !iter.done(); iter.increment()) {
    SGraphNode node = iter.get();
    out->printf(" %d{", node);
    node_print(node, the_sgraph, out, name_map);
    out->printf("}\n");
  }}

  
  // Print out each connecting edge
  {for (SNodeIter iter = the_sgraph->get_node_iterator(); 
       !iter.done(); iter.increment()) {
    SGraphNode from_node = iter.get();
    for (SNodeIter iter2(the_sgraph->get_node_successor_iterator(from_node));
	   !iter2.done(); iter2.increment()) {
	SGraphNode to_node = iter2.get();
	out->printf(" %d{", from_node);
	node_print(from_node, the_sgraph, out, name_map);
	out->printf("} ");
	out->printf(" -> ");
	out->printf(" %d{", to_node);
	node_print(to_node, the_sgraph, out, name_map);
	out->printf("} ");
	out->printf("\n");
      }
    }}
    out->printf("END GRAPH\n");
}

// Assume the given graph is clear
void
build_reverse_graph (SGraph *reverse, const SGraph *the_graph) {
  reverse->add_nodes_from_sgraph(the_graph);
  for (SEdgeIter iter(the_graph->get_edge_iterator());
       !iter.done(); iter.increment()) {
    SGraphEdge edge = iter.get();
    reverse->add_edge(edge.reverse());
  }
}




class SuifEnv;
extern "C" void init_sgraph_algs(SuifEnv *s) {
  s->require_module("sgraph");
}
