
#ifndef GRAPH_ALGS_H
#define GRAPH_ALGS_H

// Algorithms for 
//  dominators,
//  Strongly connected components
//  postorder, reverse postorder on forward and backward
//    graphs
// 
//  David Heine
//
#include "bit_vector/bit_vector_forwarders.h"
#include "sgraph/sgraph.h"
#include "sgraph/ngraph.h"
#include "sgraph/sgraph_bit.h"
#include "sgraph/sgraph_list.h"
#include "common/suif_vector.h"
#include "common/MString.h"

class ordered_sgraph_visitor {
  const SGraph *_the_sgraph;
  BitVector *_visited;  // This will be reachable set.
  bool _do_forward;        // on the graph or the graph with reversed
                                 // edges
public:
  ordered_sgraph_visitor();
  ordered_sgraph_visitor(const SGraph *the_sgraph, bool do_forward);
  virtual ~ordered_sgraph_visitor();

  void reset(const SGraph *the_sgraph, bool do_forward);

  // depth-first postorder
  void visit_postorder(SGraphNode node);
  
  // depth-first preorder
  void visit_preorder(SGraphNode node);

  const SGraph *get_sgraph() const { return _the_sgraph; }
  const BitVector *get_visited() const { return _visited; }
  BitVector *take_visited() { 
    BitVector *vis = _visited; _visited = NULL; return(vis); }

  // =>=>
  // Here is the action.  Change this in the subclass
  //
  virtual void do_node_action(SGraphNode node) {}
private:
    /* avoid default assignment ops */
    ordered_sgraph_visitor &operator =(const ordered_sgraph_visitor &);
    ordered_sgraph_visitor(const ordered_sgraph_visitor &);
};

class ordered_sgraph_list_builder : public ordered_sgraph_visitor {
  //  slist_tos<SGraphNode> *_list;
  SGraphNodeList *_list;
  bool _reverse_list;
public:
  // List ownership is passed into the constructor.
  ordered_sgraph_list_builder(const SGraph *the_sgraph,
			      bool do_forward,
			      //      slist_tos<SGraphNode> *list,
			      SGraphNodeList *list,
			      bool reverse_list);
  virtual ~ordered_sgraph_list_builder();
  void reset_builder(const SGraph *the_sgraph,
		     bool reverse_graph,
		     //		     slist_tos<SGraphNode> *list,
		     SGraphNodeList *list,
		     bool reverse_list);
  bool get_reverse_list() const { return _reverse_list; }
  SGraphNodeList *get_list() const { return _list; }

  SGraphNodeList *take_list() { 
    SGraphNodeList *vis = _list; _list = NULL; return(vis); }

  // Here is the action.  Change this in the subclass
  virtual void do_node_action(SGraphNode node);
};



// Build a subgraph with the subset of nodes reachable from
// the entry subset.
BitVector *build_reachable(const SGraph *the_sgraph,
			    SGraphNode node,
			    bool do_forward);

BitVector *build_reachable(const SGraph *the_sgraph,
			    SGraphNodeList *entries,
			    bool do_forward);

SGraphNodeList *
build_reverse_postorder_list(const SGraph *the_sgraph,
			     SGraphNode node,
			     bool do_forward);

SGraphNodeList *
build_reverse_postorder_list(const SGraph *the_sgraph,
			     const SGraphNodeList *entries,
			     bool do_forward);

void build_dominators(SGraphBit *df,
		      const SGraph *reachable,
		      SGraphNodeList *rev_postord_list,
		      SGraphNode start,
		      bool do_forward);

/*
 *  Find the immediate (post)dominators.
 */

void build_immediate_dominators(SGraph *immed_doms,
				const SGraph *reachable, 
				const SGraph *domination, 
				SGraphNodeList *rev_postord_list,
				SGraphNode root,
				bool do_forward);

void
build_dominance_frontiers (SGraphBit *df_graph,
			   const SGraph *the_sgraph,
			   const SGraph *immediate_dominators,
			   SGraphNode x,
			   bool do_forward);


void
build_iterated_dominance_frontiers (SGraph *idf_graph,
				    const SGraph *the_sgraph,
				    const SGraph *dominators);

// build the reverse graph
void
build_reverse_graph (SGraph *reverse, const SGraph *the_graph);


// This algorithm will take a directed graph and output
// a mapping from nodes to an SCC id.
// Nodes with the same SCC are in a strongly connected
// region.  When following an edge to a smaller numbered region
// we are moving out of a sub region.
//  When following an edge to a higher numbered region,
// we are moving into a sub-region.
//
// We use this algorithm to build a graph of sub-graphs
// out of a control flow graph.  In this case, we will
// insert explicit region entry and exit nodes 
// We will also mark some of the regions as Loops
// when they are single entry.
//
suif_vector<int> *build_scc(const SGraph *the_sgraph);

/** Build a nodeid -> sccid map.  Each sccid identifies a
  *   Strongly-Connected-Component.
  * @param the_sgraph   IN the original SGraph.
  * @param nodeid_sccid OUT the mapping from nodeid to sccid.
  * @return number of scc components found.
  */
unsigned build_nodeid_to_sccid(const SGraph* the_sgraph,
			       suif_vector<int>* nodeid_sccid);


/** Build a SCC graph, where each node is a sccid.
  * @param the_sgraph   IN the original SGraph.
  * @param nodedi_sccid IN the nodeid to sccid map, output of
  *                        build_nodeid_to_sccid().
  * @param scc_graph   OUT the graph of sccid.
  * @return number of nodes (sccid) added to \a scc_graph.
  * 
  * This function will add nodes and edges to \a scc_graph.
  * Each node added represents a new scc component.
  */
unsigned build_scc_graph(const SGraph* the_sgraph,
			 suif_vector<int>* nodeid_sccid,
			 SGraph* scc_graph);

/** Build a SCC subgraph, where all nodes belong to the same SCC component.
  * @param the_sgraph   IN the original SGraph.
  * @param nodeis_sccid IN the nodeid to sccid map, output of
  *                        build_nodeid_to_sccid().
  * @param sccid        IN identifies a scc component.
  * @param scc_subgraph OUT containing all nodes of the scc component.
  * @return number of nodes added to \a scc_subgraph.
  */
unsigned build_scc_subgraph(const SGraph* the_sgraph,
			    suif_vector<int>* nodeid_sccid,
			    int sccid,
			    SGraph* scc_subgraph);
/**
	Export non-named dot, unlike the following two functions.
*/
void export_dot(const SGraph *the_sgraph, ion *out);

void export_named_dot(const SGraph *the_sgraph, ion *out,
		      suif_vector<String> *node_name_map);

void print_named(const SGraph *the_sgraph, ion *out,
		 suif_vector<String> *node_name_map);

#endif /* GRAPH_ALGS_H */
