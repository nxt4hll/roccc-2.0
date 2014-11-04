/*  sgraph_bit_subgraph.h */

/*  Copyright (c) 1998 Stanford University

    All rights reserved.

    This software is provided under the terms described in
    the "suif_copyright.h" include file. */

#include "common/suif_copyright.h"

#ifndef SGRAPH_BIT_SUBGRAPH_H
#define SGRAPH_BIT_SUBGRAPH_H

//#pragma interface

  /*
   * SGraphBitSubgraph - library.
   *
   * This is a basic subgraph of a graph.
   * It has its own node list (that should be
   *  a subset of that of its base graph.)
   * Edges are included if the nodes are in
   *   the subgraph and the edge is in the parent.
   *
   * We expect 2 implementations of this,
   *   one with a list of nodes and the other with
   *   the standard bitvector of nodes.
   * 
   * It is NOT valid to modify the edge information
   * for a subgraph
   */

#include "sgraph.h"

class SGraphBitSubgraph : public SGraph {
  const SGraph *_parent;
  BitVector *_nodes;
  
public:
  SGraphBitSubgraph(const SGraph *parent);
  
  // Nodes
  // maximum node number +1
  virtual SGraphNode max_num_nodes() const;

  // Nodes
  virtual bool is_node_member(SGraphNode node) const;

  virtual void add_node (SGraphNode node);

  virtual void remove_node(SGraphNode node);

  // Edges
  //
  // There is no edge representation here.
  // just iterators over sucessors and predecessors

  virtual bool is_edge_member(const SGraphEdge &edge) const;
  virtual void add_edge(const SGraphEdge &edge);
  virtual void remove_edge(const SGraphEdge &edge);

  // printing the graph.
  // There is a common implementation of this.
  //  virtual void print(ion *out) const;
  
  // Iterators
  virtual SNodeIter get_node_iterator() const;
  virtual SNodeIter get_node_successor_iterator(SGraphNode node) const;
  virtual SNodeIter get_node_predecessor_iterator(SGraphNode node) const;

  // Build a bit vector of the nodes
  virtual BitVector *new_node_set() const;

  // Fast implementation
  void add_nodes_in_sgraph(const SGraph *SGraph);

};




#endif /* SGRAPH_BIT_SUBGRAPH_H */
