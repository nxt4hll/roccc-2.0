/*  ngraph.h */

/*  Copyright (c) 1998 Stanford University

    All rights reserved.

    This software is provided under the terms described in
    the "suif_copyright.h" include file. */

#include <common/suif_copyright.h>

#ifndef NGRAPH_BASE_H
#define NGRAPH_BASE_H

  /*
   * ngraph_base - library.
   *
   * This is a basic ngraph implementation that
   * has a translation from each node to a number
   * and uses a base sgraph to serve all numeric requests.
   *
   */

#include <common/suif_hash_map.h>
#include <common/suif_vector.h>
#include <suifkernel/suifkernel_messages.h>

#include "ngraph.h"

template <class T> class NGraphBaseNNodeIter;

template <class T>
class NGraphBase : public NGraph<T> {
  SGraph *_the_sgraph;
  suif_hash_map<T*, SGraphNode> _map;
  suif_vector<T*> _array;

  SGraphNode _next_id;

  SGraphNode assign_next_id() {
    SGraphNode n = _next_id;
    _next_id++;
    return(n);
  }
  
  NGraphBase() {}
public:
  SGraph *get_sgraph() const { return(_the_sgraph); }

  NGraphBase(SGraph *the_sgraph) {
    _the_sgraph = the_sgraph;
    _next_id = 0;
  }

  virtual size_t max_num_nodes() const {
    return _next_id;
  }

  //
  virtual T *get_nnode_from_node(SGraphNode node) const {
    return(_array[node]); 
  }

  virtual SGraphNode get_node_from_nnode(T *nnode) const {
    typename suif_hash_map<T*, SGraphNode>::iterator iter = _map.find(nnode);
    suif_assert_message(iter!=_map.end(), 
        ("There is a mismatch in the map"));
    return (*iter).second; 
  }

  // Dispatch to the_sgraph
  virtual bool is_node_member(SGraphNode node) const {
    return(get_sgraph()->is_node_member(node)); }
  // In general, this should not be called before the
  // node has been added into any local structures.
  
  virtual void add_node(SGraphNode node) {
    suif_assert(_array[node] != NULL);
    get_sgraph()->add_node(node); 
  }

  virtual void remove_node(SGraphNode node) {
    get_sgraph()->remove_node(node); 
  }
  
  virtual bool is_edge_member(const SGraphEdge &edge) const {
    return(get_sgraph()->is_edge_member(edge)); 
  }
  
  virtual void add_edge(const SGraphEdge &edge) {
    get_sgraph()->add_edge(edge); 
  }
  
  virtual void remove_edge(const SGraphEdge &edge) {
    get_sgraph()->remove_edge(edge); 
  }

  virtual void remove_nnode(T *nnode) {
    get_sgraph()->remove_node(get_node_from_nnode(nnode));
  }
  //  virtual void print_nnode(ion *out, T *nnode) const {
  //    print_node(out, get_node_from_nnode(nnode)); }

  //  virtual void print_nedge(ion *out, T *from_nnode, T *to_nnode) const {


  // Nodes
  virtual bool is_nnode_member(T *nnode) const {
    suif_assert(nnode != NULL);
    typename suif_hash_map<T*, SGraphNode>::iterator
      iter = _map.find(nnode);
    if (iter==_map.end()) return false;
    SGraphNode node = (*iter).second;
    return(get_sgraph()->is_node_member(node));
  }

  virtual SGraphNode add_nnode (T *nnode) {
    // dont add a second time.
    suif_assert(nnode != NULL);
    if (is_nnode_member(nnode))
      return(get_node_from_nnode(nnode));
    SGraphNode node = assign_next_id();
    _map.enter_value(nnode, node);
    //_array[node] = nnode;
    _array.push_back(nnode);
    add_node(node);
    return(node);
  }

  // printing the graph.
  // There is a common implementation of this.
  //  virtual void print(ion *out) const;

  // Iterators
  virtual SNodeIter get_node_iterator() const;
  virtual SNodeIter get_node_successor_iterator(SGraphNode node) const;
  virtual SNodeIter get_node_predecessor_iterator(SGraphNode node) const;
};


extern "C" void enter_NGraphBase(int *argc, char *argv[]);
extern "C" void exit_NGraphBase();

template <class T>
SNodeIter NGraphBase<T>::get_node_iterator() const {
  return(get_sgraph()->get_node_iterator());
}

template <class T>
SNodeIter NGraphBase<T>::get_node_successor_iterator(SGraphNode node) const {
  return(get_sgraph()->get_node_successor_iterator(node));
}

template <class T>
SNodeIter NGraphBase<T>::get_node_predecessor_iterator(SGraphNode node) const {
  return(get_sgraph()->get_node_predecessor_iterator(node));
}


#endif /* NGRAPH_BASE_H */
