/*  SGraphBit */

/*  Copyright (c) 1998 Stanford University

    All rights reserved.

    This software is provided under the terms described in
    the "suif_copyright.h" include file. */

#include "common/suif_copyright.h"

#ifndef SGRAPH_BIT_H
#define SGRAPH_BIT_H

#include "sgraph.h"
#include "common/common_forwarders.h"

  /*
   * SGraphBit - library.
   *
   * Implementation of the simplest graph
   *  The edges are arrays of bitvectors.
   * 
   */

class SGraphBit : public SGraph {

  typedef suif_vector<BitVector *> bit_vector_vector;
  BitVector *_nodes;
  bit_vector_vector *_edges;

public:
  SGraphBit();
  SGraphBit(const SGraphBit &);
  SGraphBit& operator=(const SGraphBit &);
  
  void clear();
  // Nodes
  // maximum node number +1
  virtual SGraphNode max_num_nodes() const;

  // Nodes
  virtual bool is_node_member(SGraphNode node) const;
  virtual void add_node (SGraphNode node);
  virtual void remove_node(SGraphNode node);
  // Printing.  
  virtual void print_node(ion *out, SGraphNode node) const;
  
  // Edges
  //
  // There is no edge representation here.
  // just iterators over sucessors and predecessors
  virtual bool is_edge_member(const SGraphEdge &edge) const;
  virtual void add_edge(const SGraphEdge &edge);
  virtual void remove_edge(const SGraphEdge &edge);

  virtual void print_edge(ion *out, const SGraphEdge &edge) const;

  // Implementation in the .cc file
  //  virtual void print(ion *out) const;
  
  // Iterators. Implementations in the .cc file
  virtual SNodeIter get_node_iterator() const;
  virtual SNodeIter get_node_successor_iterator(SGraphNode node) const;
  virtual SNodeIter get_node_predecessor_iterator(SGraphNode node) const;
  virtual BitVector *new_node_set() const;

  // Useful shortcuts.
  virtual void remove_node_successor_edges(SGraphNode node);
  virtual void remove_node_predecessor_edges(SGraphNode node);
  virtual void add_all_node_successor_edges(SGraphNode node);
  virtual void add_all_node_predecessor_edges(SGraphNode node);
  virtual void add_all_edges();
  virtual bool node_has_successors(SGraphNode node) const;


  // Shortcuts for the bitgraph implementation
  void add_nodes(const SGraph *SGraph);

  void set_node_successors(SGraphNode node, const BitVector *t,
			   bool do_forward);
  void set_node_predecessors(SGraphNode node, const BitVector *t,
			     bool do_forward);

  void set_node_successors(SGraphNode node, const BitVector *t);

  void set_node_predecessors(SGraphNode node, const BitVector *t);

  // shortcuts for creating a bitvector result.
  // these could be at the top level as well.
  BitVector *new_node_successors(SGraphNode node) const;
  BitVector *new_node_successors(SGraphNode node,
				  bool do_forward) const;

  BitVector *new_node_predecessors(SGraphNode node) const;
  BitVector *new_node_predecessors(SGraphNode node, 
				    bool do_forward ) const;
 private:
  BitVector *retrieve_bv(SGraphNode node);
  BitVector *get_bv(SGraphNode node) const;
  bool has_bv(SGraphNode node) const;
};






#endif /* SGRAPH_BIT_H */
