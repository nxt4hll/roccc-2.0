
#include "sgraph_bit.h"
#include "sgraph_bit_iter.h"
#include "sgraph_iter.h"
#include "common/suif_vector.h"
#include "bit_vector/bit_vector.h"
#include "suifkernel/suifkernel_messages.h"


/*
 * here is the iterator implementation for an SGraphBit
 */




/*
 * Now the bitvector based graph implementation
 */


SGraphBit::SGraphBit() : 
  _nodes(new BitVector),
  _edges(new suif_vector<BitVector *>)
{}
SGraphBit::SGraphBit(const SGraphBit &other) :
  _nodes(0), _edges(0)
{
  (*this) = other;
}

void SGraphBit::clear() {
  if (_nodes) delete _nodes;
  if (_edges) {
    for (bit_vector_vector::iterator iter = _edges->begin();
	 iter != _edges->end(); iter++) {
      BitVector *bv = *iter;
      if (bv != 0) delete bv;
    }
    delete _edges;
  }
  _nodes = 0; _edges = 0;
}
  


SGraphBit& SGraphBit::operator=(const SGraphBit &other) {
  clear();
  _nodes = new BitVector(*other._nodes);
  _edges = new bit_vector_vector;

  for (bit_vector_vector::iterator iter = other._edges->begin();
       iter != other._edges->end(); iter++) {
    BitVector *bv = *iter;
    if (bv == 0) {
      _edges->push_back(bv);
    } else {
      _edges->push_back(new BitVector(*bv));
    }
  }
  return(*this);
}


SGraphNode SGraphBit::max_num_nodes() const {
  return (_nodes->num_significant_bits());
}

  // Nodes
bool SGraphBit::is_node_member(SGraphNode node) const {
  return(_nodes->get_bit(node)); }
void SGraphBit::add_node (SGraphNode node) {
  _nodes->set_bit(node, true); 
}

void SGraphBit::remove_node(SGraphNode node) {
  // This is optional.
  remove_node_successor_edges(node);
  remove_node_predecessor_edges(node);
  _nodes->set_bit(node, false); } // don't mess with the edges here.
// Printing.  
void SGraphBit::print_node(ion *out, SGraphNode node) const {
  out->printf("N%u",node); }
  
  
// Edges
//
// There is no edge representation here.
// just iterators over sucessors and predecessors
bool SGraphBit::is_edge_member(const SGraphEdge &edge) const {
  if (!is_node_member(edge.from()) ||
      !is_node_member(edge.to())) return false;
  if (edge.from() >= _edges->size()) return false;
  BitVector *bv = (*_edges)[edge.from()];
  if (bv == NULL) return false;
  return(bv->get_bit(edge.to()));
}

bool SGraphBit::has_bv(SGraphNode from_node) const {
  if (_edges->size() <= from_node) {
    return(false);
  }
  return((*_edges)[from_node] != 0);
}

BitVector *SGraphBit::retrieve_bv(SGraphNode from_node) {
  while (_edges->size() <= from_node) {
    _edges->push_back(NULL);
  }
  if ((*_edges)[from_node] == NULL) {
    (*_edges)[from_node] = new BitVector;
  }
  return((*_edges)[from_node]);
}
BitVector *SGraphBit::get_bv(SGraphNode from_node) const {
  if (has_bv(from_node)) {
    return((*_edges)[from_node]);
  }
  return(NULL);
}

void SGraphBit::add_edge(const SGraphEdge &edge) {
  suif_assert(is_node_member(edge.from()));
  suif_assert(is_node_member(edge.to()));

  retrieve_bv(edge.from())->set_bit(edge.to(), true);
}

void SGraphBit::remove_edge(const SGraphEdge &edge) {
  if (!has_bv(edge.from())) return;
  retrieve_bv(edge.from())->set_bit(edge.to(), false);
}
void SGraphBit::print_edge(ion *out, const SGraphEdge &edge) const {
  out->printf("E(");
  print_node(out, edge.from());
  out->printf("->");
  print_node(out, edge.to());
  out->printf(")");
}

BitVector *SGraphBit::new_node_set() const {
  return( new BitVector(*_nodes)); 
}

// Shortcuts for the bitgraph implementation
void SGraphBit::add_nodes(const SGraph *SGraph) {
  if (!_nodes) {
    _nodes = SGraph->new_node_set();
  } else {
    BitVector *bits = SGraph->new_node_set();
    (*_nodes) |= (*bits);
    delete bits;
  }
}

void SGraphBit::set_node_successors(SGraphNode node, const BitVector *t,
				     bool do_forward) {
  if (do_forward) set_node_successors(node, t);
  else set_node_predecessors(node, t); }
void SGraphBit::set_node_predecessors(SGraphNode node, const BitVector *t,
				       bool do_forward) {
  if (!do_forward) set_node_successors(node, t);
  else set_node_predecessors(node, t); }

BitVector *SGraphBit::new_node_successors(SGraphNode node,
					    bool do_forward) const {
  if (do_forward) return(new_node_successors(node));
  else return(new_node_predecessors(node)); }

BitVector *SGraphBit::new_node_predecessors(SGraphNode node, 
					      bool do_forward) const {
  if (!do_forward) return(new_node_successors(node));
  else return(new_node_predecessors(node)); }

void SGraphBit::set_node_successors(SGraphNode node, const BitVector *t) {
  suif_assert(is_node_member(node));
  BitVector *bv = retrieve_bv(node);
  (*bv) = (*t);
}

BitVector *SGraphBit::new_node_successors(SGraphNode node) const {
  if (!has_bv(node)) {
    return new BitVector;
  } else {
    return(new BitVector(*get_bv(node)));
  }
}

void SGraphBit::set_node_predecessors(SGraphNode node, const BitVector *t) {
  unsigned num_nodes = max_num_nodes();
  for (unsigned i = 0; i < num_nodes; i++) {
    if (is_node_member(i)) {
      if (t->get_bit(i)) {
	add_edge(SGraphEdge(i, node));
      } else {
	remove_edge(SGraphEdge(i, node));
      }
    }
  }
}
BitVector *SGraphBit::new_node_predecessors(SGraphNode node) const {
  BitVector *result = new BitVector;
  unsigned num_nodes = max_num_nodes();
  for (unsigned i = 0; i < num_nodes; i++) {
    if (is_edge_member(SGraphEdge(i, node))) {
      result->set_bit(i, true);
    }
  }
  return(result);
}

SNodeIter SGraphBit::get_node_iterator() const {
  return new SGraphBitIter(_nodes, false);
}
SNodeIter SGraphBit::
get_node_successor_iterator(SGraphNode node) const {
  if (!node_has_successors(node)) return new SGraphEmptyIter;
  if (has_bv(node)) {
    return(SNodeIter( new SGraphBitIter(get_bv(node), false) ));
  } else {
    return(SNodeIter( new SGraphEmptyIter()) );
  }
}

SNodeIter SGraphBit::
get_node_predecessor_iterator(SGraphNode node) const {
  // build a bit vector and then create the iterator
  BitVector *bv = new BitVector();

  unsigned num = max_num_nodes();
  for (unsigned i = 0; i < num; i++) {
    if (is_edge_member(SGraphEdge(i, node))) { bv->set_bit(i, true); }
  }
  return(SNodeIter(new SGraphBitIter(bv, true)));
}

void SGraphBit::remove_node_successor_edges(SGraphNode node) {
  if (!has_bv(node)) return;
  delete (*_edges)[node];
  (*_edges)[node] = 0;
}
void SGraphBit::remove_node_predecessor_edges(SGraphNode node) {
  unsigned num = max_num_nodes();
  for (unsigned i =0; i < num; i++) {
    if (is_node_member(i)) { remove_edge(SGraphEdge(node, i)); }
  }
}
void SGraphBit::add_all_node_successor_edges(SGraphNode node) {
  if (_nodes == NULL) return;
  
  (*retrieve_bv(node)) = (*_nodes);
}
void SGraphBit::add_all_node_predecessor_edges(SGraphNode node) {
  unsigned num = max_num_nodes();
  for (unsigned i =0; i < num; i++) {
    if (is_node_member(i)) { add_edge(SGraphEdge(node, i)); }
  }
}
void SGraphBit::add_all_edges() {
  unsigned num = max_num_nodes();
  for (unsigned i =0; i < num; i++) {
    if (is_node_member(i)) { add_all_node_successor_edges(i); }
  }
}
bool SGraphBit::node_has_successors(SGraphNode node) const {
  BitVector *bv = get_bv(node);
  if (!bv) return false;
  return(bv->num_significant_bits() != 0);
}


