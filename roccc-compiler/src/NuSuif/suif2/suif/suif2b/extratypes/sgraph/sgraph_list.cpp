
#include "sgraph_list.h"
#include "sgraph_iter.h"
#include "sgraph_bit_iter.h"
#include "common/suif_list.h"
#include "common/suif_vector.h"
#include "bit_vector/bit_vector.h"
#include "suifkernel/suifkernel_messages.h"




/*
 * ******************************************
 * *
 * * iterators over nodes
 * *
 * ******************************************
 */
class SGraphListIter : public SGraphNodeIter {
  SGraphNodeList *_the_list;
  bool _owned; // If owned, delete it afterwards
  SGraphNodeList::iterator _iter;
public:
  SGraphListIter();
  ~SGraphListIter();
  SGraphListIter(const SGraphListIter &other);
  SGraphListIter &operator=(const SGraphListIter &other);
  SGraphListIter(SGraphNodeList *the_list, bool owned);
  void reset(SGraphNodeList *the_list, bool owned);
  virtual bool done() const;
  virtual SGraphNode get() const;
  virtual void increment();
  virtual void reset();
  SGraphNodeIter *clone() const;
};



SGraphNode SGraphList::max_num_nodes() const {
  return (_nodes->num_significant_bits());
}

// Nodes
bool SGraphList::is_node_member(SGraphNode node) const {
  return(_nodes->get_bit(node)); }

void SGraphList::add_node (SGraphNode node) {
  _nodes->set_bit(node, true); 
}

void SGraphList::remove_node(SGraphNode node) {
  // This is optional.
  remove_node_successor_edges(node);
  remove_node_predecessor_edges(node);
  _nodes->set_bit(node, false); } // don't mess with the edges here.

  
// Edges
//
// There is no edge representation here.
// just iterators over sucessors and predecessors
bool SGraphList::is_edge_member(const SGraphEdge &edge) const {
  SGraphNodeList *succ = get_succ_list(edge.from());
  if (!succ) return false;
  SGraphNodeList *pred = get_pred_list(edge.to());
  if (!pred) return false;

  // check the shorter list
  if (pred->size() < succ->size()) {
    return(list_contains(pred, edge.from()));
  }
  return (list_contains(succ, edge.to()));
}
void SGraphList::remove_edge(const SGraphEdge &edge) {
  if (!is_edge_member(edge)) return;
  list_remove(retrieve_succ_list(edge.from()), edge.to());
  list_remove(retrieve_pred_list(edge.to()), edge.from());
}

// Implementation in the .cc file
//  void print(ion *out) const;

// Iterators. Implementations in the .cc file
BitVector *SGraphList::new_node_set() const {
  if (!_nodes) { return new BitVector; }
  return( new BitVector(*_nodes)); 
}


SNodeIter SGraphList::get_node_iterator() const {
  return new SGraphBitIter(_nodes, false);
}
SNodeIter SGraphList::
get_node_successor_iterator(SGraphNode node) const {
  SGraphNodeList *l = get_succ_list(node);
  if (!l) return new SGraphEmptyIter;
  return(new SGraphListIter(l, false));
}
SNodeIter SGraphList::
get_node_predecessor_iterator(SGraphNode node) const {
  SGraphNodeList *l = get_pred_list(node);
  if (!l) return new SGraphEmptyIter;
  return(new SGraphListIter(l, false));
}

bool SGraphList::
node_has_successors(SGraphNode node) const {
  SGraphNodeList *l = get_succ_list(node);
  if (!l) return false;
  return(!l->empty());
}
bool SGraphList::
node_has_predecessors(SGraphNode node) const {
  SGraphNodeList *l = get_pred_list(node);
  if (!l) return false;
  return(!l->empty());
}


SGraphList::SGraphList() :
  _nodes(new BitVector),
  _predecessors(new sgraph_node_list_vector),
  _successors(new sgraph_node_list_vector)
{}

SGraphList::~SGraphList() {
  {for (sgraph_node_list_vector::iterator iter = _predecessors->begin();
       iter != _predecessors->end(); iter++) {
    delete (*iter);
}}
    
  {for (sgraph_node_list_vector::iterator iter = _successors->begin();
       iter != _successors->end(); iter++) {
    delete (*iter);
  }}
  delete _predecessors;
  delete _successors;
  delete _nodes;
}

void SGraphList::
add_edge(const SGraphEdge &edge) {
  // Idempotent
  suif_assert(is_node_member(edge.from()));
  suif_assert(is_node_member(edge.to()));
  if (is_edge_member(edge)) return;
  retrieve_succ_list(edge.from())->push_back(edge.to());
  retrieve_pred_list(edge.to())->push_back(edge.from());
}


/*
 * Helper methods
 */
bool SGraphList::has_succ_list(SGraphNode from_node) const {
  if (_successors->size() <= from_node) {
    return(false);
  }
  return((*_successors)[from_node] != 0);
}

SGraphNodeList *SGraphList::retrieve_succ_list(SGraphNode from_node) {
  while (_successors->size() <= from_node) {
    _successors->push_back(NULL);
  }
  if ((*_successors)[from_node] == NULL) {
    (*_successors)[from_node] = new SGraphNodeList;
  }
  return((*_successors)[from_node]);
}

SGraphNodeList *SGraphList::get_succ_list(SGraphNode from_node) const {
  if (has_succ_list(from_node)) {
    return((*_successors)[from_node]);
  }
  return(NULL);
}

// Same helpers for predecessors
bool SGraphList::has_pred_list(SGraphNode from_node) const {
  if (_predecessors->size() <= from_node) {
    return(false);
  }
  return((*_predecessors)[from_node] != 0);
}

SGraphNodeList *SGraphList::retrieve_pred_list(SGraphNode from_node) {
  while (_predecessors->size() <= from_node) {
    _predecessors->push_back(NULL);
  }
  if ((*_predecessors)[from_node] == NULL) {
    (*_predecessors)[from_node] = new SGraphNodeList;
  }
  return((*_predecessors)[from_node]);
}

SGraphNodeList *SGraphList::get_pred_list(SGraphNode from_node) const {
  if (has_pred_list(from_node)) {
    return((*_predecessors)[from_node]);
  }
  return(NULL);
}

bool SGraphList::list_contains(SGraphNodeList *l, SGraphNode node) {
  for (SGraphNodeList::iterator iter = l->begin();
       iter != l->end(); iter++) {
    if ((*iter) == node) return true;
  }
  return(false);
}
void SGraphList::list_remove(SGraphNodeList *l, SGraphNode node) {
  for (SGraphNodeList::iterator iter = l->begin();
       iter != l->end(); iter++) {
    if ((*iter) == node) {
      l->erase(iter);
      return;
    }
  }
}


/*
 * ************************************************
 *
 *  Implementation of the iterator for SGraphList
 *
 * ************************************************
 */

SGraphListIter::SGraphListIter() :
  _the_list(NULL), _owned(false), _iter() {}
SGraphListIter::SGraphListIter(const SGraphListIter &other) :
  _the_list(NULL), _owned(false), _iter() {
  reset(other._the_list, false); 
}
SGraphListIter &SGraphListIter::operator=(const SGraphListIter &other) {
  reset(other._the_list, false); 
  return(*this);
}
SGraphNodeIter *SGraphListIter::clone() const {
  if (_owned) {
    SGraphNodeList * l = new SGraphNodeList;
    for (SGraphNodeList::iterator iter = _the_list->begin();
	 iter != _the_list->end(); iter++) {
      l->push_back(*iter);
    }
    return(new SGraphListIter(l, true));
  }
  return(new SGraphListIter(_the_list, false));
}


SGraphListIter::SGraphListIter(SGraphNodeList *the_list, bool owned) :
  _the_list(NULL), _owned(true), _iter() {
  reset(the_list, owned); }
void SGraphListIter::reset(SGraphNodeList *the_list, bool owned) {
  if (_owned && _the_list != NULL) delete the_list;
  _the_list = the_list;
  _owned = owned;
  reset();
}
void SGraphListIter::reset() {
  _iter = _the_list->begin();
}
SGraphListIter::~SGraphListIter() {
  if (_owned) delete _the_list;
}

bool SGraphListIter::done() const { return _iter == _the_list->end(); }
size_t SGraphListIter::get() const { return *_iter; }
void SGraphListIter::increment() { _iter++; }
