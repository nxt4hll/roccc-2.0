/*  sgraph_list */

/*  Copyright (c) 1998 Stanford University

    All rights reserved.

    This software is provided under the terms described in
    the "suif_copyright.h" include file. */

#include "common/suif_copyright.h"

#ifndef SGRAPH_LIST_H
#define SGRAPH_LIST_H

#include "sgraph.h"
#include "common/common_forwarders.h"

  /*
   * SGraphList - library.
   *
   * Implementation of a simple graph
   *  where edges are just lists of nodes
   */

class SGraphList : public SGraph {
  
  BitVector *_nodes;

  // Each node has a list of successors and predecessors.
  // 
  // array_tos<slist_tos<>>
  typedef suif_vector<SGraphNodeList *> sgraph_node_list_vector;

  sgraph_node_list_vector *_predecessors;
  sgraph_node_list_vector *_successors;

public:
  SGraphList();
  SGraphList(const SGraphList &);
  SGraphList &operator=(const SGraphList &);
  ~SGraphList();
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

  // Implementation in the .cc file
  //  virtual void print(ion *out) const;
  
  // Iterators. Implementations in the .cc file
  virtual SNodeIter get_node_iterator() const;
  virtual SNodeIter get_node_successor_iterator(SGraphNode node) const;
  virtual SNodeIter get_node_predecessor_iterator(SGraphNode node) const;
  virtual BitVector *new_node_set() const;

  virtual bool node_has_successors(SGraphNode node) const;
  virtual bool node_has_predecessors(SGraphNode node) const;

 private:
  // helpers for the class
  SGraphNodeList *get_pred_list(SGraphNode node) const;
  SGraphNodeList *retrieve_pred_list(SGraphNode node);
  bool has_pred_list(SGraphNode node) const;

  SGraphNodeList *get_succ_list(SGraphNode node) const;
  SGraphNodeList *retrieve_succ_list(SGraphNode node);
  bool has_succ_list(SGraphNode node) const;

  // Helpers for the lists.
 public:
  static bool list_contains(SGraphNodeList *list, SGraphNode node);
  static void list_remove(SGraphNodeList *list, SGraphNode node);
};



#endif /* SGRAPH_LIST_H */
