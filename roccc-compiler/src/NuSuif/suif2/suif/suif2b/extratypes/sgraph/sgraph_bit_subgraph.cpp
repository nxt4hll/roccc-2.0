#include "sgraph_bit_subgraph.h"
#include <bit_vector/bit_vector.h>
#include "sgraph_bit_iter.h"
#include "sgraph_iter_impl.h"

SGraphBitSubgraph::SGraphBitSubgraph(const SGraph *parent) :
  _parent(parent), _nodes(new BitVector) {
}
  
  // Nodes
  // maximum node number +1
SGraphNode SGraphBitSubgraph::max_num_nodes() const { 
  return (_nodes->num_significant_bits()); }

// Nodes
bool SGraphBitSubgraph::is_node_member(SGraphNode node) const {
  return(_nodes->get_bit(node)); }

void SGraphBitSubgraph::add_node (SGraphNode node) {
  _nodes->set_bit(node, true); 
}

void SGraphBitSubgraph::remove_node(SGraphNode node) {
  _nodes->set_bit(node, false); } // don't mess with the edges here.

  // Edges
  //
  // There is no edge representation here.
  // just iterators over sucessors and predecessors

bool SGraphBitSubgraph::is_edge_member(const SGraphEdge &edge) const {
  return(_parent->is_node_member(edge.from()) &&
	 _parent->is_node_member(edge.to())
	 && _parent->is_edge_member(edge)); }

void SGraphBitSubgraph::add_edge(const SGraphEdge &edge) {
  assert(0); }
void SGraphBitSubgraph::remove_edge(const SGraphEdge &edge) {
  assert(0); }

// printing the graph.
// There is a common implementation of this.
  
// Build a bit vector of the nodes
BitVector *SGraphBitSubgraph::new_node_set() const {
  return( new BitVector(*_nodes)); }

// Fast implementation
void SGraphBitSubgraph::add_nodes_in_sgraph(const SGraph *SGraph) {
  BitVector *bits = SGraph->new_node_set();
  (*_nodes) |= (*bits);
  delete bits;
}


SNodeIter SGraphBitSubgraph::get_node_iterator() const {
  return new SGraphBitIter(_nodes, false);
}

SNodeIter SGraphBitSubgraph::
get_node_successor_iterator(SGraphNode node) const {
  if (!_parent->node_has_successors(node)) return new SGraphEmptyIter;
  SNodeIter iter(_parent->get_node_successor_iterator(node));

  SGraphNodeFilter *filter = new SGraphNodeGraphFilter(this);
    
  SNodeIter new_iter(new SGraphFilteredNodeIter(iter, filter, true));
  return(new_iter);
}

SNodeIter SGraphBitSubgraph::
get_node_predecessor_iterator(SGraphNode node) const {
  // build a bit vector and then create the iterator
  BitVector *bv = new BitVector();

  unsigned num = max_num_nodes();
  for (unsigned i = 0; i < num; i++) {
    if (is_edge_member(SGraphEdge(i, node))) { bv->set_bit(i, true); }
  }
  return(new SGraphBitIter(bv, true));
}



