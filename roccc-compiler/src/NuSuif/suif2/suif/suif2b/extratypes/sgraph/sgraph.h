/*  SGraph.h */

/*  Copyright (c) 1998 Stanford University

    All rights reserved.

    This software is provided under the terms described in
    the "suif_copyright.h" include file. */

#include "common/suif_copyright.h"

#ifndef SGRAPH_H
#define SGRAPH_H

#include "sgraph_forwarders.h"
#include <common/MString.h>
#include <common/suif_hash_map.h>

/*
 * SGraph - library.
 *
 * This is a basic graph where ALL nodes are identified
 * as numbers.
 * Edges are not represented explicitly.
 *   However there is a value class that contains two nodes
 *   and provides for edge comparison.
 * 
 * we have 2 default implementations
 *   SGraphBit - a vector of bit vectors to represent the relation
 *   SGraphList - a vector of successor/predecessor lists
 *                 that represent the relations
 *
 *
 */

#include "ion/ion_forwarders.h"
#include "common/suif_list.h"
#include "bit_vector/bit_vector_forwarders.h"

//typedef SGraphNodeList::iterator sgraph_node_list_iter;


class SGraphNodeIter;
class SGraphEdgeIter;

class SGraphEdge {
  SGraphNode _from;
  SGraphNode _to;
public:
  SGraphEdge(SGraphNode from, SGraphNode to) : _from(from), _to(to) {}
  SGraphNode to() const { return(_to); }
  SGraphNode from() const { return(_from); }
  SGraphEdge reverse() const { return(SGraphEdge(_to, _from)); }
};

/*
 * There are 3 common forms for passing around sets of nodes.
 * 1) use a BitVector
 * 2) use a SGraphNode_list  i.e. a list of node numbers
 * 3) use the nodes in a different SGraph.
 *
 */

typedef list<SGraphEdge> SGraphEdgeList;

#include "sgraph_iter.h"

class SGraph {
public:
  //typedef SGraphNodeIter node_iterator; // iterator over nodes
  //typedef SGraphEdgeIter edge_iterator; // iterator over edges

  virtual ~SGraph();
  // Nodes
  // maximum node number +1
  virtual SGraphNode max_num_nodes() const = 0;

  // Nodes
  virtual bool is_node_member(SGraphNode node) const = 0;
  virtual void add_node(SGraphNode node) = 0;
  virtual void remove_node(SGraphNode node) = 0;
  // Printing.  
  virtual void print_node(ion *out, SGraphNode node) const;
  
  
  // Edges
  //
  // There is no edge representation here.
  // just iterators over sucessors and predecessors

  virtual bool is_edge_member(const SGraphEdge &edge) const = 0;
  virtual void add_edge(const SGraphEdge &edge) = 0; 
  virtual void remove_edge(const SGraphEdge &edge) = 0;
  virtual void print_edge(ion *out, const SGraphEdge &edge) const;


  // printing the graph.
  // There is a common implementation of this.
  virtual void print(ion *out) const;
  virtual void print_debug() const;
  
  // Iterators
  // These always create a new iterator so the caller
  // MUST delete them
  virtual SNodeIter get_node_iterator() const = 0;
  virtual SNodeIter get_node_successor_iterator(SGraphNode node) const = 0;
  virtual SNodeIter get_node_predecessor_iterator(SGraphNode node) const = 0;
  virtual SEdgeIter get_edge_iterator() const;

  // Checks if node has a parent
  bool has_parent(SGraphNode node);
  
  // Useful shortcuts (helpers).
  // create the next node and return its number
  virtual SGraphNode create_node ();

  // These have a generic implementation that
  // uses the iterators, but some impementations
  // of the interface may use a better implementation
  virtual void remove_node_successor_edges(SGraphNode node);
  virtual void remove_node_predecessor_edges(SGraphNode node);
  virtual void add_all_node_successor_edges(SGraphNode node);
  virtual void add_all_node_predecessor_edges(SGraphNode node);
  virtual void add_all_edges();
  virtual bool node_has_successors(SGraphNode node) const;
  virtual bool node_has_predecessors(SGraphNode node) const;

  // functions for adding, removing based on another set
  // as a graph, bitvector or list.
  virtual void add_nodes_from_sgraph(const SGraph *SGraph);
  virtual void add_nodes_from_bits(const BitVector *bits);
  virtual void add_nodes_from_list(SGraphNodeList *list);

  virtual void remove_nodes_from_sgraph(const SGraph *SGraph);
  virtual void remove_nodes_from_bits(const BitVector *bits);
  virtual void remove_nodes_from_list(SGraphNodeList *list);

  virtual bool is_node_subset_in_sgraph(const SGraph *SGraph);
  virtual bool is_node_subset_in_bits(const BitVector *bits);
  virtual bool is_node_subset_in_list(SGraphNodeList *list);

  // Build a bit vector of the nodes
  // or a list of the nodes.  these will
  // return a new one so the caller
  // must delete it.
  virtual BitVector *new_node_set() const;
  virtual SGraphNodeList *new_node_list() const;

  // Implementations to make forward and reverse edges easier.
  // Edges with direction
  // These are virtual because they have the same name
  // as the methods without a direction
  virtual bool is_edge_member(const SGraphEdge &edge, 
			      bool do_forward) const;
  virtual void add_edge(const SGraphEdge &edge, 
			bool do_forward);
  virtual void remove_edge(const SGraphEdge &edge,
			   bool do_forward);
  virtual void print_edge(ion *out, const SGraphEdge &edge,
			  bool do_forward) const;

  // Again, implementations to make forward and reverse easier
  // successors with direction
  virtual SNodeIter get_node_successor_iterator(SGraphNode node,
						bool do_forward) const;

  virtual SNodeIter get_node_predecessor_iterator(SGraphNode node, 
						  bool do_forward) const;

  // Implementations to make forward/reverse easier
  // helpers with successors
  virtual void remove_node_successor_edges(SGraphNode node,
					   bool do_forward);
  virtual void remove_node_predecessor_edges(SGraphNode node,
					     bool do_forward);

  virtual void add_all_node_successor_edges(SGraphNode node,
					    bool do_forward);

  virtual void add_all_node_predecessor_edges(SGraphNode node,
					      bool do_forward);
  virtual bool node_has_successors(SGraphNode node,
				   bool do_forward) const;
  
  virtual bool node_has_predecessors(SGraphNode node,
				     bool do_forward) const;

 protected:
  // These should have no implementation
  SGraph &operator=(const SGraph &);
};

class SuifEnv;
extern "C" void init_sgraph(SuifEnv *);

#endif /* SGRAPH_H */
