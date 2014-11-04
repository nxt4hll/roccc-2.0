/*  ngraph.h */

/*  Copyright (c) 1998 Stanford University

    All rights reserved.

    This software is provided under the terms described in
    the "suif_copyright.h" include file. */

#include <common/suif_copyright.h>
#include <suifkernel/suifkernel_messages.h>



#ifndef NGRAPH_H
#define NGRAPH_H

//#define   TRUE   1
//#define   FALSE  0 
  /*
   * NGraph - The 'noded' graph interface
   *
   * This is a basic graph interface that conforms to
   * the sgraph interface, but adds a parameterized node type.
   *
   * I expect at least 2 initial implementation that:
   *
   * Dispatches to a sgraph and just translate the nodes
   * to numbers
   *
   * Have a number() and set_number() in the node itself
   * and query the node for the number.
   *
   */

// Need this for ion.
#include "sgraph.h"
#include "sgraph_iter.h"


template <class T> class NGraphNNodeIter;
template <class T> class NGraph;

template <class T> class NEdge {
  T* _from;
  T* _to;
public:
  NEdge(T* from, T* to) : _from(from), _to(to) {}
  T* to() const { return(_to); }
  T* from() const { return(_from); }
  NEdge reverse() const { return(NEdge(_to, _from)); }
};

/*
 * Value classes for node and edge iterators
 * that can be passed around in common cases.
 */

// This is a mixed ownership bag that allows
// for a simple

template <class T> 
class NNodeIter {
  SNodeIter _iter;
  bool _owned;
  void clone_iter();
  const NGraph<T>* _graph;

public:
  NNodeIter();
  ~NNodeIter();
  NNodeIter(NNodeIter*iter, const NGraph<T>* graph);
  NNodeIter(const SNodeIter &other);
  NNodeIter&operator=(const NNodeIter &other);

  T* get() const;
  bool done() const;
  void increment();
  void reset();

  // the suif2 form
  bool is_valid() const;
  T* current() const;
  void next();
  
  // Helper
  T* step();
  NNodeIter clone() const;
};

// value class to pass around
template <class T> 
class NEdgeIter {
  SEdgeIter _iter;
  bool _owned;
  void clone_iter();
  const NGraph<T>* _graph;

public:
  NEdgeIter();
  ~NEdgeIter();
  NEdgeIter(SEdgeIter iter, const NGraph<T>* graph);
  NEdgeIter(const NEdgeIter &other);
  NEdgeIter &operator=(const NEdgeIter &other);

  NEdge<T> get() const;
  bool done() const;
  void increment();
  void reset();

  // the suif2 form
  bool is_valid() const;
  NEdge<T> current() const;
  void next();

  // Helper
  NEdge<T> step();
  NEdgeIter clone() const;
};

template <class T> 
class NGraph : public SGraph {
public:
  // Nodes
  virtual bool is_nnode_member(T *nnode) const = 0;
  // returns the node number added.
  virtual SGraphNode add_nnode (T *nnode) = 0;
  virtual void remove_nnode(T *nnode);
  virtual T *get_nnode_from_node(SGraphNode node) const = 0;
  virtual SGraphNode get_node_from_nnode(T *nnode) const = 0;
  // Printing.
  virtual void print_nnode(ion *out, T *nnode) const;

  // Edges
  //
  // There is no edge representation here.
  // just iterators over sucessors and predecessors

  virtual bool is_nedge_member(const NEdge<T>& edge) const;
  virtual void add_nedge(const NEdge<T>& edge);
  virtual void remove_nedge(const NEdge<T>& edge);
  virtual void print_nedge(ion *out, const NEdge<T>& edge) const;

  // printing the graph.
  // There is a common implementation of this.
  //  virtual void print(ion *out) const;

  // Iterators
  virtual NGraphNNodeIter<T> *get_nnode_iterator() const;
  virtual NGraphNNodeIter<T> *get_nnode_successor_iterator(T *nnode) const;
  virtual NGraphNNodeIter<T> *get_nnode_predecessor_iterator(T *nnode) const;

  // Build a bit vector of the nodes
  //  virtual bit_vector *new_nnode_set() const = 0;

  // Useful shortcuts (helpers).
  virtual void remove_nnode_successor_edges(T *nnode);
  virtual void remove_nnode_predecessor_edges(T *nnode);
  virtual void add_all_nnode_successor_edges(T *nnode);
  virtual void add_all_nnode_predecessor_edges(T *nnode);
  //  virtual void add_all_edges() = 0;
  virtual bool nnode_has_successors(T *nnode) const;
  virtual bool nnode_has_predecessors(T *nnode) const;

  // Implementations to make forward and reverse edges easier.
  // Edges with direction
  // These are virtual because they have the same name
  // as the ones without the direction
  virtual bool is_nedge_member(const NEdge<T>& edge,
        bool do_forward) const {
    if (do_forward)  return(is_nedge_member(edge));
    else return(is_nedge_member(edge.reverse())); 
  }

  virtual void add_nedge(const NEdge<T>& edge,
        bool do_forward) {
    if (do_forward) add_nedge(edge);
    else add_nedge(edge.reverse()); 
  }

  virtual void remove_nedge(const NEdge<T>& edge,
            bool do_forward
		    ) {
    if (do_forward) remove_nedge(edge);
    else remove_nedge(edge.reverse()); 
  }

  virtual void print_nedge(ion *out, const NEdge<T>& edge,
           bool do_forward) const {
    if (do_forward) print_nedge(out, edge);
    else print_nedge(out, edge.reverse()); 
  }

  // Again, implementations to make forward and reverse easier
  // successors with direction
  virtual NGraphNNodeIter<T> *get_nnode_successor_iterator(T *nnode,
                        bool do_forward) const {
    if (do_forward) return(get_nnode_successor_iterator(nnode));
    else return(get_nnode_predecessor_iterator(nnode)); 
  }

  virtual NGraphNNodeIter<T> *get_nnode_predecessor_iterator(T *nnode,
                            bool do_forward) const {
    if (!do_forward) return(get_nnode_successor_iterator(nnode));
    else return(get_nnode_predecessor_iterator(nnode)); 
  }

  // Implementations to make forward/reverse easier
  // helpers with successors
  virtual void remove_nnode_successor_edges(T *nnode,
                   bool do_forward) {
    if (do_forward) remove_nnode_successor_edges(nnode);
    else  remove_nnode_predecessor_edges(nnode); 
  }

  virtual void remove_nnode_predecessor_edges(T *nnode,
                      bool do_forward) {
    if (!do_forward) remove_nnode_successor_edges(nnode);
    else  remove_nnode_predecessor_edges(nnode); 
  }

  virtual void add_all_nnode_successor_edges(T *nnode,
                    bool do_forward) {
    if (do_forward) add_all_nnode_successor_edges(nnode);
    else  add_all_nnode_predecessor_edges(nnode); 
  }

  virtual void add_all_nnode_predecessor_edges(T *nnode,
                      bool do_forward) {
    if (!do_forward) add_all_nnode_successor_edges(nnode);
    else  add_all_nnode_predecessor_edges(nnode); 
  }

  virtual bool nnode_has_successors(T *nnode,
                       bool do_forward) const {
    if (do_forward) return(nnode_has_successors(nnode));
    else return(nnode_has_predecessors(nnode)); 
  }

  virtual bool nnode_has_predecessors(T *nnode,
                bool do_forward) const {
    if (!do_forward) return(nnode_has_successors(nnode));
    else return(nnode_has_predecessors(nnode)); 
  }
/*
  virtual SNodeIter get_node_iterator() const;
  virtual SNodeIter get_node_successor_iterator(SGraphNode node) const;
  virtual SNodeIter get_node_predecessor_iterator(SGraphNode node) const;
*/
};


/*
 * ******************************************
 * *
 * * iterators over nodes
 * *
 * ******************************************
 */
template <class T> class NGraphNNodeIter {
public:
  /* No initialization should be specified here */
  //  virtual NGraph *get_graph() const = 0;
  virtual ~NGraphNNodeIter();
  virtual T *get() const = 0;
  virtual bool done() const = 0;
  virtual void increment() = 0;


  // Helper
  T& step() { T *nnode = get(); increment(); return(nnode); }

};// end class NGraphNNodeIter

// An empty implementation
template <class T> class NGraph_empty_iter : public NGraphNNodeIter<T> {
public:
  NGraph_empty_iter() {}
  ~NGraph_empty_iter() {}
  bool done() const { return true; }
  T get() const { assert(0); return(0); }
  void increment() { assert(0); }
};// end class NGraph_empty_iter

/*
 * Iterator based on an SNodeIter
 * with translation through an NGraph
 */
template <class T> class NGraphBaseIter : public NGraphNNodeIter<T> {
  const NGraph<T> *_parent;
  SNodeIter _parent_iter;
  bool _owned; // If owned, delete it afterwards
public:
  NGraphBaseIter() {
    _parent = NULL; _parent_iter = NULL; _owned = false; }
  NGraphBaseIter(const NGraphBaseIter<T> &other) {
    _parent = NULL; _parent_iter = NULL; _owned = false;
    reset(other._parent, other._parent_iter, false);
  }
  NGraphBaseIter(const NGraph<T> *parent,
           SNodeIter parent_iter, bool owned) {
    _parent = NULL; _owned = false;
    reset(parent, parent_iter, owned);
  }

  ~NGraphBaseIter() { reset(NULL, NULL, false); }

  void reset(const NGraph<T> *parent,
         SNodeIter parent_iter, bool owned) {
    //if (_owned _parent_iter != NULL) delete _parent_iter;
    _parent = parent;
    _parent_iter = parent_iter;
    _owned = owned;
  }

  bool done() const { return _parent_iter.done(); }
  T *get() const {
    return _parent->get_nnode_from_node(_parent_iter.get()); }
  void increment() { _parent_iter.increment(); }
};//end class NGraphBaseIter


template <class T>
class NGraph_nnode_filter {
public:
  virtual bool is_nnode_member(T *nnode) = 0;
  virtual ~NGraph_nnode_filter() {}
}; // end class NGraph_nnode_filter

// This is a simple example of a filter to filter only
// the nodes that are in a graph.
template <class T>
class NGraph_nnode_graph_filter : public NGraph_nnode_filter<T> {
  NGraph<T> *_the_NGraph;
public:
  NGraph_nnode_graph_filter(NGraph<T> *the_NGraph) { _the_NGraph = the_NGraph; }
  virtual bool is_nnode_member(T *nnode) {
    return(_the_NGraph->is_nnode_member(nnode)); }
};//end class NGraph_nnode_graph_filter



template <class T>
class NGraph_filtered_nnode_iter : public NGraphNNodeIter<T> {
  NGraphNNodeIter<T> *_base_iter;
  NGraph_nnode_filter<T> *_filter;
  bool _owned;
  SGraphNode _last;
  bool _done;
public:
  NGraph_filtered_nnode_iter() {
    _base_iter = NULL;   _filter = NULL; _owned = false; }
  
  NGraph_filtered_nnode_iter(const NGraph_filtered_nnode_iter &other) {
    _owned = false;
    reset(other._base_iter, other._filter, false);
  }
  
  NGraph_filtered_nnode_iter(NGraphNNodeIter<T> *base_iter,
			     NGraph_nnode_filter<T> *filter,
			     bool owned) {
    _owned = false;
    reset(base_iter, filter, owned);
  }
  
  ~NGraph_filtered_nnode_iter() {
    if (_owned && _filter != NULL) delete _filter; 
  }
  
  void reset(NGraphNNodeIter<T> *base_iter, NGraph_nnode_filter<T> *filter,
	     bool owned) {
    if (_owned && _filter != NULL) delete filter;
    _filter = filter;
    _base_iter = base_iter;
    _done = false;
    _last = 0;
    
    for(;!_base_iter->done(); _base_iter.increment()) {
      T *nnode = _base_iter.get();
      if (_filter->is_nnode_member(nnode)) {
	_last = NGraphNNodeIter<T>::node; 
	return;
      }
    }
    _done = true;
  }

  bool done() const { return _done; }

  SGraphNode get() const { return _last; }

  void increment() {
    if (_done) return;
    for(;!_base_iter->done(); _base_iter.increment()) {
      T *nnode = _base_iter.get();
      if (_filter->is_nnode_member(nnode)) {
	_last = NGraphNNodeIter<T>::node; 
	return;
      }
    }
    _done = true;
  }
};

/*
 * Subset based on a subset
 * with translation through an NGraph
 */
template <class T> class NGraphSubgraph : public NGraph<T> {
  const NGraph<T> *_parent;  // for looking up numbers.
  SGraph *_sub_graph;
  bool _owned;
  const NGraph<T> *get_parent() const { return _parent; }
  SGraph *get_subgraph() const { return _sub_graph; }
public:
  NGraphSubgraph() {
    _parent = NULL; _sub_graph = NGraph<T>::sub_graph; _owned = false; 
  }
  
  NGraphSubgraph(const NGraphSubgraph<T> &other) {
    _parent = NULL; _sub_graph = NGraph<T>::sub_graph; _owned = false;
    reset(other._parent, other._sub_graph, false);
  }
  
  NGraphSubgraph(NGraph<T> *parent,
           SGraph *sub_graph, bool owned) {
    _parent = NULL; _sub_graph = NULL; _owned = false;
    reset(parent, sub_graph, owned); 
  }

  ~NGraphSubgraph() { reset(NULL, NULL, false); }

  void reset(NGraph<T> *parent, SGraph *sub_graph, bool owned) {
    if (_owned && NGraph<T>::_parent_iter != NULL) delete _sub_graph;
    _parent = parent;
    _sub_graph = sub_graph;
    _owned = owned;
  }

  // Nodes
  virtual bool is_nnode_member(const T& nnode) const {
    if (!get_parent()->is_nnode_member(nnode)) return false;
    SGraphNode node = get_parent()->get_node_from_nnode(nnode);
    return(NGraph<T>::get_sub_graph()->is_node_member(nnode));
  }

  // Can only add nodes that are members of the parent.
  virtual SGraphNode add_nnode (T *nnode) {
    assert(get_parent()->is_nnode_member(nnode));
    SGraphNode node = get_parent()->get_node_from_nnode(nnode);
    return(NGraph<T>::get_sub_graph()->add_node(nnode));
  }

  virtual void remove_nnode(T *nnode) {
    if (!get_parent()->is_nnode_member(nnode)) { return; }
    SGraphNode node = get_parent()->get_node_from_nnode(nnode);
    NGraph<T>::get_sub_graph()->remove_node(nnode);
  }

  virtual T *get_nnode_from_node(SGraphNode node) const {
    return(get_parent()->get_nnode_by_node(NGraph<T>::nnode)); }
  virtual SGraphNode get_node_from_nnode(T *nnode) const {
    return(get_parent()->get_node_from_nnode(NGraph<T>::node)); }
  // Printing.
  virtual void print_nnode(ion *out, T *nnode) const {
    return(get_parent()->print_nnode(out, nnode)); }

  // Edges
  //
  // There is no edge representation here.
  // just iterators over sucessors and predecessors

  virtual bool is_nedge_member(T *from_nnode,
				  T *to_nnode) const {
    if (!is_nnode_member(from_nnode) ||
	!is_nnode_member(to_nnode)) return false;
    return(get_parent()->is_nedge_member(from_nnode, to_nnode));
  }
  virtual void add_nedge(T *from_nnode,
			 T *to_nnode) { assert(false); 
  }

  virtual void remove_nedge(T *from_nnode,
			    T *to_nnode) { assert(false); 
  }

  virtual void print_nedge(ion *out, T *from_nnode,
			   T *to_nnode) const {
    get_parent()->print_nedge(out, from_nnode, to_nnode); 
  }

  // printing the graph.
  // There is a common implementation of this.
  //  virtual void print(ion *out) const;

  // Iterators
  virtual SNodeIter get_node_iterator() const;
  virtual SNodeIter get_node_successor_iterator(SGraphNode node) const;
  virtual SNodeIter get_node_predecessor_iterator(SGraphNode node) const;

};

extern "C" void enter_NGraph(int *argc, char *argv[]);
extern "C" void exit_NGraph();


// Common implementation that translate to a node
// number and dispatch to other functions
template <class T>
void NGraph<T>::remove_nnode(T *nnode) {
  if (!is_nnode_member(nnode)) return;
  remove_node(get_node_from_nnode(nnode));
}
template <class T>
void NGraph<T>::print_nnode(ion *out, T *nnode) const {
  print_node(out, get_node_from_nnode(nnode));
}

template <class T>
bool NGraph<T>::is_nedge_member(const NEdge<T>& edge) const {
  if (!is_nnode_member(edge.from()) || !is_nnode_member(edge.to()))
    return false;
  SGraphNode from_node = get_node_from_nnode(edge.from());
  SGraphNode to_node = get_node_from_nnode(edge.to());
  return(is_edge_member(SGraphEdge(from_node, to_node)));
}

template <class T>
void NGraph<T>::add_nedge(const NEdge<T>& edge) {
  // from_nnode and to_nnode MUST already be members.
  suif_assert(is_nnode_member(edge.from()));
  suif_assert(is_nnode_member(edge.to()));
  SGraphNode from_node = get_node_from_nnode(edge.from());
  SGraphNode to_node = get_node_from_nnode(edge.to());
  add_edge(SGraphEdge(from_node, to_node));
}

template <class T>
void NGraph<T>::remove_nedge(const NEdge<T>& edge) {
  if (!is_nedge_member(edge)) { return; }
  SGraphNode from_node = get_node_from_nnode(edge.from());
  SGraphNode to_node = get_node_from_nnode(edge.to());
  remove_edge(SGraphEdge(from_node, to_node));
}

template <class T>
void NGraph<T>::print_nedge(ion *out, const NEdge<T>& edge) const {
  if (!is_nedge_member(edge)) { return; }
  SGraphNode from_node = get_node_from_nnode(edge.from());
  SGraphNode to_node = get_node_from_nnode(edge.to());
  print_edge(out, SGraphEdge(from_node, to_node));
}

template <class T>
void NGraph<T>::remove_nnode_successor_edges(T *nnode) {
  assert(is_nnode_member(nnode));
  remove_node_successor_edges(get_node_from_nnode(nnode));
}
template <class T>
void NGraph<T>::remove_nnode_predecessor_edges(T *nnode) {
  assert(is_nnode_member(nnode));
  remove_node_predecessor_edges(get_node_from_nnode(nnode));
}
template <class T>
void NGraph<T>::add_all_nnode_successor_edges(T *nnode) {
  assert(is_nnode_member(nnode));
  add_all_node_successor_edges(get_node_from_nnode(nnode));
}
template <class T>
void NGraph<T>::add_all_nnode_predecessor_edges(T *nnode) {
  assert(is_nnode_member(nnode));
  add_all_node_successor_edges(get_node_from_nnode(nnode));
}
//  virtual void add_all_edges() = 0;
template <class T>
bool NGraph<T>::nnode_has_successors(T *nnode) const {
  assert(is_nnode_member(nnode));
  return(node_has_successors(get_node_from_nnode(nnode)));
}

template <class T>
bool NGraph<T>::nnode_has_predecessors(T *nnode) const {
  assert(is_nnode_member(nnode));
  return(node_has_predecessors(get_node_from_nnode(nnode)));
}

// Common iterators
// The user must STILL define new_node_iterator...
template <class T>
NGraphNNodeIter<T> *NGraph<T>::get_nnode_iterator() const {
  return(new NGraphBaseIter<T>(this, get_node_iterator(), true));
}

template <class T>
NGraphNNodeIter<T> *NGraph<T>::get_nnode_successor_iterator(T *nnode) const {
  return(new NGraphBaseIter<T>
	 (this, get_node_successor_iterator(get_node_from_nnode(nnode)),true));
}

template <class T>
NGraphNNodeIter<T> *NGraph<T>::get_nnode_predecessor_iterator(T *nnode) const {
  return(new NGraphBaseIter<T>
	 (this, get_node_predecessor_iterator(get_node_from_nnode(nnode)),true));
}

template <class T>
NGraphNNodeIter<T>::~NGraphNNodeIter(){};

// For subgraph
// iterators
// The user must STILL define new_node_iterator...
template <class T>
SNodeIter NGraphSubgraph<T>::get_node_iterator() const {
  return(NGraph<T>::get_sub_graph()->get_node_iterator());
}

template <class T>
SNodeIter NGraphSubgraph<T>::get_node_successor_iterator(SGraphNode node) const {
  return(NGraph<T>::get_sub_graph()->get_node_successor_iterator(node));
}

template <class T>
SNodeIter NGraphSubgraph<T>::get_node_predecessor_iterator(SGraphNode node) const {
  return(NGraph<T>::get_sub_graph()->get_node_predecessor_iterator(node));
}

#endif /* NGRAPH_H */
