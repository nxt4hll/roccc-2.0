#include "sgraph.h"
#include <stdio.h>
#include "sgraph_iter.h"
#include "ion/ion.h"
#include "bit_vector/bit_vector.h"
#include "suifkernel/suif_env.h"

/*
 * Some useful iterator implementations
 */

class SGraphAllEdgeIter : public SGraphEdgeIter {
  const SGraph *_parent;
  SNodeIter _first;
  SNodeIter _second;
public:
  SGraphAllEdgeIter(const SGraph *parent);
  ~SGraphAllEdgeIter();
  /* No initialization should be specified here */
  //  virtual SGraph *get_graph() const = 0;
  virtual SGraphEdge get() const;
  virtual bool done() const;
  virtual void increment();
  virtual void reset();
  virtual SGraphEdgeIter *clone() const;
};

/*
 * Implementation of the above iterator
 *
 */ 
SGraphAllEdgeIter::SGraphAllEdgeIter(const SGraph *parent) :
  _parent(parent)
{
  reset();
}

SGraphAllEdgeIter::~SGraphAllEdgeIter() {
  //  if (_first != NULL) delete _first;
  //  if (_second != NULL) delete _second;
}

SGraphEdgeIter *SGraphAllEdgeIter::clone() const {
  return new SGraphAllEdgeIter(_parent);
}

  
  /* No initialization should be specified here */
  //  virtual SGraph *get_graph() const = 0;
SGraphEdge SGraphAllEdgeIter::get() const {
  return SGraphEdge(_first.get(), _second.get());
}

bool SGraphAllEdgeIter::done() const {
  return(_first.done());
}
void SGraphAllEdgeIter::increment() {
  _second.increment();
  while (_second.done()) {
    //delete _second;
    _first.increment();
    if ( _first.done()) {
      //_second.clear();
      //_second = NULL;
      return;
    }
    _second = _parent->get_node_successor_iterator(_first.get());
  }
}

void SGraphAllEdgeIter::reset() {
  _first = _parent->get_node_iterator();
  if (_first.done()) {
    //_second.clear();
    //    _second = NULL;
    return;
  }
  _second = _parent->get_node_successor_iterator(_first.get());
  while (_second.done()) {
    //delete _second;
    _first.increment();
    if (_first.done()) {
      //_second = NULL;
      return;
    }
    _second = _parent->get_node_successor_iterator(_first.get());
  }
}




// Common  implementations
SGraph::~SGraph() {}
void SGraph::print_debug() const { 
  print(stderr_ion); 
  fflush(stderr); 
}

void SGraph::print(ion *out) const {
  for (SNodeIter iter = get_node_iterator();
       !iter.done(); iter.increment()) {
    print_node(out, iter.get());
    out->printf("\n");
    for (SNodeIter successor_iter(get_node_successor_iterator(iter.get()));
	 !successor_iter.done(); successor_iter.increment()) {
      print_edge(out, SGraphEdge(iter.get(), successor_iter.get()));
      out->printf("\n");
    }
  }
}



void SGraph::print_edge(ion *out, const SGraphEdge &edge) const {
  out->printf("E(");
  print_node(out, edge.from());
  out->printf("->");
  print_node(out, edge.to());
  out->printf(")");
}

void SGraph::print_node(ion *out, SGraphNode node) const {
  out->printf("N%u",node);
}

SEdgeIter SGraph::get_edge_iterator() const {
  return(new SGraphAllEdgeIter(this));
}

bool SGraph::has_parent(SGraphNode node){
  SNodeIter iter = get_node_predecessor_iterator(node);
  return !iter.done();
};

SGraphNode SGraph::create_node () {
  SGraphNode node = max_num_nodes();
  add_node(node); 
  return(node);
}


// More common implementations that use the iterators.
void SGraph::remove_node_successor_edges(SGraphNode node) {
  // make a list and then delete the edges.
  SGraphNodeList list;
  //  SGraphNodeIter *iter = get_node_successor_iterator(node);
  for (SNodeIter iter(get_node_successor_iterator(node));
       !iter.done(); iter.increment()) {
    list.push_back(iter.get());
  }

  while (!list.empty()) {
    SGraphNode to_node = list.front();
    remove_edge(SGraphEdge(node, to_node));
  }
}
void SGraph::remove_node_predecessor_edges(SGraphNode node) {
  // make a list and then delete the edges.
  SGraphNodeList  list;
  for (SNodeIter iter(get_node_predecessor_iterator(node));
       !iter.done(); iter.increment()) {
    list.push_back(iter.get());
  }

  while (!list.empty()) {
    SGraphNode from_node = list.front();
    remove_edge(SGraphEdge(from_node, node));
  }
}

void SGraph::add_all_node_successor_edges(SGraphNode node) {
  for (SNodeIter iter(get_node_iterator()); 
       !iter.done(); iter.increment()) {
    add_edge(SGraphEdge(node, iter.get()));
  }
}


void SGraph::add_all_node_predecessor_edges(SGraphNode node) {
  for (SNodeIter iter(get_node_iterator());
       !iter.done(); iter.increment()) {
    add_edge(SGraphEdge(iter.get(), node));
  }
}

void SGraph::add_all_edges() {
  for (SNodeIter iter(get_node_iterator());
       !iter.done(); iter.increment()) {
    add_all_node_successor_edges(iter.get());
  }
}

bool SGraph::node_has_successors(SGraphNode node) const {
  // create an iterator and check if done.
  SNodeIter iter(get_node_successor_iterator(node));
  bool retval = !iter.done();
  return(retval);
}

bool SGraph::node_has_predecessors(SGraphNode node) const {
  // create an iterator and check if done.
  SNodeIter iter(get_node_predecessor_iterator(node));
  bool retval = !iter.done();
  return(retval);
}

void SGraph::add_nodes_from_sgraph(const SGraph *SGraph) {
  for (SNodeIter iter(SGraph->get_node_iterator());
       !iter.done(); iter.increment()) {
    add_node(iter.get());
  }
}

void SGraph::add_nodes_from_bits(const BitVector *bits) {
  size_t num_bits = bits->num_significant_bits();
  for (unsigned i = 0; i < num_bits; i++) {
    if (bits->get_bit(i)) {
      add_node(i);
    }
  }
}
void SGraph::add_nodes_from_list(SGraphNodeList *lst) {
  for (SGraphNodeList::iterator iter = lst->begin();
       iter != lst->end(); iter++) {
    add_node(*iter);
  }
}

void SGraph::remove_nodes_from_sgraph(const SGraph *graph) {
  for (SNodeIter iter(graph->get_node_iterator());
       !iter.done(); iter.increment()) {
    remove_node(iter.get());
  }
}

void SGraph::remove_nodes_from_bits(const BitVector *bits) {
  size_t num_bits = bits->num_significant_bits();
  for (unsigned i = 0; i < num_bits; i++) {
    if (bits->get_bit(i)) {
      remove_node(i);
    }
  }
}
void SGraph::remove_nodes_from_list(SGraphNodeList *lst) {
  for (SGraphNodeList::iterator iter = lst->begin();
       iter != lst->end(); iter++) {
    remove_node(*iter);
  }
}

bool SGraph::is_node_subset_in_sgraph(const SGraph *SGraph) {
  for (SNodeIter iter(SGraph->get_node_iterator());
       !iter.done(); iter.increment()) {
    if (!is_node_member(iter.get())) { return false; }
  }
  return(true);
}

bool SGraph::is_node_subset_in_bits(const BitVector *bits) {
  size_t num_bits = bits->num_significant_bits();
  for (unsigned i = 0; i < num_bits; i++) {
    if (bits->get_bit(i)) {
      if (!is_node_member(i)) { return false; }
    }
  }
  return(true);
}
bool SGraph::is_node_subset_in_list(SGraphNodeList *lst) {
  for (SGraphNodeList::iterator iter = lst->begin();
       iter != lst->end(); iter++) {
    if (!is_node_member(*iter)) { return false; }
  }
  return(true);
}


BitVector *SGraph::new_node_set() const {
  BitVector *bits = new BitVector;
  for (SNodeIter iter(get_node_iterator());
       !iter.done(); iter.increment()) {
    bits->set_bit(iter.get(), true);
  }
  return(bits);
}

SGraphNodeList *SGraph::new_node_list() const {
  SGraphNodeList *list = new SGraphNodeList;
  for (SNodeIter iter(get_node_iterator());
       !iter.done(); iter.increment()) {
    list->push_back(iter.get());
  }
  return(list);
}


bool SGraph::is_edge_member(const SGraphEdge &edge,
			    bool do_forward) const {
  if (do_forward)  return(is_edge_member(edge));
  else return(is_edge_member(edge.reverse())); }

void SGraph::add_edge(const SGraphEdge &edge,
		      bool do_forward) {
  if (do_forward) add_edge(edge);
  else add_edge(edge.reverse()); }

void SGraph::remove_edge(const SGraphEdge &edge,
			 bool do_forward) {
  if (do_forward) remove_edge(edge);
  else remove_edge(edge.reverse()); }

void SGraph::print_edge(ion *out, const SGraphEdge &edge,
			bool do_forward) const {
  if (do_forward) print_edge(out, edge); 
  else print_edge(out, edge.reverse()); }
  


  // Again, implementations to make forward and reverse easier
  // successors with direction
SNodeIter SGraph::
get_node_successor_iterator(SGraphNode node,
			    bool do_forward) const {
  if (do_forward) return(get_node_successor_iterator(node));
  else return(get_node_predecessor_iterator(node)); }

SNodeIter SGraph::
get_node_predecessor_iterator(SGraphNode node, 
			      bool do_forward) const {
  if (!do_forward) return(get_node_successor_iterator(node));
  else return(get_node_predecessor_iterator(node)); }

  // Implementations to make forward/reverse easier
  // helpers with successors
void SGraph::
remove_node_successor_edges(SGraphNode node,
			    bool do_forward) {
  if (do_forward) remove_node_successor_edges(node);
  else  remove_node_predecessor_edges(node); }
void SGraph::
remove_node_predecessor_edges(SGraphNode node,
			      bool do_forward) {
   if (!do_forward) remove_node_successor_edges(node);
   else  remove_node_predecessor_edges(node); }

void SGraph::
add_all_node_successor_edges(SGraphNode node,
			     bool do_forward) {
  if (do_forward) add_all_node_successor_edges(node);
  else  add_all_node_predecessor_edges(node); }

void SGraph::
add_all_node_predecessor_edges(SGraphNode node,
			       bool do_forward) {
  if (!do_forward) add_all_node_successor_edges(node);
  else  add_all_node_predecessor_edges(node); }
bool SGraph::node_has_successors(SGraphNode node,
				 bool do_forward) const {
  if (do_forward) return(node_has_successors(node));
  else return(node_has_predecessors(node)); }

bool SGraph::node_has_predecessors(SGraphNode node,
				   bool do_forward) const {
  if (!do_forward) return(node_has_successors(node));
  else return(node_has_predecessors(node)); }


bool is_sgraph_node_list_member(SGraphNodeList *l, SGraphNode n) {
   for (SGraphNodeList::iterator iter = l->begin();
	iter != l->end(); iter++) {
	if ((*iter) == n)  return(true);
   }
   return(false);
}


extern "C" void enter_sgraph(int *argc, char *argv[]) {
  
}
extern "C" void exit_sgraph() {

}



extern "C" void init_sgraph(SuifEnv *s) {
  s->require_module("bit_vector");
}
//
// SGraphFilteredNodeIter
//
